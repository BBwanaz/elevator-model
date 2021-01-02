/*
		Authors:	Janahan Dhushenthen (56777220)
					Brandon Bwanakocha (35366525)
		   Date:	November 13, 2020
	Description:	Elevator 2 Process:  updates statuses of elevator 2 object
*/
#define _CRT_SECURE_NO_WARNINGS

#include "..\rt.h"
#include "..\Elevator.h"
#include "..\Queue.h"

CRendezvous simStart("Start", 11);	// Global rendezvous to synchronize the start of the simulation
CRendezvous simEnd("End", 3);		// Global rendezvous to synchronize the end of the simulation

int main(int argc, char* argv[])
{
	simStart.Wait();	// Wait for all threads/processes to synchronize before starting simulation

	CMailbox e2Mailbox;									// Mailbox to receive commands from the dispatcher
	UINT Message = 0;									// Command received from the dispatcher
	Elevator e2("Elevator2");							// Object for elevator 2
	CCondition* Door[10];								// Condition for the doors on each floor
	CSemaphore* eL2 = new CSemaphore("EMutex2", 4, 4);	// Counting sempahore for number of passengers
	Queue* floors = new Queue();						// Queue to store requested floors

	int currentFloor = Message;							// Gives current floor number
	int destinationRow = FLOOR(Message);				// What row current floor should be at
	int currentRow = FLOOR(Message);					// Current row the elevator is at
	bool doorflag = true;								// Flag that signals if the floor's door has been opened at least once
	int door = CLOSED;
	int destinationFloor;
	int dir = STAT;
	int status = IN_SERVICE;
	int num_pass = 0;
	char buffer[15];

	// Create condition for each door
	for (int i = 0; i < 10; i++) {
		sprintf(buffer, "EDoor2%i", i);
		Door[i] = new CCondition(buffer);
	}

	e2.Update_Status(Message, currentFloor, currentRow, door, dir, num_pass);	// Updates the datapool for elevator 2

	while (true) {

		// Update elevator status when there is a new message in the Mailbox
		if (e2Mailbox.TestForMessage()) {
			Message = e2Mailbox.GetMessage();
			floors->insert(Message % 10, dir, currentFloor);
			status = (Message / 1000) % 1000;
		}

		destinationFloor = floors->peek();

		// Do nothing
		if (destinationFloor == EMPTYQUEUE) {
			dir = STAT;
		}

		// Stop elevator if out of service
		else if (status == OUT_OF_SERVICE) {
			floors->reset();
			dir = STAT;
		}

		// Go to requested floors
		else {
			destinationRow = FLOOR(destinationFloor);

			// At destination
			if (currentRow == destinationRow) {
				door = OPEN;
				Door[destinationFloor]->Signal();											// Signal passenger threads that doors are open

				e2.Update_Status(Message, currentFloor, currentRow, door, dir, num_pass);	// Updates the datapool for elevator 2
				Sleep(2.5 * EDELAY);

				door = CLOSED;
				num_pass = 4 - eL2->Read();													// Semaphores count downwards
				Door[destinationFloor]->Reset();
				floors->pop();																// Check for next destination

				// End simulation
				if (status == ENDSIM && currentFloor == 0) {
					simEnd.Wait();	// Wait for all threads/processes to synchronize before ending simulation
					break;
				}
			}

			// Move down to destination
			else if (currentRow < destinationRow) {
				currentRow++;
				dir = DOWN;
			}

			// Move up to destination
			else {
				currentRow--;
				dir = UP;
			}
		}

		currentFloor = 10 - (currentRow + 1) / 4;	// Update current floor depending on current row

		e2.Update_Status(Message, currentFloor, currentRow, door, dir, num_pass);	// Updates the datapool for elevator 2
		Sleep(EDELAY);
	}

	// Delete the dynamically allocated conditions after the end of the simulation
	for (int i = 0; i < 10; i++) {
		delete Door[i];
	}
	delete eL2;

	simEnd.Wait();	// Wait for all threads/processes to synchronize before ending simulation

	return 0;
}