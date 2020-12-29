#!/usr/bin/python3
#
# easydb.py
#
# Definition for the Database class in EasyDB client
#
import socket
import struct
import types
import math
from .exception import*
from .packet import*

RECIEVE_BUFFER_SIZE = 4096

#class function defining the database
class Database:

	schema=[]
	serverClosed = False
		
	def __repr__(self):
		return "<EasyDB Database object>"


	#initialize the database 
	def __init__(self, tables):
		#Define the connection to the server and index each table name
		self.database_indices = {}
		self.dbserver = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.schema = tables
		#Error Handling for wrong index access
		try:
			self.schema = tables

			#go through each element in tables and record the index and values while errorchecking
			for data_index, data_name in enumerate(tables):

				#checking table name validity
				if data_name[0][0] == "_" or data_name[0][0].isdigit() or data_name == "id":
					raise ValueError("Error, ValueError occured because of invalid table name")
				if data_name[0] in self.database_indices:
					raise ValueError("Error, ValueError occured because of duplicate table names names for: " + data_name[0])
				if isinstance(data_name[0], str) == False:
					raise TypeError("Error, TypeError occured because the table name is not a string")
			
				#if table name is valid, update self.database_indices
				self.database_indices[data_name[0]] = data_index; #add table name's index to self.database_indices
				#store the column names to check for duplicates
				column_names = []
				
				#iter gives iterator to element vs enumerate
				for column in iter(data_name[1]):
					if isinstance(column[0], str) == False:
						raise TypeError("Error, TypeError occured because the column name is not a string")
					elif column[0][0].isdigit() or column[0][0] == "_" or column[0] == "id":
						raise ValueError("Error, ValueError occured because of invalid column name")
					elif column[0] in column_names:
						raise ValueError("Error, ValueError occured because of duplicate column names ")
					else:
						column_names.append(column[0])
						
			#check if column type is valid, check for foreign reference e.g. table name doesnt exist or incorrect order
			for data_name in iter(tables):
				for column in iter(data_name[1]):
					if (isinstance(column[1], (int,float,str)) == False) and not column[1] is str and not column[1] is float and not column[1] is int :
						raise ValueError("Error, ValueError occured because the column type is not accepted ")
					elif isinstance(column[1],str) and column[1] not in self.database_indices:
						raise IntegrityError("Error, IntegrityError occured because the table name cannot be referenced")
					elif isinstance(column[1],str) and self.database_indices[data_name[0]] <= self.database_indices[column[1]]:
						raise IntegrityError("Error, IntegrityError occured due to an out of order foreign reference")
						
		#if program had error parsing through loop, raise index error (enumerate)
		except IndexError:
			raise IndexError("Error, IndexError occured because of malformed tb")
		pass


	#connect database to server
	def connect(self, host, port):
		#check if the server isnt already on
		if self.serverClosed == False:
			
			#connect to the server and read the packet
			self.dbserver.connect((host,port))
			dbserver_connect_packed = self.dbserver.recv(4096)		
			dbserver_connect = struct.unpack('>i',dbserver_connect_packed)
			
			#check errorcodes from server response
			if dbserver_connect == OK:
				return True
			elif dbserver_connect == SERVER_BUSY:
				dbserver.close()
				return False
			elif dbserver_connect == NOT_FOUND:
				raise ObjectDoesNotExist("Error, ObjectDoesNotExist occured while connecting")
				return False
			elif dbserver_connect == BAD_FOREIGN:
				raise InvalidReference("Error, InvalidReference occured while connecting")
				return False
			elif dbserver_connect == TXN_ABORT:
				raise TransactionAbort("Error, TransactionAbort occured while connecting")
				return False
			elif dbserver_connect == BAD_TABLE or dbserver_connect == BAD_VALUE or dbserver_connect == BAD_ROW or dbserver_connect == BAD_REQUEST:
				raise PacketError("Error, PacketError occured due to error code while connecting: " + str(dbserver_connect))
				return False
			else:
				#if no exception provided return true
				return True
				
		pass


	#close the database
	def close(self):

		#check if server is active
		if self.serverClosed == False:
			
			#send request to close server
			exit_inform = struct.pack('>i',EXIT)
			self.dbserver.send(exit_inform)
			self.dbserver.shutdown(socket.SHUT_RDWR)			
			self.dbserver.close()

			#close the server and set dbserverclosed to true so that you can activate in connect again
			self.dbserverClosed = True
			return True
		else:
			#if server is closed
			print("Server already closed")			
			return False
		pass


	#insert a row to the database
	def insert(self, table_name, values):
		
		#if table name is invalid
		if not table_name in self.database_indices:
			raise PacketError("table does not exist")
			
		#if pk is an integer, then pack drop with the index of table name + 1
		#if table cant be found
		indexnum = self.database_indices[table_name] + 1 
		insert_request = struct.pack('>ii',INSERT, indexnum )
		new_row_list = []
		output_request = insert_request

		#check the number of elements in the list to make sure that you dont have an problem inserting
		if len(values) != len(self.schema[self.database_indices[table_name]][1]):
			raise PacketError("wrong number of elements")
		
		database_type = []
		#Checks type of each column in database and stores it into database_type		
		for i in self.schema[indexnum-1][1]:
			if i[1] == int:
				database_type.append(INTEGER)
			elif i[1] == float:
				database_type.append(FLOAT)
			elif i[1] == str:
				database_type.append(STRING)
			else:
				database_type.append(FOREIGN)
				
		#Checks if type of each column and values match
		for i in range(0,len(values)):
			typeCheck = 0
			if type(values[i]) == int:
				if database_type[i] == FOREIGN:
					typeCheck = FOREIGN
				else:
					typeCheck = INTEGER
			elif type(values[i]) == float:
				typeCheck = FLOAT
			elif type(values[i]) == str:
				typeCheck = STRING
			else:
				typeCheck = FOREIGN
			if database_type[i] != typeCheck:
				raise PacketError("Not correct values")
			
		data_count = struct.pack('>i', len(values))

		#commandList holds struct row
		commandList = []
		#For every value, check if int, float, string, or foreign
		for i,value in enumerate(values):
			value_type = type(value)
			value_size = 0
			buf = value
			if value_type == int:
				value_size = 8
				#Integer
				if database_type[i] == INTEGER:
					commandList.append(struct.pack(">i", INTEGER))
					commandList.append(struct.pack(">i", value_size))
					commandList.append(struct.pack(">q", buf))
				#Foreign (checks if original database type for this index if foreign)
				elif database_type[i] == FOREIGN:
					commandList.append(struct.pack(">i", FOREIGN))
					commandList.append(struct.pack(">i", value_size))
					commandList.append(struct.pack(">q", buf))
			#Float
			if value_type == float:
				value_size = 8
				commandList.append(struct.pack(">i", FLOAT))
				commandList.append(struct.pack(">i", value_size))
				commandList.append(struct.pack(">d",buf))
			#String
			if value_type == str:
				commandList.append(struct.pack(">i", STRING))
				#If length is divisible by 4
				if len(value) % 4 == 0:
					value_size = len(value)
					commandList.append(struct.pack(">i", value_size))
					commandList.append(buf.encode('ascii'))
				#If not, add padding 
				else:
					remainder = len(value)%4
					value_size = math.ceil(len(value)/4) * 4
					paddedBytes = b'\x00' * (4-remainder)
					commandList.append(struct.pack(">i", value_size))
					commandList.append(buf.encode('ascii')+ b'\x00'*(4-remainder))
			
		joinedcommandList = b''.join(commandList)


		output_request = output_request + data_count + joinedcommandList

		
		error_code = -1
		while error_code != OK:
			#send package 
			self.dbserver.send(output_request)
			#recieve response 
			dbserver_response_packed = self.dbserver.recv(4096)	
			error_code = struct.unpack ('>i', dbserver_response_packed[:4])[0]

			#check the errorcode and output response
			if error_code == OK:
				response_id = struct.unpack ('>q', dbserver_response_packed[4:12])[0]
				version = struct.unpack ('>q', dbserver_response_packed[12:20])[0]
				continue
			elif error_code == BAD_VALUE:
				raise InvalidReference()
			elif error_code == BAD_FOREIGN:
				raise InvalidReference()
			elif error_code == NOT_FOUND:
				raise ObjectDoesNotExist()
			elif error_code == TXN_ABORT:
				raise TransactionAbort()

				
		return response_id, version 
		
		pass


	#update a row element 
	def update(self, table_name, pk, values, version=None):

		#initialize bytearrays that will be used to update row element
		packed_row_values = b''
		update_request=b''
		update_key =b''
		indexnum = 0
		
		#Checking if table is in the database
		if not table_name in self.database_indices:
			raise PacketError("table does not exist")
		
		#if pk is not an integer
		if type(pk) != int:
			raise PacketError("Error, Pk is not an integer")

		newversion = -1
		#if version is not an integer and is not default
		if isinstance(version,int) == False and version is not None:
			raise PacketError("Error, version is not an integer")
		elif version is None:
			newversion = 0
		elif isinstance(version,int):
			newversion = version

		#Pack request if pk and version are okay
		if isinstance(pk,int) == True and (isinstance(version,int) == True or version is None):
			indexnum = self.database_indices[table_name]+1
			update_request = struct.pack ('>ii',UPDATE,indexnum)
	
		#Pack key
		update_key = struct.pack('>qq',pk,newversion)
		update_count = len(values) ##This is for row.count

		database_type = []
		#checks if the types from input and the types in the database are the same
		
		#Checks type of each column in database and stores it into database_type		
		for i in self.schema[indexnum-1][1]:
			if i[1] == int:
				database_type.append(INTEGER)
			elif i[1] == float:
				database_type.append(FLOAT)
			elif i[1] == str:
				database_type.append(STRING)
			else:
				database_type.append(FOREIGN)
				
		#Checks if type of each column and values match
		for i in range(0,len(values)):
			typeCheck = 0
			if type(values[i]) == int:
				if database_type[i] == FOREIGN:
					typeCheck = FOREIGN
				else:
					typeCheck = INTEGER
			elif type(values[i]) == float:
				typeCheck = FLOAT
			elif type(values[i]) == str:
				typeCheck = STRING
			else:
				typeCheck = FOREIGN
			if database_type[i] != typeCheck:
				raise PacketError("Not correct values")

		#commandList holds struct row
		commandList = []
		#For every value, check if int, float, string, or foreign
		for i,value in enumerate(values):
			value_type = type(value)
			value_size = 0
			buf = value
			if value_type == int:
				value_size = 8
				#Integer
				if database_type[i] == INTEGER:
					commandList.append(struct.pack(">i", INTEGER))
					commandList.append(struct.pack(">i", value_size))
					commandList.append(struct.pack(">q", buf)) ##q
				#Foreign (checks if original database type for this index if foreign)
				elif database_type[i] == FOREIGN:
					commandList.append(struct.pack(">i", FOREIGN))
					commandList.append(struct.pack(">i", value_size))
					commandList.append(struct.pack(">q", buf)) ## q
			#Float
			if value_type == float:
				value_size = 8
				commandList.append(struct.pack(">i", FLOAT))
				commandList.append(struct.pack(">i", value_size))
				commandList.append(struct.pack(">d",buf)) ###d
			#String
			if value_type == str:
				commandList.append(struct.pack(">i", STRING))
				#If length is divisible by 4
				if len(value) % 4 == 0:
					value_size = len(value)
					#print("valuesize"+str(value_size))
					commandList.append(struct.pack(">i", value_size))
					commandList.append(buf.encode('ascii'))
				#If not, add padding 
				else:
					remainder = len(value)%4
					value_size = math.ceil(len(value)/4) * 4
					#print("valuesizefixed"+str(value_size))
					paddedBytes = b'\x00' * (4-remainder)
					commandList.append(struct.pack(">i", value_size))
					commandList.append(buf.encode('ascii')+ b'\x00'*(4-remainder))
			
		joinedcommandList = b''.join(commandList)

		
		totalsend = update_request + update_key + struct.pack ('>i',update_count) + joinedcommandList
		
		#print(totalsend)
		self.dbserver.send(totalsend)

		#recieve response 
		dbserver_response_packed = self.dbserver.recv(4096)
		
		#Unpack response
		dbserver_response = struct.unpack('>i', dbserver_response_packed[:4])[0]
		
		#check server response size and make sure you can access the array
		if(len(dbserver_response_packed) <= 4) == False:
			version = struct.unpack('>q', dbserver_response_packed[4:12])[0]

		#errorchecking from response packet
		if (dbserver_response == OK):
			if version is None:
				return 1
			else:		    	
				return version
		elif dbserver_response == NOT_FOUND:
		    	raise ObjectDoesNotExist("Error, Object Does Not Exist")
		elif (dbserver_response == TXN_ABORT):
		    	raise TransactionAbort("Error, Transaction Abort")
		elif (dbserver_response == BAD_FOREIGN):
		    	raise InvalidReference("Error, Bad Foreign")
		elif (dbserver_response == BAD_TABLE  or dbserver_response == BAD_VALUE or dbserver_response == BAD_ROW or dbserver_response == BAD_REQUEST):
		    	raise PacketError("Error, bad table, value, row or request")	
		else:
			print("no response")

		#if no error, return the version 
		return version
		pass


	#remove a row from the database
	def drop(self, table_name, pk):
		
		#check if table_name in the database
		if not table_name in self.database_indices:
			raise PacketError("Error, table does not exist")
			
		#if pk is an integer, then pack drop with the index of table name + 1
		if isinstance(pk, int) == True:
			#if table cant be found
			indexnum = self.database_indices[table_name] + 1 
			drop_request = struct.pack('>ii',DROP, indexnum )
		#if pk isnt of type int
		else:
			raise PacketError("Error, Pk not an integer")
		
		#pack the id number
		id_number = struct.pack('>q', pk)
		drop_package = drop_request + id_number
		
		#send package 
		self.dbserver.send(drop_package)
		
		#recieve response 
		dbserver_response_packed = self.dbserver.recv(4096)
		dbserver_response = struct.unpack('>i', dbserver_response_packed[:4])[0]
		if(len(dbserver_response_packed) <= 4) == False:
			version = struct.unpack('>q', dbserver_response_packed[4:12])[0]
		
		#read data from server and confirm that it has been recieved, check for errors
		if (dbserver_response == NOT_FOUND):
			raise ObjectDoesNotExist("Error, row specified does not exist at location")
		elif (dbserver_response == OK):
			return True
		else:
			print("Server returned no response")
			
		pass


	#retrieve row information from the database
	def get(self, table_name, pk):
		
		#initialize lists to be used to read data 
		type_list = []
		size_list = []
		row_list = []
		row_data_packed =[]
		version = 0
		
		#check if table name is valid
		if not table_name in self.database_indices:
			raise PacketError("Error, table does not exist")
			
		#if pk is an integer, then pack drop with the index of table name + 1
		if isinstance(pk, int) == True:
			#if table cant be found
			indexnum = self.database_indices[table_name] + 1 
			get_request = struct.pack('>ii',GET, indexnum )

		#if pk isnt of type int
		else:
			raise PacketError("Error, Pk not an integer")
		
		#pack the id number
		id_number = struct.pack('>q', pk)
		get_package = get_request + id_number
		
		#send package 
		self.dbserver.send(get_package)
		
		#recieve response 
		dbserver_response_packed = self.dbserver.recv(4096)
		
		#recieve the message from the server to check for errors before getting value 
		error_code = struct.unpack('>i',dbserver_response_packed[:4])[0]
		
		#check if table index isnt found
		if error_code == NOT_FOUND:
			raise ObjectDoesNotExist("Error, specified table index does not exist")
		
		#check length of response to makes sure its larger than 4 bytes
		if(len(dbserver_response_packed) <= 4) == False:
			
			#log the version row element count and the row data to respective variables
			version = struct.unpack('>q', dbserver_response_packed[4:12])[0]
			row_element_count = struct.unpack('>i', dbserver_response_packed[12:16])[0]		
			row_data_packed = dbserver_response_packed[16:]

			#read each row element and append to row list			
			for value in range(row_element_count):
				
				#add type, size and element name to respective arrays
				type_list.append(struct.unpack('>i',row_data_packed[:4])[0])
				size_list.append(struct.unpack('>i',row_data_packed[4:8])[0])
				row_data_packed = row_data_packed[8:]
				
				#check for errors and behave differently depending on type
				if(type_list[value] == STRING):
					row_var = (struct.unpack(('>' + str(size_list[value]) + 's'),row_data_packed[:size_list[value]])[0]).decode()
					row_list.append(row_var.replace('\x00',''))
				elif(type_list[value] == INTEGER):
					row_list.append(struct.unpack('>q',row_data_packed[:size_list[value]])[0])
				elif(type_list[value] == FLOAT):
					row_list.append(struct.unpack('>d',row_data_packed[:size_list[value]])[0])
				elif(type_list[value] == FOREIGN):
					row_list.append(struct.unpack('>q',row_data_packed[:size_list[value]])[0])
				elif(type_list[value] == NULL):
					print('NULL type row element')
				row_data_packed = row_data_packed[size_list[value]:]
			
		return(row_list,version)
			
		pass


	#scan the database 
	def scan(self, table_name, op, column_name=None, value=None):

		#check if table name exits in the database
		if not table_name in self.database_indices:
			raise PacketError("Error, table does not exist")

		#log the index of the table name
		indexnum = self.database_indices[table_name] + 1 

		#errorcheck for valid input
		if column_name is None and op!= 1:
			raise PacketError("Error, column name is not specified")
		if value is None and op!= 1:
			raise PacketError("Error, value is not specified")
		if op not in range(1,6):
			raise PacketError("Error, operator is not specified")
		
		#Checks type of each column in database and stores it into database_type
		colCheck = False		
		colType = -1
		foreignName = ""
		valueType = type(value)
		wrongValue = False
		columnNumber = 1
		if op != 1:
			for i in self.schema[indexnum-1][1]:
				if column_name == i[0]:
					colCheck = True
					if i[1] == valueType:
						wrongValue = True
						colType = INTEGER
					elif i[1] == float:
						wrongValue = True
						colType = FLOAT
					elif i[1] == str:
						wrongValue = True
						colType = STRING
					else:
						colType = FOREIGN
				if colCheck == False:			
					columnNumber +=1

			#print("before checking foreign stuff")
			if colCheck == False  and column_name != "id":
				raise PacketError("Error, column name not in table")
			
			#if type is foreign, deal in specific way
			if colType == FOREIGN:
				'''
				if valueType == str:
					#if foreignName != column_name:
					#	raise PacketError("Error, foreign name is not column name")
					wrongValue = True
				else:
					raise PacketError("Error, column name is foreign but value is not a string")
				'''
				print("foreign")
			#if value isnt correct, raise packet error
			'''
			if wrongValue == False and column_name != "id":
				raise PacketError("Error, wrong value for column")
			'''
		#commandList holds struct row
		commandList = []
		commandListNull = []
		commandListID = []
		#For every value, check if int, float, string, or foreign

		value_size = 0
		buf = value
		if op == 1:
			commandListNull.append(struct.pack(">i", NULL))
			commandListNull.append(struct.pack(">i", 0))
		if column_name == "id":
			commandListID.append(struct.pack(">i", FOREIGN))
		if valueType == int:
			value_size = 8
			#Integer
			commandListID.append(struct.pack(">i", value_size))
			if colType == INTEGER:
				commandList.append(struct.pack(">i", INTEGER))
				commandList.append(struct.pack(">i", value_size))
				commandList.append(struct.pack(">q", buf)) 
				commandListNull.append(struct.pack(">q", buf)) ##q

			#Foreign (checks if original database type for this index if foreign)
			elif colType == FOREIGN:
				commandList.append(struct.pack(">i", FOREIGN))
				commandList.append(struct.pack(">i", value_size))
				commandList.append(struct.pack(">q", buf)) ## q
				commandListNull.append(struct.pack(">q", buf)) ## q
			commandListID.append(struct.pack(">q", buf)) ##q
			#Float
		if valueType == float:
			value_size = 8
			commandList.append(struct.pack(">i", FLOAT))
			commandList.append(struct.pack(">i", value_size))
			commandList.append(struct.pack(">d",buf)) ###d
			commandListNull.append(struct.pack(">d",buf)) ###d
			commandList.append(struct.pack(">i", value_size))
			commandListID.append(struct.pack(">d",buf)) ###d
		#String
		if valueType == str:
			if colType == FOREIGN:
				commandList.append(struct.pack(">i", FOREIGN))
			else:
				commandList.append(struct.pack(">i", STRING))
			#If length is divisible by 4
			if len(value) % 4 == 0:
				value_size = len(value)
				commandList.append(struct.pack(">i", value_size))
				commandList.append(buf.encode('ascii'))
				commandListNull.append(buf.encode('ascii'))
				commandListID.append(struct.pack(">i", value_size))
				commandListID.append(buf.encode('ascii'))
				
			#If not, add padding 
			else:
				remainder = len(value)%4
				value_size = math.ceil(len(value)/4) * 4
				paddedBytes = b'\x00' * (4-remainder)
				commandList.append(struct.pack(">i", value_size))
				commandList.append(buf.encode('ascii')+ b'\x00'*(4-remainder))
				commandListNull.append(buf.encode('ascii')+ b'\x00'*(4-remainder))
				commandListID.append(struct.pack(">i", value_size))
				commandListID.append(buf.encode('ascii')+ b'\x00'*(4-remainder))
			
		scan_value = b''.join(commandList)
		scan_value_null = b''.join(commandListNull)
		scan_value_ID = b''.join(commandListID)

		scan_request = struct.pack('>ii',SCAN, indexnum)
		if op == 1:
			scan_column = struct.pack('>ii', 0, op)
			final_scan = scan_request + scan_column + scan_value_null
		elif column_name == "id":
			scan_column = struct.pack('>ii', 0, op)
			final_scan = scan_request + scan_column + scan_value_ID
		else:
			scan_column = struct.pack('>ii', columnNumber,op)
			final_scan = scan_request + scan_column + scan_value


		self.dbserver.send(final_scan)

		#recieve response 
		dbserver_response_packed = self.dbserver.recv(4096)
		
		#Unpack response
		#if(len(dbserver_response_packed) <= 4) == False:
		dbserver_response = struct.unpack('>i', dbserver_response_packed[:4])[0]
		return_list = list()
		if(len(dbserver_response_packed) <= 4) == False:
			count = struct.unpack('>i', dbserver_response_packed[4:8])[0]
			return_count= dbserver_response_packed[8:]
			for value in range(count):
				return_list.append(struct.unpack('>q',return_count[value*8:(value+1)*8])[0])


		if dbserver_response == OK:	    	
			return return_list
		elif dbserver_response ==  BAD_QUERY:
			raise PacketError("Error, bad query")
		return return_list
		

		pass
                


