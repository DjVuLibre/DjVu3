#include <stdio.h>
#include <string.h>


int
djvufile(const char *firstbytes, FILE *verbose)
{
  const char *s = firstbytes;
  if (strncmp(s, "AT&T", 4) == 0)
    s += 4;
  if (strncmp(s, "FORM", 4) == 0)
    {
      if (strncmp(s+8, "BM44", 4) == 0)
        {
          if (verbose) 
            fprintf(verbose,"a gray DjVu wavelet file");
          return 1;
        }
      if (strncmp(s+8, "PM44", 4) == 0)
        {
          if (verbose) 
            fprintf(verbose,"a color DjVu wavelet file");
          return 1;
        }
      if (strncmp(s+8, "DJVU", 4) == 0)
        {
          if (verbose) 
            fprintf(verbose,"a single-page DjVu file");
          return 1;
        }
      if (strncmp(s+8, "DJVM", 4) == 0)
        {
          if (verbose) 
            fprintf(verbose,"a multi-page DjVu file");
          if (strncmp(s+12, "DIR0", 4) == 0)
            {
              if (verbose) 
                fprintf(verbose, " (obsolete bundled file)");          
              return 1;
            }
          if (strncmp(s+12, "DIRM", 4) == 0)
            {
              if (verbose) 
                fprintf(verbose, " (%s)", ((s[17]&0x80)?"bundled":"indirect"));
              return 1;
            }
          if (verbose) 
            fprintf(verbose," (looks broken)");
          return 0;
        }
      if (strncmp(s+8, "DJVI", 4) == 0)
        {
          if (verbose) 
            fprintf(verbose,"a shared file for a DjVu document (not viewable)");
          return 0;
        }
      if (strncmp(s+8, "THUM", 4) == 0)
        {
          if (verbose) 
            fprintf(verbose,"a thumbnail file for a DjVu document (not viewable)");
          return 0;
        }
    }
  if (verbose) fprintf(verbose, "not a DjVu file");
  return 0;
}





int 
main(int argc, char **argv)
{
  FILE *verbose = stdout;
  int quiet = 0;
  int status = 0;
  /* Process options */
  if (argc>1 && !strcmp(argv[1],"-q"))
    {
      verbose = NULL;
      argc--;
      argv++;
    }
  /* Usage */
  if (argc<= 1)
    {
      DjVuPrintError("%s",
              "Usage  djvufile [-q] <filenames>\n"
              "  Identifies a djvu file.\n"
              "  Option '-q' makes the command silent.\n"
              "  The return code indicates whether file is viewable.\n");
      exit(10);
    }
  /* Process */
  while (argc>1)
    {
      FILE *f = 0;
      char firstbytes[24];
      if (verbose) 
        fprintf(verbose,"\"%s\" is ", argv[1]);
      status = 0;
      if (! (f = fopen(argv[1],"rb")))
        {
          status = 2;
          if (verbose) 
            fprintf(verbose,"not found");
        }
      else if (fread(firstbytes, 1, sizeof(firstbytes), f) != sizeof(firstbytes))
        {
          status = 2;
          if (verbose) 
            fprintf(verbose,"too short");          
        }
      else if (!djvufile(firstbytes, verbose))
        {
          status = 1;
        }
      if (f)
        fclose(f);
      if (verbose)
        fprintf(verbose,".\n");
      argc--;
      argv++;
    }
  /* Return code */
  exit(status);
}
