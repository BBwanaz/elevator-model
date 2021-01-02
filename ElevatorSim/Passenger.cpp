/*
		Authors:	Janahan Dhushenthen (56777220)
					Brandon Bwanakocha (35366525)
		   Date:	November 13, 2020
	Description:	Simulated Passenger Class
*/
#define _CRT_SECURE_NO_WARNINGS

#include "Passenger.h"
#include <time.h>
#include "Elevator.h"
#include "IO\Screen.h"

// Constructor
Passenger::Passenger(int id) : id(id) {

	Screen s("Screen");
	char buffer[100];
	data = new Pdata;

	data->dest = rand() % 10;													// Assign Random Destination
	data->origin = rand() % 10;												    // Assign Random Origin Floor

	srand(time(NULL));
	
	while (data->origin == data->dest)											// Make sure origin and destination are different
		data->origin = rand() % 10;

	data->dir = data->dest > data->origin ? UP : DOWN;							// Assign Direction based on origin and destination floor
	data->id  = id;

	PMutex =  new CMutex("PMutex");												// Define Mutex to protect Pipe2
	Pipe2  =  new CTypedPipe <Pdata>("PPipe", 200);								// Pipeline to communicate with dispatcher
    
	sprintf(buffer, "**P(%d)%s", data->dest, data->dir == UP ? "up" : "dwn");	
	s.WriteToScreen(21, FLOOR(data->origin)+2, buffer);

	sprintf(buffer, "WBPipe%i", id);
	WBPipe = new CTypedPipe <int>(buffer, 200);									// Dispatcher informs us which elevator is coming 
}

// Destructor
Passenger::~Passenger() {
	delete data;
	delete Pipe2;
	delete WBPipe;
}

// Sends comands to I/O 
void Passenger::sendCmd(int cmd) {

	PMutex->Wait();												
																
	data->cmd = cmd;							// What type of command we are sending (UP_OR_DOWN_COMMAND = 0)									
	Pipe2->Write(data);							// Send data to I/O		
	
	PMutex->Signal();
}

int Passenger::main() {

	int elev;						
	int mov;													// Integer to shift passanger
	char buffer[20];

	Screen s("Screen");
	
	sendCmd(UP_OR_DOWN_COMMAND);								// Send  Up or Down Command

	WBPipe->Read(&elev);										// Dispatcher tells us which elevator is coming for our request

	mov = (elev == 1) ? -8 : 8;									// Move passanger icon towards elevator that comes to pick up
	sprintf(buffer, "           ");
	s.WriteToScreen(19, FLOOR(data->origin) + 2, buffer);		// Write passanger Icon
	sprintf(buffer, "P(%i) to F%i", id, data->dest);
	s.WriteToScreen(19 + mov, FLOOR(data->origin) + 2, buffer);

	sprintf(buffer, "EDoor%i%i", elev,data->origin);
	CCondition* Door = new CCondition(buffer);					// Create a named Condition that corresponds to the opening of a door at specific floor data->origin

	Door->Wait();												// wait for our particular door to be open
	delete Door;												// Delete the dynamically allocated pointer

	sprintf(buffer, "EMutex%i", elev);
	CSemaphore* eL = new CSemaphore(buffer, 4, 4);				// Count how many passangers in elevator as pasenger enters

	eL->Wait();

	sprintf(buffer, "            ");
	s.WriteToScreen(19 + mov, FLOOR(data->origin) + 2, buffer); // Clear 

	sendCmd(elev);												// Send Destination command to elev 

	sprintf(buffer, "EDoor%i%i", elev, data->dest);
	CCondition* DoorOut = new CCondition(buffer);				// Wait for door out to open so we exit

	DoorOut->Wait();
	delete DoorOut;												// Delete dynamically allocated pointer

	eL->Signal();												// Signal the counting Semaphore that we are out of the elevator and terminate object

	sprintf(buffer, "P(%i) Exit  ", id);
	s.WriteToScreen(19 + mov, FLOOR(data->dest) + 2, buffer);
	Sleep(1500);

	sprintf(buffer, "               ");
	s.WriteToScreen(19 + mov, FLOOR(data->dest) + 2, buffer);	// Clear 

	delete eL;

	return 0;
}