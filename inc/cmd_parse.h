/****************************
* CMD PARSE
* CARROT HU
* 2024.09.04
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
#include "../Inc/dynpool.h"

#define CMD_PARSE_VERSION    "1.0.2"

#define CMD_PARSE_ELEMENT_DELIMITER(c)		(c == '(' || c == ',' || c == ')' || c == ';' )
#define CMD_PARSE_CHAR_IGNORE(c)			(c == ' ' || c == '\t' || c == '\r')
#define CMD_PARSE_END(c)					(c == '\n' || c == '\0')

    typedef int8_t cmd_parse_status_t;

    cmd_parse_status_t cmd_parse_one(dynpool_t* pool, char* cmd, uint16_t len);
    cmd_parse_status_t parse_params(dynpool_t* pool, char* cmd, uint16_t len);


#ifdef __cplusplus
}
#endif

#endif /* _CMD_PARSE_H_ */