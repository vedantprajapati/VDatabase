#!/usr/bin/python3
#
# orm.py
#
# Definition for setup and export function
#

from .easydb import Database
from .table import MetaTable, Table
from .field import *

# Return a database object that is initialized, but not yet connected.
#   database_name: str, database name
#   module: module, the module that contains the schema
def setup(database_name, module):
    # Check if the database name is "easydb".
	db = list()

	if database_name != "easydb":
		raise NotImplementedError("Support for %s has not implemented"%(
		str(database_name)))
	module_attr = module.__dict__
	for element in module_attr:
		if (element[0] != '_' and element != 'orm' ):
			field_check = list()
			for attribute in module_attr[element].__dict__:
				if attribute[0] != '_':
					type_field = module_attr[element].__dict__[attribute]
					if isinstance(type_field, String):
						field_check.append((attribute, str))
					elif isinstance(type_field, Integer):
						field_check.append((attribute, int))
					elif isinstance(type_field, Float):
						field_check.append((attribute, float))
					elif isinstance(type_field, Foreign):
						field_check.append((attribute,type_field.table.__name__))
					elif isinstance(type_field, Coordinate):					
						field_check.append((attribute, float))
					elif isinstance(type_field, DateTime):	
						field_check.append((attribute, str))
					else:
						if type(module_attr[element]) is not type(DateTime) or type(module_attr[element]) is not type(Coordinate):
							raise AttributeError('Type provided not recognized')
			if element!= 'datetime':
				db.append((element,tuple(field_check)))
	return Database(tuple(db))

# Return a string which can be read by the underlying database to create the 
# corresponding database tables.
#   database_name: str, database name
#   module: module, the module that contains the schema
def export(database_name, module):

    # Check if the database name is "easydb".
	if database_name != "easydb":
		raise NotImplementedError("Support for %s has not implemented"%(
		str(database_name)))

	typedict = {int: 'integer', str: 'string', float: 'float' }
	
	db = []

	module_attr = module.__dict__
	for element in module_attr:
		if (element[0] != '_' and element != 'orm' ):
			field_check = list()
			for attribute in module_attr[element].__dict__:
				if attribute[0] != '_':
					type_field = module_attr[element].__dict__[attribute]
					if isinstance(type_field, String):
						field_check.append((attribute, str))
					elif isinstance(type_field, Integer):
						field_check.append((attribute, int))
					elif isinstance(type_field, Float):
						field_check.append((attribute, float))
					elif isinstance(type_field, Foreign):
						field_check.append((attribute,type_field.table.__name__))
					elif isinstance(type_field, Coordinate):
						field_check.append((attribute, float))
					elif isinstance(type_field, DateTime):
						field_check.append((attribute, str))
					else:
						if type(module_attr[element]) is not type(DateTime) or type(module_attr[element]) is not type(Coordinate):
							raise AttributeError('Type provided not recognized')
			if element!= 'datetime':
				db.append((element,tuple(field_check)))
	db = tuple(db)

	output = ''
	for t in db:
		output = output + t[0] + ' { '
		for a in t[1]:
			if a[1] not in typedict:
				output = output + a[0] + ' : ' + a[1] + ' ; '
			else:
				output = output + a[0] + ' : ' + typedict[a[1]] + ' ; '
		output = output + '} '
	return output

