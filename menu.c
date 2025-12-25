/*!
 *======================================================================================================================
 * 
 * @file		menu.c
 *
 * @brief	 	Menu implementation 
 *
 *======================================================================================================================
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>

#include "menu.h"



/*!
 *----------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		menu_t *menu_create(char *name, char *desc, char *help, char *args, 
 *                                  int (*handler)(int argc, char **argv))
 *
 *  @brief	Create a new menu item
 *
 *  @param	name:		menu item name
 *  @param	desc:		menu item description
 *  @param	help:		menu item help 
 *  @param	args:	menu arg string 
 *  @param	handler:	menu handler function 
 *
 *  @return	pointer to menu item; NULL if failed to create
 *
 *----------------------------------------------------------------------------------------------------------------------
 */
menu_t *menu_create(char *name, char *desc, char *help, char *args, 
		    int (*handler)(struct _menu *m, int argc, char **argv, void *p_usr))
{
   if (name==NULL || desc==NULL) return NULL;

   menu_t *m = (menu_t *)calloc(1, sizeof(menu_t));
   if (m != NULL) 
   {
      m->name = name;
      m->desc = desc;
      m->help = help; 
      m->args = args; 
      m->handler = handler; 
      m->child = NULL;
      m->parent = NULL;
      m->next = NULL;
      m->prev = NULL;
   }

   return m; 
}


/*!
 *----------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int menu_destroy(menu_t *m)
 *
 *  @brief	Free menu item and its children
 *
 *  @return	0 if success; negative otherwise
 *
 *----------------------------------------------------------------------------------------------------------------------
 */
int menu_destroy(menu_t *m)
{
   if (m == NULL) return -1;

   if (m->child != NULL)
      menu_destroy(m->child);

   if (m->prev != NULL)
      m->prev->next = m->next;

   free(m);
   return 0;
}


/*!
 *----------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int menu_add_child(menu_t *parent, menu_t *child)
 *
 *  @param	parent:		parent node
 *  @param	child:		child node
 *
 *  @return	0 if success; negative otherwise
 *
 *----------------------------------------------------------------------------------------------------------------------
 */
int menu_add_child(menu_t *parent, menu_t *child)
{
   if (parent==NULL || child==NULL) return -1;

   child->parent = parent;
   parent->child = child;

   return 0;
}


/*!
 *----------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int menu_add_peer(menu_t *m, menu_t *peer)
 *
 *  @brief	Add a peer node (along with its attached peer nodes) to 'm'
 *
 *  @param	m:		lead peer node
 *  @param	peer:	 	new peer	
 *
 *  @return	0 if success; negative otherwise
 *
 *----------------------------------------------------------------------------------------------------------------------
 */
int menu_add_peer(menu_t *m, menu_t *peer)
{
   if (m==NULL || peer==NULL) return -1;

   menu_t *tail = m;
   while (tail->next != NULL) tail = tail->next;
   peer->prev = tail; 
   tail->next = peer;

   return 0;
}


/*!
 *----------------------------------------------------------------------------------------------------------------------
 *
 *  @fn		int menu_process(menu_t *m, int argc, char **argv, void *p_usr)
 *
 *  @brief	Process menu from root node 'm'
 *
 *----------------------------------------------------------------------------------------------------------------------
 */
int menu_process(menu_t *m, int argc, char **argv, void *p_usr)
{
   int rc = -1;
   if (m==NULL || argc==0 || argv==NULL) return -1;

  
   // Check if first token matches self or peer commands
   bool found = false;
   while (!found) 
   {
      if (m != NULL && 0 == strcmp(argv[0], m->name)) 
         found = true;
      else
         m = m->next;
   }

   // If found, process child if present; otherwise call handler
   if (found) 
   {
      if (m->child != NULL)                              // If child exist, process child recursively
      {
         rc = menu_process(m->child, argc-1, &argv[1], p_usr);
	 if (rc < 0)
            printf("  Hint: %s\n", m->help);
	 return rc;
      }
      else if (m->handler != NULL)                       // No child; call handler if defined
      {							  
         rc = (*m->handler)(m, argc-1, &argv[1], p_usr);
	 if (rc < 0)
            printf("Error %d. Hint: %s\n", rc, m->help);
         return rc;
      }
   }
   
   printf("  Error: something is wrong...\n");  
   if (m->help != NULL)
      printf("  Hint: %s\n", m->help);

   return -1;
}



