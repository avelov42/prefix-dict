#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "stdio.h"
#include "trie.h"
#include "error_handling.h"

#define c(x) ((int)x - 97)

static bool last_op_success;

static int tape_position;
static char tape[WORD_TAPE_LENGTH];

static int inserted_words;
static Data_Of_Word data_of_word[MAX_WORDS_IN_FILE];

static int node_count;
static Node* root;

//can't be negative
static int word_len(Data_Of_Word word)
{
    assert(word.end - word.start + 1 >= 0);
    return word.end - word.start + 1;
}

static Node* new_node(Node* parent)
{
    Node* ret = malloc(sizeof(Node));
    if(ret == NULL) report_error(MEMORY);
    memset(ret, 0, sizeof(Node));
    ret->parent = parent;
    node_count++;
    return ret;
}

static void delete_node_safe(Node* node, bool safe)
{

    for(int i = 0; i < LETTERS_IN_ALPHABET; i++)
        if(node->child[i] != NULL)
        {
            if(safe)
                report_error(LOGIC);
            else
                delete_node_safe(node->child[i], false);
        }
    node_count--;
    free(node);
    return;
}

static void add_word_to_tape(char* word)
{
    assert(word != NULL);
    int len = strlen(word);
    data_of_word[inserted_words].start = tape_position;
    data_of_word[inserted_words].end = tape_position + len - 1;
    memcpy(&(tape[tape_position]), word, sizeof(char) * len); //ommiting \0
    tape_position += len;
    inserted_words++;
    return;
}

static void remove_last_word_from_tape(void)
{
    int start = data_of_word[inserted_words-1].start;
    int len = word_len(data_of_word[inserted_words-1]);
    memset(&(tape[start]), 0, sizeof(char) * len);
    tape_position = data_of_word[inserted_words-1].start;
    data_of_word[inserted_words-1].start = 0;
    data_of_word[inserted_words-1].end = 0;
    inserted_words--;
    return;

}

///ANALYZE!!!
static int get_common_prefix_length(Data_Of_Word a, Data_Of_Word b)
{
    int len = 0;
    while(a.start+len <= a.end
          && b.start+len <= b.end
          && tape[a.start+len] == tape[b.start+len]) len++; //should do the job
    return len;
}

//conservative strategy, uses node's word
static Data_Of_Word get_common_prefix(Node* node, Data_Of_Word word)
{
    int len = get_common_prefix_length(node->word, word);
    Data_Of_Word ret;
    ret.start = node->word.start;
    ret.end = ret.start + len - 1;
    return ret;
}

static Data_Of_Word get_word_rest(Node* node, Data_Of_Word word)
{
    int len = get_common_prefix_length(node->word, word);
    Data_Of_Word ret;
    ret.start = word.start + len;
    ret.end = word.end;
    return ret;
}

static Data_Of_Word get_node_rest(Node* node, Data_Of_Word word)
{
    int len = get_common_prefix_length(node->word, word);
    Data_Of_Word ret;
    ret.start = node->word.start + len;
    ret.end = node->word.end;
    return ret;
}

//what if word is already in trie???
//if we ever had to break any node, it means that word is not in trie
static Node* insert_word_to_node(Node* node, Node* parent, Data_Of_Word word)
{
    if(node == NULL)
    {
        Node* ret = new_node(parent);
        ret->word = word;
        ret->is_word = true;
        last_op_success = true;
        return ret;
    }
    else
    {
        //node to which word is going to be added surely contains its own word
        //lets find common prefix of these words
        Data_Of_Word prefix = get_common_prefix(node, word);
        assert(word_len(prefix) > 0);
        //so we got a common prefix, which will be in returned node
        //we also need to have rest after word and node->word
        //because word_rest will be added to prefix-node
        //and node_rest will be also added (with children!) to prefix-node
        Data_Of_Word node_rest = get_node_rest(node, word);
        Data_Of_Word word_rest = get_word_rest(node, word);

        Node* returned = node;
        if(word_len(node_rest) != 0) //we must break node into two
        {
            returned = new_node(parent); //creating new-node, the "upper node"
            returned->word = prefix; //making it representing prefix

            node->word = node_rest; //old node-word is cut to node-rest which is non-zero
            //is_word is saved, children too
            //let's link it to returned ("new-node")
            int id = c(tape[node_rest.start]);
            returned->child[id] = node;
            node->parent = returned;
            last_op_success = true; ///NEW
        }
        //at this moment, if node_rest was non-zero, we've got returned node
        //containing prefix and one child - the given node with it's child and is_word
        //if node_rest was zero, returned-node is equal to node
        //no matter to which node points returned, we must now add word_rest to this node
        if(word_len(word_rest) == 0) //easy first
        {
            last_op_success = !returned->is_word;
            returned->is_word = true;
        }
        else
        {
            int id = c(tape[word_rest.start]); //in some cases child[id] may be null, np.
            returned->child[id] = insert_word_to_node(returned->child[id], returned, word_rest);
        }
        return returned;
    }

}

void trie_init(void)
{
    inserted_words = 0;
    tape_position = 1;
    node_count = 0;
    last_op_success = false;
    memset(tape, 0, sizeof(char) * WORD_TAPE_LENGTH);
    memset(data_of_word, 0, sizeof(Data_Of_Word) * MAX_WORDS_IN_FILE);
    root = new_node(NULL);
    return;
}


int trie_insert(char* word)
{
    assert(word != NULL);
    assert(strlen(word));
    //zagwarantuj istnienie root'a

    add_word_to_tape(word);
    int id = c(word[0]);
    root->child[id] = insert_word_to_node(root->child[id], root, data_of_word[inserted_words-1]);
    if(last_op_success) //word correctly added
    {
        last_op_success = false;
        return inserted_words-1;
    }
    else
    {
        remove_last_word_from_tape();
        return -1;
    }
}


int trie_prev(int word_number, int start, int end)
{
    if(!(0 <= word_number && word_number < inserted_words)) //out of bound word
        return -1;
    if(data_of_word[word_number].start == 0) //word has been deleted
        return -1;

    Data_Of_Word word;
    word.start = data_of_word[word_number].start + start;
    word.end = data_of_word[word_number].start + end;
    int src_len = word_len(data_of_word[word_number]);

    if(!(0 <= start && start < src_len))
        return -1;
    if(!(0 <= end && end < src_len))
        return -1;
    if(start > end)
        return -1;

    int id = c(tape[word.start]);
    root->child[id] = insert_word_to_node(root->child[id], root, word);
    if(last_op_success)
    {
        data_of_word[inserted_words].start = word.start;
        data_of_word[inserted_words].end = word.end;
        inserted_words++;
        last_op_success = false;
        return inserted_words-1;
    }
    else return -1;
}

//first call may be null, but is non-zero
static Node* find_word_node(Node* node, Data_Of_Word word)
{
    assert(word_len(word) > 0);
    if(node == NULL)
        return NULL;
    else //inside a node
    {
        int pref_len = get_common_prefix_length(node->word, word);
        int node_len = word_len(node->word);
        if(pref_len < node_len) //word cannot be found anyway
            return NULL;
        assert(pref_len == node_len);
        word.start += pref_len;
        if(word_len(word) == 0)
        {
            if(node->is_word)
                return node;
            else
                return NULL;
        }
        else //still some characters, current node fully matches, can go deeper
        {
            //cuz word-len is nonzero, word.start is well defined and belongs to word
            int id = c(tape[word.start]); //OK
            return find_word_node(node->child[id], word); //arg1 may be null, if it is, NULL will be returned
        }
    }
    assert(42 * 0);
}


static void fix_after_delete(Node* node)
{
    if(node == root)
        return;
    if(node->is_word) //nothing to do here (jetpack)
        return;
    int children_count = 0;
    int child_pos = -1;
    for(int i = 0; i < LETTERS_IN_ALPHABET; i++)
    {
        if(node->child[i] != NULL)
        {
            children_count++;
            child_pos = i;
        }
    }
    if(children_count > 1)
        return;
    if(children_count == 1)
    {
        Node* parent = node->parent;
        Node* child = node->child[child_pos];
        child->word.start -= word_len(node->word);
        assert(tape[node->word.start] == tape[child->word.start]);
        node->child[child_pos] = NULL; //to delete safely
        delete_node_safe(node, true);
        child->parent = parent;
        int id = c(tape[child->word.start]);
        if(parent != NULL)
            parent->child[id] = child;
        fix_after_delete(parent);
    }
    else //children count == 0
    {
        Node* parent = node->parent; //temp parent of deleted node
        int id = c(tape[node->word.start]); //position of this node in parents children
        parent->child[id] = NULL;
        delete_node_safe(node, true);

        fix_after_delete(parent);
    }
    return;
}

int trie_delete(int word_number)
{
    if(!(0 <= word_number && word_number < inserted_words))
        return -1;
    if(data_of_word[word_number].start == 0)
        return -1;

    int id = c(tape[data_of_word[word_number].start]); //taking index of first char of word to delete
    Node* word_node = find_word_node(root->child[id], data_of_word[word_number]); //finding it's final node
    if(word_node == NULL)
        return -1; //word_node not found
    assert(word_node->is_word);
    word_node->is_word = false;
    fix_after_delete(word_node); //bugs here
    ///WHAT HAPPENS IF I MEMSET TO ZERO TAPE OF THIS WORD?
    data_of_word[word_number].start = 0;
    data_of_word[word_number].end = 0;
    return word_number;
}

///some assertions should be added
static bool match_word_with_node(Node* node, Data_Of_Word word)
{
    assert(word_len(word) >= 0);
    if(word_len(word) == 0)
        return true;
    else //non-empty word
    {
        if(node == NULL) //inside null and still we have some chars?
            return false;
        else
        {
            int pref_len = get_common_prefix_length(node->word, word);
            int node_len = word_len(node->word);
            if(pref_len == node_len) //whole node matched
            {
                word.start += pref_len;
                assert(word_len(word) >= 0);
                if(word_len(word) == 0) //whole word matched in tree
                    return true;
                else //still some chars to match
                {
                    int id = c(tape[word.start]);
                    if(node->child[id] == NULL)
                        return false;
                    else
                        return match_word_with_node(node->child[id], word);
                }
            }
            else //pref_len < node_len, only part of word matched
            {
                word.start += pref_len;
                assert(word_len(word) >= 0);
                if(word_len(word) == 0) //nothing left
                    return true;
                else //some characters left, but nowhere to go!
                    return false;
            }
        }
    }
    assert(42 * 0);
}

bool trie_find(char* word)
{
    assert(word != NULL);
    assert(strlen(word));
    add_word_to_tape(word);
    int id = c(word[0]);
    bool ret = match_word_with_node(root->child[id], data_of_word[inserted_words-1]);
    remove_last_word_from_tape();
    return ret;
}

#ifndef NDEBUG
static void indent(int n)
{
    for(int i = 0; i < n; i++)putchar('-');
}

static void print_tree(Node* node, int level)
{
    if(node == NULL)
        return;
    if(node == root)
        putchar('#');
    indent(level);
    for(int i = node->word.start; i <= node->word.end; i++)
        putchar(tape[i]);
    if(node->is_word)
        putchar('*');
    printf(" (%p), parent: (%p)", node, node->parent);
    putchar('\n');
    for(int i = 0; i < LETTERS_IN_ALPHABET; i++)
        print_tree(node->child[i], level + word_len(node->word));
    return;
}
#endif //NDEBUG

void trie_clear(void)
{
    delete_node_safe(root, false);
    trie_init();
    //print_tree(root, 0);
    return;
}

void trie_done(void)
{
    delete_node_safe(root, false);
    return;
}

int trie_node_count(void)
{
    return node_count == 1 ? 0 : node_count;
    //root is always present in backend, but we are hiding it.
}
