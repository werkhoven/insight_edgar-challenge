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


//---------------------------------------------------------------//

int power10(int exponent)
{

    int result = 1;
    for(int i=1; i<=exponent; i++)
    {
        result = result * 10;
    }
    return result;
}


//---------------------------------------------------------------//

// divide by 2, rounding up
int ceil2(int a)
{
    if(a%2)a = a/2 + 1;
    else a = a/2;
    return a;
}







//---------------------------------------------------------------//

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

        rank = rank + shift*sort;            // update ip rank number
        //printf("probing rank: %d\n",rank);

        // is ip less than, equal to, or greater than current rank
        //printf("probing rank: %d of %d\n",rank,nActive);
        //printf("matching:\n %s\n %s\n",ip,sess[rank2idx[rank]].ip);
        sort = strcmp(ip,S[rank2idx[rank-1]].ip);                  
        if(sort==0)
        {
            //printf("match found\n");
            break;
        }
        else if(sort>0)
        {                            
            //printf("no match - %d remaining\n",nrem);
            nrem = nrem - subidx;
            shift = ceil2(nrem);
            subidx = shift;
            sort=1;
        }
        else if(sort<0)
        {
            //printf("no match - %d remaining\n",nrem);
            nrem = subidx - 1;
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

    //printf("matching finished- rank: %d,  sort %d\n", rank, sort);
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
        int n = nActive+1;              // increment active users
        idx = n-1;                        // append new entry to struct

        // sort rank index map to reflect new rank list
        if(sort==1){
            rank++;
        }
        
        //printf("\nupdated mappings rank: %i\n",rank-1);
        for(int i=n-1; i>rank-1; i--){
            rank2idx[i] = rank2idx[i-1]; 
        }
        rank2idx[rank-1] = n-1;
        
        for(int i=0; i<n; i++){
            idx2rank[rank2idx[i]] = i+1;
        }
        //idx2rank[n-1] = rank;
        /*
        printf("\n - sort update - %i\n",sort);
        for(int i=0; i<n; i++)
        {
            printf("rank2idx: %i,  idx2rank: %i,  \n",rank2idx[i],idx2rank[i]-1);
        }
        */


    }


    //printf("matching finished- idx: %d,  sort %d\n", idx, sort);
    return idx;

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
                if(strlen(ls.time)!=8)
                {
                    printf("abnormal tstring: %s\n",ls.time);
                }
                break;

        }
        i++;
    }
    return ls;
}

// parse time string and convert to sec
int str2time(char * tstr){
   
    if(strlen(tstr)==8)
    {
        
        int i=0;
        int sec = 0;
        const int secper[] = {3600,60,1};
        const char* tok;
        
        for (tok = strtok(tstr, ":\n");
                tok && *tok;
                tok = strtok(NULL, ":\n"))
        {
            
            // scale digits by decimal position
            int str_digit;
            int tmp_t = 0;
            for(int j = 1; j<=strlen(tok); j++)
            { 
                str_digit = tok[j-1]-'0';
                str_digit = str_digit * power10(strlen(tok)-j);
                tmp_t = tmp_t + str_digit; 
            }

            // scale result by num sec/per
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

//---------------------------------------------------------------//


// parse time string and convert to sec
char *time2str(int t)
{

    int nchar = 8;
    static char tstr[8+1];      // output str
    int hhmmss[3];              // numeric values hr,min,sec

    t = t % 86400;              // ensure t < 1 day
    hhmmss[0] = t/3600;         // num hrs
    t = t % 3600;
    hhmmss[1] = t/60;           // num min
    t = t % 60;
    hhmmss[2] = t;              // num sec

    // convert numeric values to str in HH:MM:SS format
    for(int i=0; i<nchar; i++)
    {

        int subidx = i/(sizeof(hhmmss)/sizeof(int));
        char c;

        switch(i%3){
            case 0:
                c = '0' + hhmmss[subidx]/10;
                hhmmss[subidx] = hhmmss[subidx]%10;
                break;
            case 1:
                c = '0' + hhmmss[subidx];
                break;
            case 2:
                c = ':';
                break;
        }
        tstr[i] = c;
    }
    tstr[nchar+1] = '\0';

    return tstr;
}


//---------------------------------------------------------------//

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



/* Function to sort an array using insertion sort*/
void array_sort(int array[], int n)
{
   int i, val, j;
   for (i = 1; i < n; i++)
   {
       val = array[i];
       j = i-1;

       /* Move elements of arr[0..i-1], that are
          greater than key, to one position ahead
          of their current position */
       while (j >= 0 && array[j] > val)
       {
           array[j+1] = array[j];
           j = j-1;
       }
       array[j+1] = val;
   }
}



//---------------------------------------------------------------//

void trim_sessions(SESSION S[], int out[], int n_out, 
        int rank2idx[], int idx2rank[], int *nActive)
{

    
    // delete expired sessions and shrink session struct to the left
    for(int i=out[0]; i<*nActive; i++)
    {

        // calculate num positions to shift
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

        S[i]=S[i+shift];                  // shrink struct

    }

    // query deleted ranks and sort
    int delrank[n_out];
    for(int i=0; i<n_out; i++)
    {
        delrank[i] = idx2rank[out[i]];
    }
    array_sort(delrank, n_out);

    /*
    printf("before decrement\n");
    for(int i=0; i<*nActive; i++)
    {
        printf("idx: %i,  rank: %i\n",i,idx2rank[i]-1);
    }
    * */
    // update rank/index mapping
    for(int i=0; i<*nActive; i++)
    {
        
        int rankshift=0, idxshift=0;
        for(int j=n_out; j>0; j--)
        {
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
    /*
    printf("after decrement - before shrink\n");
    for(int i=0; i<*nActive; i++)
    {
        printf("idx: %i,  rank: %i\n",i,idx2rank[i]-1);
    }
     * */


    // shrink rank mapping
    for(int i=out[0]; i<*nActive; i++)
    {

        // calculate num positions to shift
        int shift = n_out;
        for(int j=n_out-1; j>=0; j--)
        {
            //printf("comparing %i < %i\n",i,out.data[j]);
            if(i<out[j]-1){
                shift = shift - 1;
            }  
        }
        //printf("replacing idx2rank %i with %i\n",i,i+shift);
        idx2rank[i]=idx2rank[i+shift];

    }
    /*
    printf("idx2rank: after shrink\n");
    for(int i=0; i<*nActive-out.size; i++)
    {
        printf("idx: %i,  rank: %i\n",i,idx2rank[i]-1);
    }
    */


    // shrink index mapping
    //printf("deleting rank: %i\n",delrank[0]-1);
    for(int i=delrank[0]-1; i<*nActive; i++)
    {

        // calculate num positions to shift
        int shift = n_out;
        for(int j=n_out-1; j>=0; j--)
        {
            //printf("comparing %i >= %i\n",i,delrank[j]-1);
            if(i>=delrank[j]-1){
                break;
            }
            else{
                //printf("decrementing shift\n");
                shift--;
            }     
        }

        //printf("replacing %i with %i\n",i,i+shift);
        rank2idx[i]=rank2idx[i+shift];

    }
   
   
   // decrement nActive
   *nActive = *nActive - n_out;

}


// ---------------------------------------------------------------//





#ifdef __cplusplus
}
#endif

#endif /* PARSE_UTILS_H */

