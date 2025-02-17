#include <stdlib.h>

typedef enum {
    OBJ_INT,
    OBJ_PAIR
} ObjectType; 

typedef struct sObject {

    /* The next object in the list of all objects. */
	struct sObject *next; 

	ObjectType type;
	unsigned char marked; 

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
#define INITIAL_GC_THRESHOLD 10

typedef struct {

    /* The total number of currently allocated objects. */
	int numObjects;

    /* The number of objects required to trigger a GC. */
	int maxObjects;

	/* The first object in the list of all objects. */
	Object *firstObject;

	Object *stack[STACK_MAX];
	int stackSize;
} VM; 

VM* newVM() {
	VM *vm = malloc(sizeof(VM));
	vm->stackSize = 0;
	vm->firstObject = NULL;

	vm->numObjects = 0;
	vm->maxObjects = INITIAL_GC_THRESHOLD;
	return vm;
}

void push(VM* vm, Object* value) {
	assert(vm->stackSize < STACK_MAX, "Stack overflow!");
	vm->stack[vm->stackSize++] = value; 
}

Object* pop(VM* vm) {
	assert(vm->stackSize > 0, "Stack underflow!");
	return vm->stack[--vm->stackSize];
}

Object* newObject(VM* vm, ObjectType type) {

    if (vm->numObjects == vm->maxObjects) gc(vm);

    /* Create object... */
	Object *object = malloc(sizeof(Object));
	object->type = type;
	object->marked = 0;

    /* Insert it into the list of allocated objects. */
	object->next = vm->firstObject;
	vm->firstObject = object;

	vm->numObjects++; 
	return object;
}

void pushInt(VM* vm, int intValue) {
	Object *object = newObject(vm, OBJ_INT);
	object->value = intValue;
	push(vm, object);
}

Object* pushPair(VM* vm) {
	Object *object = newObject(vm, OBJ_PAIR);
	object->tail = pop(vm);
	object->head = pop(vm);

	push(vm, object);
	return object; 
}

void mark(Object* object) {

    /* If already marked, we're done. Check this first
       to avoid recursing on cycles in the object graph. */

    if (object->marked) return; 

	object->marked = 1; 

    if (object->type == OBJ_PAIR) {
		mark(object->head);
		mark(object->tail);
	}
}

void markAll(VM* vm) {
	for (int i = 0; i < vm->stackSize; i++) {
		mark(vm->stack[i]);
	}
}

void sweep(VM* vm) {
	Object **object = &vm->firstObject; 
    while (*object) {
        if (!(*object)->marked) {
            /* This object wasn't reached, so remove it from the list
               and free it. */
			Object *unreached = *object;

			*object = unreached->next;
			free(unreached); 
		} else {
            /* This object was reached, so unmark it (for the next GC)
               and move on to the next. */
			(*object)->marked = 0;
			object = &(*object)->next;
		}
	}
	vm->numObjects--; 
}

void gc(VM* vm) {
	int numObjects = vm->numObjects;

	markAll(vm);
	sweep(vm);

	vm->maxObjects = vm->numObjects * 2; 
}