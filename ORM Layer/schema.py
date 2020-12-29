import orm

if orm.DateTime.implemented:
    from datetime import datetime

class User(orm.Table):
    firstName = orm.String()
    lastName = orm.String()
    height = orm.Float(blank=True)
    age = orm.Integer(blank=True)

    def __repr__(self):
        return "<User: %s %s>"%(self.firstName, self.lastName)
    
class Account(orm.Table):   
    user = orm.Foreign(User)
    type = orm.String(choices=["Savings", "Chequing",], default="Chequing")
    balance = orm.Float(blank=True)
    
    def __repr__(self):
        return "<Account: %s's %s>"%(self.user.firstName, self.type)

if orm.Coordinate.implemented:
    class Capital(orm.Table):
        location = orm.Coordinate()
        name = orm.String()
        
        def __repr__(self):
            return "<Capital: %s>"%(self.name)
            
if orm.DateTime.implemented:
    class Parade(orm.Table):
        location = orm.Foreign(Capital)
        start = orm.DateTime(default=datetime.now)
        end = orm.DateTime(blank=True)
        
        def __repr__(self):
            return "<Parade: %d/%d/%d - %s>"%(self.start.year, 
                self.start.month, self.start.day, self.location.name)
