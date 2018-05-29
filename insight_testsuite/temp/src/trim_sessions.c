#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "struct_def.h"


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
