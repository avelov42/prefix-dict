#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <ctype.h>
#include "parse.h"
#include "error_handling.h"
#include "consts.h"

#define SPACE_CODE 32
#define LF_CODE 10
#define INVALID_NUMBER (-17)

typedef enum
{
    WORD,
    NUMBER,
    EMPTY
} Lexem_Type;

typedef struct
{
    int start;
    int end; //inclusively
    Lexem_Type type;
} Lexem;

/**
 * @brief get_lexem_array Does magic, comments inside.
 * @param line
 * @return Array of MAX_LEXEMS_IN_FILE if input line is correct, and there are no more than 4 lexems. Otherwise returns NULL.
 */
static Lexem* get_lexem_array(char* line)
{
    int curr_pos = 0;
    int lexem_number = 0;
    Lexem* ret_lexems = malloc(sizeof(Lexem) * MAX_LEXEMS_IN_LINE);
    if(ret_lexems == NULL) report_error(MEMORY);
    for(int i = 0; i < MAX_LEXEMS_IN_LINE; i++) //init after malloc, replaces memset
    {
        ret_lexems[i].type = EMPTY;
        ret_lexems[i].start = -1;
        ret_lexems[i].end = -1;
    }

    bool in_word = false;
    bool in_number = false;
    bool in_zero = false;
    bool in_spaces = true; //we can assume, that we are coming from an infinite sea of spaces..
    bool invalid_character_occured = false;

    curr_pos = -1; //assuming that line[-1] is space, we consider character at [0]
    //while looking at line[curr_pos+1] take into account that line is MAX_LINE_LENGTH+1 long.
    while(curr_pos < MAX_LINE_LENGTH && line[curr_pos+1] != 0) //line max index is 10^5, it is terminating 0
    {
        assert(in_word + in_number + in_zero + in_spaces == 1);
        curr_pos++;

        if(in_word)
        {
            if(islower(line[curr_pos])) continue; // still word
            else if(line[curr_pos] == SPACE_CODE || line[curr_pos] == LF_CODE) // end of word
            {
                ret_lexems[lexem_number].end = curr_pos-1;
                lexem_number++;
                in_word = false;
                in_spaces = true;

            }
            else invalid_character_occured = true; // if number
        }
        else if(in_number)
        {
            if(isdigit(line[curr_pos])) continue;
            else if(line[curr_pos] == SPACE_CODE || line[curr_pos] == LF_CODE)
            {
                ret_lexems[lexem_number].end = curr_pos-1;
                lexem_number++;
                in_number = false;
                in_spaces = true;
            }
            else invalid_character_occured = true;
        }
        else if(in_zero)
        {
            if(line[curr_pos] == SPACE_CODE || line[curr_pos] == LF_CODE)
            {
                ret_lexems[lexem_number].end = curr_pos-1;
                lexem_number++;
                in_zero = false;
                in_spaces = true;
            }
            else invalid_character_occured = true;
        }
        else if(in_spaces)
        {
            if(line[curr_pos] == SPACE_CODE || line[curr_pos] == LF_CODE) continue;

            //in case MLIN lexems were parsed and there is still something in line
            //we abort parsing - cuz we dont like these additional chars.
            if(lexem_number == MAX_LEXEMS_IN_LINE) invalid_character_occured = true;
            else if(islower(line[curr_pos])) //into a word
            {
                ret_lexems[lexem_number].start = curr_pos;
                ret_lexems[lexem_number].type = WORD;
                in_spaces = false;
                in_word = true;
            }
            else if(line[curr_pos] == '0') //into a zero-starting number
            {
                ret_lexems[lexem_number].start = curr_pos;
                ret_lexems[lexem_number].type = NUMBER;
                in_spaces = false;
                in_zero = true;
            }
            else if(isdigit(line[curr_pos])) //non-zero starting number
            {
                ret_lexems[lexem_number].start = curr_pos;
                ret_lexems[lexem_number].type = NUMBER;
                in_spaces = false;
                in_number = true;
            }
            else invalid_character_occured = true;
        }
        else assert(false);
        if(invalid_character_occured)
        {
            free(ret_lexems);
            return NULL;
        }
    }
    return ret_lexems;
}

/**
 * @brief get_word_from_lexem
 * @param lex
 * @param line
 * @return A word or an empty string if lexem type is EMPTY.
 */
static char* get_word_from_lexem(Lexem* lex, char* line)
{
    assert(lex != NULL);

    int word_len = lex->type == EMPTY ? 0 : lex->end - lex->start + 1;
    char* ret = malloc(sizeof(char) * (word_len+1)); //if empty, null string is created
    if(ret == NULL) report_error(MEMORY);
    memset(ret, 0, sizeof(char) * (word_len+1));

    memcpy(ret, &line[lex->start], sizeof(char) * word_len);
    ret[word_len] = 0;
    return ret;
}

/**
 * @brief get_number_from_lexem
 * @param lex
 * @param line
 * @return A non-negative number or INVALID_NUMBER, if lexem represents too large number to fit into int.
 */
static int get_number_from_lexem(Lexem* lex, char* line)
{
    assert(lex != NULL);
    assert(lex->type == NUMBER);

    char* as_word = get_word_from_lexem(lex, line);
    int digit_count = strlen(as_word);
    if(digit_count > 10)
    {
        free(as_word);
        return INVALID_NUMBER;
    }
    if(digit_count == 10)
    {
        long long val = atoll(as_word);
        if(val > (long long) INT_MAX)
        {
            free(as_word);
            return INVALID_NUMBER;
        }
    }

    int val = atoi(as_word);
    free(as_word);
    return val;
}

void parse_line(char* line, Command_Data* command_data)
{
    assert(line != NULL);
    assert(command_data != NULL);
    assert(is_command_data_valid(command_data));

    Lexem* lexem_array = get_lexem_array(line); //getting few first lexems

    if(lexem_array != NULL && lexem_array[0].type == WORD) //if first lexem is a word
    {
        char* first_string = get_word_from_lexem(&lexem_array[0], line); //we are getting that word

        if(!strcmp(first_string, "insert")) //if it is insert
        {
            //it should have only word argument and nothing more
            if(lexem_array[1].type == WORD && lexem_array[2].type == EMPTY && lexem_array[3].type == EMPTY)
            {
                command_data->command = INSERT;
                command_data->word = get_word_from_lexem(&lexem_array[1], line);
            }
            //else command_data remains with NO_OPERATION, same as below
        }
        if(!strcmp(first_string, "prev"))
        {
            if(lexem_array[1].type == NUMBER && lexem_array[2].type == NUMBER && lexem_array[3].type == NUMBER)
            {
                command_data->command = PREV;
                command_data->word_number = get_number_from_lexem(&lexem_array[1], line);
                command_data->first = get_number_from_lexem(&lexem_array[2], line);
                command_data->last = get_number_from_lexem(&lexem_array[3], line);
                //numbers may appear to be INVALID_NUMBER at this moment, it is checked at the end
            }
        }
        if(!strcmp(first_string, "find"))
        {
            if(lexem_array[1].type == WORD && lexem_array[2].type == EMPTY && lexem_array[3].type == EMPTY)
            {
                command_data->command = FIND;
                command_data->word = get_word_from_lexem(&lexem_array[1], line);
            }
        }
        if(!strcmp(first_string, "delete"))
        {
            if(lexem_array[1].type == NUMBER && lexem_array[2].type == EMPTY && lexem_array[3].type == EMPTY)
            {
                command_data->command = DELETE;
                command_data->word_number = get_number_from_lexem(&lexem_array[1], line);
            }
        }
        if(!strcmp(first_string, "clear"))
        {
            if(lexem_array[1].type == EMPTY && lexem_array[2].type == EMPTY && lexem_array[3].type == EMPTY)
            {
                command_data->command = CLEAR;
            }
        }
        //if none matches, nothing changes, thus "NO_OPERATION" is returned
        free(first_string);
    }
    free(lexem_array);

    //get_number_from_lexem may return INVALID_NUMBER, so let's consider it as an invalid input.
    if(command_data->first == INVALID_NUMBER || command_data->last == INVALID_NUMBER || command_data->word_number == INVALID_NUMBER)
        reset_command_data(command_data);
    return;
}
