#ifndef __TOKENIZER_H
#define __TOKENIZER_H

#include <stdint.h>

void initToken(char *str);
int checkTokens(char *str);
char *getToken(uint8_t tokenNum);
void setToken(char *value, uint8_t tokenNum);
uint8_t countTokens();

#endif
