/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   parseLog.c
 * Author: zw
 *
 * Created on May 24, 2018, 1:21 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parselog.h"



int main(int argc, char *argv[])
{
    
    if(argc < 1){
        printf("error: no input paths specified\n");
        return 1;
    }
    
    // open files and check for valid pointers
    FILE *fLog = fopen(argv[1], "r");
    FILE *fDur = fopen(argv[2], "r");
    FILE *fSes = fopen(argv[3], "w");
    if(fLog==NULL) printf("Error opening log file");
    if(fDur==NULL) printf("Error opening inactivity_period file");
    if(fSes==NULL) printf("Error opening write file");
    rewind(fLog);
    
    // var definitions
    int n = 40000;                          // num users
    int min_users = n/2;                    // minimum user num
    SESSION *S = malloc(n*sizeof(*S));      // session struct array
    int *rank2idx = malloc(n*sizeof(int));  // rank mapping of ip list
    int *idx2rank = malloc(n*sizeof(int));  // idx mapping of rank list
    rank2idx[0]=0;
    idx2rank[0]=0;
    int nActive = 0;                        // total active sessions
    int t_curr=0;                           // time stamp on current line (sec)
    int t_prev=0;                           // time of last t_update (sec)
    int dur = 0;                            // max session duration (sec)
    int ct = 0;
    int max_out = 0;
    
    // get session duration from file
    char txtline[2048];
    fgets(txtline, sizeof txtline, fDur);
    for(int i=0; i<strlen(txtline)-1; i++){
        int str_digit = txtline[i]-'0';
        dur = dur + str_digit * intlog10(strlen(txtline)-i-2);
    }
    fclose(fDur);
    
    LINE ls;                                // parsed single line data
    fgets(txtline, sizeof txtline, fLog);   // read log header
    
    while (fgets(txtline, sizeof txtline, fLog))
    {
        
        ct++;
        
        // parse single line
        ls = parseLine(txtline, ",\n");   
        
        // update t_current and search rank sorted list for ip match
        int idx = matchIP(ls.ip, S, nActive, rank2idx, idx2rank);   

        t_curr = str2time(ls.time);
        if(t_curr == -1)
        {
            t_curr = t_prev;
        }

        // append new session or update existing session data
        if((idx >= nActive) | nActive==0){

            strcpy(S[nActive].ip,ls.ip);
            strcpy(S[nActive].start_date,ls.date);
            strcpy(S[nActive].request_date,ls.date);
            S[nActive].start_t = t_curr;
            S[nActive].request_t = t_curr;
            S[nActive].request_tlag = t_curr;
            S[nActive].request_n = 1;
            nActive++;
        }
        else{

            strcpy(S[idx].request_date,ls.date);
            S[idx].request_t = t_curr;
            S[idx].request_n++;
        }
        
        
        // check for expired sessions if clock advances
        if(t_curr!=t_prev){
                        
            t_prev = t_curr;                // set time stamp of previous line
            int n_out = 0;                  // num sessions to output
            int out[2000];                  // placeholder for output indices
            
            // identify number and indices of sessions to output
            t_update(t_curr, dur, S, nActive, out, &n_out);
            if(n_out>max_out){
                max_out = n_out;
            }
            
            // write data to output and remove expired sessions from S
            if(n_out)
            {
                write_data(fSes, S, out, n_out);
                trim_sessions(S, out, n_out, rank2idx, idx2rank, &nActive);
            }
        }
        
        // manage memory usage
        if(nActive > n - 2)
        { 
            n = n*2;
            printf("allocating %i active users\n",n);
            S = realloc(S,n*(sizeof(*S)));
            rank2idx = realloc(rank2idx,n*(sizeof(*rank2idx)));
            idx2rank = realloc(idx2rank,n*(sizeof(*idx2rank)));
        
        }
        else if(nActive < n/2 && nActive > min_users)
        {
            n = n/2;
            printf("allocating %i active users\n",n);
            S = realloc(S,n*(sizeof(*S)));
            rank2idx = realloc(rank2idx,n*(sizeof(*rank2idx)));
            idx2rank = realloc(idx2rank,n*(sizeof(*idx2rank)));
        }
         

    }
    
    // write any remaining sessions at end of file
    int n_out = nActive;
    int out[nActive];
    for(int i=0; i<nActive; i++){
        out[i] = i;
    }
    if(n_out)
    {
        write_data(fSes, S, out, n_out);
        trim_sessions(S, out, n_out, rank2idx, idx2rank, &nActive);
    }
    
    printf("total lines parsed: %i,  peak active user number: %i\n",ct,max_out);

    // close output file
    fclose(fSes);
    return 0;
}

