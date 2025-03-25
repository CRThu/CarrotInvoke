/****************************
* CMD PARSE
* CRTHu
* 2025.03.25
*****************************/
#pragma once
#ifndef _CMD_PARSE_H_
#define _CMD_PARSE_H_

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // !_CRT_SECURE_NO_WARNINGS

#ifdef __cplusplus
extern "C"
{
    #endif

    #include <inttypes.h>
    #include <string.h>
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include "dynpool.h"

    #define CMD_PARSE_VERSION                   "1.1.0"

    #define CMD_PARSE_ELEMENT_DELIMITER(c)		(c == '(' || c == ',' || c == ')' || c == ';' )
    #define CMD_PARSE_CHAR_IGNORE(c)            (c == ' ' || c == '\t' || c == '\r')
    #define CMD_PARSE_END(c)                    (c == '\n' || c == '\0')

    typedef int8_t cmd_parse_status_t;

    cmd_parse_status_t cmdparse_from_string(dynpool_t* pool, char* cmd, uint16_t* cmd_size);
    cmd_parse_status_t cmdparse_from_buffer(dynpool_t* pool, uint8_t* cmdbuf, uint16_t offset, uint16_t size, uint16_t* cmd_size);


    #ifdef __cplusplus
}
#endif

#endif /* _CMD_PARSE_H_ */