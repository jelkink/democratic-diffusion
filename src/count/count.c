#include "error.h"

#include <stdio.h>

int main(int argc, char **argv)
{
  int file_id, batch, start, end, sum, row, data;
  FILE *infile;
  FILE *outfile;
  char buf[200];

  if (argc < 4)
    eprintf("Usage: count <batch> <startfile> <endfile>\n");

  batch = atoi(argv[1]);
  start = atoi(argv[2]);
  end = atoi(argv[3]);

  for (file_id = start; file_id <= end; ++file_id)
    {
      printf("Analysing file %d\n", file_id);

      sprintf(buf, "count%d_%d.csv", batch, file_id);
      outfile = fopen(buf, "w");

      if (outfile == NULL)
	eprintf("Could not open file %s for writing!\n", buf);

      sprintf(buf, "batch%d_%d_countries.data", batch, file_id);
      infile = fopen(buf, "r");

      if (infile == NULL)
	eprintf("Could not open file %s!\n", buf);

      sum = row = 0;
      while ((data = getc(infile)) != EOF)
	{
	  if (data == '\n')
	    {
	      fprintf(outfile, "%d,%d,%d\n", file_id, row, sum);
	      sum = 0;
	      ++row;
	    }
	  else
	    sum += data - '0';
	}

      fclose(infile);
      fclose(outfile);
    }
  
  return 0;
}
