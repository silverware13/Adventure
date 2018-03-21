// Assignment #2
// 
// Buildrooms:
//
// Creates a series of files that hold descriptions of the
// in-game rooms and how the rooms are connected.
//
// Name: Zachary Thomas
// Email: thomasza@oregonstate.edu
// Date: 2/01/2018
//
// Citation: Adapted the sample code given to us for this assignment.
// 2.2 Program Outlining in Program 2
// https://oregonstate.instructure.com/courses/1662152/pages/2-dot-2-program-outlining-in-program-2
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

void procDir (char[256]); // Create a directory.
void randArray (char[10][9]); // Shuffle rooms.
void makeConnections (int[7][6]); // Make connections between rooms.
int isGraphFull(int[7][6]); // Checks if we have enough connections.
void addRandomConnection(int[7][6]); // Adds a random connection between rooms.
int getRandomRoom(); // Gets a random room number.
int canAddConnectionFrom(int[7][6], int); // Checks to see if a connection is valid.
int connectionAlreadyExists(int[7][6], int, int); // Check for existing connection.
void connectRoom(int[7][6], int, int); // Connects two rooms.
int isSameRoom(int, int); // Checks if two rooms are the same room.
void createRoomFiles(char[256], char[10][9], int[7][6]); // Creates the room files.

int main (int argc, char **argvi) {
	
	// Seed our random number generation using the current time.	
	srand(time(0));

	// We make a directory for our room files.
	char dirName[256];	
	procDir(dirName);

	// Store our room names in an array. Names taken from Clue board game.
	char room[10][9] = {"BallRoom", "BilardRm", "Cellar", "Consrvry", "DiningRm", "Hall", "Kitchen", "Library", "Lounge", "Study"};
	randArray(room); // We mix up the strings in our array randomly.
	
	// Generate connections between rooms.
	int roomCon[7][6];
	memset(roomCon, -1, sizeof(roomCon)); // Set all connections to -1.
	makeConnections(roomCon);
	
	// Create room files.
	createRoomFiles(dirName, room, roomCon);
	
	return 0;

}
	
// This function creates the directory that will hold our
// room files and has the process id.
//
// Input:
// 1: Pointer to a character array that will hold the directory name.
void procDir (char dirName[256]) {
	
	int pid = getpid(); // We get our process id.
	sprintf(dirName, "%s%d", "thomasza.rooms.", pid); // Get our dir name.
	mkdir(dirName, 0700); // Make our directory.

}

// This function scrambles the rooms in our room array.
//
// Input:
// 1: Array of room names.
void randArray (char room[10][9]) {
	
	int i;
	int ranRoom;
	char cpyRoom[10][9];
	for (i = 0; i < 10; i++) {
		
		// We find a room we have not transfered to our final array.	
		do {
			ranRoom = rand() % 10;
		} while(strcmp(room[ranRoom], "NULL") == 0);
		
		// Transfer a random room to our final array.
		strncpy(cpyRoom[i], room[ranRoom], 9);
		strncpy(room[ranRoom], "NULL", 9);
	
	}

	// We copy back the newly mixed rooms back to our original array.	
	for (i = 0; i <10; i++) {
	
		strncpy(room[i], cpyRoom[i], 9);

	}
		
}

// Make random connections until all rooms have 3 to 6 outbound connections.
//
// Input: 
// 1: Pointer to an int array that holds all the connections between rooms.
void makeConnections (int roomCon[7][6]) {

	while (isGraphFull(roomCon) == 0) {
		addRandomConnection(roomCon);
	}

}

// Returns true if all rooms have at least 3 outbound connections, false otherwise.
//
// Input: 
// 1: Pointer to an int array that holds all the connections between rooms.
int isGraphFull(int roomCon[7][6]) {
	
	// Loop through connections checking three at a time.
	int i;
	for (i = 0; i < 7; i++) {
		
		// Check for missing connections. If found return false.
		if (roomCon[i][0] == -1) {
			return 0;
		} else if (roomCon[i][1] == -1) {
			return 0;
		} else if (roomCon[i][2] == -1) {
			return 0;
		}

	}

	// If all connections are valid we return true.
	return 1;

}

// Adds a random, valid outbound connection from a room to another room.
//
// Input: 
// 1: Pointer to an int array that holds all the connections between rooms.
void addRandomConnection(int roomCon[7][6]) {

	int A; // We use ints to keep track of the room number.
	int B;

	// Make sure we can add a connection to room A.
	while(1) {
		
		A = getRandomRoom();
	
		if (canAddConnectionFrom(roomCon, A) == 1) {
			break;
		}
	}

	// Make sure we can add a connection to room B, that they are
	// not the same room, and that they do not already have a connection.
	do {
		B = getRandomRoom();
	}
	while (canAddConnectionFrom(roomCon, B) == 0 || isSameRoom(A, B) == 1 || connectionAlreadyExists(roomCon, A, B) == 1);

	connectRoom(roomCon, A, B); // Connect room A and room B. 
	connectRoom(roomCon, B, A); 

}

// Returns a random room number, does NOT validate if connection can be added.
int getRandomRoom() {

	return (rand() % 10);

}

// Returns true if a connection can be added from Room A (< 6 outbound connections), false otherwise.
//
// Input: 
// 1: Pointer to an int array that holds all the connections between rooms.
// 2: Room A number.
int canAddConnectionFrom(int roomCon[7][6], int A) {
	
	if(roomCon[A][5] == -1) {
		return 1;
	} else {
		return 0;
	}

}

// Returns true if a connection from Room A to Room B already exists, false otherwise.
//
// Input: 
// 1: Pointer to an int array that holds all the connections between rooms.
// 2: Room A number. 
// 3: Room B number.
int connectionAlreadyExists(int roomCon[7][6], int A, int B) {
	
	// Check for a connection from room A to room B.
	int i;
	for (i = 0; i < 6; i++) {
		
		if (roomCon[A][i] == B) {
			return 1;
		}

	}

	// Check for a connection from room B to room A.
	for (i = 0; i < 6; i++) {
		
		if (roomCon[B][i] == A) {
			return 1;
		}

	}
	
	// If we found no connections return false.
	return 0;

}

// Connects Room A to room B, does not check if this connection is valid.
//
// Input: 
// 1: Pointer to an int array that holds all the connections between rooms.
// 2: Room A number. 
// 3: Room B number.
void connectRoom(int roomCon[7][6], int A, int B) {

	// Add an outgoing connection from room A to room B.
	int i;
	for (i = 0; i < 6; i++) {
		
		if (roomCon[A][i] == -1) {
			roomCon[A][i] = B;
			break;
		}

	}

}

// Returns true if rooms x an y are the same room, false otherwise.
//
// Input:
// 1: Room A number. 
// 2: Room B number.
int isSameRoom(int A, int B) {
	
	if (A == B) {
		return 1;
	} else {
		return 0;
	}
	
}

// Create our room files in our room directory.
//
// Input:
// 1: Path to our directory.
// 2: The an array with the names of our ten rooms.
// 3: An array with all of our room connections. 
void createRoomFiles(char dirName[256], char room[10][9], int roomCon[7][6]) {
	
	char fileIn[1000] = { 0 }; // This is what we will save to file.
	char dirPath[75] = { 0 }; // This is the path to file.
	int i;
	int ii; 

	// Create seven room files.
	for(i = 1; i < 8; i++) {
		
		// Get our path for saving our room.
		sprintf(dirPath, "./%s/Room%i", dirName, i); 
		
		// Set the room name.
		sprintf(fileIn, "%s%s\n", "ROOM NAME: ", room[i-1]);
		
		// Set our connections.
		for(ii = 0; ii < 6; ii++) {
			
			// Add only valid connections.
			if(roomCon[i-1][ii] != -1) {
				sprintf(fileIn, "%s%s%d%s%s\n", fileIn, "CONNECTION ", ii+1, ": ", room[roomCon[i-1][ii]]);
			} else {
				break;
			}

		}

		// Set the room type.
		if(i == 1) {
			sprintf(fileIn, "%s%s\n", fileIn, "ROOM TYPE: START_ROOM");
		} else if(i == 7) {
			sprintf(fileIn, "%s%s\n", fileIn, "ROOM TYPE: END_ROOM");
		} else {
			sprintf(fileIn, "%s%s\n", fileIn, "ROOM TYPE: MID_ROOM");
		}		
		
		// Make, write, and close our file.
		FILE *fp;
		fp = fopen( dirPath, "w" );
		fwrite(fileIn, 1, sizeof(fileIn), fp);
		fclose(fp);

		// Clear the text we plan to save to our next file.
		memset(fileIn, 0, sizeof(fileIn));

	}

}
