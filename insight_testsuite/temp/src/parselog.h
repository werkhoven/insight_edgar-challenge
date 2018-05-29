#ifndef PARSE_LOG_H
#define PARSE_LOG_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "struct_def.h"

// ------ TYPE DEFINITIONS ---- //

// single text line 
typedef struct{
    char ip[35], date[20], time[20];
} LINE;


// single session data
typedef struct{
    char ip[35], start_date[20], request_date[20];
    int  start_t, request_t, request_tlag, request_n;
} SESSION;

LINE parse_line(char* line, char* delim);

int ceil2(int a);

int matchIP(char *ip, SESSION S[], int nActive, int *rank2idx, int *idx2rank);

int intlog10(int n);

int t_update(int t, int dur, SESSION S[], int nActive, int out[], int *n_out);

int str2time(char * tstr);

char *time2str(int t);

void write_data(FILE *fp, SESSION S[], int out[], int n_out);

void array_sort(int array[], int n);

void trim_sessions(SESSION S[], int out[], int n_out, 
        int rank2idx[], int idx2rank[], int *nActive);





#ifdef __cplusplus
}
#endif

#endif /* PARSE_LOG_H */
