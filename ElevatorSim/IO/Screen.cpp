/*
	Authors:		Janahan Dhushenthen (56777220)
					Brandon Bwanakocha (35366525)
	Date:			November 13, 2020
	Description:	Screen Monitor Class
*/

#include "Screen.h"

// Constructor, which accepts a name as a parameter
Screen::Screen(string Name) 
{

	// Initialize mutex with unique name
	myMutex = new CMutex(string("__Mutex__") + string(Name));	
}

// Destructor, which deletes the mutex
Screen::~Screen()
{
	delete myMutex;
}

// Function to write message at specified coordinates
void Screen::WriteToScreen(int x, int y, char message[100])
{
	myMutex->Wait();

	MOVE_CURSOR(x, y);
	printf(message);
	
	myMutex->Signal();
}
