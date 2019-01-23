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
#define FILE_TXT 32768
#define ROOT 0
#define NAME_SIZE 261
#define EMPTY_CHAR ""
#define R_CHOICE 1					
#define R_LEADER 2
#define S_LEADER 1
#define S_NONLIDER 0
#define MAX_NR_FILES 26

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

void display_HT(TYPE_NODE *HT[]);

void write_HT_to_file(TYPE_NODE *HT[], FILE *fp);

S_WORD make_word(char *text, char *document);

void display_word(S_WORD w);

void write_word_to_file(TYPE_NODE * p, FILE *fp);

void append_char(char* s, char c);

void get_file_names(const char *path);

void read_words(TYPE_NODE *HT[], FILE *fp, char *document);

S_WORD parse_line(char *string, char *delimiter);