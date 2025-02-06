#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Define constants for token types
#define MAX_LEN 256

#define MAX_TOKENS 100  // Define a maximum number of tokens to handle

typedef struct {
    char *values[MAX_TOKENS];  // Array of strings to hold token values
    int count;                 // Number of tokens found
} TokenList;

TokenList tokenize_func(const char *input);

void free_token_list(TokenList *tokenList);

#endif  // TOKENIZER_H
