#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "command_data.h"
#include "error_handling.h"

///reviewed 22.07.2015 9:51

bool is_command_data_valid(Command_Data* obj)
{
    if(obj == NULL)
        return false;
    switch(obj->command)
    {
    case INSERT:
    case FIND:
        if(obj->word == NULL)
            return false;
        if(obj->word[0] == 0) //zero-length string
            return false;
        if(obj->word_number != UNUSED_FIELD_VALUE
                || obj->first != UNUSED_FIELD_VALUE
                || obj->last != UNUSED_FIELD_VALUE)
            return false;
        break;
    case DELETE:
        if(obj->word != NULL)
            return false;
        if(obj->word_number < 0)
            return false;
        if(obj->first != UNUSED_FIELD_VALUE
                || obj->last != UNUSED_FIELD_VALUE)
            return false;
        break;
    case PREV:
        if(obj->word != NULL)
            return false;
        if(obj->word_number < 0)
            return false;
        if(obj->first < 0)
            return false;
        if(obj->last < 0)
            return false;
       break;
    case CLEAR:
    case NO_OPERATION:
        if(obj->word != NULL)
            return false;
        if(obj->word_number != UNUSED_FIELD_VALUE
                || obj->first != UNUSED_FIELD_VALUE
                || obj->last != UNUSED_FIELD_VALUE)
            return false;
        break;
    }
    return true;
}

Command_Data* new_command_data(void)
{
    Command_Data* ret = malloc(sizeof(Command_Data));
    if(ret == NULL) report_error(MEMORY);
    memset(ret, 0, sizeof(Command_Data));
    reset_command_data(ret); //init after malloc
    return ret;
}

void reset_command_data(Command_Data* obj)
{
    assert(obj != NULL);
    obj->command = NO_OPERATION;
    obj->first = UNUSED_FIELD_VALUE;
    obj->last = UNUSED_FIELD_VALUE;
    obj->word_number = UNUSED_FIELD_VALUE;
    if(obj->word != NULL)
        free(obj->word);
    obj->word = NULL;
    return;
}

void free_command_data(Command_Data* obj)
{
    assert(obj != NULL);
    if(obj->word != NULL)
        free(obj->word);
    free(obj);
}
