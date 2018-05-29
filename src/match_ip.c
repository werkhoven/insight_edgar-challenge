#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "struct_def.h"


// divide by 2, rounding up
int ceil2(int a)
{
    if(a%2)a = a/2 + 1;
    else a = a/2;
    return a;
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
