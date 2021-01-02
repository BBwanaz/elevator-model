/*
	Authors:		Janahan Dhushenthen (56777220)
					Brandon Bwanakocha (35366525)
	Date:			November 13, 2020
	Description:	Screen Monitor Class
*/

#ifndef   __Screen__
#define   __Screen__

#include "../rt.h"

class Screen
{

private:

	CMutex* myMutex;	// Mutex to synchronize writes

public:

	Screen(string Name);	// Constructor
	~Screen();				// Destructor

	void WriteToScreen(int x, int y, char message[100]);	// Function to write to screen
};

#endif

