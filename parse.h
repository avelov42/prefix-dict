#ifndef PARSE_H
#define PARSE_H
#include "command_data.h"

#define MAX_LEXEMS_IN_LINE 4

/**
 * @brief parse_line Eats a line, gives a command_data structure.
 * @param line Pointer to input line of #MAX_LINE_LENGTH+1 length.
 * @return Wskaźnik do struktury poprawnej struktury danych
 */
void parse_line(char* line, Command_Data* command_data);

#endif // PARSE_H

