#ifndef LIST_H
#define LIST_H
/*
This stuff is just to make sure that if by accident the header file gets included twice in the same file, everything in it won't be redefined causing 
a compiler error. With those in place the complier will just ignore the header if its included more than once in the file. 
*/

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

//Using an enum just makes things easier so that we don't need to memorize which numbers correspond to which material and we can just use them (for examples see main)
typedef enum
{
    NONE      = 0,
    STEEL     = 1,
    ALUMINIUM = 2,
    BLACK     = 3,
    WHITE     = 4
} Material;

typedef enum
{
	READ_IN_INT = 0,
	EXIT_PREP_INT = 1, 
	STEPPER_MOTOR = 2,
	DUMP = 3,
	SYSTEM_PAUSE = 4,
	RAMP_DOWN = 5
} State;

typedef struct item_t
{
    Material material;
	// if we need to add more data about the item we can put it here

    struct item_t* next;
    struct item_t* prev;
} Item;

typedef struct list_t
{
    Item* head;
    Item* tail;
    unsigned int _size;

    void (*create)(struct list_t*);
    void (*destroy)(struct list_t*);
    unsigned int (*size)(struct list_t*);
    bool (*empty)(struct list_t*);

    void (*push_back)(struct list_t*, Material);
    Item* (*pop_front)(struct list_t*);

    // We can add more functions here as needed, syntax is:
    // <return-type> (*<name>)(<params>);
} List;

List* new_list();
void delete_list(List*);

void init_list_table(List*);

void create(List*);
void destroy(List*);
unsigned int size(List*);
bool empty(List*); // yes bools do exist if you use the right header :D

void push_back(List*, Material);
Item* pop_front(List*);

#endif
