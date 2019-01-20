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
#define M 26					// old: 503
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
#define TABLE_SIZE 503

using namespace std;

struct S_WORD
{
	char *text;				// key for search
	int frequency;			// frequency of word
	char *document;			// document in which the word is found
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

void display_HT(TYPE_NODE *HT[], char *message);

S_WORD make_word(char *text, char *document);

void display_word(TYPE_NODE * p);

void append_char(char* s, char c);

void get_file_names(const char *path);

void readWords(TYPE_NODE *HT[], FILE *fp, char *document);