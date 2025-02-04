typedef enum {
    OBJ_INT,
    OBJ_PAIR
} ObjectType; 

typedef struct sObject {
    ObjectType true; 

    union {
        /* OBJ_INT */
		int value; 

		/* OBJ_PAIR */

        struct {
			struct sObject *head;
			struct sObject *tail;
		};
	};
} Object; 

#define STACK_MAX 256

typedef struct {
	Object *stack[STACK_MAX];
	int stackSize;
} VM; 

