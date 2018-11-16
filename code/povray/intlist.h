
#include <iostream.h>

#ifndef __INTLIST_H

#define __INTLIST_H

typedef struct _int_list {
  int i;
  struct _int_list *next;
}
int_list;

extern void int_list_insert ( int_list **l, int i );

extern int_list ** int_list_find ( int_list **l, int i );

extern int int_list_is_member ( int_list **l, int i );

extern ostream & int_list_print ( ostream &o, int_list *l );

extern int int_list_remove ( int_list **l, int i );

extern int _int_list_remove ( int_list **l, int i, int_list **pt );

extern void int_list_clear ( int_list **l );

extern int int_list_length ( int_list *l );

#endif
