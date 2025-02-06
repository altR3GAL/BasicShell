#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tokenize_func.h"

#define MAX_LEN 256
#define MAX_TOKENS 100  // Define a maximum number of tokens to handle

// tokenize helper function that does the explicit job of tokenizing the given input
TokenList tokenize_func(const char *input) {
    // create/instantiate variables
    TokenList tokenList;
    tokenList.count = 0;

    const char *ptr = input;

    while (*ptr != '\0') {
        // Skip whitespace
        while (isspace(*ptr)) {
            ptr++;
        }

        if (*ptr == '\0') break;  // End of string

        // Allocate memory for the token value
        char *tokenValue = (char *)malloc(MAX_LEN * sizeof(char));
        if (!tokenValue) {
            perror("Failed to allocate memory");
            exit(EXIT_FAILURE);
        }
        memset(tokenValue, 0, MAX_LEN * sizeof(char)); // Clear the memory

        // Handle special characters and words
        switch (*ptr) {
            // Store the single character as a string, all of these behave similar so I grouped them
            case '(': case ')': case '<': case '>':
            case ';': case '|':
                strcpy(tokenValue, (char[]){*ptr, '\0'});
                ptr++;
                break;
            case '"': // added an extra case for quotes since I want to maintain them/keep quoted things as one
                ptr++;  // Move past the opening quote
                char *tokenPtr = tokenValue;
                while (*ptr && *ptr != '"') {
                    if (*ptr == '\\' && (*(ptr + 1) == '"' || *(ptr + 1) == '\\')) {
                        // Handle escaped quotes or backslashes
                        *tokenPtr++ = *(++ptr);
                    } else {
                        *tokenPtr++ = *ptr;
                    }
                    ptr++;
                }
                *tokenPtr = '\0';  // Null-terminate the string
                if (*ptr == '"') ptr++;  // Move past the closing quote
                break;
            default:
                // Handle words (sequences of non-special characters)
                if (isalnum(*ptr) || *ptr == '-') {
                    char *tokenPtr = tokenValue;
                    while (*ptr && !isspace(*ptr) && strchr("<>|();", *ptr) == NULL) {
                        *tokenPtr++ = *ptr++;
                    }
                    *tokenPtr = '\0';
                } else if (isalnum(*ptr) || *ptr == '_') {
                    char *tokenPtr = tokenValue;
                    while (*ptr && !isspace(*ptr) && strchr("<>|();", *ptr) == NULL) {
                        *tokenPtr++ = *ptr++;
                    }
                    *tokenPtr = '\0';  // Null-terminate the string
                } else {
                    free(tokenValue);  // Free allocated memory on error
                    tokenValue = NULL; // Mark as NULL
                }
                break;
        }

        // Store the token value if it's valid
        if (tokenValue && tokenList.count < MAX_TOKENS) {
            tokenList.values[tokenList.count++] = tokenValue;
        } else {
            free(tokenValue);  // Free memory if not used
        }
    }
    return tokenList;
}

// helper to free the memory if needed to in main function
void free_token_list(TokenList *tokenList) {
    for (int i = 0; i < tokenList->count; i++) {
        free(tokenList->values[i]);  // Free each allocated token value
    }
}