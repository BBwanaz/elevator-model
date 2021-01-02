/*
		Authors:	Janahan Dhushenthen (56777220)
					Brandon Bwanakocha (35366525)
		   Date:	November 13, 2020
	Description:	Queue Class: Queues Elevator requests
*/

#include "Queue.h"
#include "Elevator.h"

// Constructor
Queue::Queue() {

	// Nullify all pointers
	floorQueue = nullptr;
	last = nullptr;
	iterator = nullptr;
	tmp = nullptr;
}

// Destructor
Queue::~Queue() {

	// Delete all pointers
	delete floorQueue;
	delete last;
	delete iterator;
	delete tmp;
}

// Create a single floor Node
Node* Queue::create(int floor) {
	Node* ptr = new Node;				

	ptr->floor = floor;
	ptr->next = nullptr;
	ptr->prev = nullptr;

	return ptr;
}

// Enqueue floors in decreasing order if the elevator is going down so that we won't miss a stop on the way down
void Queue::goingDown(int floor, int direction) {
	iterator = floorQueue;

	if (iterator == nullptr) {											// If list is somehow empty then create new list
		floorQueue = create(floor);
		return;
	}
		
	while (iterator->next != nullptr && floor < iterator->floor) {		// If floor to be inserted is less than floor at current node, then go to the next node down the queue
		iterator = iterator->next;
	}

	if (iterator->floor > floor) {										// if the floor is lower than the last floor then append floor at the end
		iterator->next = create(floor);
		iterator->next->prev = iterator;
		last = iterator->next;
		return;
	}

	tmp = create(floor);

	// Insert new floor in front of current Node floor if the Node floor is less than neew floor
	if (iterator != floorQueue) {
		tmp->prev = iterator->prev;      // Only update previous if we are not at the head pointer
		iterator->prev->next = tmp;
		iterator->prev = tmp;
		tmp->next = iterator;

		return;
	}

	floorQueue = tmp;
	floorQueue->next = iterator;
	return;
}

// Enqueue Floor in increasing order given elevator is going up
void Queue::goingUp(int floor, int direction) {
	iterator = floorQueue;

	if (iterator == nullptr) {											// If list is somehow empty then create new list
		floorQueue = create(floor);
		return;
	}

	while (iterator->next != nullptr && floor > iterator->floor) {		// Check to see if floor to be inserted is greater than current Node floor
		iterator = iterator->next;
	}

	if (iterator->floor < floor) {                          // if the floor is lhigher than the last floor then append floor at the end
		iterator->next = create(floor);
		iterator->next->prev = iterator;
		last = iterator->next;
		return;
	}

	tmp = create(floor);

	// Insert new floor in front of current Node floor if the Node floor is less than neew floor
	if (iterator != floorQueue) {
		tmp->prev = iterator->prev;      // Only update previous if we are not at the head pointer
		iterator->prev->next = tmp;
		iterator->prev = tmp;
		tmp->next = iterator;

		return;
	}

	floorQueue = tmp;
	floorQueue->next = iterator;
	return;
}

// Inserts a new destination floor whenever elevator receives a Message
void Queue::insert(int floor, int direction, int curr) {
	// If list is empty, then just create node and put it to head
	if (floorQueue == nullptr) {
		floorQueue = create(floor);
		last = floorQueue;
		return;
	}

	if ((direction == DOWN && floor < curr) || (direction == UP && floor < curr)) {   // If new destination floor is in the opposite direction as the Node floor, insert as tho going down so that elevator goes to the end and then comes back
		goingDown(floor, direction);
	}

	else {
		goingUp(floor, direction);
	}

	return;
}

// Removes the floor at the top of the queue after the doors on that floor open
void Queue::pop() {

	if (floorQueue == nullptr) {
		return;
	}

	tmp = floorQueue->next;
	delete floorQueue;

	floorQueue = tmp;
	return;
}

// Gets the value of the next destination floor at the top of the queue without deleting it
int Queue::peek() {
	if (floorQueue == nullptr)
		return EMPTYQUEUE;
	return floorQueue->floor;
}

// Removes everything from the queue
void Queue::reset() {

	if (floorQueue == nullptr)
		return;
	iterator = floorQueue->next;

	while (iterator != nullptr) {
		tmp = iterator;
		iterator = iterator->next;
		delete tmp;
	}

	delete floorQueue;
	floorQueue = nullptr;
}