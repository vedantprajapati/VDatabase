#!/usr/bin/python3
#
# fields.py
#
# Definitions for all the fields in ORM layer
#
from datetime import datetime

def checker_choice(choices, item_type):
	for choice in choices:
		if isinstance(choice, item_type) != True:
			raise TypeError('one of the choices is of wrong type')

class Field:
	def __init__(self, type):
		self.type = type

class Integer:   
	
	def __init__(self, blank=-1, default=None, choices=None):

		if choices != None:
			checker_choice(choices, int)

		if blank == False:
			raise AttributeError("blank is false")
		if default is None:
			default = 0
		else:
			blank = True
			if isinstance(default, int) == False:
				raise TypeError('default is of wrong type')
			if choices != None and default not in choices:
				raise TypeError('default cannot be found in choices')

		self.blank = blank
		self.default = default
		self.choices = choices
		pass
		
						
	def setname(self,name):
		self.name = '_' + name

	def __set__(self, inst, value):

		
		if value == "DEFAULT" and self.default == 0:
			value = 0
			self.blank = True
		else:
			if value != "DEFAULT":
				if self.default != 0 and self.default != -99999:
					value = self.default	
					self.default = -99999
				else:
					value = value		
			elif value == "DEFAULT":
				value = self.default
		if self.choices:	
			if value not in self.choices and value != 0:
				raise ValueError ("not in choices")
		if type(value) != int:
			raise TypeError("trying ")
		setattr(inst, self.name, value)

	def __get__(self, inst,cls):
		return getattr (inst, self.name)

class Float: 
	def __init__(self, blank=-1, default=None, choices=None):
		if blank == False:
			raise AttributeError("blank is false")
		if choices != None:
			checker_choice(choices, float)
		if default is None:
			default = 0.0
		else:
			blank = True
			if isinstance(default, float) == False:
				raise TypeError('default is of wrong type')
			if choices != None and default not in choices:
				raise TypeError('default cannot be found in choices')
	
		self.blank = blank
		self.default = default

		self.choices = choices
		pass


	def setname(self,name):
		self.name = '_' + name

	def __get__(self, inst,cls):
		return getattr (inst, self.name)

	def __set__(self, inst, value):
		if value == "DEFAULT" and self.default == 0.0:
			value = 0.0
			self.blank = True
		else:
			if value != "DEFAULT":
				if self.default != 0.0 and self.default != -99999.99:
					value = self.default	
					self.default = -99999.99
				else:
					value = value		
			elif value == "DEFAULT":
				value = self.default
		if self.choices:	
			if value not in self.choices and value != 0.0:
				raise ValueError ("not in choices")
		setattr(inst, self.name, float(value))

class String:
	def __init__(self, blank=-1, default=None, choices=None):
		if blank == False:
			raise AttributeError("blank is false")
		if choices != None:
			checker_choice(choices, str)
		if default is None:
			default == ''
		else:
			blank = True
			if isinstance(default, str) == False:
				raise TypeError('default is of wrong type')
			if choices != None and default not in choices:
				raise TypeError('default cannot be found in choices')

		self.blank = blank
		self.default = default
		self.choices = choices	
		pass		

	def setname(self,name):
		self.name = '_' + name

	def __get__(self, inst,cls):
		if self.name in inst.__dict__:
			return inst.__dict__[self.name]
		return getattr (inst, self.name)

	def __set__(self, inst, value):

		if self.blank == -1 and value == "DEFAULT":
			raise AttributeError ("Blank not set")
		if value == "DEFAULT" and self.default is not None:
			value = ""	
			self.blank = True

		if self.default is not None and self.default != "-999999.99":
			value = self.default	
			self.default = "-999999.99"

		if self.choices:	
			if value not in self.choices and value != "":
				raise ValueError ("not in choices")
		setattr(inst, self.name, value)

class Foreign:
	def __init__(self, table, blank=-1):
		self.table = table
		self.blank = blank
		pass
	
	def setname(self,name):
		self.name = '_' + name

	def __get__(self, inst,cls):
		return getattr (inst, self.name)

	def __set__(self,inst,value):
		if type(value) == int:
			raise TypeError("Setting int to foreign")
		setattr(inst, self.name, value)


class DateTime:
	implemented = True

	def get_default(default):
		if callable(default):
			value = default()
		else:
			value = default
		return value


	def setname(self,name):
		self.name = '_' + name

	def __init__(self, blank=False, default=None, choices=None):
		if choices != None:
			checker_choice(choices, dateTime)
		
		if default == None:
			#default = datetime.fromtimestamp(0)
			#default = datetime.now()
			default = 0
		else:
			blank = True
			if callable(default):
				value = default()
			else:
				value = default
	
			#if isinstance(default, DateTime) == False:
			#	raise TypeError('default is of wrong type')
			if choices != None and default not in choices:
				raise TypeError('default cannot be found in choices')
	
		self.blank = blank
		self.default = default
		self.choices = choices		
		pass

	def __get__(self, inst,cls):
		return getattr (inst, self.name)

	def __set__(self,inst,value):
		setattr(inst, self.name, value)

class Coordinate:
	implemented = True

	def __init__(self, blank=False, default=None, choices=None):
		if choices != None:
			checker_choice(choices,tuple)
		
		if default == None:
			default = (0.0, 0.0)
		else:
			blank = True
			#if isinstance(default, tuple) == False:
			#	raise TypeError('default is of wrong type')
			if choices != None and default not in choices:
				raise TypeError('default cannot be found in choices')
			if isinstance(default[0], int) or isinstance(default[0], float):
				if isinstance(default[1], int) or isinstance(default[1], float):
					pass
			else:
				raise TypeError('coordinate values of wrong type')
			if default[0]>90 or default[0]<-90 or default[1]>180 or default[0]<-180:
				raise ValueError('coordinates out of range')
		self.blank = blank
		self.default = default
		self.choices = choices		
		pass

	def setname(self,name):
		self.name = '_' + name

	def __get__(self, inst,cls):
		return getattr (inst, self.name)

	def __set__(self,inst,value):
		if isinstance(value,tuple) == False and value != "DEFAULT":
			raise TypeError('wrong type for coordinate')
		if value == "DEFAULT":
			value = (0.0,0.0)
		if isinstance(value[0], int) or isinstance(value[0], float):
			if isinstance(value[1], int) or isinstance(value[1], float):
				pass
		else:
			raise TypeError('coordinate values of wrong type')

		if value[0]>90 or value[0]<-90 or value[1]>180 or value[0]<-180:
			raise ValueError('coordinates out of range')

		if self.default == (0.0,0.0) and value == 'DEFAULT':
			value = (0.0,0.0)
			self.blank = True
		else:
			if value != "DEFAULT":
				if self.default != 0.0 and self.default != (-99999.99,-999999.99):
					value = self.default	
					self.default = (-99999.99,-999999.99)
				else:
					value = value		
			elif value == "DEFAULT":
				value = self.default
		if self.choices:	
			if value not in self.choices and value != (0.0,0.0):
				raise ValueError ("not in choices")

		setattr(inst, self.name, value)


