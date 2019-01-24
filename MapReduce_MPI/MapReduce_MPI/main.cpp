/*
 ============================================================================
 Name        : main.cpp
 Author      : Alexandru Grigoras
 Version     : 0.5
 Copyright   : Alexandru Grigoras
 Project     : MapReduce_MPI
 Description : Parallel algorythm for indexing words from a list of
			   files (includes map and reduce phases)
 ============================================================================
 */

// Header
#include "header.h"

// Main function
int main(int argc, char* argv[]) {
	// Variables:
	/// mpi
	int myRank;										// rank of process 
	int processCount;								// number of processes  
	int tag = 0;									// tag for communication
	MPI_Status status;								// return status for receive 
	/// char arrays and file pointers
	char *message;									// message buffer for receive
	char *filePath;									// stores path to the file / file name
	char *filePathResult;							// stores path to the file / file name
	char fileNames[MAX_NR_FILES][NAME_SIZE];		// stores the file names
	FILE *fp_read;									// opened file object
	errno_t err;									// error message for file open problem
	FILE *fp_write;									// opened file object
	errno_t err_write;								// error message for file open problem
	/// hash table
	TYPE_NODE *HT[M];
	/// leader choice
	int nrDimensions = 2;							// processes net
	int nrElemDim = NR_PROCESSES;					// number of processes on each dimension
	int dims[NR_PROCESSES] = {1, NR_PROCESSES};		// elements of each dimension
	int periods[] = { 1, 1 };						// periodicity of dimensions
	int foundLeader = false;							// flag that activates when the leader is found
	int statute = S_LEADER;							// initially, each process is leader
	int left;										// left neighbour of current process
	int right;										// right neighbour of current process
	int round = R_CHOICE;							// tag for messages
	MPI_Comm commCart;								// cartezian topology
	/// elapsed time
	double elapsedSecs = 0.0;
	double sendMessage;
	double receiveMessage;

	/// MPI initialization
	MPI_Init(&argc, &argv);							// start up MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);			// get process rank
	MPI_Comm_size(MPI_COMM_WORLD, &processCount);	// get number of processes
	MPI_Cart_create(MPI_COMM_WORLD, nrDimensions, dims, periods, 1, &commCart);	// create cartezian topology
	MPI_Comm_rank(commCart, &myRank);				// get current rank from cartezian topology
	MPI_Comm_size(commCart, &processCount);			// get number of processes from cartezian topology

	/// Check if the  process number is correct
	if (processCount != NR_PROCESSES)
	{
		printf("> Number of processes isn't correct! It should be: %d", NR_PROCESSES);
		exit(EXIT_FAILURE);
	}

	/// Allocate memory for char arrays
	message = (char*)malloc(NAME_SIZE*sizeof(char));			
	filePath = (char*)malloc(NAME_SIZE * sizeof(char));
	filePathResult = (char*)malloc(NAME_SIZE * sizeof(char));

	/// Initialize Hash table
	initialize_HT(HT);

	/* ----------------------------------------------------------------------------------------------------	* 
	 * MAP PHASE																							*
	 * ----------------------------------------------------------------------------------------------------	*/	

	/// if the process is ROOT
	if (myRank == ROOT) {
		get_file_names(DIR_NAME, fileNames);			// get file names
	}
	/// if the process is worker
	else {
		/// receive file name
		MPI_Recv(message, strlen(message) + 1, MPI_CHAR, ROOT, tag, MPI_COMM_WORLD, &status);
		snprintf(filePath, strlen(DIR_NAME) + 1 + strlen(message) + 1, "%s%c%s", DIR_NAME, '/', message);

#ifdef SHOW_RECEIVED_FILE
		printf(" [%d] received file name: %s\n", myRank, filePath);
#endif

		// open file with received name
		err = fopen_s(&fp_read, filePath, "r");
		if (err)
		{
			perror("> Error while opening the file received.\n");
			exit(EXIT_FAILURE);
		}

		// measure time
		clock_t begin = clock();
		
		read_words(HT, fp_read, message);
		fclose(fp_read);
		
#ifdef WRITE_HT_FILE
		/// write words on file
		snprintf(filePathResult, strlen(DIR_NAME_RESULT) + 1 + strlen(message) + 1, "%s%c%s", DIR_NAME_RESULT, '/', message);
		err_write = fopen_s(&fp_write, filePathResult, "w");
		/// verify if the file was opened successfully
		if (err_write)
		{
			perror("> Error while opening the file result.\n");
			exit(EXIT_FAILURE);
		}
		write_HT_to_file(HT, fp_write, false);
		fclose(fp_write);
#endif

#ifdef SHOW_HT
		display_HT(HT);
#endif
		
		/// get elapsed time
		clock_t end = clock();
		elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
	}

	// find the highest time
	sendMessage = elapsedSecs;
	MPI_Cart_shift(commCart, 1, 1, &left, &right);
	MPI_Send(&sendMessage, 1, MPI_DOUBLE, right, round, commCart);
	while (!foundLeader)
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
				foundLeader = true;
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
			foundLeader = true;
			break;
		}
	}

#ifdef SHOW_LEADER 
	if (statute == S_LEADER)
	{
		printf("> MAP Phase - Process[%d] TOOK THE MOST TIME: %lf\n", myRank, sendMessage);
	}
	else 
	{
		printf("> MAP Phase - Process[%d] took: %lf\n", myRank, sendMessage);
	}
#endif

	/* ----------------------------------------------------------------------------------------------------	*
	 * REDUCE PHASE (READ FROM FILE)																		*
	 * ----------------------------------------------------------------------------------------------------	*/

	// get data from file if process is ROOT
	if (myRank == ROOT)
	{
		// measure time
		clock_t begin = clock();

		char tmp[1024] = { 0x0 };
		errno_t err;
		FILE *in;
		char delim[2] = " ";

		for (int i = 0; i < MAX_NR_FILES; i++)
		{
			snprintf(filePathResult, strlen(DIR_NAME_RESULT) + 1 + strlen(fileNames[i]) + 1, "%s%c%s", DIR_NAME_RESULT, '/', fileNames[i]);

			err = fopen_s(&in, filePathResult, "r");			// open file on command line

			if (err)
			{
				perror("File open error");
				exit(EXIT_FAILURE);
			}
			while (fgets(tmp, sizeof(tmp), in) != 0)			// read a record
			{
				int i = 0;
				S_WORD newWord;

				newWord = parse_line(tmp, delim);				// create a new word
				
				insert_HT(HT, newWord);
			}
			fclose(in);
		}
		FILE *out;

		// write words to file
		snprintf(filePathResult, strlen(DIR_NAME_RESULT) + 1 + strlen(FINAL_RESULT) + 1, "%s%c%s", DIR_NAME_RESULT, '/', FINAL_RESULT);
		err_write = fopen_s(&out, filePathResult, "w");

#ifdef WRITE_HT_FILE
		write_HT_to_file(HT, out, true);
#endif
		fclose(out);

		/// get elapsed time
		clock_t end = clock();
		elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;

		printf("> REDUCE Phase - Process[%d] TOOK: %lf\n", myRank, elapsedSecs);
		printf("> Total time: %lf\n", elapsedSecs + receiveMessage);
	}

	/* ----------------------------------------------------------------------------------------------------	*
	 * REDUCE PHASE (SENDING MESSAGE WITH MPI) - NOT WORKING !												*
	 * ----------------------------------------------------------------------------------------------------	*/

	/*
	/// create a type for struct S_WORD
	int nitems = 3;
	int blocklengths[3] = { NAME_SIZE, 1, NAME_SIZE };
	MPI_Datatype types[3] = { MPI_CHAR, MPI_INT, MPI_CHAR };
	MPI_Datatype mpi_word_type;
	MPI_Aint offsets[3];

	offsets[0] = offsetof(S_WORD, text);
	offsets[1] = offsetof(S_WORD, frequency);
	offsets[2] = offsetof(S_WORD, document);

	MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_word_type);
	MPI_Type_commit(&mpi_word_type);

	/// create a type for TYPE_NODE
	nitems = 2;
	MPI_Datatype mpi_node_type;
	MPI_Aint offsets_node[2];
	int blocklengths_node[2] = { 1, 1 };
	MPI_Datatype types_node[2] = { mpi_word_type, mpi_node_type };

	offsets[0] = offsetof(TYPE_NODE, word);
	offsets[1] = offsetof(TYPE_NODE, next);

	MPI_Type_create_struct(nitems, blocklengths_node, offsets_node, types_node, &mpi_node_type);
	MPI_Type_commit(&mpi_node_type);
	
	if (myRank == ROOT)
	{
		for (int i = 0; i < NR_PROCESSES - 1; i++) {
			/// test
			int receivedRank;
			MPI_Recv(&receivedRank, 1, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
			printf("%d receiving %d from %d\n", myRank, receivedRank, status.MPI_SOURCE);

			/// to be implemented
			//MPI_Recv(HT, M, mpi_word_type, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
			//HT_list.push_back(HT);
			//display_HT(HT);
		}
	}
	else
	{
		/// test
		MPI_Send(&myRank, 1, MPI_INT, ROOT, tag, MPI_COMM_WORLD);
		/// to be implemented
		//MPI_Send(HT, M, mpi_word_type, ROOT, tag, MPI_COMM_WORLD);

	}
	*/

	// ----------------------------------------------------------------------------------------------------------------------

	/// remove cartezian communication and dealocate memory
	MPI_Comm_free(&commCart);
	free(message);
	free(filePath);
	free(filePathResult);

	/// MPI finalization
	MPI_Finalize();								// shut down MPI

	return 0;
}
