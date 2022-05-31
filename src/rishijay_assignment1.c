/**
 * @rishijay_assignment1
 * @author  Rishi Joshi <rishijay@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include <stdio.h>
#include <stdlib.h>

#include "../include/global.h"
#include "../include/logger.h"

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/*Clear LOGFILE*/
	fclose(fopen(LOGFILE, "w"));

	/*Start Here*/
	if (argc != 3) { 
		perror("./[name of file] [c/s] [port]"); exit(-1);
		
	}
	else { if (strcmp("s", argv[1]) == 0) { // running as SERVER
			server(argv[2]); 
		}
		else if (strcmp("c", argv[1]) == 0) { // running as CLIENT
			client(argv[2]); 
		}
		else { perror("invalid arguments"); 
		exit(-1); }	 }

	return 0;
}


