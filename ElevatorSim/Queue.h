/*
		Authors:	Janahan Dhushenthen (56777220)
					Brandon Bwanakocha (35366525)
		   Date:	November 13, 2020
	Description:	Queue Class: Queues Elevator requests
*/

#ifndef   __Queue__
#define   __Queue__

#include "rt.h"

#define EMPTYQUEUE -1

typedef struct Node{
	int floor;								// Destination floor
	Node* next;								// Next Node
	Node* prev;								// Previous Node
}Node;

class Queue
{

private:

	Node* floorQueue;						// Head pointer to queue
	Node* last;								// Pointer to the last node in queue
	Node* iterator;							// Iterating pointer
	Node* tmp;								// Temporary pointer for some actions

public:

	Node* create(int floor);								// Creates a Node
	
	void insert(int floor, int direction, int curr);		// adds a Node to the Queue
	void pop();
	int peek();												// look at the top of the queue
	void goingDown(int, int);								// sort the list if elevetor is going down
	void goingUp(int, int);				     				// sort the list if elevetor is going up
	void reset();											// clears the queue

	 Queue();												// Constructor
	~Queue();												//Destructor
};

#endif
