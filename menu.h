/*!
 *======================================================================================================================
 * 
 * @file		menu.h
 *
 * @brief	 	Menu header file 
 *
 *======================================================================================================================
 */
#ifndef __MENU_H__
#define __MENU_H__

struct _menu {
   char *name;
   char *desc;
   char *help;
   char *args;
   int (*handler)(struct _menu *m, int argc, char **argv, void *p_usr);
   struct _menu *child;
   struct _menu *parent;
   struct _menu *next;
   struct _menu *prev;
};
typedef struct _menu menu_t;


menu_t *menu_create(char *name, char *desc, char *help, char *args, 
		    int (*handler)(struct _menu *m, int argc, char **argv, void *p_usr));
int menu_destroy(menu_t *m);
int menu_add_child(menu_t *parent, menu_t *child); 
int menu_add_peer(menu_t *parent, menu_t *peer); 


#endif // __MENU_H__
