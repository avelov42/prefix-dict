#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>

#include "command_data.h"
#include "consts.h"
#include "parse.h"
#include "error_handling.h"
#include "trie.h"

bool is_verbose;

/**
 * @brief parse_arguments Simple parsing of program arguments.
 * @param argc
 * @param argv
 */
void parse_arguments(int argc, char** argv)
{
    assert(argc > 0);
    assert(argv != NULL);
    //ignoring argv[] nullity - OverPower..

    if(argc == 1)
        is_verbose = false;
    else if(argc == 2 && strcmp(argv[1], "-v") == 0)
        is_verbose = true;
    else
    {
        fprintf(stderr, "Invalid arguments. Use: [-v]\n");
        exit(EXIT_SUCCESS);
    }
    return;
}

/**
 * @brief do_command Executes proper trie_[command] respectively to command_data.
 * @param command_data Struct providing operation type and necessary data.
 */
void do_command(Command_Data* command_data)
{
    assert(command_data != NULL);
    assert(is_command_data_valid(command_data));

    int ret = 1;
    switch(command_data->command)
    {
    case INSERT:
        ret = trie_insert(command_data->word);
        if(ret < 0) printf("ignored\n");
        else printf("word number: %d\n", ret);
        break;
    case FIND:
        if(trie_find(command_data->word)) printf("YES\n");
        else printf("NO\n");
        break;
    case DELETE:
        ret = trie_delete(command_data->word_number);
        if(ret < 0) printf("ignored\n");
        else printf("deleted: %d\n", ret);
        break;
    case PREV:
        ret = trie_prev(command_data->word_number, command_data->first, command_data->last);
        if(ret < 0) printf("ignored\n");
        else printf("word number: %d\n", ret);
        break;
    case CLEAR:
        trie_clear();
        printf("cleared\n");
        break;
    case NO_OPERATION:
        printf("ignored\n");
        break;
    }
    if(is_verbose && (command_data->command != NO_OPERATION && command_data->command != FIND))
    {
        if(ret >= 0) fprintf(stderr, "nodes: %d\n", trie_node_count());
    }
    return;

}

/**
 * @brief loop Reads consequtive lines, parses them and executes commands.
 * Uses a single command_data structure to store info about command.
 */
void loop(void)
{
    char line[MAX_LINE_LENGTH+1]; //+1 -> endingzero0
    char* returned_ptr;
    //using structure on heap, because it contains pointer to string
    //which must be freed while exiting scope of structure.
    Command_Data* command_data = new_command_data();
    while(true)
    {
        returned_ptr = fgets(line, MAX_LINE_LENGTH, stdin);        

        if(returned_ptr != NULL) //if not pure eof or error
        {
            reset_command_data(command_data);
            parse_line(line, command_data);
            do_command(command_data);
        }
        if(ferror(stdin))
        {
            report_error(FILE_READ);
            break;
        }
        if(feof(stdin))
            break;

    }
    free_command_data(command_data);
    command_data = NULL;
    return;
}

int main(int argc, char** argv)
{
    parse_arguments(argc, argv);
    trie_init(); //do not touch.
    loop();

    is_verbose = false; //to avoid additional nodes: %d
    trie_done(); //necessary?
    return 0;
}
