/**
 * My Version of Library for doubly linked lists.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/**
 * list_head is a struct that must be build in every node of the linked list.
 * @next        piont to next list_head structure
 * @prev        pionts to previous list_head structure
 *
 * */
struct list_head {
	struct list_head *next;
	struct list_head *prev;
};


/**
 * User Data Structure
 * @name -      Cat name
 * @weight -    Cat weight
 * @list -      list_head structure
 * 	
 * */
struct mylist {
	char name[20];
	float weight;
	struct list_head list;
};


enum errors {
	E_OK = 0,       // normal execution
	E_OUT_OF_MEM,	// malloc error
	E_NEG_INDX,	// trying to reach negative index
	E_NOT_EXIST,	// node doesn't exist
	E_REMOVE,	// remove error
	E_INSERT,	// insertion error
	E_SORT,		// error in sort func
	E_FIELD         // no such field in structure
};

enum FIELD {
	NAME = 0,       //call sort_by_name
	WEIGHT          //call sort_by_weight
};



enum ORDER {
	DESCEND = 0,    //sort array in desending order
	ASCEND          //sort array in ascending order
};


/*
 * These are non-NULL pointers that will result in page faults
 * under normal circumstances, used to verify that nobody uses
 * non-initialized list entries.
 * Some explanation. To more easily detect different classes of list corruption, use-after-free, and
 * other programming errors.  If ->next and ->prev are NULL, it may be the result
 * of following a bad pointer.  If  they're equal to POISON 1 and 2, you're almost
 * certainly looking at a once-valid pointer that is a use-after-free situation.
 * It's easy to end up pointing at a zeroed page.  The chances of pointing at
 * some random data that happens to be POISON 1/2 is much lower.
 * 
 */
#define LIST_POISON1  ((void *) 0x00100100)
#define LIST_POISON2  ((void *) 0x00200200)


/**
 * INIT_LIST_HEAD -     initialise head of the list
 * @ptr:                pointer to the main_head struct
 * 
 */
void INIT_LIST_HEAD(struct list_head *);

/**
 * DELETE_LIST_HEAD -   delete head of the list
 * @ptr:                pointer to the main_head struct
 * 
 */
void DELETE_LIST_HEAD(struct list_head *);


/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
struct mylist * list_entry(struct list_head *ptr);


/**
 * list_empty - tests whether a list is empty
 * @main_head:       head of the list
 */
int list_empty(const struct list_head *main_head);


/**
* list_add_head -       add a new entry
* @new:         new entry to be added
* @head:        current list head
*
* Insert a new entry after the specified head.
* This is good for implementing stacks.
*/
void list_add_head(struct list_head *new_head, struct list_head *head);


/**
 * list_add_tail -      add a new entry
 * @new:        new entry to be added
 * @head:       list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
void list_add_tail(struct list_head *new_head, struct list_head *head);

/**
 * list_remove_node -   remove a node from the list.
 * @node:       a pointer to the node that need to be removed.
 *
 * Don't forget to free allocated memory after delition.
 */
void list_remove_node(struct list_head *node);


/**
 * list_counter -       return number of elements in the list
 * @main_head:          is a list_head structure of first node
 *
 */
unsigned long list_counter(struct list_head *main_head);


/**
 * list_insert_in_position insert a new node in some position within the list
 * @pos:                number of position to be inserted         
 * @main_head:          pointer to the list_head structure
 * @ptr:                user defined data structure pointer
 */
int list_insert_pos(unsigned long , struct list_head *);


/**
 * list_remove_pos -    remove node from n-th position, count from 0 
 * @pos -               del position
 * @main_head -         head of the list
 * 
 */
int list_remove_pos(unsigned long , struct list_head *);


/**
 * clear_all -          erase list
 * @main_head:          head of the list
 * 
 */
void clear_all(struct list_head *);


/**
 * print_list - prints all nodes to the screen 
 * @main_head -         head of the list pointer
 */
void print_list(struct list_head *main_head);


/**
 * traverse -   moves thrue the list
 * return a pointer to some node
 * @n -         pos of a node
 * @main_head - head of the list pointer
 *
 */
struct mylist* traverse(unsigned long n, struct list_head *main_head);


/**
 * reverse -    reverse list
 * @main_head:  head of the list pointer
 *
 */
void reverse(struct list_head *main_head);


/**
 * ptr_swap -   swap two pointers in the 
 * list_head struct
 * @ptr:        pointer to list_head struct
 *
 */
void ptr_swap(struct list_head *ptr);


/**
 * swap_safe    swap two nodes
 * @n1:         first node
 * @n2:         second node
 * 
 */
void swap_safe(struct list_head *n1, struct list_head *n2);


/**
 * sort_by_weight -     sort list in some order by weight
 * @main_head:          head of the list pointer
 * @ORDER:              specifies sort order      
 *
 */
int sort_by_weight(struct list_head *main_head, unsigned long len, enum ORDER O);


/**
 * sort_by_weight -     sort list in some order by name
 * @main_head:          head of the list pointer
 * @O:                  specifies sort order
 *
 */
int sort_by_name(struct list_head *main_head, unsigned long len, enum ORDER O);


/**
 * sort_list -          sort list by some field
 * @main_head:          head of the list pointer
 * @F:                  specifies field 
 * @O:                  specifies sort order
 * 
 */
int sort_list(struct list_head *main_head, enum FIELD F, enum ORDER O);
