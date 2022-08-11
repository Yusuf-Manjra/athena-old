# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration


"""
This module defines utilities for the jet config.
These are mainly to allow to "lock" the standard, reference definitions so
they are not accidentally changed by some user scripts.
"""

class lproperty(property):
    """Build on the standard property to allow a property to be locked if the holding class has its _locked attribute set to True """
    def lsetter(self, func):
        def lockedfunc(self, v):
            if self._locked:
                raise Exception("Error "+func.__name__+" is locked. Either clone or unlock",self)
            func(self,v)
        
        return self.setter(lockedfunc)

def make_lproperty( func  ):
    """creates a property from a class method (or a str) which can be locked if the holding class has its _locked attribute set to True

    usage : 
    class A:
        # simple locked property
        x = make_lproperty('x')

        # same but using decorator
        @make_lproperty
        def y(self): pass       

        # same but now with customized setter :
        @make_lproperty
        def z(self): pass
        @z.lsetter
        def z(self, v):
            print("setting z to ",v)
            self._z = v

    """
    if isinstance(func, str):
        pname = func
    else:
        pname = func.__name__
    pname_i =  '_'+pname
       
    def _getter(self):
        return getattr(self,pname_i, None)
    def _setter(self, v):
        if self._locked:            
            raise AttributeError("Error property '"+pname+"' is locked. Either clone or unlock",self)
        setattr(self, pname_i, v)
    return lproperty(_getter, _setter)

def make_alias( prop  ):
    """Returns a property which act as a read-only alias to existing prop """
    def _getter(self):
        return getattr(self,prop,None)
    def _setter(self,v):
        raise AttributeError("Cannot set alias "+prop+" from "+str(self) )
    return property(_getter, _setter)

def listClassLProperties(cls):
    # # if cls has a predefined _allowedattributes, use it, else start with []
    lprops = getattr(cls, '_allowedattributes', [])
    # allow all lproperty attached to cls
    lprops += [k for (k,v) in cls.__dict__.items() if isinstance(v,lproperty) ]
    # and the corresponding internal _xyz members
    lprops +=[ '_'+k for k in lprops]
    # also allow all what is allowed from the base classes
    for base in cls.__bases__:
        lprops += listClassLProperties(base)
    return lprops
    
def onlyAttributesAreProperties(cls):
    """Transforms the input class cls so the only attributes which can be set are the lproperty of the class.
    Best used as a decorator. Ex : 
    @onlyAttributesAreProperties
    class A:
        myprop0 = make_lproperty('myprop0')
    
    a = A()
    a.myprop0 = 0 # ok 
    a.mypropO = 3 # impossible
    """
    # build the list of attributes allowed to be set : these are the properties and _locked
    #cls._allowedattributes = [k for (k,v) in cls.__dict__.items() if isinstance(v,lproperty) ]
    #cls._allowedattributes +=[ '_'+k for k in cls._allowedattributes]
    cls._allowedattributes = listClassLProperties( cls )
    cls._allowedattributes += ['_locked']

    # flag to activate the prevention of adding new attributes. we set it at the end of __init__ 
    cls._nomoreattributes=False
    cls._locked = False
    
    cls.__init__origin = cls.__init__ 
    
    # define a new __init__ for this class.
    # the 'finalinit' argument allows to avoid locking the allowed attribute : this is to be used when a derived class wants to call the init of it's base class.
    def initwraper(self, *l,finalinit=True, **args):
        cls.__init__origin(self, *l,**args)
        self._nomoreattributes = finalinit
    cls.__init__ = initwraper
        
    # define a __setattr__ for this class
    def setattr(self, k, v):
        if self._nomoreattributes and k not in self._allowedattributes:
            raise AttributeError("Setting attribute "+k+" on "+str(self)+" not allowed")
        super(cls,self).__setattr__(k,v)        
    cls.__setattr__ = setattr

    return cls

def clonable(cls):
    """Transforms the input class cls by adding a clone() method. 
    This clone() method returns a clone instance with a _locked attribute set to False by default (so the clone is modifiable) """

    def recc_lock(o, lock):
        if hasattr(o,"_locked"):
            o._locked = lock
            for k,v in o.__dict__.items():
                recc_lock(v,lock)
        
    def clone(self, lock=False, **args):
        from copy import deepcopy
        o = deepcopy(self)
        o._locked = False # unlock so we can modfiy the clone with user given arguments
        for k,v in args.items():
            setattr(o,k,v)
        recc_lock(o,lock) # make sure lock is propagated to all lockable sub-object
        return o
    cls.clone = clone
    return cls


class ldict(dict):
    """A dictionnary which items can not be modified once set.
    
    Also implements 2 additional features :
     - a clone method to conveniently create a new version with only some items changed
     - all items are also attributes (i.e. d['a'] can be accessed by d.a : this is mostly for easier interactive inspection)
    """
    def __init__(self, **args):
        super().__init__(**args)
        # make key aivailable as attribute
        for k,v in args.items():
            super().__setattr__(k,v)
            
        
    def __setitem__(self, k, v):
        if k in self:
            raise KeyError("Can't override key "+k)
        super().__setitem__(k,v)
        super().__setattr__(k,v)

    def update(self, **args): # we need to redefine it
        for k,v in args.items():
            self[k]=v

    def clone(self,**args):
        from copy import deepcopy
        o = deepcopy(self)
        for k,v in args.items():
            # bypass the protection to update the user given items
            dict.__setitem__(o,k,v)            
            dict.__setattr__(o,k,v)
        return o
            
def flattenDic(inD):
    """returns a copy of the inD dictionnary with no nested directories. Instead the content of nested dict inside inD are indexed by keys constructed from nested keys :
    {'A' : 3 , 'B' : { 'C':5, 'D': 6} } --> {'A' : 3 , 'B.C':5, 'B.D': 6}
    This works only if nested dic have str keys.

    This function is used mainly in trigger config
     """
    outD = {}
    for k,v in inD.items():
        if isinstance(v,dict):
            subD = flattenDic(v)
            for subk, subv in subD.items():
                outD[f'{k}.{subk}']=subv                
        else:
            outD[k]=v
    return outD
