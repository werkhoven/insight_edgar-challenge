/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   parse_utils.h
 * Author: zw
 *
 * Created on May 27, 2018, 11:06 AM
 */

#ifndef PARSE_UTILS_H
#define PARSE_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --------------------------- TYPE DEFINITIONS --------------------------- //

// dynamic data vector    
typedef struct{
    int *data;
    unsigned int size;
}Vector;

// single text line 
typedef struct{
    char ip[35], date[20], time[20];
} LINE;


// single session data
typedef struct{
    char ip[35], start_date[20], request_date[20];
    int  start_t, request_t, request_tlag, request_n;
} SESSION;



// ----------------------------- FUNCTIONS ------------------------------ //



// integer log base 10 of n
int intlog10(int n)
{

    int result = 1;
    for(int i=1; i<=n; i++)
    {
        result = result * 10;
    }
    return result;
}


// divide by 2, rounding up
int ceil2(int a)
{
    if(a%2)a = a/2 + 1;
    else a = a/2;
    return a;
}




// update session time stamps and output indices 
// of sessions exceeding the max duration
int t_update(int t, int dur, SESSION S[], int nActive, int out[], int *n_out)
{
    
    t = t-1;
    for(int i=0; i<nActive; i++)
    {

        if(t - S[i].request_t >= dur){
            out[*n_out] = i;
            *n_out = *n_out + 1;
        } 
        S[i].request_tlag = S[i].request_t;
    }
}



// parse single line of text file into relevant fields
LINE parseLine(char* line, char* delim)
{

    LINE ls;

    int i=0;
    const char* tok;
    for (tok = strtok(line, delim);
            tok && *tok;
            tok = strtok(NULL, delim))
    {      
        switch(i)
        {
            case 0:
                strcpy(ls.ip, tok);
                break;
            case 1:
                strcpy(ls.date, tok);
                break;
            case 2:
                strcpy(ls.time, tok);
                break;
        }
        i++;
    }
    return ls;
}




// parse time string and convert to sec
int str2time(char * tstr){
   
    // if valid string format
    if(strlen(tstr)==8)
    {
        
        int i=0;                            // num parses
        int sec = 0;                        // cumulative num seconds
        const int secper[] = {3600,60,1};   // second per (hr,min,sec)
        const char* tok;
        
        for (tok = strtok(tstr, ":\n");
                tok && *tok;
                tok = strtok(NULL, ":\n"))
        {
            
            // scale digits by decimal position and sum to tmp_t
            int str_digit;
            int tmp_t = 0;
            for(int j = 1; j<=strlen(tok); j++)
            { 
                str_digit = tok[j-1]-'0';                       
                str_digit = str_digit * intlog10(strlen(tok)-j);
                tmp_t = tmp_t + str_digit; 
            }

            // scale result by num sec/per and add to cumulative sec
            sec = sec + tmp_t * secper[i];
            i++;         
        }

        if(i==3)
        {
            return sec;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

    
    
}




// generate string from time (sec) in HH:MM:SS format
char *time2str(int t)
{

    int nchar = 8;
    static char tstr[8+1];      // output str
    int hhmmss[3];              // numeric values hr,min,sec

    t = t % 86400;              // ensure t < 1 day 
    hhmmss[0] = t/3600;         // num hrs remaining after division
    t = t % 3600;
    hhmmss[1] = t/60;           // num min remaining after division
    t = t % 60;
    hhmmss[2] = t;              // num sec remaining after division

    // convert numeric values to str in HH:MM:SS format
    for(int i=0; i<nchar; i++)
    {

        int subidx = i/(sizeof(hhmmss)/sizeof(int));
        char c;

        switch(i%3){
            case 0:
                c = '0' + hhmmss[subidx]/10;            // val at tens position
                hhmmss[subidx] = hhmmss[subidx]%10;     // decrement to remainder
                break;
            case 1:
                c = '0' + hhmmss[subidx];               // val at ones position
                break;
            case 2:
                c = ':';
                break;
        }
        tstr[i] = c;
    }
    
    // null terminate str and output
    tstr[nchar+1] = '\0';           
    return tstr;
}



// attempt to match current ip against existing sessions via search tree
// and update rank order lists if no match is found
int matchIP(char *ip, SESSION S[], int nActive, int *rank2idx, int *idx2rank){


    // do search on sorted IP list
    int sort = 0;                       // (match=0,sort_up=1,sort_down=-1)
    int rank = ceil2(nActive);          // ip rank number
    int nrem = nActive;                 // n remaining candidate matches
    int shift = rank;                   // magnitude of next rank shift
    int idx;                            // session index
    int subidx = rank;                  // relative index of current ...
                                        // ...rank among remaining candidates 

    
    while(nrem>0)
    {

        rank = rank + shift*sort;                   // update ip rank number
        sort = strcmp(ip,S[rank2idx[rank-1]].ip);   // compare addresses                  
        if(sort==0)
        {
            break;                                  // match found, stop search
        }
        else if(sort>0)
        {                            
            nrem = nrem - subidx;                   // shift right
            shift = ceil2(nrem);
            subidx = shift;
            sort=1;
        }
        else if(sort<0)
        {
            nrem = subidx - 1;                      // shift left
            shift = ceil2(nrem);
            if(nrem%2)
            {
                subidx = nrem - (nrem - shift);
            }
            else
            {
                subidx = nrem - (nrem - shift) + 1;
            }
            sort=-1;
        }
    }


    // output session index and update rank list
    if(sort==0){

        if(nActive==0)
        {
            idx = 0;
            idx2rank[idx] = 1;
        }
        else
        {
            idx = rank2idx[rank-1];         // output index of matched IP
            idx2rank[idx] = rank;
        }

    }
    else 
    {
        int n = nActive+1;                  // increment active users
        idx = n-1;                          // append new entry to struct

        
        if(sort==1){
            rank++;                         // insert above current rank
        }
        
        // sort rank index map to reflect new rank list
        for(int i=n-1; i>rank-1; i--){
            rank2idx[i] = rank2idx[i-1]; 
        }
        rank2idx[rank-1] = n-1;
        
        for(int i=0; i<n; i++){
            idx2rank[rank2idx[i]] = i+1;
        }

    }

    return idx;
}




// output data from expired sessions to file
void write_data(FILE *fp, SESSION S[], int out[], int n_out)
{

    // print values to file for expired sessions
    for(int i=0; i<n_out; i++)
    {

        // query index and session duration and enforce min duration
        int idx = out[i];

        // print output
        fprintf(fp,"%s,",S[idx].ip);
        fprintf(fp,"%s ",S[idx].start_date);
        fprintf(fp,"%s,",time2str(S[idx].start_t));
        fprintf(fp,"%s ",S[idx].request_date);
        fprintf(fp,"%s,",time2str(S[idx].request_t));
        fprintf(fp,"%i,",S[idx].request_t - S[idx].start_t + 1);
        fprintf(fp,"%i\n",S[idx].request_n);
         

    }

}



// sort array via insertion sort
void array_sort(int array[], int n)
{
   int i, val, j;
   for (i = 1; i < n; i++)
   {
       val = array[i];
       j = i-1;

       while (j >= 0 && array[j] > val)
       {
           array[j+1] = array[j];
           j = j-1;
       }
       array[j+1] = val;
   }
}



// remove expired sessions by shrinking arrays to the left
void trim_sessions(SESSION S[], int out[], int n_out, 
        int rank2idx[], int idx2rank[], int *nActive)
{

    
    // shrink session struct
    for(int i=out[0]; i<*nActive; i++)
    {

        // calculate num positions to shift current idx
        int shift = n_out;
        for(int j=n_out-1; j>=0; j--)
        {

            if(i>=out[j]){
                break;
            }
            else{
                shift--;
            }

        }

        S[i]=S[i+shift];        // shrink struct by overwriting expired data

    }

    // query deleted ranks and sort
    int delrank[n_out];
    for(int i=0; i<n_out; i++)
    {
        delrank[i] = idx2rank[out[i]];
    }
    array_sort(delrank, n_out);


    // update rank/index mapping
    for(int i=0; i<*nActive; i++)
    {
        
        // decrement index/rank values 
        int rankshift=0, idxshift=0;
        for(int j=n_out; j>0; j--)
        {
            // decrement if rank/index is higher than deleted rank/index
            if(idx2rank[i]>delrank[j]){     
                rankshift++;
            }
            if(rank2idx[i]>out[j]){
                idxshift++;
            }
        }
        idx2rank[i] = idx2rank[i] - rankshift;
        rank2idx[i] = rank2idx[i] - idxshift;

    }



    // shrink rank mapping
    for(int i=out[0]; i<*nActive; i++)
    {

        // calculate num positions to shift
        int shift = n_out;
        for(int j=n_out-1; j>=0; j--)
        {
            if(i<out[j]-1){
                shift = shift - 1;
            }  
        }
        idx2rank[i]=idx2rank[i+shift];

    }

    // shrink index mapping
    for(int i=delrank[0]-1; i<*nActive; i++)
    {

        // calculate num positions to shift
        int shift = n_out;
        for(int j=n_out-1; j>=0; j--)
        {
            if(i>=delrank[j]-1){
                break;
            }
            else{
                shift--;
            }     
        }
        
        rank2idx[i]=rank2idx[i+shift];
    }
     
   // decrement nActive
   *nActive = *nActive - n_out;

}


#ifdef __cplusplus
}
#endif

#endif /* PARSE_UTILS_H */

