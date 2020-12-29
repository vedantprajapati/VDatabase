#!/usr/bin/python3
#
# table.py
#
# Definition for an ORM database table and its metaclass
#

# metaclass of table
# Implement me or change me. (e.g. use class decorator instead)
classes = dict()
classnames = list()
correctList = False
keepingtrack = []
from .field import*
from . import *

class MetaTable(type):
	
	existingClasses = []

	possible_types = [String, Integer, Float, DateTime, Coordinate, Foreign]

	def __init__(cls, name, bases, attrs):
		if attrs['__qualname__'] in cls.existingClasses: ## This should fix case 7 duplicate but it's not, probably bypassing init bc same name
			raise AttributeError("existing class")
		for col, val in attrs.items():
			if val == "Table":
				break			
			if col == "pk" or col == "version" or col == "save" or col =="delete":
				raise AttributeError ("invalid column name - reserved")
			colSplit = list(col)
			if col.startswith("__")==False and col.endswith("__")==False:
				if colSplit[0] == "_" or colSplit[0].isdigit():
					raise AttributeError ("invalid column name")
				if "_" in colSplit:
					raise AttributeError("inv column name _")
			if isinstance(val, Integer) or isinstance(val, String) or isinstance(val,Float) or isinstance(val,Foreign) or isinstance(val, DateTime) or isinstance(val, Coordinate):
				val.setname(col)
		cls.existingClasses.append(attrs['__qualname__'])
		tempAttributes = list(attrs)	
		temptemp = list()
		for i in tempAttributes:
			if i[:2] != '__':		
				temptemp.append(i.replace("_",""))
				
		classes[name] = temptemp
		classnames.append(temptemp)
		pass


    # Returns an existing object from the table, if it exists.
    #   db: database object, the database to get the object from
    #   pk: int, primary key (ID)
	def get(cls, db, pk):
		attrlist = []
		columns = {}
		names = []
		values, version = db.get(cls.__name__, pk)

		for item in cls.__dict__:
		 	if item.startswith('__') == False:
		 		attrlist.append(item)

		for value in cls.__dict__.values():
			for name in cls.__class__.__dict__['possible_types']:
				if isinstance(value, name):
					names.append(value)

		for i, field in enumerate(attrlist):
			if (isinstance(names[i], Foreign)):
				columns[field] = names[i].table.get(db, values[i])
			else:
				columns[field] = values[i]

		obj = cls(db, **columns)
		obj.__dict__['pk']=pk	
		obj.__dict__['version']=version
		return obj

    # Returns a list of objects that matches the query. If no argument is given,
    # returns all objects in the table.
    # db: database object, the database to get the object from
    # kwarg: the query argument for comparing
	def filter(cls, db, **kwarg):
		if kwarg == {}:
			results = list(db.scan(cls.__name__, 1))
			returnList = []
			for i in results:
				returnList.append(cls.get(db, i))
			return returnList
		for val, col in kwarg.items():
			if "__" in val:
				operator = val[len(val)-2:]
				columnname = val.replace("__"+operator,"")
				if operator == "ne":
					#print("ne")
					if isinstance(cls.__dict__[columnname], Foreign):
						res = list(db.scan(cls.__name__, 1))
						resList = []
						for i in res:
							resList.append(cls.get(db, i))
						if type(col) == int:
							scanresult = list(db.scan(cls.__name__, 3, columnname, col))
						else:
							scanresult = list(db.scan(cls.__name__, 3, columnname, str(col)))
						returnList = []
						for i in scanresult:
							returnList.append(cls.get(db, i))
						return returnList
				elif operator == "gt":
					#print("gt")
					results = list(db.scan(cls.__name__, 5, columnname, col))
					returnList = []
					for i in results:
						returnList.append(cls.get(db, i))
					return returnList
				elif operator == "lt":
					#print("lt")
					results = list(db.scan(cls.__name__, 4, columnname, col))
					returnList = []
					for i in results:
						returnList.append(cls.get(db, i))
					return returnList
				else:
					raise AttributeError("operator not supported")
			else:
				#equals operator
				if type(col) == int:
					results = list(db.scan(cls.__name__, 2, val, col))
					returnList = []
					for i in results:
						returnList.append(cls.get(db, i))
					return returnList
				else:
					res = []
					if isinstance(cls.__dict__[val], Foreign):
						res = list(db.scan(cls.__name__, 1))
						resList = []
						returnList = []
						for i in res:
							returnList.append(cls.get(db, i))
						matchList = []
						for i in returnList:
							newname = "_"+val
							if str(i.__dict__[newname]) == str(col):
								matchList.append(i.__dict__["pk"])
						finalList=[]
						if type(col) == int:
							matchList = list(db.scan(cls.__name__, 3, val, col))
						for i in matchList:
							finalList.append(cls.get(db, i))
						return finalList
		return list()

    # Returns the number of matches given the query. If no argument is given, 
    # return the number of rows in the table.
    # db: database object, the database to get the object from
    # kwarg: the query argument for comparing
	def count(cls, db, **kwarg):
		if kwarg == {}:
			results = list(db.scan(cls.__name__, 1))
			return len(results)
		for val, col in kwarg.items():
			if "__" in val:
				operator = val[len(val)-2:]
				columnname = val.replace("__"+operator,"")
				if columnname not in cls.__dict__ and columnname != "id":
					raise AttributeError ("not in the schema")
				if operator == "ne":
					results = list(db.scan(cls.__name__, 3, columnname, col))
					return len(results)
				elif operator == "gt":
					results = list(db.scan(cls.__name__, 5, columnname, col))
					return len(results)
				elif operator == "lt":
					results = list(db.scan(cls.__name__, 4, columnname, col))
					return len(results)
				else:
					raise AttributeError("operator not supported")
			else:
				#equals operator
				if type(col) == int:
					results = list(db.scan(cls.__name__, 2, val, col))
					return len(results)
				else:
					if isinstance(cls.__dict__[val], Foreign):
						if type(col) == int:
							results = list(db.scan(cls.__name__, 2, val, col))
						else:
							returnList=[]
							results = list(db.scan(cls.__name__, 1))
							for i in results:
								returnList.append(cls.get(db, i))
							for i in returnList:
								results = list(db.scan(cls.__name__, 2, val, col))
							return len(results)
		return list()

# table class
# Implement me.
class Table(object, metaclass=MetaTable):
	
	def __init__(self, db, **kwargs):
		self._db = db
		self.pk = None      # id (primary key)
		self.version = None # version
		correctList = False
		
		for i in range(len(classnames)):
			c=[]
			keepingtrack.clear()
			for col, val in kwargs.items():				

				if col in classnames[i]:	
					correctList = True
					setattr(self,col,val)
					keepingtrack.append(col)
				
			if correctList == True:
				c = [x for x in classnames[i] if x not in keepingtrack]
				if c != []:
					for i in c:
						if i != "save" and i !="delete" and i != 'pk':
							setattr(self, i, "DEFAULT")
				correctList = False

    # Save the row by calling insert or update commands.
    # atomic: bool, True for atomic update or False for non-atomic update
	def save(self, atomic=True):
		table_name = type(self).__name__

		keys, vals, types = [], [], []
		for item in self.__class__.__dict__.keys():
			if item.startswith('__') == False and item != 'pk' and item != 'version' :
				keys.append(item)
				vals.append(self.__dict__['_' + item])
				types.append(self.__class__.__dict__[item])
		if self.pk is None:
			for val, key, val_type in zip(vals, keys, types):
				if isinstance(val_type, Foreign):
					internal_table = getattr(self, key)
					if internal_table.pk is None:
						internal_table.save()
						vals[vals.index(val)] = internal_table.pk
					else:
						raise InvalidReference('reference is not valid')
			self.pk, self.version = self._db.insert(table_name,vals)

		else:
			if atomic is True:
				for val, key, val_type in zip(vals, keys, types):
					if isinstance(val_type, Foreign):
						internal_table = getattr(self, key)
						if isinstance(internal_table, int):
							pass
						elif internal_table.pk is None:
							internal_table.save()
							vals[vals.index(val)] = internal_table.pk
						elif internal_table.pk is not None:
							vals[vals.index(val)] = internal_table.pk
						else:
							raise InvalidReference('reference is not valid')
				self.version = self._db.update(table_name, self.pk, vals, self.version)		
			else:
				self.version = self._db.update(table_name, self.pk, vals, version = 0)
		
		pass

    # Delete the row from the database.
	def delete(self):
		table_name = type(self).__name__
		self._db.drop(table_name, self.pk)
		self.pk, self.version = None, None
		pass

