#include "tokenizer.h"

#include <stdlib.h>
#include <string.h>

char *tokens[8];

/*
	Functions for tokenization currently processed command packet
*/
const char* delim = " ";

void initToken(char *str)
{
	int i = 0;
	char *ptr = str;
	while (i != 8)
	{
		tokens[i] = strtok_r(ptr, delim, &ptr);
		if(tokens[i] == NULL) {
			break;
		}
		i++;
	}
}

static void tokenize(char *str, char* token[])
{
	int i = 0;
	char *ptr = str;
	while (i != 8)
	{
		token[i] = strtok_r(ptr, delim, &ptr);
		if(token[i] == NULL) {
			break;
		}
		i++;
	}
}

char *getToken(uint8_t tokenNum)
{
	return tokens[tokenNum];
}

void setToken(char *value, uint8_t tokenNum)
{
	tokens[tokenNum] = value;
}

uint8_t countTokens()
{
	uint8_t i = 0;
	for (; getToken(i) != NULL; i++);
	return i;
}

int checkTokens(char *str)
{
	char* temp_tokens[8];
	//uint8_t packetlen = strlen(str);
	char string[strlen(str)];
	strcpy(string, str);
	tokenize(string, temp_tokens);

	int i = 0;
	while (i != 8)
	{
		if(tokens[i] != NULL || temp_tokens[i] != NULL) {
			if(strcmp(tokens[i], temp_tokens[i]) != 0)
				return 0;
			i++;
		}
		if(tokens[i] == NULL && temp_tokens[i] == NULL)
			return 1;
	}
	return 1;
}
