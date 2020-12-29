#!/usr/bin/python3
#
# main.py
#
# run: starts interactive shell
# export: saves the output of orm.export, used by EasyDB 
# initialize the server, to the specified file
#

from schema import *
import orm
import schema

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
        
        port = args.get(2, 1234)
        host = args.get(3, "localhost")
                
        # create db object
        db = orm.setup("easydb", schema)
        
        # keep only objects defined in the schema module
        items = { k:v for k, v in globals().items()
            if getattr(v, '__module__', None) == "schema" }
        items['db'] = db
        
        # start the database connection
        db.connect(host, port)
        
        # start interactive mode
        code.interact(local=items)
        
        # close database connection
        db.close()
    
    elif len(args) in [2, 3] and args[1] == "export":
        output = orm.export("easydb", schema) 
        if len(args) == 3:
            # write outout of export() to file
            with open(args[2], "wt") as f:
                f.write(output)
        else:
            print(output)
    
    else:
        print("usage:", sys.argv[0], "run [PORT=1234] [HOST=localhost]")
        print("\tstarts interactive shell")
        print("usage:", sys.argv[0], "export [FILE]")
        print("\texports schema to FILE or print to console")

if __name__ == "__main__":
    main()
