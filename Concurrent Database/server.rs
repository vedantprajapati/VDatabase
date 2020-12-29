/*
 * server.rs
 *
 * Implementation of EasyDB database server
 *
 * University of Toronto
 * 2019
 */

use std::net::TcpListener;
use std::net::TcpStream;
use std::io::Write;
use std::io;
use std::sync::Arc;
use std::sync::Mutex;
use std::thread;
use packet::Command;
use packet::Response;
use packet::Network;
use schema::Table;
use database;
use database::Database;

fn single_threaded(listener: TcpListener, table_schema: Vec<Table>, verbose: bool)
{
    /* 
     * you probably need to use table_schema somewhere here or in
     * Database::new 
     */
    // let mut db=  Database::new(table_schema);

    // for stream in listener.incoming() {
    //     let stream = stream.unwrap();
        
    //     if verbose {
    //         println!("Connected to {}", stream.peer_addr().unwrap());
    //     }
        
    //     match handle_connection(stream, &mut db) {
    //         Ok(()) => {
    //             if verbose {
    //                 println!("Disconnected.");
    //             }
    //         },
    //         Err(e) => eprintln!("Connection error: {:?}", e),
    //     };
    // }
}

fn multi_threaded(listener: TcpListener, table_schema: Vec<Table>, verbose: bool)
{
    let mut thread_list = vec![];
    let db = Database::new(table_schema);
    let mutex_db = Arc::new(Mutex::new(db));
    let mutex_count = Arc::new(Mutex::new(0));

    for line in listener.incoming() {
        let line = line.unwrap();
        let instance_db = Arc::clone(&mutex_db);
        let instance_count = Arc::clone(&mutex_count);

        thread_list.push(
            thread::spawn(move || {
                match handle_connection(line, &instance_db, &instance_count){
                    Ok(()) => {
                        // if no error, pass, else return error
                    },
                    Err(x) => eprintln!("connection error"),

                };
            })
        );
    }
    for link in thread_list.into_iter(){
        link.join().unwrap();
    }

}

/* Sets up the TCP connection between the database client and server */
pub fn run_server(table_schema: Vec<Table>, ip_address: String, verbose: bool)
{
    let listener = match TcpListener::bind(ip_address) {
        Ok(listener) => listener,
        Err(e) => {
            eprintln!("Could not start server: {}", e);
            return;
        },
    };
    
    println!("Listening: {:?}", listener);
    
    /*
     * TODO: replace with multi_threaded
     */
    multi_threaded(listener, table_schema, verbose);
}

impl Network for TcpStream {}

/* Receive the request packet from ORM and send a response back */
fn handle_connection(mut stream: TcpStream, db: & Arc<Mutex<Database>>,mutex_count: & Arc<Mutex<i32>>) 
    -> io::Result<()> 
{
    /* 
     * Tells the client that the connction to server is successful.
     * respond with SERVER_BUSY when attempting to accept more than
     * 4 simultaneous clients
     * uses some of starter code posted on piazza
     */



    let mut count_connections = mutex_count.lock().unwrap();

    if *count_connections == 4{
        stream.respond(&Response::Error(Response::SERVER_BUSY))?;//check if at 4 clients
        return Ok(());
    }
    *count_connections += 1;
    std::mem::drop(count_connections);//to unlock
    stream.respond(&Response::Connected)?;

    loop {
        let request = match stream.receive() {
            Ok(request) => request,
            Err(e) => {
                /* respond error */
                stream.respond(&Response::Error(Response::BAD_REQUEST))?;
                let mut count_connections = mutex_count.lock().unwrap();
                *count_connections -= 1;
                return Err(e);
            },
        };
        
        /* we disconnect with client upon receiving Exit */
        if let Command::Exit = request.command {
            let mut count_connections = mutex_count.lock().unwrap();
            *count_connections -=1;
            break;
        }
        
        /* Send back a response */
        let mut db = db.lock().unwrap();
        let response = database::handle_request(request, &mut *db);
        
        stream.respond(&response)?;
        stream.flush()?;

    }

    Ok(())
}

