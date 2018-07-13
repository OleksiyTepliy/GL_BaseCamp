#include "lib.h"

int main(int argc, char* argv[])
{
        /* code */
struct list_head *main_head = malloc(sizeof(struct list_head));
if (main_head == NULL)
        exit(E_OUT_OF_MEM);

INIT_LIST_HEAD(main_head);

char *s[] = { "Tiger", "Smokey", "Misty", "Oscar", "Missy", "Tigger", "Sosis", "Kotishche", "Vaska", "Myrzik" };
struct mylist *tmp;
struct list_head *current_head = main_head;
        for (int i = 0; i < 10; i++) {
                tmp = malloc(sizeof(struct mylist));
                if (tmp == NULL) 
                        exit(E_OUT_OF_MEM);
                tmp->weight = (float) rand() / (float) rand();  // some numbers
                strcpy(tmp->name, s[i]);
                list_add_head(&(tmp->list), current_head);
                current_head = &tmp->list;
        }

        print_list(main_head);

        /*      INSERTION BLOCK FUNC INSERT     BEGIN   */
        printf("Provide a position for a new NODE\n");
        unsigned long x;
        scanf("%lu", &x);
        list_insert_pos(x, main_head);
        print_list(main_head);
        printf("Count of elements == %lu\n", list_counter(main_head));
        /*      INSERTION BLOCK FUNC INSERT     END     */ 


	/*      DELETE NODE BLOCK FUNC REMOVE     BEGIN	*/
        printf("Provide a position witch you want to delete\n");
        scanf("%lu", &x);
        list_remove_pos(x, main_head);
        print_list(main_head);
        /*      DELETE NODE BLOCK FUNC REMOVE     END	*/


	/*      COUNT BLOCK FUNC COUNT     BEGIN	*/
        printf("Count of elements == %lu\n", list_counter(main_head));
        /*      COUNT BLOCK FUNC COUNT     END		*/
        

        /*	SORT BLOCK FUNC SORT	BEGIN		*/
        printf("***SORT_BY_NAME***\n");
        sort_list(main_head, NAME, DESCEND);
        print_list(main_head);
        printf("***SORT_BY_WEIGHT***\n");
        sort_list(main_head, WEIGHT, ASCEND);
        print_list(main_head);
	/*	SORT BLOCK FUNC SORT	END		*/


        /*	REVERSE BLOCK FUNC SORT	BEGIN		*/
        printf("REVERSE NOW\n");
        reverse(main_head);
        print_list(main_head);
	/*	REVERSE BLOCK FUNC SORT	END		*/         
        

        /*      FREE LIST BLOCK FUNC CLEAR       BEGIN	*/
        printf("FREE LIST\n");
        clear_all(main_head);
        if (list_empty(main_head)){
                printf("LIST IS EMPTY\n");
                DELETE_LIST_HEAD(main_head);
                return E_OK;
        }
        exit(E_REMOVE);
        /*      FREE LIST BLOCK FUNC CLEAR       END     */
}
