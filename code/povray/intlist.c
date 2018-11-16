
#include "intlist.h"

void int_list_insert ( int_list **l, int i )
{
  int_list *tmp = new int_list;
  tmp->i = i;
  tmp->next = *l;
  *l = tmp;
}

int_list ** int_list_find ( int_list **l, int i )
{
  int_list **res = l;

  while(*res!=NULL)
    {
      if ((*res)->i==i)
	return res;
      res = &((*res)->next);
    }
  return res;
}

int int_list_is_member ( int_list **l, int i )
{
  return !!*int_list_find(l,i);
}

ostream & int_list_print ( ostream &o, int_list *l )
{
  cout << "( ";
  while (l)
    {
      cout << l->i << " ";
      l=l->next;
    }
  cout << ")";
  return o;
}

int int_list_remove ( int_list **l, int i )
{
  if (!*l)
    return -1;
  if ((*l)->i==i)
    {
      int_list *tmp = (*l)->next;
      delete *l;
      *l = tmp;
      return 0;
    }
  
  int_list *il = *l;
  while (il->next)
    {
      if (il->next->i==i)
	{
	  int_list *tmp = il->next;
	  il->next = il->next->next;
	  delete tmp;
	  return 0;
	}
      il = il->next;
    }
  return -1;
} 

int _int_list_remove ( int_list **l, int i, int_list **pt )
{
  int res = 0;

  if (!*l)
    return 0;
  if ((*l)->i==i)
    {
      int_list *tmp = (*l)->next;
      if (*pt==*l)
	{
	  *pt = tmp;
	  res = 1;
	}
      delete *l;
      *l = tmp;
      return res;
    }
  
  int_list *il = *l;
  while (il->next)
    {
      if (il->next->i==i)
	{
	  int_list *tmp = il->next;
	  if (*pt==tmp)
	    {
	      *pt = il;
	      res = 0;
	    }	  
	  il->next = il->next->next;
	  delete tmp;
	  return res;
	}
      il = il->next;
    }
  return -1;
} 

void int_list_clear ( int_list **l )
{
  while (*l)
    {
      int_list *tmp = (*l)->next;
      delete *l;
      *l = tmp;
    }
  *l = NULL;
}

int int_list_length ( int_list *l )
{
  int res = 0;
  for ( int_list *i=l; i; i=i->next )
    res++;
  return res;
}
