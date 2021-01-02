/*
		Authors:	Janahan Dhushenthen (56777220)
					Brandon Bwanakocha (35366525)
		   Date:	November 13, 2020
	Description:	IO Process: reads in user input and prints simulation to screen
*/

#define _CRT_SECURE_NO_WARNINGS

#include "..\rt.h"
#include "..\Elevator.h"
#include "Screen.h"
#include "..\Passenger.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Structure to store characters typed in as commands
typedef struct Command {
	char c1, c2;		// command characters
	int c3;				// passenger ID (KEYBOARD_IN = 10)
}
Command;

CRendezvous simStart("Start", 11);				// Global rendezvous to synchronize the start of the simulation
CTypedPipe <Command> Pipe1("Pipe1", 200);		// Global pipeline to communicate with the dispatcher
CMutex* PipeMutex = new CMutex("PipeMutex");	// Mutex to manage writing to the pipeline
bool simPassFlag;								// Flag to start or stop randomly simulating passengers
bool simulating;								// Global flag to indicate if the simulation is still going

// Thread to output the current status of Elevator2
UINT __stdcall printElevator1(void* args)
{	
	simStart.Wait();	// Wait for all threads/processes to synchronize before starting simulation

	Screen s("Screen");
	char buffer[100];

	Elevator e1("Elevator1");
	ElevatorStatus* status = (ElevatorStatus*)malloc(sizeof(ElevatorStatus));

	char directionChar;
	char doorChar;
	int row;
	int column;

	while (simulating) {

		e1.Get_Elevator_Status(IO_PROCESS, status);	// Consume updated elevator status

		// Elevator border depends on its fault status and direction
		if (status->fault_status == OUT_OF_SERVICE) {
			directionChar = 'X';
		}
		else {
			if (status->direction == UP) {
				directionChar = 'U';
			}
			else if(status->direction == DOWN){
				directionChar = 'D';
			}
			else { 
				directionChar = '-'; 
			}
		}

		// Elevator wall depends on door status
		if (status->door_status == OPEN) {
			doorChar = ' ';
		}
		else {
			doorChar = '|';
		}

		// Clear current output in elevator shaft
		sprintf(buffer, "     ||");
		int i;
		for (i = 3; i < 42; i++) {
			if ((i - 2) % 4 != 0) {
				s.WriteToScreen(2, i, buffer);
			}
		}

		// Draw elevator with updated status
		sprintf(buffer, "%c %c %c%c%c", directionChar, directionChar, directionChar, doorChar, doorChar);
		row = status->currRow;
		column = 2;
		s.WriteToScreen(column, row, buffer);

		sprintf(buffer, "=======");
		s.WriteToScreen(column, FLOOR(status->currFloor) - 1, buffer);

		sprintf(buffer, "%c %d %c%c%c", directionChar, status->num_passengers, directionChar, doorChar, doorChar);
		row = status->currRow + 1;
		column = 2;
		s.WriteToScreen(column, row, buffer);

		sprintf(buffer, "%c %c %c%c%c", directionChar, directionChar, directionChar, doorChar, doorChar);
		row = status->currRow + 2;
		column = 2;
		s.WriteToScreen(column, row, buffer);

		sprintf(buffer, "=======");
		s.WriteToScreen(column, FLOOR(status->currFloor) + 3, buffer);

		// Write statuses in table
		sprintf(buffer, "Elevator 1 |   %d   |%s       |%s |%s| ", status->currFloor, status->direction == STAT ? "   -" : status->direction ? "DOWN" : "UP  ", status->door_status ? "OPEN  " : "CLOSED", !status->fault_status ? "In Service      " : "Out of Service  ");
		s.WriteToScreen(50, 4, buffer);
	}

	return 0;
}

// Thread to output the current status of Elevator2
UINT __stdcall printElevator2(void* args)
{
	simStart.Wait();	// Wait for all threads/processes to synchronize before starting simulation

	Screen s("Screen");
	char buffer[100];

	Elevator e2("Elevator2");
	ElevatorStatus* status = (ElevatorStatus*)malloc(sizeof(ElevatorStatus));
	
	char directionChar;
	char doorChar;
	int row;
	int column;

	while (simulating) {

		e2.Get_Elevator_Status(IO_PROCESS, status);	// Consume updated elevator status

		// Elevator border depends on its fault status and direction
		if (status->fault_status == OUT_OF_SERVICE) {
			directionChar = 'X';
		}
		else {
			if (status->direction == UP) {
				directionChar = 'U';
			}
			else if(status->direction == DOWN) {
				directionChar = 'D';
			}
			else { 
				directionChar = '-'; 
			}
		}

		// Elevator wall depends on door status
		if (status->door_status == OPEN) {
			doorChar = ' ';
		}
		else {
			doorChar = '|';
		}

		// Clear current output in elevator shaft
		sprintf(buffer, "||     ");
		int i;
		for (i = 3; i < 42; i++) {
			if ((i - 2) % 4 != 0) {
				s.WriteToScreen(38, i, buffer);
			}
		}

		// Draw elevator with updated status
		sprintf(buffer, "%c%c%c %c %c", doorChar, doorChar, directionChar, directionChar, directionChar);
		row = status->currRow;
		column = 38;
		s.WriteToScreen(column, row, buffer);
		
		sprintf(buffer, "=======");
		s.WriteToScreen(column, FLOOR(status->currFloor) -1 , buffer);

		sprintf(buffer, "%c%c%c %d %c", doorChar, doorChar, directionChar, status->num_passengers, directionChar);
		row = status->currRow+1;
		column = 38;
		s.WriteToScreen(column, row, buffer);

		sprintf(buffer, "%c%c%c %c %c", doorChar, doorChar, directionChar, directionChar, directionChar);
		row = status->currRow + 2;
		column = 38;
		s.WriteToScreen(column, row, buffer);
		
		sprintf(buffer, "=======");
		s.WriteToScreen(column, FLOOR(status->currFloor) + 3, buffer);
		
		// Write statuses in table
		sprintf(buffer, "Elevator 2 |   %d   |%s       |%s |%s| ", status->currFloor, status->direction == STAT ? "   -" : status->direction ? "DOWN" : "UP  ", status->door_status ? "OPEN  " : "CLOSED", !status->fault_status ? "In Service      " : "Out of Service  ");
		s.WriteToScreen(50, 6, buffer);
	
	}

	return 0;
}

// Thread that reads inputted commands
UINT __stdcall getCommands(void* args)	
{	
	simStart.Wait();	// Wait for all threads/processes to synchronize before starting simulation

	Screen s("Screen");
	char buffer[100];

	// Initialize command data
	Command* command = new Command;
	command->c1 = 'X';
	command->c2 = 'X';

	while (true) {
		
		// Get inputted characters
		command->c1 = _getch();
		sprintf(buffer, "Command: %c", command->c1);
		s.WriteToScreen(1, 45, buffer);
		sprintf(buffer, " ");
		s.WriteToScreen(11, 45, buffer);
		
		command->c2 = _getch();
		sprintf(buffer, "%c", command->c2);
		s.WriteToScreen(11, 45, buffer);
		
		command->c3 = KEYBOARD_IN;	// Signal that command was written by a human

		// Start simulating passengers
		if (command->c1 == 'd' && command->c2 == '+') {
			simPassFlag = true;
		}

		// Stop simulating passengers
		else if (command->c1 == 'd' && command->c2 == '-') {
			simPassFlag = false;
		}

		// Send commands to the dispatcher
		else {
			PipeMutex->Wait();
			Pipe1.Write(command);
			PipeMutex->Signal();

			if (command->c1 == 'e' && command->c2 == 'e') {
				break;
			}
		}	
	}

	return 0;
}

// Thread that gets commands from active objects through the pipeline "PPipe"
UINT __stdcall getObjCmd(void* args) {

	simStart.Wait();	// Wait for all threads/processes to synchronize before starting simulation

	CTypedPipe <Pdata> Pipe2("PPipe", 200);		// Pipeline that communicates with passenger objects
	Pdata* data = new Pdata;					// Passenger data
	Command* command = new Command;
	
	while (simulating) {
				
		Pipe2.Read(data);						// Read data from passenger

		if (data->cmd == UP_OR_DOWN_COMMAND) {
			if (data->dir == UP) {
				command->c1 = 'u';				// Floor direction: up
			}
			else {
				command->c1 = 'd';				// Floor direction: down
			}
			command->c2 = '0' + data->origin;	// Floor where the passenger is at
		}

		else {
			command->c1 = '0' + data->cmd;		// Get elevator number
			command->c2 = '0' + data->dest;		// Get destination of elevator
		}
				
		command->c3 = data->id;					// ID of passenger so that the dispatcher knows which passenger to communicate with
		
		PipeMutex->Wait();
		Pipe1.Write(command);					// Write command to dispatcher
		PipeMutex->Signal();
	}

	return 0;
}

UINT __stdcall simulatePassengers(void* args) {
	
	simStart.Wait();	// Wait for all threads/processes to synchronize before starting simulation

	// Dynamically create 10 passengers each time
	Passenger* passengers[10];
	int i = 0;

	while (simulating) {

		// If "d+", then start creating passenger objects dynamically
		if (simPassFlag) {
			if (i < 10) {
				passengers[i] = new Passenger(i);
				passengers[i]->Resume();
				i++;
			}

			// 10 passengers have been created
			else {
				for (i = 0; i < 10; i++) {
					passengers[i]->WaitForThread();
					delete passengers[i];
				}
				i = 0;
			}
		}

		Sleep(rand() % 9000 + 1000);	// Wait for 1-10 seconds between each creation
	}

	return 0;
}

int main(int argc, char* argv[])
{
	simulating = true;
	CMailbox ioMailbox;	// Mailbox to receive commands from the dispatcher
	Screen s("Screen");
	char buffer[100];

	// Print duplex with 2 elevator shafts
	sprintf(buffer, "        ===============================        \n");
	s.WriteToScreen(0, 0, buffer);
	sprintf(buffer, " Elevator 1 ======================= Elevator 2 \n");
	s.WriteToScreen(0, 1, buffer);
	sprintf(buffer, "||===========================================||\n");
	s.WriteToScreen(0, 2, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 3, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 4, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 5, buffer);
	sprintf(buffer, "||====================(9)====================||\n");
	s.WriteToScreen(0, 6, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 7, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 8, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 9, buffer);
	sprintf(buffer, "||====================(8)====================||\n");
	s.WriteToScreen(0, 10, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 11, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 12, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 13, buffer);
	sprintf(buffer, "||====================(7)====================||\n");
	s.WriteToScreen(0, 14, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 15, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 16, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 17, buffer);
	sprintf(buffer, "||====================(6)====================||\n");
	s.WriteToScreen(0, 18, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 19, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 20, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 21, buffer);
	sprintf(buffer, "||====================(5)====================||\n");
	s.WriteToScreen(0, 22, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 23, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 24, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 25, buffer);
	sprintf(buffer, "||====================(4)====================||\n");
	s.WriteToScreen(0, 26, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 27, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 28, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 29, buffer);
	sprintf(buffer, "||====================(3)====================||\n");
	s.WriteToScreen(0, 30, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 31, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 32, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 33, buffer);
	sprintf(buffer, "||====================(2)====================||\n");
	s.WriteToScreen(0, 34, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 35, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 36, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 37, buffer);
	sprintf(buffer, "||====================(1)====================||\n");
	s.WriteToScreen(0, 38, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 39, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 40, buffer);
	sprintf(buffer, "||     ||                             ||     ||\n");
	s.WriteToScreen(0, 41, buffer);
	sprintf(buffer, "||====================(0)====================||\n");
	s.WriteToScreen(0, 42, buffer);

	// Print status table
	sprintf(buffer, "           | Floor | Direction | Door  | General Status | ");
	s.WriteToScreen(50, 2, buffer);
	sprintf(buffer, "           |-------+-----------+-------+----------------| ");
	s.WriteToScreen(50, 3, buffer);
	sprintf(buffer, "           |-------+-----------+-------+----------------| ");
	s.WriteToScreen(50, 5, buffer);
	sprintf(buffer, "           |--------------------------------------------| ");
	s.WriteToScreen(50, 7, buffer);

	// Declare threads for input and output of simulation
	CThread t1(printElevator1, ACTIVE);
	CThread t2(printElevator2, ACTIVE);
	CThread t3(getCommands, ACTIVE);
	CThread t4(getObjCmd, ACTIVE);
	CThread t5(simulatePassengers, ACTIVE);
	srand(time(NULL));
	
	simStart.Wait();	// Wait for all threads/processes to synchronize before starting simulation
	
	while (simulating) {

		// Terminate simulation if a message has been received from the dispatcher
		if (ioMailbox.TestForMessage()) {
			simulating = false;
		}
	}

	// Wait for threads to finish
	t1.WaitForThread();
	t2.WaitForThread();
	t3.WaitForThread();
	t4.WaitForThread();
	t5.WaitForThread();

	return 0;
}