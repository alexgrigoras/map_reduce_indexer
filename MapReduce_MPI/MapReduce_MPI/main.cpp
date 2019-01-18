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
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"

/// Define macros
#define NR_PROCESSES 10

int main(int argc, char* argv[]) {
	/// Variables
	int my_rank;								// rank of process 
	int count;									// number of processes  
	MPI_Status status;							// return status for receive 

	/// MPI initialization
	MPI_Init(&argc, &argv);						// start up MPI
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);	// find out process rank
	MPI_Comm_size(MPI_COMM_WORLD, &count);		// find out number of processes


	/// MPI finalization
	MPI_Finalize();								// shut down MPI

	return 0;
}
