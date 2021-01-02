/*
		Authors:	Janahan Dhushenthen (56777220)
					Brandon Bwanakocha (35366525)
		   Date:	November 13, 2020
	Description:	Elevator Monitor Class
*/

#include "Elevator.h"

// Constructor, which accepts a name as a parameter
Elevator::Elevator(string name)
{
	// Initialize semaphores with unique names
	ps1 = new CSemaphore("__PS1__" + name, 0, 1);
	cs1 = new CSemaphore("__CS1__" + name, 1, 1);
	ps2 = new CSemaphore("__PS2__" + name, 0, 1);
	cs2 = new CSemaphore("__CS2__" + name, 1, 1);

	// Initialize datapool and datapool pointer with unique name
	dp = new CDataPool("__DataPool__" + name, sizeof(struct ElevatorStatus));
	dpPointer = (struct ElevatorStatus*)dp->LinkDataPool();
}

// Destructor, which deletes the semaphores and datapool
Elevator::~Elevator()
{
	delete ps1;
	delete cs1;
	delete ps2;
	delete cs2;
	delete dp;
}

// Interface function for consumer processes (IO and Dispatcher), which gets the elevator status
void Elevator::Get_Elevator_Status(int process, ElevatorStatus* status) {
	
	// Dispatcher Process: Gets status while performing synchronization 
	if (process) {
		ps2->Wait(); // Elevator wait

		status->destFloor = dpPointer->destFloor;
		status->direction = dpPointer->direction;
		status->door_status = dpPointer->door_status;
		status->fault_status = dpPointer->fault_status;
		status->num_passengers = dpPointer->num_passengers;
		status->currFloor = dpPointer->currFloor;
		status->currRow = dpPointer->currRow;
		
		cs2->Signal(); // IO or Dispatcher Signal
	}

	// IO Process: Gets status while performing synchronization
	else {
		ps1->Wait();
		
		status->destFloor = dpPointer->destFloor;
		status->direction = dpPointer->direction;
		status->door_status = dpPointer->door_status;
		status->fault_status = dpPointer->fault_status;
		status->num_passengers = dpPointer->num_passengers;
		status->currFloor = dpPointer->currFloor;
		status->currRow = dpPointer->currRow;
		
		cs1->Signal();
	}
}

// Interface function for producer processes (Elevator1 and Elevator2), which updates the elevator status
void Elevator::Update_Status(UINT Message, int currFloor, int currRow, int door_status,int direction,int num_pass) {
	
	cs1->Wait();
	cs2->Wait();

	// Decode message to get each data field in ElevatorStatus
	int destFloor, fault_status, num_passengers;
	destFloor = Message % 10;
	Message = Message / 10;
	Message = Message / 10;
	Message = Message / 10;
	fault_status = Message % 10;
	Message = Message / 10;
	num_passengers = num_pass;
		
	dpPointer->destFloor = destFloor;
	dpPointer->direction = direction;
	dpPointer->door_status = door_status;
	dpPointer->fault_status = fault_status;
	dpPointer->num_passengers = num_passengers;
	dpPointer->currFloor = currFloor;
	dpPointer->currRow = currRow;

	ps1->Signal();
	ps2->Signal();
}

