/*
		Authors:	Janahan Dhushenthen (56777220)
					Brandon Bwanakocha (35366525)
		   Date:	November 13, 2020
	Description:	Simulated Passenger Class
*/

#ifndef   __Pasenger__
#define   __Pasenger__

#include "rt.h"

#define UP_OR_DOWN_COMMAND 0

// Structure that contains data of each passanger
typedef struct Pdata {
	int origin;					// Origin Floor
	int dest;					// Destination Floor
	int dir;					// Direction
	int id;						// id of passanger
	int cmd;					// either outside or inside command
}Pdata;

class Passenger:public ActiveClass
{
private:

	CMutex* PMutex;				// Protects Pipeline 2
	Pdata* data;                // Head pointer to Passenger  data
	
	int main();					// Main function
	int id;						// Simulated Passanger ID
	
	CTypedPipe <Pdata>* Pipe2;	// Pipeline to communicate with I/O
	CTypedPipe <int>* WBPipe;

public:

	Passenger(int id);			// Constructor
	~Passenger();				// Destructor
	
	void sendCmd(int);			// Function that sends command to I/O
};

#endif

