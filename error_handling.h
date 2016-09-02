#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

#define report_error(x) _report_error(x, __LINE__, __func__, __FILE__)

typedef enum
{
    MEMORY,
    FILE_READ,
    LOGIC
} Error_Type;

void _report_error(Error_Type type, int line, const char* func, const char* file);

#endif // ERROR_HANDLING_H

