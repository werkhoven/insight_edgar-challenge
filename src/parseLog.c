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
#include "parse_utils.h"



int main()
{
    // open files and check for valid pointers
    //FILE *fLog = fopen("/home/zw/Documents/log.csv", "r");
    FILE *fLog = fopen("/home/zw/Downloads/log20170214/log20170214.csv", "r");
    FILE *fDur = fopen("/home/zw/Documents/inactivity_period.txt", "r");
    FILE *fSes = fopen("/home/zw/Documents/sessionization.txt", "w");
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
        dur = dur + str_digit * power10(strlen(txtline)-i-2);
    }
    fclose(fDur);
    
    fgets(txtline, sizeof txtline, fLog);
    
    LINE ls;
    
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
                        
            if((t_curr - t_prev)>1000 || t_curr<0)
            {
                printf("\n");
                printf("\nrow: %i,  tcurr: %i,  tstamp: %s,\n",ct,t_curr,ls.time);
            }
            t_prev = t_curr;
            
            int n_out = 0;                              // num indices to output
            int out[20000];                             // placeholder for output indices
            t_update(t_curr, dur, S, nActive, out, &n_out);
            if(n_out>max_out){
                max_out = n_out;
            }
            if((ct%10000)==0)
            {
                printf("\n max output: %i\n", max_out);
            }
            
            
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
    
    printf("%i\n",ct);

    /*
    for(int i=0; i<nActive; i++){
        printf("rank: %i,  idx: %i\n",i,rank2idx[i]);
    }
    for(int i=0; i<nActive; i++){
        printf("idx: %i,  rank: %i\n",i,idx2rank[i]-1);
    }
    for(int i=0; i<nActive; i++){
       printf("%d - ip: %s\n",i, sess[rank2idx[i]].ip); 
    }
     * */

    fclose(fSes);
}

