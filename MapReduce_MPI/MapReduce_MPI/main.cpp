/*
 ============================================================================
 Name        : main.cpp
 Author      : Alexandru Grigoras
 Version     : 0.1
 Copyright   : Alexandru Grigoras
 Description : MapReduce MPI
 ============================================================================
 */

/// Header
#include "header.h"

int main(int argc, char* argv[]) {
	/// Variables
	int myRank;									// rank of process 
	int count;									// number of processes  
	int tag = 0;								// tag for communication
	MPI_Status status;							// return status for receive 
	char *message;								// message buffer for receive
	char *filePath;								//
	FILE *fp;									//
	errno_t err;
	///
	TYPE_NODE *HT[M];
	///
	int nrDimensions = 2;					// plasa de procesoare are 1 dimensiune
	int nrElemDim = NR_PROCESSES;			// numarul de elemente pe fiecare dimensiune in parte = NP
	int dims[NR_PROCESSES] = { 1, NR_PROCESSES };	// elementele din dimensiune
	int periods[] = { 1, 1 };				// periodicitatea este pe ambele dimensiuni
	int gasitLider = false;
	int statute = S_LEADER;					// initial, fiecare proces considera ca e lider (ca e cel mai bun)
	int ID;									// ID-ul fiecarui proces
	int left;								// vecinii procesului curent
	int right;
	int round = R_CHOICE;					// tag for messages
	int vectorRanks[NR_PROCESSES];
	MPI_Comm commCart;						// construim o topologie de tip cartezian
	///
	double elapsedSecs = 0.0;
	double sendMessage;
	double receiveMessage;

	/// MPI initialization
	MPI_Init(&argc, &argv);						// start up MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);		// find out process rank
	MPI_Comm_size(MPI_COMM_WORLD, &count);		// find out number of processes
	MPI_Cart_create(MPI_COMM_WORLD, nrDimensions, dims, periods, 1, &commCart);
	MPI_Comm_rank(commCart, &myRank);			// aflam rank-ul procesului curent
	MPI_Comm_size(commCart, &count);			// aflam numarul de procese

	for (int i = 0; i < 26; i++)
	{
		vectorRanks[i] = i;
	}

	MPI_Scatter(vectorRanks, 1, MPI_INT, &ID, 1, MPI_INT, ROOT, commCart);

	MPI_Cart_shift(commCart, 1, 1, &left, &right);

	message = (char*)malloc(NAME_SIZE*sizeof(char));			
	filePath = (char*)malloc(NAME_SIZE * sizeof(char));

	initialize_HT(HT);

	// if the process is ROOT
	if (myRank == ROOT) {
		get_file_names(DIR_NAME);					// get file names and number of them
	}
	// if the process is worker
	else {
		// receive file name
		MPI_Recv(message, strlen(message) + 1, MPI_CHAR, ROOT, tag, MPI_COMM_WORLD, &status);
		snprintf(filePath, strlen(DIR_NAME) + 1 + strlen(message) + 1, "%s%c%s", DIR_NAME, '/', message);
		//printf(" [%d] received file name: %s\n", myRank, filePath);

		// open file with received name
		err = fopen_s(&fp, filePath, "r");
		if (err)
		{
			perror("> Error while opening the file.\n");
			exit(EXIT_FAILURE);
		}
		// measure time
		clock_t begin = clock();
		
		readWords(HT, fp);

		/*
		if (myRank == 20)
		{
			printf("<%s,{", message);
			display_HT(HT);
			printf("}>\n");
		}
		*/

		// get elapsed time
		clock_t end = clock();
		elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
		// printf("Process %d finished in %lf seconds", myRank, sendMessage);
	}
	sendMessage = elapsedSecs;

	MPI_Send(&sendMessage, 1, MPI_DOUBLE, right, round, commCart);

	while (!gasitLider)
	{
		MPI_Recv(&receiveMessage, 1, MPI_DOUBLE, left, MPI_ANY_TAG, commCart, &status);

		switch (status.MPI_TAG)
		{
		case R_CHOICE:
			if (receiveMessage == sendMessage)
			{
				statute = S_LEADER;
				round = R_LEADER;
				MPI_Send(&sendMessage, 1, MPI_DOUBLE, right, round, commCart);
				gasitLider = true;
			}
			else if (receiveMessage > sendMessage)
			{
				MPI_Send(&receiveMessage, 1, MPI_DOUBLE, right, round, commCart);
			}
			break;
		case R_LEADER:
			statute = S_NONLIDER;
			round = R_LEADER;
			MPI_Send(&receiveMessage, 1, MPI_DOUBLE, right, round, commCart);
			gasitLider = true;
			break;
		}
	}

	if (statute == S_LEADER)
	{
		printf("> Process[%d] TOOK THE MOST TIME: %lf\n", myRank, sendMessage);
	}
	else 
	{
		printf("> Process[%d] took: %lf\n", myRank, sendMessage);
	}

	MPI_Comm_free(&commCart);
	free(message);
	free(filePath);

	/// MPI finalization
	MPI_Finalize();								// shut down MPI

	return 0;
}
