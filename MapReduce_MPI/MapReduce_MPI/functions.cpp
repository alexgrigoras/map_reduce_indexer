/*
 ============================================================================
 Name        : functions.cpp
 Type		 : functions
 Project     : MapReduce_MPI
 ============================================================================
 */

/// Header file
#include "header.h"

// dispersion for storing in the hash map based on sum of ASCII codes
int dispersion_f1(char *key)
{
	int i, suma;
	suma = 0;
	for (i = 0; i < strlen(key); i++)
	{
		suma = suma + *(key + i);
	}
	return suma % M;
}

// dispersion for storing in the hash map based on the first letter
int dispersion_f2(char *key)
{
	return (*(key + 0) - 97) % NR_PROCESSES;				
}

// dispersion function
int dispersion_func(char *key)
{
	return dispersion_f1(key);
}

// initialize hash table with zeros
void initialize_HT(TYPE_NODE *HT[])
{
	for (int i = 0; i < M; i++)
		HT[i] = 0;
}

// search a word in the hash table
TYPE_NODE* search_HT(TYPE_NODE *HT[], char* text)
{
	int h = dispersion_func(text);
	TYPE_NODE* p = HT[h];
	while (p != 0)
	{
		if (strcmp(text, p->word.text) == 0)
		{
			return p;
		}
		p = p->next;
	}
	return 0;
}

// create a new word
S_WORD make_word(char *text, char *document, int nr_of_documents = 1, int frequency = 1)
{
	S_WORD w;

	w.text = new char[strlen(text) + 1];
	strcpy_s(w.text, strlen(text) + 1, text);
	if (nr_of_documents >= 1 && frequency >= 1)
	{
		w.nr_docs = nr_of_documents;
		w.frequency[0] = frequency;
		w.document[0] = new char[strlen(document) + 1];
		strcpy_s(w.document[0], strlen(document) + 1, document);
	}
	else {
		w.nr_docs = 1;
		w.frequency[0] = 1;
		w.document[0] = new char[strlen(document) + 1];
		strcpy_s(w.document[0], strlen(document) + 1, document);
	}

	return w;
}

// display the hash table records on console
void display_HT(TYPE_NODE *HT[])
{
	for (int i = 0; i < M; i++) {
		if (HT[i] != 0)
		{
			// cout << "Records with dispersion: " << i << endl;
			TYPE_NODE* p = HT[i];
			int count = 0;
			while (p != 0)
			{
				count++;
				display_word(p->word);
				if (p->next != 0)
				{
					printf(", ");
				}
				p = p->next;
			}
			// cout << "There are: "<< count << " records" << endl;
		}
	}
}

// display a single word and it's occurencies
void display_word(S_WORD w)
{
	int i;
	printf("<%s, {%s:%d", w.text, w.document[0], w.frequency[0]);
	for(i = 1; i< w.nr_docs; i++)
	{
		printf(",%s:%d", w.document[i], w.frequency[i]);
	}
	printf("}>");
}

// write the hash table records on a specified file
void write_HT_to_file(TYPE_NODE *HT[], FILE *fp, bool format)
{
	for (int i = 0; i < M; i++) 
	{
		if (HT[i] != 0)
		{
			TYPE_NODE* p = HT[i];
			int count = 0;
			while (p != 0)
			{
				count++;
				if (!format)
				{
					write_word_to_file(p->word, fp);
				}
				else 
				{
					write_word_to_file_formatted(p->word, fp);
				}
				p = p->next;
			}
		}
	}
}

// write a single word to file 
void write_word_to_file(S_WORD w, FILE *fp)
{
	int i;
	fprintf(fp, "%s %d",w.text, w.nr_docs);
	for (i = 0; i < w.nr_docs; i++)
	{
		fprintf(fp, " %s %d", w.document[i], w.frequency[i]);
	}
	fprintf(fp, "\n");
}

// write a single word to file 
void write_word_to_file_formatted(S_WORD w, FILE *fp)
{
	int i;
	fprintf(fp, "<%s, {%s:%d", w.text, w.document[0], w.frequency[0]);
	for (i = 1; i < w.nr_docs; i++)
	{
		fprintf(fp, ",%s:%d", w.document[i], w.frequency[i]);
	}
	fprintf(fp, "}>\n");
}

// insert a word in the hash table 
void insert_HT(TYPE_NODE *HT[], S_WORD w)
{
	int i, foundDoc = false;
	int docNumber = 0;
	TYPE_NODE* p = new TYPE_NODE;
	p->word = w;						

	int h = dispersion_func(p->word.text);

	if (HT[h] == 0)							// nu exista inregistrare cu index h
	{
		HT[h] = p;							// pun pointer
		p->next = 0;						// informatie de inlantuire
	}
	else
	{
		TYPE_NODE* q = search_HT(HT, p->word.text);
		if (q == 0)							// nu exista o înregistrare de cheia respectiva
		{
			p->next = HT[h];				// inserez in fata
			HT[h] = p;
		}
		else
		{
			foundDoc = false;
			docNumber = q->word.nr_docs;

			for (i = 0; i < docNumber; i++)
			{
				if (strcmp(p->word.document[0], q->word.document[i]) == 0)
				{
					q->word.frequency[i] = q->word.frequency[i] + 1;
					foundDoc = true;
				}
			}
			
			if(!foundDoc)
			{				
				if (docNumber > 0 && docNumber < MAX_NR_FILES)
				{
					q->word.nr_docs = docNumber + 1;
					q->word.frequency[docNumber] = p->word.frequency[0];
					q->word.document[docNumber] = new char[256];
					strcpy_s(q->word.document[docNumber], strlen(p->word.document[0]) + 1, p->word.document[0]);
				}	
			}
		}
	}
}

// delete a word (text only) from hash table
void delete_HT(TYPE_NODE *HT[], char *word_text)
{
	int h = dispersion_func(word_text);
	if (HT[h] == 0)							// nu exista inregistrare cu index h
	{
		cout << "> Word doesn't exists: " << word_text << endl;
	}
	else
	{
		TYPE_NODE* q = search_HT(HT, word_text);
		if (q == 0)							// nu exista o înregistrare de cheia respectiva
		{
			cout << "> We don't have what to delete!" << endl;
		}
		else
		{
			cout << "> Word exists: " << word_text << endl;
			HT[h] = q->next;
			delete q;
		}
	}
}

// inserts a char to a char array
void append_char(char* char_arr, char c) {
	int len = strlen(char_arr);
	char_arr[len] = c;
	char_arr[len + 1] = '\0';
}

// searches for file names on a folder on disk and inserts it in a char array
void get_file_names(const char *path, char array[][NAME_SIZE])
{
	/// Variables
	int process = 0;							/// process to send file name
	int tag = 0;								/// tag for communication
	DIR *d;										///
	struct dirent *dir;							///
	char *fileName;								/// stores file name

	fileName = (char*)malloc(NAME_SIZE * sizeof(char));

	d = opendir(path);
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (dir->d_type == FILE_TXT)
			{
				strcpy_s(fileName, strlen(dir->d_name) + 1, dir->d_name);
				strcpy_s(array[process], strlen(fileName) + 1, fileName);
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

// read words from a file (already mapped) and inserts it in a hash table
void read_words(TYPE_NODE *HT[], FILE *fp, char *document)
{
	char c;
	bool nextWord = false;
	char word[1024] = "";
	S_WORD w;

	while ((c = fgetc(fp)) != EOF)
	{
		if ((c == ' ' || c == '\n') && !nextWord)
		{
			/// create new word
			nextWord = true;
			w = make_word(word, document);
			insert_HT(HT, w);
			/// empty read word
			strcpy_s(word, strlen(EMPTY_CHAR) + 1, EMPTY_CHAR);
		}
		else if (c >= -1 && c <= 255 && isalpha(c))
		{
			nextWord = false;
			append_char(word, tolower(c));
		}
	}
}

// reads a line from a file and returns a word struct 
S_WORD parse_line(char *string, char *delimiter)
{
	char *token;
	char *next_token;
	int fld = 0;
	char arr[MAX_NR_FILES * 2 + 2][NAME_SIZE] = { 0x0 };
	int i;

	token = strtok_s(string, delimiter, &next_token);

	while (token) {
		strcpy_s(arr[fld], strlen(token) + 1, token);
		fld++;
		token = strtok_s(NULL, delimiter, &next_token);
	}

	S_WORD word;

	int nr_docs = atoi(arr[1]);
	int freq = atoi(arr[3]);

	word = make_word(arr[0], arr[2], nr_docs, freq);

	return word;
}

// convert character array to integer
int char2int(char *array)
{
	int number = 0;
	int mult = 1;

	size_t n = strlen(array);

	/// for each character in array
	while (n--)
	{
		/// if not digit or '-', check if number > 0, break or continue 
		if ((array[n] < '0' || array[n] > '9') && array[n] != '-') {
			if (number)
				break;
			else
				continue;
		}

		if (array[n] == '-') {      /// if '-' if number, negate, break 
			if (number) {
				number = -number;
				break;
			}
		}
		else {                      /// convert digit to numeric value
			number += (array[n] - '0') * mult;
			mult *= 10;
		}
	}

	return number;
}

// removes space from string
char* process(char *text) 
{
	int length, c, d;
	char *start;

	c = d = 0;

	length = strlen(text);

	start = (char*)malloc(length + 1);

	if (start == NULL)
		exit(EXIT_FAILURE);

	while (*(text + c) != '\0') {
		if (*(text + c) == ' ') {
			int temp = c + 1;
			if (*(text + temp) != '\0') {
				while (*(text + temp) == ' ' && *(text + temp) != '\0') {
					if (*(text + temp) == ' ') {
						c++;
					}
					temp++;
				}
			}
		}
		*(start + d) = *(text + c);
		c++;
		d++;
	}
	*(start + d) = '\0';

	return start;
}