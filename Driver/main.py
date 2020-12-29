#!/usr/bin/python3
#
# main.py
#
# starts interactive shell for testing the EasyDB driver
#

import easydb
from easydb import operator

tb = (
    ("User", (                  # table_name
        ("firstName", str),     # (column_name, type)
        ("lastName", str),
        ("height", float),
        ("age", int),
    )),
    
    ("Account", (
        ("user", "User"),       # (column_name, table_reference)
        ("type", str),
        ("balance", float),
    )),
)

class List(list):
    def get(self, index, default):
        try:
            return self[index]
        except:
            return default

def main():
    import sys   
    args = List(sys.argv)

    if len(args) >= 2 and args[1] == "run":
        import code
        host = args.get(2, "localhost")
        port = args.get(3, 8080)
                
        # create db object
        db = easydb.Database(tb)
        
        items = { 
            'db' : db,
            'operator' : operator,
        }
        
        # start the database connection
        db.connect(host, port)
        
        # start interactive mode
        code.interact(local=items)
        
        # close database connection
        db.close()
    
    else:
        print("usage:", sys.argv[0], "run [HOST [PORT]]")
        print("\tstarts interactive shell")

if __name__ == "__main__":
    main()
