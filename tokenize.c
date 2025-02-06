#include "tokenize_func.h"

// main function to run and test the tokenize_func I wrote.
int main(int argc, char **argv) {
    char input[MAX_LEN];  // Limit input length

    if (fgets(input, sizeof(input), stdin) != NULL) {
        TokenList tokenList = tokenize_func(input);
        for (int i = 0; i < tokenList.count; i++) {
            printf("%s\n", tokenList.values[i]);
        }
        free_token_list(&tokenList); // Free allocated memory for tokens
    }

    return 0;
}