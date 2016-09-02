#ifndef TRIE_H
#define TRIE_H

#include <stdbool.h>
#include "consts.h"

#define WORD_TAPE_LENGTH MAX_CHARS_IN_FILE

/**
  * Adress and length of a word in the wordTape
  */
typedef struct
{
    int start;
    int end;
} Data_Of_Word;

typedef struct Node
{
    Data_Of_Word word;
    bool is_word;
    struct Node* parent;
    struct Node* child[LETTERS_IN_ALPHABET];
    //how about parent?
} Node;

typedef struct
{

} Dictionary;

/**
 * @brief trie_init Required before any usage.
 */
void trie_init(void);
/**
 * @brief trie_insert
 * @param word
 * @return Number of word or -1 in case of failure (word already present).
 */
int trie_insert(char* word);
int trie_prev(int word_number, int start, int end);
int trie_delete(int word_number);
bool trie_find(char* word);
void trie_clear(void);
int trie_node_count(void);
void trie_done(void);


#endif // TRIE_H

