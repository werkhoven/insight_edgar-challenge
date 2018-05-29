Hi

# Summary

Since the data is simultaneously streamed from the to and from file, my primary concern with scalability and real-time feedback was optimizing cases where the inactivity period gets very large. This means that searching and sorting the data will be the likely bottle neck. My approach was to reduce the total number of comparisons needed to determine if any given entry belongs to a new or existing user. In practice, this primarily consisted of the following to strategies:

		1. Use the rank-order of IP addresses to explore a search tree (match_ip.c) 
		   to determine if any active user had a matching address
		2. Sort a rank->index mapping of the session data to avoid constantly 
		   sorting the session data itself
	
# Approach

Overall function of the parsing script (parse_log.c):

	1. Read and parse a line from the log file into the IP, date, and time
	2. Search rank order list of addresses to identify duplicate IPs
		* unique 	-> 	append to session list
		* duplicate	->	update session data
	3. Check for and record indices of expired sessions if the clock advances
	4. Output the data to file
	5. Update the rank-order listing to reflect new session list
	
# Dependencies

Standard C libraries were used to write this code. No dependencies should be needed

	
	

	

