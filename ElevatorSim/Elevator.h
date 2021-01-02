/*
		Authors:	Janahan Dhushenthen (56777220)
					Brandon Bwanakocha (35366525)
		   Date:	November 13, 2020
	Description:	Elevator Monitor Class
*/

#ifndef   __Elevator__
#define   __Elevator__

#include "rt.h"

// Possible directions of the elevators
#define UP		0
#define DOWN	1
#define STAT    2	// Stationary

// Possible statuses of the elevator doors
#define CLOSED	0
#define OPEN	1

// Possible fault statuses of the elevators
#define IN_SERVICE		0
#define	OUT_OF_SERVICE	1

// IDs for consumer processes
#define IO_PROCESS			0
#define DISPATCHER_PROCESS	1

// Delays for Elevetors
#define EDELAY 200
#define ENDSIM 9

#define FLOOR(X) (10-X)*4-1	// Macro that gets row number from floor number
#define KEYBOARD_IN		10	// Signal representing keyboard input

// Structure encapsulating all data that describes the elevator status
struct ElevatorStatus {
	int currFloor;		// Current Floor We are at
	int direction;		// Destination Floor
	int door_status;	// Door open or closed
	int fault_status;	// In or out of service
	int num_passengers;	// Number of Passangers in elevator
	int destFloor;		// Destination floor for elevator
	int currRow;		// Current row at which ELEVATOR is 
};

class Elevator
{

private:

	// Semaphores for single producer/double consumer
	CSemaphore* ps1;
	CSemaphore* cs1;
	CSemaphore* ps2;
	CSemaphore* cs2;

	// Datapool and datapool pointer used to store the elevator status
	CDataPool* dp;
	struct ElevatorStatus* dpPointer;

public:

	Elevator(string name);	// Constructor
	~Elevator();			// Destructor

	void Get_Elevator_Status(int process, ElevatorStatus* status);												// Interface function to get the elevator status
	void Update_Status(UINT Message, int currFloor, int currRow, int doorStatus, int direction, int num_pass);	// Interface function to update the elevator status
};

#endif