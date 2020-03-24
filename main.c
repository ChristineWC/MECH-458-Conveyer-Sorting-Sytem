#include "list.h"

#include <stdio.h>

typedef struct material_counts_t
{
	int steel_count;
	int aluminium_count;
	int black_count;
	int white_count;
} MaterialCounts;

MaterialCounts material_counts;

void init_material_counts()
{
	material_counts.steel_count     = 0;
	material_counts.aluminium_count = 0;
	material_counts.black_count     = 0;
	material_counts.white_count     = 0;
}

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

int main()
{
	// To create a list, just invoke new_list. At this point all
	// of the data is correctly initialized and your list is ready to go.
	List* list = new_list();

	// Initialize your global variables.
	init_material_counts();

	// To add a new item, just use push_back like this:
	list->push_back(list, STEEL);
	list->push_back(list, ALUMINIUM);
	list->push_back(list, BLACK);
	list->push_back(list, WHITE);
	// so here because of the enum we can just use the words STEEL, ALUMINUM, etc…
	//instead of saying if it is 1 then its aluminum so we have to turn the stepper motor this far.

	// To remove an element do this:
	Item* front = list->pop_front(list);
	free(front);

	//to display the size of the list to the LCD:
	// list->size(list) holds the value. Display it to the LCD using proper LCD syntax

	//if you wanted to get a “property” of the item for example the material of an item you would do this:

	Item* it = list->head;
	Material mat = it->material;
	// mat would contain the material of the first thing in the list and we can do things based on that

	//to add 1 to the materials count:
	material_counts.steel_count++; // because it is a global struct you should be able to access it

	// to delete the list:
	delete_list(list);

	return 0;

}
