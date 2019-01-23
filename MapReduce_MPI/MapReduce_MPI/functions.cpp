#include "header.h"

int f(char *key)
{
	/*
	int i, suma;
	suma = 0;
	for (i = 0; i < strlen(key); i++) 
	{
		suma = suma + *(key + i);
	}
	return suma % M;
	*/
	return (*(key + 0) - 97) % NR_PROCESSES;				// for storing in the hash map based on the first letter
}

void initialize_HT(TYPE_NODE *HT[])
{
	for (int i = 0; i < M; i++)
		HT[i] = 0;
}

TYPE_NODE* search_HT(TYPE_NODE *HT[], char* text)
{
	int h = f(text);
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

S_WORD make_word(char *text, char *document)
{
	S_WORD w;

	w.text = new char[strlen(text) + 1];
	strcpy_s(w.text, strlen(text) + 1, text);
	w.frequency = 1;
	w.document = new char[strlen(document) + 1];
	strcpy_s(w.document, strlen(document) + 1, document);

	return w;
}

void display_HT(TYPE_NODE *HT[])
{
	for (int i = 0; i < M; i++) {
		if (HT[i] != 0)
		{
			//cout << "Records with dispersion: " << i << endl;
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
			//cout << "There are: "<< count << " records" << endl;
		}
	}
}

void display_word(S_WORD w)
{
	printf("<%s,{%s,%d}>", w.text, w.document, w.frequency);
}



void write_HT_to_file(TYPE_NODE *HT[], FILE *fp)
{
	for (int i = 0; i < M; i++) {
		if (HT[i] != 0)
		{
			//cout << "Records with dispersion: " << i << endl;
			TYPE_NODE* p = HT[i];
			int count = 0;
			while (p != 0)
			{
				count++;
				write_word_to_file(p, fp);
				p = p->next;
			}
			//cout << "There are: "<< count << " records" << endl;
		}
	}
}

void write_word_to_file(TYPE_NODE * p, FILE *fp)
{
	fprintf(fp, "%s %s %d\n", p->word.text, p->word.document, p->word.frequency);
}

void insert_HT(TYPE_NODE *HT[], S_WORD w)
{
	TYPE_NODE* p = new TYPE_NODE;
	p->word = w;						
	int h = f(p->word.text);
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
			q->word.frequency = q->word.frequency + 1;
		}
	}
}

void delete_HT(TYPE_NODE *HT[], char *word)
{
	int h = f(word);
	if (HT[h] == 0)							//nu exista inregistrare cu index h
	{
		cout << "Word doesn't exists: " << word << endl;
	}
	else
	{
		TYPE_NODE* q = search_HT(HT, word);
		if (q == 0)							// nu exista o înregistrare de cheia respectiva
		{
			cout << "We don't have what to delete!" << endl;
		}
		else
		{
			cout << "Word exists: " << word << endl;
			HT[h] = q->next;
			delete q;
		}
	}
}

void append_char(char* s, char c) {
	int len = strlen(s);
	s[len] = c;
	s[len + 1] = '\0';
}

void get_file_names(const char *path)
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
				strcpy_s(fileName, strlen(dir->d_name) + 1, dir->d_name);
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
			// create new word
			nextWord = true;
			w = make_word(word, document);
			insert_HT(HT, w);
			// empty read word
			strcpy_s(word, strlen(EMPTY_CHAR) + 1, EMPTY_CHAR);
		}
		else if (c >= -1 && c <= 255 && isalpha(c))
		{
			nextWord = false;
			append_char(word, tolower(c));
		}
	}
}

S_WORD parse_line(char *string, char *delimiter)
{
	char *token;
	char *next_token;
	int fld = 0;
	char arr[MAX_NR_FILES][NAME_SIZE] = { 0x0 };
	int i;

	token = strtok_s(string, delimiter, &next_token);

	while (token) {
		strcpy_s(arr[fld], strlen(token) + 1, token);
		fld++;
		token = strtok_s(NULL, delimiter, &next_token);
	}

	S_WORD word;

	word.text = arr[0];
	word.document = arr[1];
	word.frequency = atoi(arr[2]);

	return word;
}
