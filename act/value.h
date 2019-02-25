/*************************************************************************
 *
 *  This file is part of the ACT library
 *
 *  Copyright (c) 2019 Rajit Manohar
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
 *
 **************************************************************************
 */
#ifndef __ACT_VALUE_H__
#define __ACT_VALUE_H__


class ActNamespace;
class InstType;
class ActId;
struct ValueIdx;

/*
  Connections

  For non-parameter types, we represent connections.
   - ring of connections
   - union-find tree
*/
struct act_connection {
  ValueIdx *vx;			// identifier that has been allocated

  act_connection *parent;	// parent for id.id or id[val]
  
  act_connection *up;
  act_connection *next;
  act_connection **a;	// slots for root arrays and root userdefs

  ActId *toid();		// the ActId name for this connection entity
  
  bool isglobal();		// returns true if this is a global
				// signal

  bool isPrimary() { return (up == NULL) ? 1 : 0; }
  bool isPrimary(int i)  { return a[i] && (a[i]->isPrimary()); }
  
  
  // returns true when there are other things connected to it
  bool hasDirectconnections() { return next != this; }
  bool hasDirectconnections(int i) { return a[i] && a[i]->hasDirectconnections(); }
  
  // returns true if this is a complex object (array or user-defined)
  // with potential subconnections to it
  bool hasSubconnections() { return a != NULL; }
  int numSubconnections();
  ValueIdx *getvx();

  unsigned int getctype();
  // 0 = standard  "x"
  // 1 = array element "x[i]"
  // 2 = port "x.y" 
  // 3 = array element + port "x[i].y"


  act_connection *primary(); // return primary designee for this connection
  
};


/*
  Values: this is the core expanded data structure
*/
struct ValueIdx {
  InstType *t;
  struct act_attr *a;			// attributes for the value
  struct act_attr **array_spec;	// array deref-specific value
  
  unsigned int init:1;	   /**< Has this been allocated? 
			         0 = no allocation
				 1 = allocated
			    */
  unsigned int immutable:1;	/**< for parameter types: immutable if
				   it is in a namespace or a template
				   parameter */

  ActNamespace *global;	     /**< set for a namespace global; NULL
				otherwise. Note that global =>
				immutable, but not the other way
				around. */
  
  union {
    long idx;		   /**< Base index for allocated storage for
			      ptypes */
    struct {
      act_connection *c;	/**< For non-parameter types */
      const char *name;		/**< the base name, from the hash
				   table lookup: DO NOT FREE */
    } obj;
  } u;

  /* assumes object is not a parameter type */
  bool hasConnection()  { return init && (u.obj.c != NULL); }
  bool hasConnection(int i) { return init && (u.obj.c != NULL) && (u.obj.c->a) && (u.obj.c->a[i]); }

  bool hasSubconnections() { return hasConnection() && connection()->hasSubconnections(); }
  
  bool isPrimary() { return !hasConnection() || (u.obj.c->isPrimary()); }
  bool isPrimary(int i) { return !hasConnection(i) || (u.obj.c->a[i]->isPrimary()); }
  act_connection *connection() { return init ? u.obj.c : NULL; }
  const char *getName() { return u.obj.name; }
};

#endif /* __ACT_VALUE_H__ */
