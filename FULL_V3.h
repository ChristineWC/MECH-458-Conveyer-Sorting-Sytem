//header file

typedef enum
{
    BLACK    = 0,
    STEEL    = 50,
    WHITE    = 100,
    ALUMINUM = 150
} Material;

typedef struct item{
	Material	mat;
	struct item *next;
} Item;


void	initLink	(Item **newLink);
void 	setup		(Item **h, Item **t);
void 	clearQueue	(Item **h, Item **t);
void 	enqueue		(Item **h, Item **t, Item **nL);
void 	dequeue		(Item **h, Item **deQueuedLink);
Material firstValue	(Item **h);
char 	isEmpty		(Item **h);
int 	size		(Item **h, Item **t);

