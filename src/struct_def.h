#ifndef STRUCT_DEF_H
#define STRUCT_DEF_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ---- TYPE DEFINITIONS ---- //


// single text line 
typedef struct{
    char ip[35], date[20], time[20];
} LINE;


// single session data
typedef struct{
    char ip[35], start_date[20], request_date[20];
    int  start_t, request_t, request_tlag, request_n;
} SESSION;



#ifdef __cplusplus
}
#endif

#endif /* STRUCT_DEF_H */
