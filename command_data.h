#ifndef COMMAND_DATA_H
#define COMMAND_DATA_H

#include <stdbool.h>

typedef enum
{
    INSERT,
    FIND,
    DELETE,
    PREV,
    CLEAR,
    NO_OPERATION
} Command;

//this cannot be same value as INVALID_NUMBER in parse module
#define UNUSED_FIELD_VALUE (-144)
typedef struct
{
    Command command;
    char* word; //insert, find,
    int word_number; //prev, delete
    int first, last; //prev
} Command_Data;

/**
 * @brief is_valid Checks if the given Command_Data struct is valid.
 * @param command_data
 * @return True if everything is good, false otherwise.
 * The function assumes that unused fields are set to
 * #UNUSED_FIELD_VALUE.<br>
 * Function     Uses<br>
 * insert       word (non-zero)<br>
 * find         as above<br>
 * delete       word_number<br>
 * prev         word_number, first, last<br>
 * clear        none<br>
 * no_operation none<br>
 */
bool is_command_data_valid(Command_Data* command_data);

/**
 * @brief new_command_data Constructor
 * @return Allocates memory and returns a reseted Command_Data struct.
 */
Command_Data* new_command_data(void);

/**
 * @brief free_command_data Deletes everything inside structure and itself.
 * @param command_data
 */
void free_command_data(Command_Data* command_data);

/**
 * @brief reset_command_data Sets fields of structure to default values.
 * @param command_data
 * command == NO_OPERATION
 * word == NULL
 * [rest] == #UNUSED_FIELD_VALUE
 */
void reset_command_data(Command_Data*);

#endif // COMMAND_DATA_H

