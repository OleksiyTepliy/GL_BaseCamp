
/**
 * My Version of Library for doubly linked lists.
 */
#include "lib.h"


inline void INIT_LIST_HEAD(struct list_head *ptr)                
{
        ptr->next = ptr;
        ptr->prev = ptr;
}


inline void DELETE_LIST_HEAD(struct list_head *ptr)
{
        free(ptr);
}


inline struct mylist * list_entry(struct list_head *ptr)
{
        return (struct mylist *) ((char *) (ptr) - (unsigned long) (&((struct mylist *) 0)->list));
}


inline int list_empty(const struct list_head *main_head)
{
	return main_head->next == main_head;
}


inline void list_add_head(struct list_head *new_head, struct list_head *head) 
{
        new_head->next = head->next;
        new_head->prev = head;
        head->next->prev = new_head;
        head->next = new_head;
}


inline void list_add_tail(struct list_head *new_head, struct list_head *head) 
{
        new_head->next = head;
        new_head->prev = head->prev;
        head->prev->next = new_head;
        head->prev = new_head; 
}


inline void list_remove_node(struct list_head *node) 
{
        node->prev->next = node->next;
        node->next->prev = node->prev;
        node->next = LIST_POISON1;
	node->prev = LIST_POISON2;
}


unsigned long list_counter(struct list_head *main_head)
{
        struct list_head *tmp;
        unsigned long count = 0;
        for(tmp = main_head; tmp->next != main_head; tmp = tmp->next) {
                ++count;
        }
        return count;
}


int list_insert_pos(unsigned long pos, struct list_head *main_head)
{
        unsigned long n = list_counter(main_head);
        if (pos < 0 || pos > n - 1) {
                fprintf(stderr, "**CANT ADD** Please provide \
                valid number between 0 and %lu", n);
                exit(E_INSERT);
        }
        /* find place for insertion */
        struct list_head *tmp = main_head;
        for (int i = 0; i < pos; i++) {
                tmp = tmp->next;
        }
        /* allocate memory for new node */
        struct mylist *ptr = malloc(sizeof(struct mylist));
        if (ptr == NULL)
                exit(E_OUT_OF_MEM);
        /* adding node in place */
        printf("Enter Name :");
        char name[20];
        scanf("%s", name);
        printf("\nEnter Weight 0.00 kg :");
        float weight = 0;
        scanf("%f", &weight);
        strcpy(ptr->name, name);
        ptr->weight =  weight;
        list_add_head(&(ptr->list), tmp);
        printf("ADDITION SUCSSES\n");
        return E_OK;
}


int list_remove_pos(unsigned long pos, struct list_head *main_head)
{
        unsigned long n = list_counter(main_head);
        if (pos < 0 || pos > n) {
                fprintf(stderr, "**CANT REMOVE**, Please provide valid \
                number between 1 and %lu", n);
                exit(E_REMOVE);
        }
        /* find position for delition */
        struct list_head *tmp = main_head;
        for (int i = 0; i <= pos; i++) {
                tmp = tmp->next;
        }
        list_remove_node(tmp);
        /* free node pointer */
        free(list_entry(tmp));
        return E_OK;
}


void clear_all(struct list_head *main_head)
{
        struct list_head *pos, *n;
        for (pos = main_head->next, n = pos->next; pos != main_head; pos = n, n = pos->next) {
                list_remove_node(pos);
                free(list_entry(pos));
        }
}


struct mylist* traverse(unsigned long n, struct list_head *main_head)
{
        if (n < 0) {
                fprintf(stderr, "**Error** , n < 0\n");
                exit(E_NEG_INDX);
        }
        struct list_head *pos;
        unsigned long k; 
        for (k = 0, pos = main_head->next; k < n; pos = pos->next, k++) {
                if (pos->next == main_head) {
                        fprintf(stderr, "**LIST ENDED**, last element is %lu\n", k);
                        exit(E_NOT_EXIST);
                }
        }
        return list_entry(pos);
}

void print_list(struct list_head *main_head)
{
        struct mylist *node_ptr;
        struct list_head *pos;
        for (pos = main_head->next; pos != main_head; pos = pos->next) {
                node_ptr = list_entry(pos);
                printf("%s weight = %4.2fkg\n", node_ptr->name, node_ptr->weight);
        }
}

void ptr_swap(struct list_head *ptr)
{
        struct list_head *tmp = ptr->next;
        ptr->next = ptr->prev;
        ptr->prev = tmp;
}


void reverse(struct list_head *main_head)
{
        struct list_head *tmp, *pos;
        for (pos = main_head->next; pos != main_head; pos = tmp) {
                tmp = pos->next;
                ptr_swap(pos);
        }
        ptr_swap(main_head);
}


void swap_safe(struct list_head *n1, struct list_head *n2)
{
        struct list_head *tmpn2p, *tmpn2n;
        tmpn2p = n2->prev;
        tmpn2n = n2->next;
        
        if (n1->next == n2 && n2->prev == n1) {
                n1->prev->next = n2;
                n2->next->prev = n1;
                n2->next = n1;
                n1->next = tmpn2n;
                n2->prev = n1->prev;
                n1->prev = n2;
        }
        else if (n2->next == n1 && n1->prev == n2) {
                n2->prev->next = n1;
                n1->next->prev = n2;
                n2->next = n1->next;
                n2->prev = n1;
                n1->next = n2;
                n1->prev = tmpn2p;
        }
        else {
                n1->prev->next = n2;
                n2->prev = n1->prev;
                n1->next->prev = n2;
                n2->next = n1->next;

                tmpn2p->next = n1;
                n1->prev = tmpn2p;
                tmpn2n->prev = n1;
                n1->next = tmpn2n;
        }
}

int equal_count(struct list_head *main_head, struct list_head *compared, enum FIELD F)
{
      return E_OK;  
}


int sort_by_weight(struct list_head *main_head, unsigned long len, enum ORDER O)
{
        int swap_count;
        struct mylist *n1, *n2;
        for (unsigned int i = 0; i < len - 1; i++) {
                swap_count = 0;
                for (int j = 0; j < len - i - 1; j++) {
                        n1 = traverse(j, main_head);
                        n2 = traverse(j + 1, main_head);
                        if (O == ASCEND) {
                                if (n1->weight > n2->weight) {
                                        swap_safe(&n1->list, &n2->list);
                                        ++swap_count;
                                }
                        }
                        if (O == DESCEND) {
                                if (n1->weight < n2->weight) {
                                        swap_safe(&n1->list, &n2->list);
                                        ++swap_count;
                                }
                        }
                }
                if (swap_count == 0)
                        return E_OK;
        }
        fprintf(stderr, "**SORT** ERR\n");
        exit(E_SORT);
}


int sort_by_name(struct list_head *main_head, unsigned long len, enum ORDER O)
{
        int swap_count, res;
        struct mylist *n1, *n2;
        for (unsigned int i = 0; i < len - 1; i++) {
                swap_count = 0;
                for (int j = 0; j < len - i - 1; j++) {
                        n1 = traverse(j, main_head);
                        n2 = traverse(j + 1, main_head);
                        res = strcmp(n1->name, n2->name);
                        if (O == ASCEND) {
                                if (res > 0) {
                                        swap_safe(&n1->list, &n2->list);
                                        ++swap_count;
                                }
                        }
                        if (O == DESCEND) {
                                if (res < 0) {
                                        swap_safe(&n1->list, &n2->list);
                                        ++swap_count;
                                }
                        }
                }
                if (swap_count == 0)
                        return E_OK;
        }
        fprintf(stderr, "**SORT** ERR\n");
        exit(E_SORT);
}


int sort_list(struct list_head *main_head, enum FIELD F, enum ORDER O)
{
        unsigned long len = list_counter(main_head);
        if (F == WEIGHT) {
                sort_by_weight(main_head, len, O);
                return E_OK;
        }
        if(F == NAME) {
                sort_by_name(main_head, len, O);
                return E_OK;
        }
        fprintf(stderr, "**FILED EROOR**\n");
        exit(E_FIELD);
}








// some problem with pointers, func hung up.
// int sort_list(const struct list_head *main_head, int param)
// {       // bubble sort
//         struct list_head *i, *j;
//         int swap_count;
//         for (i = main_head->next; i != main_head; i = i->next) {
//                 swap_count = 0;
//                 for (j = main_head->next; j != main_head->prev; j = j->next) {
//                         if (list_entry(j)->weight > list_entry(j->next)->weight) {
//                                 struct list_head *pos; struct mylist *node_ptr;
//                                 swap_safe(j, j->next);
//                                 ++swap_count;
//                                 for (pos = main_head->next; pos != main_head; pos = pos->next) {
//                                         node_ptr = list_entry(pos);
//                                 }
//                         }
//                 }
//                 if (swap_count == 0) {
//                         return E_OK;
//                 }
//         }
//         fprintf(stderr, "**SORT** ERR\n");
//         exit(E_SORT);
// }
