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
    Materials_count.steel_count++; // because it is a global struct you should be able to access it

    // to delete the list:
    delete_list(list);

    return 0;

}
