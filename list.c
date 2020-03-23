#include "list.h"

// This part is to forward-declare all functions here.
void init_list_table(List*);

void create(List*);
void destroy(List*);
unsigned int size(List*);
bool empty(List*); // yes bools do exist if you use the right header :D 

void push_back(List*, Material);
Item* pop_front(List*);

//Below are the functions and everything they do

List* new_list() //this is to initialize the list
{
    List* list = (List*)calloc(1, sizeof(List));

    init_list_table(list);
    list->create(list);

    return list;
}

void delete_list(List* list) //this function is to make sure that the list pointer is null
{
    list->destroy(list);
    free(list);
}

void init_list_table(List* list)// since this is using function pointers, this lets us use the syntax NameOfList->nameOfFunction(parameters); examples can be seen in main
{
    list->create  = create;
    list->destroy = destroy;
    list->size    = size;
    list->empty   = empty;

    list->push_back = push_back;
    list->pop_front = pop_front;
}

void create(List* this) // part of list initialization
{
    this->head  = NULL;
    this->tail  = NULL;
    this->_size = 0;
}

void destroy(List* this)
{
    if (this->empty(this))
    {
        return;
    }

    Item* it;
    while (this->size(this) != 0)
    {
        it = this->pop_front(this);
        free(it);
        it = NULL;
    }
}

unsigned int size(List* this)
{
    return this->_size;
}

bool empty(List* this)
{
    return this->_size == 0;
}

void push_back(List* this, Material m) // use this function to add stuff to the back of the list
{
    Item* new_item = (Item*)calloc(1, sizeof(Item));

    new_item->material = m;

    if (this->empty(this))
    {
        this->head     = new_item;
        this->tail     = new_item;
        new_item->next = NULL;
        new_item->prev = NULL;
    }
    else
    {
        new_item->prev   = this->tail;
        new_item->next   = NULL;
        this->tail->next = new_item;
        this->tail       = new_item;
    }

    this->_size++;
}

Item* pop_front(List* this) // use this function to delete stuff at the front of the list
{
    if (this->empty(this))
    {
        return NULL;
    }

    Item* front;
    if (this->_size == 1)
    {
        front      = this->head;
        this->head = NULL;
        this->tail = NULL;
    }
    else
    {
        front            = this->head;
        this->head       = this->head->next;
        this->head->prev = NULL;
        front->next      = NULL;
    }

    this->_size--;
    return front;
}
