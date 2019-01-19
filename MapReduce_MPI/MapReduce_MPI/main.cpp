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
#include <list>	
#include <ctime>
#include "mpi.h"

using namespace std;

/// Define macros
#define NR_PROCESSES 26
#define DIR_NAME "test_files"
#define FILE_TXT 32768
#define ROOT 0
#define NAME_SIZE 261
#define EMPTY_CHAR ""
#define R_CHOICE 1					
#define R_LEADER 2
#define S_LEADER 1
#define S_NONLIDER 0

struct words
{
	char *word;
	int frequency;
};

void appendChar(char* s, char c) {
	int len = strlen(s);
	s[len] = c;
	s[len + 1] = '\0';
}

void getFileNames(const char *path)
{
	/// Variables
	int process = 0;							// process to send file name
	int tag = 0;								// tag for communication
	DIR *d;										//
	struct dirent *dir;							//
	char *fileName;								// stores file name

	fileName = (char*)malloc(NAME_SIZE * sizeof(char));

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
		printf("> Could not open current directory");
		exit(EXIT_FAILURE);
	}
}

list<words> readWords(FILE *fp)
{
	char c;
	bool nextWord = false;
	char word[1024] = "";
	list<words> wordList;

	while ((c = fgetc(fp)) != EOF)
	{
		if ((c == ' ' || c == '\n') && !nextWord)
		{
			// create new word
			nextWord = true;
			words wordVar;
			wordVar.word = (char*)malloc(NAME_SIZE * sizeof(char));
			strcpy_s(wordVar.word, strlen(word) + 1, word);
			wordVar.frequency = 1;

			// check if it exists
			list<words>::iterator ListItem;
			bool elemExists = false;
			for (ListItem = wordList.begin(); ListItem != wordList.end(); ++ListItem)
			{
				if (strcmp(ListItem->word, wordVar.word) == 0)
				{
					ListItem->frequency++;
					elemExists = true;
				}
			}

			// add it in list if it doesn't exists
			if (!elemExists)
			{
				wordList.push_back(wordVar);
			}

			// empty read word
			strcpy_s(word, strlen(EMPTY_CHAR) + 1, EMPTY_CHAR);
		}
		else if(c >= -1 && c <= 255 && isalpha(c))
		{ 
			nextWord = false;
			appendChar(word, tolower(c));
		}
	}

	return wordList;
}

void display_words(char *fileName, list<words> wordList, bool sort = false)
{
	// sort words
	if(sort)
	{
		wordList.sort([](const words &f, const words &s) {
			if (strcmp(f.word, s.word) < 0)
			{
				return true;
			}
			else
			{
				return false;
			}
		});
	}
	
	// display words
	printf("<%s,{", fileName);
	for (std::list<words>::iterator it = wordList.begin(); it != wordList.end(); ++it)
	{
		printf("%s:%d", it->word, it->frequency);
		++it;
		if (it != wordList.end())
		{
			printf(",");
		}
		--it;
	}
	printf(">\n");
}

int main(int argc, char* argv[]) {
	/// Variables
	int myRank;									// rank of process 
	int count;									// number of processes  
	int tag = 0;								// tag for communication
	MPI_Status status;							// return status for receive 
	char *message;								// message buffer for receive
	char *filePath;								//
	FILE * fp;									//
	errno_t err;
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

	// if the process is ROOT
	if (myRank == ROOT) {
		getFileNames(DIR_NAME);					// get file names and number of them
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

		// list of words
		list<words> wordList;
		wordList = readWords(fp);
	
		//display_words(message, wordList);

		// get elapsed time
		clock_t end = clock();
		elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
		//printf("Process %d finished in %lf seconds", myRank, elapsedSecs);
	}

	sendMessage = elapsedSecs;
	

	MPI_Send(&sendMessage, 1, MPI_INT, right, round, commCart);

	while (!gasitLider)
	{
		MPI_Recv(&receiveMessage, 1, MPI_INT, left, MPI_ANY_TAG, commCart, &status);

		switch (status.MPI_TAG)
		{
		case R_CHOICE:
			if (receiveMessage == sendMessage)
			{
				statute = S_LEADER;
				round = R_LEADER;
				MPI_Send(&sendMessage, 1, MPI_INT, right, round, commCart);
				gasitLider = true;
			}
			else if (receiveMessage > sendMessage)
			{
				MPI_Send(&receiveMessage, 1, MPI_INT, right, round, commCart);
			}
			break;
		case R_LEADER:
			statute = S_NONLIDER;
			round = R_LEADER;
			MPI_Send(&receiveMessage, 1, MPI_INT, right, round, commCart);
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

	/// MPI finalization
	MPI_Finalize();								// shut down MPI

	return 0;
}
