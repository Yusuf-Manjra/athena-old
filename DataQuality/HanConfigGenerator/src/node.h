// emacs: this is -*- c++ -*-
//
//   @file    node.h        
//
//                   
//  
//      
//
//   $Id: node.h, v0.0   Fri  8 Apr 2016 00:57:16 CEST sutt $


#ifndef  NODE_H
#define  NODE_H

#include <iostream>
#include <string>
#include <vector>


#include "TObject.h"

class node : public std::vector<node*> {

public: 

  enum TYPE { HISTOGRAM, DIRECTORY, DUFF }; 
  
public:
  
  node( node* n=0, const std::string d="", TObject* t=0 ) : 
    mname("duff"), mparent(n), mtype(DUFF), mpath(""), mdepth(d), mobj(t) { 
    if ( t!=0 )  mname = t->GetName();
    mhirate = std::pair<std::string, double>( "", 0); 
  } 
  
  virtual ~node() { } 

  void               name( const std::string& n)  { mname=n; }
  const std::string& name() const { return mname; }
  
  void        path(const std::string& p) { mpath=p; }
  std::string path() const               { return mpath; }
  
  node*       parent()       { return mparent; }
  const node* parent() const { return mparent; }

  std::string depth() const { return mdepth; }

  void type(TYPE t)         { mtype=t; }
  virtual TYPE type() const { return mtype; }
  
  std::string stype() const { 
    if ( type()==DIRECTORY ) return "DIRECTORY";
    if ( type()==HISTOGRAM ) return "HISTOGRAM";
    return "DUFF";
  };
  
  const TObject* object() const { return mobj; }
  TObject*       object()       { return mobj; }

  void addrate( const std::string& s, double r ) { 
    addrate( std::pair<std::string, double>( s, r ) );
  }

  void addrate( const std::pair<std::string, double>& r ) { 
    if ( r.second > mhirate.second ) mhirate = r;
  }

  const std::pair<std::string, double>& rate() const { return mhirate; }


public:

  std::string mname;
  node*       mparent;
  TYPE        mtype;

  std::string mpath;

  std::string mdepth;

  TObject*    mobj;

  std::pair<std::string, double> mhirate; 

};



inline std::ostream& operator<<( std::ostream& s, const node& n ) {
  s << n.depth() << n.name() << "::" << n.stype() << " : obj " << n.object() << " : size " << n.size() << "\tpath " << n.path();
  if      ( n.type()==node::HISTOGRAM ) return s;
  else if ( n.size() ) {  
    if ( n.parent() ) s << "\t( parent " << n.parent()->name() << " )";
    if ( n.rate().first!="" )s << "\t\t:::(max rate chain " << n.rate().first << " " << n.rate().second << " ):::";
    for ( unsigned i=0 ; i<n.size() ; i++ ) { 
      //      if ( n[i]->type()!=node::HISTOGRAM ) 
      s << "\n" << i << " " << n.depth() << " " << *n[i];
    }
  }
  return s;
}




#endif  // NODE_H 










