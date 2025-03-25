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
    #include <stdbool.h>
    #include "dynpool.h"

    #define CMD_PARSE_VERSION                   "1.1.0"

    typedef enum
    {
        CMDPARSE_OK = 0,
        CMDPARSE_INVALID_FORMAT = -1,
        CMDPARSE_EMPTY_STRING = -2,
        CMD_PARSE_POOL_ERROR = -3
    } cmd_parse_status_t;

    cmd_parse_status_t cmdparse_from_buffer(dynpool_t* pool, const uint8_t* buf, uint16_t offset, uint16_t size, uint16_t* len);
    cmd_parse_status_t cmdparse_from_string(dynpool_t* pool, const char* str, uint16_t* len);

    #ifdef __cplusplus
}
#endif

#endif /* _CMD_PARSE_H_ */