/*
 ============================================================================
 Name        : main.cpp
 Author      : Alexandru Grigoras
 Version     : 0.1
 Copyright   : Alexandru Grigoras
 Description : MapReduce MPI
 ============================================================================
 */

/// Libraries
#include <stdio.h>	
#include <stdlib.h>
#include <fstream>
#include <dirent.h> 
#include "mpi.h"

using namespace std;

/// Define macros
#define NR_PROCESSES 26
#define DIR_NAME "test_files"
#define FILE_TXT 32768
#define ROOT 0
#define NAME_SIZE 261
#define FILE_PATH "test_files/"

void getFileNames(const char *path)
{
	/// Variables
	int process = 0;							// process to send file name
	int tag = 0;								// tag for communication
	DIR *d;										//
	struct dirent *dir;							//
	char fileName[NAME_SIZE];					// stores file name

	d = opendir(path);
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (dir->d_type == FILE_TXT)
			{
				strcpy_s(fileName, sizeof(dir->d_name), dir->d_name);
				process++;
				MPI_Send(fileName, strlen(fileName) + 1, MPI_CHAR, process, tag, MPI_COMM_WORLD);
			}
		}
		closedir(d);
	}
	else
	{
		printf("Could not open current directory");
	}
}
/*
void readWords(FILE *f) 
{
	char x[1024];
	// assumes no word exceeds length of 1023
	while (fscanf(f, " %1023s", x) == 1) {
		puts(x);
	}
}*/

int main(int argc, char* argv[]) {
	/// Variables
	int myRank;									// rank of process 
	int count;									// number of processes  
	int tag = 0;								// tag for communication
	MPI_Status status;							// return status for receive 
	char message[NAME_SIZE];					// message buffer for receive
	char filePath[NAME_SIZE+12];				//
	char folder[] = "test_files/\0";				//
	FILE * fp;									//

	/// MPI initialization
	MPI_Init(&argc, &argv);						// start up MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);		// find out process rank
	MPI_Comm_size(MPI_COMM_WORLD, &count);		// find out number of processes

	if (myRank == ROOT) {
		getFileNames(DIR_NAME);					// get file names and number of them
	}
	else {
		MPI_Recv(message, strlen(message) + 1, MPI_CHAR, ROOT, tag, MPI_COMM_WORLD, &status);
		strcat_s(filePath, NAME_SIZE, folder);
		//strcat_s(message, sizeof(message) + 1, filePath);
		printf("[%d] received file name: \%s\n", myRank, filePath);
		//fp = fopen(, "w+");
	}

	/// MPI finalization
	MPI_Finalize();								// shut down MPI

	return 0;
}
