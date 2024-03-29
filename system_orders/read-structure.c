/* 
 * read-structure - Reads a structure variable on a file
 * Usage: read-structure
 * Bruno Mondelo Giaramita
 * 2017-06-28
*/

#include <stdio.h>


#define PROGRAM_NAME "read-structure"
/* The program official name */

#define BIN_FILE "/tmp/saved.bin"
/* File to save all the binaries */

struct rec
{
  char * name;
};
/* Structure to write */

int
main ()
{
  
  FILE * binfile;
  /* File descriptor of the binary file */
  
  struct rec new_record;
  /* The structure to save */
  
  if ((binfile = fopen(BIN_FILE, "rb")) == NULL)
  {
    fprintf(stderr, "%s - Unable to open the binary file : %s\n",
      PROGRAM_NAME, BIN_FILE);
    return 1;
  }
  /* Open the binary file descriptor, error is a criticall error */
  
  while (fread(&new_record, sizeof(struct rec), 1, binfile) > 0)
    printf("- Value of name : %s\n", new_record.name);
  /* Read a set of items */
  
  fclose(binfile);
  /* Close file descriptor */
  
  return 0;
  
}


