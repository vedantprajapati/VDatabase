/*
 * database.rs
 *
 * Implementation of EasyDB database internals
 *
 * University of Toronto
 * 2019
 */
use packet::{Command, Request, Response, Value};
use std::collections::HashMap;
use schema::{Table, Column};

/* OP codes for the query command */
pub const OP_AL: i32 = 1;
pub const OP_EQ: i32 = 2;
pub const OP_NE: i32 = 3;
pub const OP_LT: i32 = 4;
pub const OP_GT: i32 = 5;
pub const OP_LE: i32 = 6;
pub const OP_GE: i32 = 7;

/*
pub struct ID {
    table_num:i32,
    row_num: i32,
}

impl ID{
    pub fn new(table_number: i32, row_number:i64) -> ID{            
            table_num = table_number;
            row_num = 0;
            }
 

    pub fn access(table_number: i32, id: & mut ID) -> ID{            
            return ID.

            }
   
}*/

/* You can implement your Database structure here
 * Q: How you will store your tables into the database? */
pub struct Row{
    vals: Vec<Value>,
    ver: i64, //was i32, needed to change b/c of mismatched vals
}




pub struct Database { 
    types_tables: HashMap<i32, Table>,
    data_tables: HashMap<i32, HashMap<i64, Row>>,
    //temporary until i find a better solution
    row_num: i64,
}

impl Database{
    pub fn new(schema_tables: Vec<Table>) -> Database{
        let mut types_tables = HashMap::new();
        let mut data_tables = HashMap::new();
        
        for elem in schema_tables.into_iter(){
            data_tables.insert(elem.t_id, HashMap::new());
            //println!("{}", elem.t_name);
            types_tables.insert(elem.t_id,elem);

        }

        Database{
            data_tables: data_tables,
            types_tables: types_tables,
            row_num: 0,
        }

    }
}

/* Receive the request packet from client and send a response back */
pub fn handle_request(request: Request, db: & mut Database) 
    -> Response  
{           
    /* Handle a valid request */
    let result = match request.command {
        Command::Insert(values) => 
            handle_insert(db, request.table_id, values),
        Command::Update(id, version, values) => 
             handle_update(db, request.table_id, id, version, values),
        Command::Drop(id) => handle_drop(db, request.table_id, id),
        Command::Get(id) => handle_get(db, request.table_id, id),
        Command::Query(column_id, operator, value) => 
            handle_query(db, request.table_id, column_id, operator, value),
        /* should never get here */
        Command::Exit => Err(Response::UNIMPLEMENTED),
    };
    
    /* Send back a response */
    match result {
        Ok(response) => response,
        Err(code) => Response::Error(code),
    }
}

/*
 * TODO: Implment these EasyDB functions
 */
 
fn handle_insert(db: & mut Database, table_id: i32, values: Vec<Value>) 
    -> Result<Response, i32> 
{
    //Immutable borrows or else it'll throw an error @ get, table is the table to insert, type_table is the types of the tables
    if let Some(table) = (& db.data_tables).get(&table_id){
        if let Some(type_table) = (&db.types_tables).get(&table_id){

            let input_num_cols = values.len();
            let col_num = type_table.t_cols.len();
            //Checking if the number of columns is equal to the number of columns in the table
            if input_num_cols != col_num{
                return Err(Response::BAD_ROW);
            }
            
            //Iterates through all of the input columns to check types
            for i in 0..col_num{
                //table_type contains the type of that specific column
                let table_type = Column::type_as_str(&type_table.t_cols[i]);
                let input_type = &values[i];
                //println!("{:?}",input_type);
                //println!("{}",Column::type_as_str(&type_table.t_cols[i]));

                //Matching the input types to their corresponding strings
                let check_input = match input_type {
                    Value::Integer(i64) => String::from("integer"),
                    Value::Float(f64) => String::from("float"),
                    Value::Text(string) => String::from("string"),
                    Value::Foreign(i64) => String::from("foreign"),
                    _ => String::from("null"),
                };
                
                //If the types don't match, check further
                if check_input != table_type{
                    //Checks that the column type is not a foreign
                    if table_type.len() < 7{
                        return Err(Response::BAD_VALUE);
                    }
                    else{
                        //If the table column type is a foreign, that the input is a foreign as well
                        //remember that table_type returns foreign(1) --> 1 is the table ID
                        if &table_type[..7] != check_input{
                            return Err(Response::BAD_VALUE);
                    }
                    }
                } 
               
                //Checking if it's a foreign of not
                if check_input =="foreign"{

                    //table_id is the table that it refers
                    let table_id = type_table.t_cols[i].c_ref;

                    //Row is the row number that insert wants to refer to 
                    let mut row:i64 = 0; //or else it'll throw an uninitialized error
                    if let Value::Foreign(i64) = input_type{
                        row = *i64;
                    }  
                    //Checks that it's a valid row in the table
                     let return_get = handle_get(db, table_id, row);
                
                        //If get returns an error, means that table or row doesn't exist yet, throw error
                        if let Result::Err(a) = return_get{
                            return Err(Response::BAD_FOREIGN);
                        }
                }
            }

            //Borrowing mutably here
            if let Some(table2) = (&mut db.data_tables).get_mut(&table_id){
    
                //Find the current row number that we have
                let row_count_borrow = & mut db.row_num;
                //Increment it by 1 (since current db.row_num is the row of the last insert
                let row_count = row_count_borrow.clone() + 1;

                //Wrapping it in a row
                let insert_val = Row { vals: values, ver: 1};

                //Insert it in the row
                table2.insert(row_count,insert_val);
                
                //Change the row_num in the Database
                db.row_num = row_count;
                return Ok(Response::Insert(row_count,1));
            }
            else{
                return Err(Response::BAD_TABLE);
            }
        }   
        else{
            return Err(Response::BAD_TABLE);
        }
    }
    else{
        return Err(Response::BAD_TABLE);
    }
}

fn handle_update(db: & mut Database, table_id: i32, object_id: i64, version: i64, values: Vec<Value>) -> Result<Response, i32> 
{
    //Immutable borrows or else it'll throw an error @ get, table is the table to insert, type_table is the types of the tables
    if let Some(table) = (& db.data_tables).get(&table_id){
        if let Some(type_table) = (&db.types_tables).get(&table_id){

            let input_num_cols = values.len();
            let col_num = type_table.t_cols.len();

            //Checking if the number of columns is equal to the number of columns in the table
            if input_num_cols != col_num{
                return Err(Response::BAD_ROW);
            }

            //Checks that it's a valid row in the table
                let return_get = handle_get(db, table_id, object_id);
                
                //If get returns an error, means that table or row doesn't exist yet, throw error
                let response = match return_get{
                    Result::Err(a)=> return Err(Response::NOT_FOUND),
                    Result::Ok(a) => a,
                };
            
                //Find the version number of the row based on the GET response
                let curr_version_num = match response{
                    Response::Get(i64, _) => i64,
                    _ => 0,
                };
            
            //Iterates through all of the input columns to check types
            for i in 0..col_num{
                //table_type contains the type of that specific column
                let table_type = Column::type_as_str(&type_table.t_cols[i]);
                let input_type = &values[i];
                //println!("{:?}",input_type);
                //println!("{}",Column::type_as_str(&type_table.t_cols[i]));

                //Matching the input types to their corresponding strings
                let check_input = match input_type {
                    Value::Integer(i64) => String::from("integer"),
                    Value::Float(f64) => String::from("float"),
                    Value::Text(string) => String::from("string"),
                    Value::Foreign(i64) => String::from("foreign"),
                    _ => String::from("null"),
                };
                
                //If the types don't match, check further
                if check_input != table_type{
                    //Checks that the column type is not a foreign
                    if table_type.len() < 7{
                        return Err(Response::BAD_VALUE);
                    }
                    else{
                        //If the table column type is a foreign, that the input is a foreign as well
                        //remember that table_type returns foreign(1) --> 1 is the table ID
                        if &table_type[..7] != check_input{
                            return Err(Response::BAD_VALUE);
                    }
                    }
                } 
               
                //Checking if it's a foreign of not
                if check_input =="foreign"{

                    //table_id is the table that it refers
                    let table_id = type_table.t_cols[i].c_ref;

                    //Row is the row number that insert wants to refer to 
                    let mut row:i64 = 0; //or else it'll throw an uninitialized error
                    if let Value::Foreign(i64) = input_type{
                        row = *i64;
                    }  
                    //Checks that it's a valid row in the table
                     let return_get = handle_get(db, table_id, row);
                
                        //If get returns an error, means that table or row doesn't exist yet, throw error
                        if let Result::Err(a) = return_get{
                            return Err(Response::BAD_FOREIGN);
                        }
                }
            }

            //Borrowing mutably here
            if let Some(table2) = (&mut db.data_tables).get_mut(&table_id){

                let v;
                //If version input is 0, always just increment 1 and allow it to be updated
                if version !=0{
                    //If the version num from get() isnt the same as the one that's passed in, through an error
                    if curr_version_num != version{
                        return Err(Response::TXN_ABORT);
                    }
                    
                }
                v = version + 1;            
        
                let insert_val = Row { vals: values, ver: v};

                //Inserting into a hashmap when the key exists is just an update
                table2.insert(object_id,insert_val);
               
                return Ok(Response::Update(v));
            }
            else{
                return Err(Response::BAD_TABLE);
            }
        }   
        else{
            return Err(Response::BAD_TABLE);
        }
    }
    else{
        return Err(Response::BAD_TABLE);
    }
}

fn handle_drop(db: & mut Database, table_id: i32, object_id: i64) 
    -> Result<Response, i32>
{

    let instance_types = &mut db.types_tables;
    let instance_check_type = instance_types.get_mut(&table_id);
    let mut to_drop = vec![];
    let mut foreign_keys = vec![];

    
    if let Some(instance_table_check) = instance_check_type {
       
        for (instance_key, instance_val) in instance_types{
            for (index, instance_schema) in instance_val.t_cols.iter().enumerate(){
                if instance_schema.c_ref != table_id{

                }
                else{
                    foreign_keys.push((*instance_key, index));
                }
            }
        }
        for (instance_key, index) in foreign_keys{
            if let Some(instance_row) = db.data_tables.get_mut(&instance_key){
                for (instance_col_key, instance_col_val) in instance_row{ 
                    match instance_col_val.vals[index]{
                        Value::Foreign(row_id) => if row_id == object_id{
                            to_drop.push((instance_key, *instance_col_key));
                        },
                        _ => panic!("cant have non foreign value here"),
                    }
                }
            }
        }

        //cascade case
        for (instance_key, instance_col_key) in to_drop{ 
            let recursive_drop = handle_drop(db,instance_key,instance_col_key);
        }
        if let Some(instance_row) = (&mut db.data_tables).get_mut(&table_id){
            if let Some(instance_object) = instance_row.remove(&object_id) {
                
                Ok(Response::Drop)
            }
            else{
                Err(Response::NOT_FOUND)
            }
        }
        else{
            Err(Response::BAD_TABLE)
        }

    }
    else {
        return Err(Response::BAD_TABLE);
    }
    

}

fn handle_get(db: & Database, table_id: i32, object_id: i64) 
    -> Result<Response, i32>
{
    if let Some(instance_row) = (& db.data_tables).get(&table_id){
        if let Some(value_row) = (instance_row).get(&object_id){
            return Ok(Response::Get(value_row.ver,&value_row.vals));
        }
        else{
            return Err(Response::NOT_FOUND);
        }
    }
    else{
        return Err(Response::BAD_TABLE);
    }
}

fn handle_query(db: & Database, table_id: i32, column_id: i32,
    operator: i32, other: Value) 
    -> Result<Response, i32>
{
    //println!("tableid:{} columnid:{} operator:{} ", table_id, column_id, operator);
    let check_input;
    let mut correct_col = 0;
    if table_id == 0{
        //println!("bad query table id is 0");
        return Err(Response::BAD_TABLE);
    }
    if let Some(instance_row) = (& db.data_tables).get(&table_id){

        if let Some(instance_check_type) = (& db.types_tables).get(&table_id){
        
            let type_cols = &instance_check_type.t_cols;
            if column_id != 0{
                correct_col = (column_id - 1) as usize;
            }
            if type_cols.len() < correct_col{
                //println!("bad query lengths");
                return Err(Response::BAD_QUERY);
            }

            let table_type = Column::type_as_str(&instance_check_type.t_cols[correct_col as usize]);
            let other2 = &other;
           
            check_input = match other2 {
                    Value::Integer(i64) => String::from("integer"),
                    Value::Float(f64) => String::from("float"),
                    Value::Text(string) => String::from("string"),
                    Value::Foreign(i64) => String::from("foreign"),
                    _ => String::from("null"),
                };
         

            //If the types don't match, check further
                if (check_input != table_type) && (check_input != "null"){
                    //println!("{}",table_type.len());
                    //Checks that the column type is not a foreign
                    if table_type.len() < 7{
                        //println!("bad query not matching types w/o foreign");
                        return Err(Response::BAD_QUERY);
                    }
                    else{
                        //If the table column type is a foreign, that the input is a foreign as well
                        //remember that table_type returns foreign(1) --> 1 is the table ID
                        if &table_type[..7] != check_input{
                            //println!("bad query not matching types w/ foreign");
                            return Err(Response::BAD_QUERY);
                    }
                    }
                } 

        }
        else{
            //println!("bad query bad table");
            return Err(Response::BAD_TABLE);
        }
    
        
        if operator == OP_AL {
            if column_id != 0 {
                //println!("bad query op is al but column number isnot 0 ");
                return Err(Response::BAD_QUERY);
           }
        }
        else if (operator < 0) && (operator > 6){
            //println!("bad query operator not in range");
            return Err(Response::BAD_QUERY);
        }

        if check_input == "foreign"{
            
            if operator != OP_EQ && operator != OP_NE{
                //println!("bad query column id is 0 and foreign, but picks operators that aren't applicable");
                return Err(Response::BAD_QUERY);
            }
        }

        if column_id == 0{
            
            if operator != OP_EQ && operator != OP_NE && operator != OP_AL{
                //println!("bad query column id is 0 and foreign, but picks operators that aren't applicable");
                return Err(Response::BAD_QUERY);
            }
        }

        let mut returnvec = vec![];

        if operator == OP_AL {
            for (key,value) in instance_row{
                returnvec.push(*key);
            }
        }
        else if operator == OP_EQ{
            for (key,value) in instance_row{
                let value_bor = &value.vals[correct_col];
                if value_bor == &other{
                    returnvec.push(*key);
                }
            }
        }
        else if operator == OP_NE{
        for (key,value) in instance_row{
                let value_bor = &value.vals[correct_col];
                if value_bor != &other{
                    returnvec.push(*key);
                }
            }
        }
        else if operator == OP_LT{
        for (key,value) in instance_row{
                let value_bor = &value.vals[correct_col];
                if value_bor < &other{
                    returnvec.push(*key);
                }
            }
        }
        else if operator == OP_GT{
        for (key,value) in instance_row{
                let value_bor = &value.vals[correct_col];
                if value_bor > &other{
                    returnvec.push(*key);
                }
            }
        }
        else if operator == OP_LE{
        for (key,value) in instance_row{
                let value_bor = &value.vals[correct_col];
                if value_bor <= &other{
                    returnvec.push(*key);
                }
            }
        }
        else if operator == OP_GE{
        for (key,value) in instance_row{
                let value_bor = &value.vals[correct_col];
                if value_bor >= &other{
                    returnvec.push(*key);
                }
            }
        }

        return Ok(Response::Query(returnvec));
    }
    else{
        return Err(Response::BAD_TABLE);
    }
}

