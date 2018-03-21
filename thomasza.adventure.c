// Assignment #2
// 
// Adventure:
//
// Provides interface for playing game
// using most recently generated rooms.
//
// Name: Zachary Thomas
// Email: thomasza@oregonstate.edu
// Date: 2/01/2018
//
// Citation: Adapted the sample code given to us for this assignment.
// 2.4 Manipulating Directories
// http://oregonstate.instructure.com/courses/1662152/pages/2-dot-4-manipulating-directories/
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; // Create a mutex lock.

void* game (); // Start the game.
void* checkTime (); // Make a thread display the current time.
void showLocation (char[256], int*, char[7][10], char[6][10], char[50][10]); // Display the users current location.
void getDirectory (char[256]); // Get the newest directory.
void useInput (char*, char[256], int*, int*, char[7][10], char[6][10], char[50][10]); // Action based on the users input.
void getRooms (char[256], char[7][10]); // Get the name of our seven rooms.

int main (int argc, char **argvi) {

	// Create a thread for our game.
	pthread_t threadGame;
	pthread_create( &threadGame, NULL, game, NULL ); // This thread starts the game.
		
	// Join thread.
	pthread_join(threadGame, NULL);
	
	// Destroy mutex lock.	
	pthread_mutex_destroy(&lock);

	return 0;

}

// Start the game for the user.
void* game () {

	// Lock mutex.
	pthread_mutex_lock(&lock);
	
	// Buffer info.
	char *buffer;
	size_t bufSize = 256;
	size_t characters;
	buffer = (char *)malloc(bufSize * sizeof(char));

	// Get the newest room directory.
	char dirName[256];	
	getDirectory(dirName);

	// Get the array of our room names.
	char room[7][10] = { 0 };
	getRooms(dirName, room);	
	
	int curRoom = 1; // Set starting room.
	int steps = 0; // The number of steps we have taken.
	char history[50][10]; // Holds the rooms that are traveled to.
	memset(history, '\0', sizeof(history));

	// Show the users location.
	char roomCon[6][10] = { 0 }; // Holds current room connections.
	showLocation(dirName, &curRoom, room, roomCon, history);

	// Loop until the game ends.
	while(curRoom != 7) { 
	
		characters = getline(&buffer, &bufSize, stdin); // Get input from user.	
		useInput(buffer, dirName, &curRoom, &steps, room, roomCon, history); // Do something with input.
	
	}

	// Free buffer.
	free(buffer);

	// Unlock mutex.		
	pthread_mutex_unlock(&lock);

}

// Create a file with the current time and display it to the user.
void* checkTime () {

	pthread_mutex_lock(&lock); // Lock our mutex.
	
	// Create a string that contains the current time.
	char timeString[50]; // Holds our time as a string.
	memset(timeString, '\0', sizeof(timeString)); // Null all elements in array.
	time_t t;
	struct tm *tmp;
	time(&t);
		
	// Saves the current time to this string.
	tmp = localtime(&t);
	strftime( timeString, 50, "%l:%M%P, %A, %B %d, %Y\n", tmp);
	
	// Create  a file with the current time.
	FILE *fp;
	fp = fopen( "./currentTime.txt", "w" ); // Open the file.
	int sizeTime = strlen(timeString); // Save the length of string.
	fwrite(timeString, 1, sizeTime, fp); // Write to file.
	printf("\n%s\nWHERE TO? >", timeString); // Show time and prompt.	
	fclose(fp); // Close the file.	

	pthread_mutex_unlock(&lock); // Unlock our mutex.
	
}

// This function gets the path of the newest room directory.
//
// Input:
// 1: Pointer to a character array that will hold the directory name.
void getDirectory (char dirName[256]) {
	
	int dirTime = -1; // Timestamp of newest directory.
	char dirPrefix[32] = "thomasza.rooms."; // The prefix of the directory.
	memset(dirName, '\0', sizeof(dirName)); // Null all elements in array.

	DIR* dirToCheck; // The current directory.
	struct dirent *fileInDir; // Holds the current subdir of the start dir.
	struct stat dirAttributes; // Holds info about subdir.

	dirToCheck = opendir("."); // Opens up current directory.

	// Check if current directory can be opened.	
	if (dirToCheck > 0) {	
		
		// Check each file in this directory.
		while ((fileInDir = readdir(dirToCheck)) != NULL) {	
			
			// Make sure the file has the prefix.
			if(strstr(fileInDir->d_name, dirPrefix) != NULL) {				
				// Get directory info.
				stat(fileInDir->d_name, &dirAttributes);
				
				// Check to see if time is larger.
				if((int)dirAttributes.st_mtime > dirTime) {
				
					// We set our directory to the newest directory.
					dirTime = (int)dirAttributes.st_mtime;
					memset(dirName, '\0', sizeof(dirName));
					strcpy(dirName, fileInDir ->d_name);
				
				}
			}
		}
	}

	closedir(dirToCheck); // Close the directory we opened.

}

// This function displays the users current location.
//
// Input: 
// 1: Directory of rooms.
// 2: Current room number.
// 3: String of arrays.
// 4: Strings of connections.
// 5: History of  rooms traveled to.
void showLocation (char dirName[256], int* curRoom, char room[7][10], char roomCon[6][10], char history[50][10]) {
	
	int i;
	int len; // Length of a string.
	memset(roomCon, '\0', sizeof(roomCon[0][0]) * 6 * 10); // Clear old connections.
	char roomName[10] = { 0 }; // Name of current room.
	char roomConStr[72] = { 0 }; // Connections of current room.
	char curString[32] = { 0 }; // Current string we are looking at.

	// Get the path to our room.
	char dirPath[256];
	sprintf(dirPath, "./%s/Room%i", dirName, *curRoom);
	
	// Open the correct file.	
	FILE *fp;
	fp = fopen( dirPath, "r" );
	
	// Error if we can't open file.
	if (fp < 0) {
		fprintf(stderr, "Could not open file.");
		exit(1);
	}

	// Display the room name.
	fscanf(fp, "%*s%*s%s", roomName);
	printf("CURRENT LOCATION: %s\n", roomName);	
	len = strlen(roomName); // Get length of the room name.
	roomName[len] = '\n'; // Add a newline to roomName for future comparisons.
	
	// Save the rooms we have visited.
	for(i = 0; i < 50; i++) {
		
		// Find the first empty string. 
		if(strlen(history[i]) == 0) {
			
			// Check if this is the first room saved.
			if(i == 0) {
				strcpy(history[0], roomName); // Save this room.
				break;
			} else {
				
				// Make sure this room isn't the same as the last
				// saved room. Otherwise we haven't changed rooms.
				if(strcmp(history[i-1], roomName) != 0) {
					strcpy(history[i], roomName); // Save this room.
					break;
				} else {
					break;
				}
					
			}				
		}	
	}	

	// Store the room connections as a string.
	i = 0;
	printf("POSSIBLE CONNECTIONS: ");
	while(strcmp(curString, "TYPE:") != 0) {	
		
		// Save each connection.	
		if(strcmp(curString, "CONNECTION") == 0) {
				
			fscanf(fp, "%*s%s", curString); // Get a connection.
			strcpy(roomCon[i], curString); // Save our connection.
			len = strlen(roomCon[i]); // Get length of our con str.
			roomCon[i][len] = '\n'; // Add a newline to our string.
			sprintf(roomConStr, "%s%s, ", roomConStr, curString); // Add this connection to our string.
			i++;
		}	
	
		fscanf(fp, "%s", curString); // Read the next string from the file.
		
	}

	len = strlen(roomConStr); // Get the string length.
	roomConStr[len-2] = '.'; // Change the last char.
	printf("%s\n", roomConStr); // Display the connections.	
	
	// Display text prompt.
	printf("WHERE TO? >");	
	
	// Close file.
	fclose(fp);
		
}

// Perform an action based on the user input.
//
// Input:
// 1: String of users input.
// 2: Directory of rooms.
// 3: Current room.
// 4: The number of steps we have taken.
// 5: Array of our seven rooms.
// 6: Strings of connections.
// 7: History of  rooms traveled to.
void useInput (char* buffer, char dirName[256], int* curRoom, int* steps, char room[7][10], char roomCon[6][10], char history[50][10]) {

	// See if the user typed 'time'.
	if(strcmp(buffer, "time\n") == 0) {

		// Unlock mutex.		
		pthread_mutex_unlock(&lock);
		
		// Create a thread.
		pthread_t threadTime;
		pthread_create( &threadTime, NULL, checkTime, NULL ); // This thread gets the time.
		
		// Join thread.
		pthread_join(threadTime, NULL);
			
		// Lock mutex.
		pthread_mutex_lock(&lock);

		return;		

	// See if the user entered a valid room.
	} else {
		
		int i;
		int w = 0;
		
		// First make sure user entered a possible connection.
		for(i = 0; i < 6; i++) {
			
			// Compare to each connection in our array.
			if(strcmp(buffer, roomCon[i]) == 0) {
				w = 1;
				break;
			}
		
		}
	
		// Connection is valid find our room.
		for(i = 0; i < 7 && w == 1; i++) {	
			
			// Compare to each room in our array.
			if(strcmp(buffer, room[i]) == 0) {
				
				*steps += 1; // Add a step to our count.
				*curRoom = i+1; // Set our current room to what we entered.
				printf("\n"); // Print a newline.
				
				// Make sure we didn't beat the game.
				if( *curRoom == 7) {
					printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\nYOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", *steps);
					
					// Print our history of up to 49 rooms.
					for(i = 1; i < 50; i++) {
					 
						if(strlen(history[i]) != 0) {
							printf("%s", history[i]);
						} else {
							break;
						}

					}
					
					// Print current/ending room.
					printf("%s", room[6]);

					return;

				} else {
					
					// If we didn't beat the game we go to the next room.
					showLocation(dirName, curRoom, room, roomCon, history);
					return;
				}
			}
		}
	}

	// If the user didn't enter 'time' or a valid room give message.	
	printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
	showLocation(dirName, curRoom, room, roomCon, history);
		
}

// Store the names of our seven rooms in the input array.
//
// Input:
// 1: Directory of rooms.
// 2: Array to store names.
void getRooms (char dirName[256], char room[7][10]) {
	
	int i;
	char dirPath[256]; // Stores the path to a room.

	for(i = 1; i < 8; i++) {
		
		// Get room path.
		sprintf(dirPath, "./%s/Room%i", dirName, i);
	
		// Open the correct file.	
		FILE *fp;
		fp = fopen( dirPath, "r" );
	
		// Error if we can't open file.
		if (fp < 0) {
			fprintf(stderr, "Could not open file.");
			exit(1);
		}

		// Store the room in our array.
		fscanf(fp, "%*s%*s%s", room[i-1]);
		int len = strlen(room[i-1]); // Get the string length.
		room[i-1][len] = '\n'; // Add a newline to the end.
		fclose(fp); // Close file.

	}	
			
}
