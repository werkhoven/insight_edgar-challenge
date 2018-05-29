/* Function: str2time
 * --------------------
 * 	converts time string (HH:MM:SS) to number of seconds
 * 
 *	tstr:		-> 	time stamp string 
 *	returns:	->  number of seconds relative to 00:00:00 (0-86400)
 *
 * Function: time2str
 * --------------------
 * 	converts number of seconds to time stramp string (HH:MM:SS)
 * 
 *	t:			-> 	number of seconds relative to 00:00:00 (0-86400)
 *	returns:	->  time stamp string (HH:MM:SS)
 *
 * Function: t_update
 * --------------------
 * 	updates session output indices based on time since last request if time
 *	since last requests exceeds inactivity period
 * 
 *	t:			-> 	time (sec) acquired from log time stamps
 *	S:			->	struct containing session data
 *	nActive:	->	number of active user sessions
 *	out:	 	-> 	array to hold session indices to write to file
 * 	n_out		-> 	number of sessions to write to file
 *
 * Function: write_data
 * --------------------
 * 	prints expired session data to output (sessionization.txt)
 * 
 *	fp:			-> 	file pointer to output file
 *	S:			->	struct containing session data
 *	out:		->  array of indices of S to write to file
 *	n_out:		->	number of sessions to output
 *	
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "struct_def.h"

// integer compatible log base 10 of n
int intlog10(int n)
{
    int result = 1;
    for(int i=1; i<=n; i++)
    {
        result = result * 10;
    }
    return result;
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
                hhmmss[subidx] = hhmmss[subidx]%10;     // get remainder
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
