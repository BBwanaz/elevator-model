/*
		Authors:	Janahan Dhushenthen (56777220)
					Brandon Bwanakocha (35366525)
		   Date:	November 13, 2020
	Description:	Dispatcher Process: reads input from IO and sends commands to each elevator
*/

#define _CRT_SECURE_NO_WARNINGS

#include "..\rt.h"
#include "..\Elevator.h"
#include <time.h>

// Structure to store characters typed in as commands
typedef struct Command {
	char c1, c2;
	int c3;
}
Command;

Command command;					// Global variable to store the commands read in from IO
ElevatorStatus* e1Status;			// Global variable to store the elevator status of elevator 1
ElevatorStatus* e2Status;			// Global variable to store the elevator status of elevator 1
CRendezvous simStart("Start", 11);	// Global rendezvous to synchronize the start of the simulation
CRendezvous simEnd("End", 3);		// Global rendezvous to synchronize the end of the simulation
bool simulating;					// Global flag to indicate if the simulation is still going

// Function to choose elevator depending on request origin
int chooseElevator(char dir, char cfloor) {

	int e1 = 1;
	int e2 = 2;
	char down = 'd';
	char up = 'u';
	int e1avail = 0;
	int e2avail = 0;
	int floor = cfloor - '0';
	srand(time(NULL));

	// Handle up requests that are above either elevator, which are already going up
	e1avail = (dir == up && e1Status->direction == UP && floor >= e1Status->currFloor);
	e2avail = (dir == up && e2Status->direction == UP && floor >= e2Status->currFloor);
	
	// If both are going up, choose the closest one
	if (e1avail && e2avail) {
		if (abs(e1Status->currFloor - floor) < abs(e2Status->currFloor - floor))
			return e1;

		if (abs(e1Status->currFloor - floor) > abs(e2Status->currFloor - floor))
			return e2;
	}

	// Elevator 1 is available for the call
	if (e1avail)
		return e1;
		
	// Elevator 2 is available for the call
	if (e2avail)
		return e2;

	// Handle down requests that are below either elevator, which are already going down
	e1avail = (dir == down && e1Status->direction == DOWN && floor <= e1Status->currFloor);
	e2avail = (dir == down && e2Status->direction == DOWN && floor <= e2Status->currFloor);

	// If both are going down, choose the closest one
	if (e1avail && e2avail) {
		if (abs(e1Status->currFloor - floor) < abs(e2Status->currFloor - floor))
			return e1;

		if (abs(e1Status->currFloor - floor) > abs(e2Status->currFloor - floor))
			return e2;
	}
		
	// Elevator 1 is available for the call
	if (e1avail)
		return e1;
		
	// Elevator 2 is available for the call
	if (e2avail)
		return e2;

	// If either of the elevators are stationary the choose one
	e1avail = e1Status->direction == STAT;
	e2avail = e2Status->direction == STAT;
		
	// If both are stationary, choose the closest one
	if (e1avail && e2avail) {
		if (abs(e1Status->currFloor - floor) < abs(e2Status->currFloor - floor))
			return e1;

		if (abs(e1Status->currFloor - floor) > abs(e2Status->currFloor - floor))
			return e2;
	}

	// Elevator 1 is available for the call
	if (e1avail)
		return e1;

	// Elevator 2 is available for the call
	if (e2avail)
		return e2;

	// If elevator 1 is going up and there is a down request, which is below
	if (e1Status->direction == UP && dir == down && floor < e1Status->currFloor)
		return e1;

	// If elevator 2 is going up and there is a down request, which is below
	if (e2Status->direction == UP && dir == down && floor < e2Status->currFloor)
		return e2;

	// If elevator 1 is going down and there is an up request, which is above
	if (e1Status->direction == DOWN && dir == up && floor > e1Status->currFloor)
		return e1;

	// If elevator 2 is going down and there is an up request, which is above
	if (e2Status->direction == DOWN && dir == up && floor > e2Status->currFloor)
		return e2;

	// Default: choose random elevator
	return rand() % 2 + 1;
}

// Thread that reads in commands from the IO process
UINT __stdcall Readio(void* args)
{
	simStart.Wait();	// Wait for all threads/processes to synchronize before starting simulation

	// Get each child process from the arguments
	CProcess e1Process = *(CProcess*)(args);
	CProcess e2Process = *((CProcess*)(args)+1);
	CProcess ioProcess = *((CProcess*)(args)+2);

	int elev1 = 1;
	int elev2 = 2;
	char buffer[15];
	UINT Message;

	CTypedPipe <Command> Pipe1("Pipe1", 200);	// Pipeline to communicate with dispatcher
	CTypedPipe <int>* WBPipe;					// Additional pipeline to let each passanger know which elevator is coming

	while (simulating) {
				
		Pipe1.Read(&command);						// Read commands from dispatcher

		sprintf(buffer, "WBPipe%i", command.c3);
		WBPipe = new CTypedPipe <int>(buffer, 200); // Write back pipeline

		// Handle add fault command
		if (command.c1 == '-') {
			if (command.c2 == '1') {
				Message = 1000 * OUT_OF_SERVICE + 1 * e1Status->currFloor;
				e1Process.Post(Message);
			}
			else if (command.c2 == '2') {
				Message = 1000 * OUT_OF_SERVICE + 1 * e2Status->currFloor;
				e2Process.Post(Message);
			}
		}

		// Handle remove fault command
		else if (command.c1 == '+') {
			if (command.c2 == '1') {
				Message = 1000 * IN_SERVICE + 1 * e1Status->currFloor;
				e1Process.Post(Message);
			}
			else if (command.c2 == '2') {
				Message = 1000 * IN_SERVICE + 1 * e2Status->currFloor;
				e2Process.Post(Message);
			}
		}

		// Handle terminate command
		else if (command.c1 == 'e' && command.c2 == 'e') {
			Message = 1000 * ENDSIM;
			e1Process.Post(Message);
			e2Process.Post(Message);

			simEnd.Wait();	// Wait for elevators to go to floor 0		
			
			// Stop the remaining processes/threads
			ioProcess.Post(Message);
			simulating = false;
		}

		// Send commands to elevator 2 if elevator 1 is out of service
		else if (e1Status->fault_status == OUT_OF_SERVICE) {
			if (command.c1 == 'u' || command.c1 == 'd' || command.c1 == '2') {
				if (command.c2 >= '0' && command.c2 <= '9') {
					Message = 10000 * 1 + 1000 * e2Status->fault_status + 100 * e2Status->door_status + 10 * e2Status->direction + 1 * (command.c2 - '0');
					e2Process.Post(Message);

					if (command.c3 != KEYBOARD_IN)
						WBPipe->Write(&elev2);
				}
			}
		}

		// Send commands to elevator 1 if elevator 2 is out of service
		else if (e2Status->fault_status == OUT_OF_SERVICE) {
			if (command.c1 == 'u' || command.c1 == 'd' || command.c1 == '1') {
				if (command.c2 >= '0' && command.c2 <= '9') {
					Message = 10000 * 1 + 1000 * e1Status->fault_status + 100 * e1Status->door_status + 10 * e1Status->direction + 1 * (command.c2 - '0');
					e1Process.Post(Message);

					if (command.c3 != KEYBOARD_IN)
						WBPipe->Write(&elev1);
				}
			}
		}

		// Handle commands from inside elevator 1
		else if (command.c1 == '1') {
			if (command.c2 >= '0' && command.c2 <= '9') {
				Message = 10000 * 1 + 1000 * e1Status->fault_status + 100 * e1Status->door_status + 10 * e1Status->direction + 1 * (command.c2 - '0');
				e1Process.Post(Message);
			}
		}
		
		// Handle commands from inside elevator 2
		else if (command.c1 == '2') {
			if (command.c2 >= '0' && command.c2 <= '9') {
				Message = 10000 * 1 + 1000 * e2Status->fault_status + 100 * e2Status->door_status + 10 * e2Status->direction + 1 * (command.c2 - '0');
				e2Process.Post(Message);
			}
		}
			
		// Handle up/down commands from outside elevators
		else if (command.c1 == 'u' || command.c1 == 'd') {
			if (command.c2 >= '0' && command.c2 <= '9') {
				if (chooseElevator(command.c1, command.c2) == 1) {
					Message = 10000 * 1 + 1000 * e1Status->fault_status + 100 * e1Status->door_status + 10 * e1Status->direction + 1 * (command.c2 - '0');
					e1Process.Post(Message);

					if (command.c3 != KEYBOARD_IN)
						WBPipe->Write(&elev1);
				}
				else {
					Message = 10000 * 1 + 1000 * e2Status->fault_status + 100 * e2Status->door_status + 10 * e2Status->direction + 1 * (command.c2 - '0');
					e2Process.Post(Message);

					if (command.c3 != KEYBOARD_IN)
						WBPipe->Write(&elev2);
				}
			}		
		}

		delete WBPipe;
	}

	return 0;
}

// Thread that gets the elevator status of elevator 1
UINT __stdcall Reade1(void* args) 
{
	simStart.Wait();	// Wait for all threads/processes to synchronize before starting simulation

	CProcess e1Process = *(CProcess*)(args);
	Elevator e1("Elevator1");

	while (simulating) {
		e1.Get_Elevator_Status(DISPATCHER_PROCESS, e1Status);
	}

	return 0;
}

// Thread that gets the elevator status of elevator 2
UINT __stdcall Reade2(void* args) 
{
	simStart.Wait();	// Wait for all threads/processes to synchronize before starting simulation

	CProcess e2Process = *(CProcess*)(args);
	Elevator e2("Elevator2");

	while (simulating) {
		e2.Get_Elevator_Status(DISPATCHER_PROCESS, e2Status);
	}

	return 0;
}

int main()
{
	simulating = true;

	// Allocate memory for the elevator status pointers
	e1Status = (ElevatorStatus*)malloc(sizeof(ElevatorStatus));
	e2Status = (ElevatorStatus*)malloc(sizeof(ElevatorStatus));

	// Declare child processes: Elevator1, Elevator2, IO
	CProcess e1Process("C:\\Users\\USER\\Desktop\\Academic Resources\\Year 4\\CPEN 333\\assignment-1\\Assignment1\\x64\\Debug\\Elevator1.exe",
		NORMAL_PRIORITY_CLASS,																								
		PARENT_WINDOW,
		ACTIVE																												
	);
	CProcess e2Process("C:\\Users\\USER\\Desktop\\Academic Resources\\Year 4\\CPEN 333\\assignment-1\\Assignment1\\x64\\Debug\\Elevator2.exe",
		NORMAL_PRIORITY_CLASS,																								
		PARENT_WINDOW,
		ACTIVE																												
	);
	CProcess ioProcess("C:\\Users\\USER\\Desktop\\Academic Resources\\Year 4\\CPEN 333\\assignment-1\\Assignment1\\x64\\Debug\\IO.exe",
		NORMAL_PRIORITY_CLASS,																								
		PARENT_WINDOW,
		ACTIVE
	);

	CProcess processes[3] = { e1Process, e2Process, ioProcess };	// Declare pointer to the child processes, which are passed into each thread
	
	CThread readThread(Readio, ACTIVE, processes);					// Declare thread to read in commands from the IO process
	CThread e1Thread(Reade1, ACTIVE, &processes[0]);				// Declare thread to get the elevator status of elevator 1
	CThread e2Thread(Reade2, ACTIVE, &processes[1]);				// Declare thread to get the elevator status of elevator 2

	// Wait for child processes to finish
	e1Process.WaitForProcess();
	e2Process.WaitForProcess();
	ioProcess.WaitForProcess();

	// Wait for threads to finish
	readThread.WaitForThread();										
	e1Thread.WaitForThread();										
	e2Thread.WaitForThread();										

	return 0;
}