#pragma once

#include <iostream>
#include <string.h>
#include <stdio.h>	
#include <stdlib.h>
#include <fstream>
#include <dirent.h> 
#include <list>	
#include <ctime>
#include "mpi.h"

/// Define macros
#define M 26				// old: 503
#define NR_PROCESSES 26
#define DIR_NAME "test_files"
#define DIR_NAME_RESULT "result_files"
#define FINAL_RESULT "final_result.txt"
#define FILE_TXT 32768
#define ROOT 0
#define NAME_SIZE 261
#define EMPTY_CHAR ""
#define R_CHOICE 1					
#define R_LEADER 2
#define S_LEADER 1
#define S_NONLIDER 0
#define MAX_NR_FILES 25

using namespace std;

struct S_WORD
{
	char *text;								// key for search
	int nr_docs;							// number of documents in which the word is present
	int frequency[MAX_NR_FILES * 2];			// frequency of word
	char *document[MAX_NR_FILES * 2];			// document in which the word is found
};

struct TYPE_NODE
{
	S_WORD word;
	TYPE_NODE *next;
};

// dispersion function
int f(char *key);

void initialize_HT(TYPE_NODE *HT[]);

TYPE_NODE* search_HT(TYPE_NODE *HT[], char* text);

void insert_HT(TYPE_NODE *HT[], S_WORD w);

void delete_HT(TYPE_NODE *HT[]);

void display_HT(TYPE_NODE *HT[]);

void write_HT_to_file(TYPE_NODE *HT[], FILE *fp);

S_WORD make_word(char *text, char *document, int nr_of_documents, int frequency);

void display_word(S_WORD w);

void write_word_to_file(S_WORD w, FILE *fp);

void append_char(char* s, char c);

void get_file_names(const char *path, char array[][NAME_SIZE]);

void read_words(TYPE_NODE *HT[], FILE *fp, char *document);

S_WORD parse_line(char *string, char *delimiter);

int char2int(char *array, size_t n);

char* process(char *text);