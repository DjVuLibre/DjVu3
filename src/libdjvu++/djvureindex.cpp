#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include "DjVuDocument.h"
#include "IFFByteStream.h"
#include "GOS.h"
#include "GContainer.h"
#include "debug.h"

class TempDir {
  private:
    char *dirpath;
  public:
    enum ExitAction {RENAME_ALL,REMOVE_ALL};
    TempDir(const char[],ExitAction=REMOVE_ALL);
    ~TempDir();
    inline operator const char *(void) const {return dirpath;}
    int RenameAll(FILE *) const;
    int RemoveAll(FILE *) const;
    int addtotmp(const char filename[],FILE *log) const;
  private:
    ExitAction action;
};

TempDir::TempDir(const char path[],ExitAction a)
: action(a)
{
  int pathlen=strlen(path);
  const char REMOVE_ALL_ext[]="tmp";
  const char RENAME_ALL_ext[]="save";
  dirpath=new char [pathlen+20];
  strcpy(dirpath,path);
  struct stat statbuf;
  char *p=dirpath+pathlen;
  if((stat(path,&statbuf)>=0)&&(S_ISDIR(statbuf.st_mode)))
  {
    if(*(p-1) != '/')
    {
      *(p++)='/';
    }
  }else
  {
    *(p++)='.';
  }
  const char *ext=(a==REMOVE_ALL)?REMOVE_ALL_ext:RENAME_ALL_ext;
  strcpy(p,ext);
  if(stat(dirpath,&statbuf)>=0)
  {
    unsigned long id=(unsigned long)getpid();
    int i=0;
    for(sprintf(p,"%s.%08lx",ext,id);
      stat(dirpath,&statbuf)>=0;
      sprintf(p,"%s.%08lx.%d",ext,id,++i));
  }
  if(mkdir(dirpath,0755)<0)
  {
    perror("mkdir");
    THROW("Failed to create temporary directory.");
  }
}

int
TempDir::addtotmp(const char filename[],FILE *log) const
{
  const char *basename=strrchr(filename,'/');
  if(!basename++)
    basename=filename;
  char *newname=new char [strlen(basename)+strlen(dirpath)+2];
  sprintf(newname,"%s/%s",dirpath,basename);
  int retval=rename(filename,newname);
  if(log) fprintf(log,"Renaming: %s to %s\n",filename,newname);
  delete [] newname;
  return retval;
}

int
TempDir::RenameAll(FILE *log) const
{
  int retval=0;
  int parentlen=strlen(dirpath)+3;
  int dirpathlen=strlen(dirpath);
  char *parent=new char [parentlen+1];
  sprintf(parent,"%s/..",dirpath);
  DIR *dir;
  if((dir=opendir(dirpath)))
  {
    struct dirent *entry;
    while(dir&&(entry=readdir(dir)))
    {
      if((entry->d_name[0] != '.')||
         ((entry->d_name[1])&& 
         ((entry->d_name[1] != '.')||entry->d_name[2])))
      {
        char *oldname=new char [dirpathlen+strlen(entry->d_name)+2];
        sprintf(oldname,"%s/%s",dirpath,entry->d_name);
        char *newname=new char [parentlen+strlen(entry->d_name)+2];
        sprintf(newname,"%s/%s",parent,entry->d_name);
        if(rename(oldname,newname)<0)
        {
          retval=(-1);
          fprintf(stderr,"Failed to rename %s to %s\n",oldname,newname);
        }else if(log)
        {
          fprintf(log,"Renaming: %s to %s\n",oldname,newname);
        }
        delete [] oldname;
        delete [] newname;
      }
    }
    closedir(dir);
  }
  delete [] parent;
  return retval;
}

static int
rmrecursive
(const char path[],FILE *log)
{
  int retval=0;
  int pathlen=strlen(path);
  if((unlink(path)<0)&&(rmdir(path)<0))
  {
    DIR *dir;
    if((dir=opendir(path)))
    {
      struct dirent *entry;
      while(dir&&(entry=readdir(dir)))
      {
        if((entry->d_name[0] != '.')||
           ((entry->d_name[1])&& 
           ((entry->d_name[1] != '.')||entry->d_name[2])))
        {
          char *p=new char [pathlen+strlen(entry->d_name)+2];
          off_t t=telldir(dir);
          closedir(dir);
          sprintf(p,"%s/%s",path,entry->d_name);
          rmrecursive(p,log);
          delete [] p;
          if((dir=opendir(path)))
          {
            seekdir(dir,t);
          }else
          {
            dir=0;
            break;
          }
        } 
      }
      if(dir) closedir(dir);
    }
    if((unlink(path)<0)&&(rmdir(path)<0))
    {
      fprintf(stderr,"Failed to remove %s\n",path);
      retval=(-1);
    }else if(log)
    {
      fprintf(log,"Removing: %s\n",path);
    }
  }else if(log)
  {
    fprintf(log,"Removing: %s\n",path);
  }
  return retval;
}

int
TempDir::RemoveAll(FILE *log) const
{
  int retval=(-1);
  if(rmrecursive(dirpath,log)>=0)
  {
    if(mkdir(dirpath,0755)>=0)
    {
      retval=0;
    }
  }
  return retval;
}

TempDir::~TempDir()
{
  if(rmdir(dirpath)<0)
  {
    switch(action)
    {
      case REMOVE_ALL:
        RemoveAll(stderr);
        break;
      case RENAME_ALL:
        RenameAll(stderr);
        break;
    }
  }
}

void
usage(const char prog[],int status)
{
  fprintf(status?stderr:stdout,"Usage:\n\t%s <oldindex> <newindex>\n\n",prog);
  fputs(
    "All files in the old multipage format refered to by the <oldindex> file\n"
    "Are replaced by the new multipage format with the <newindex> file.\n\n",
    status?stderr:stdout);
  fprintf(status?stderr:stdout,"-or-\n\t%s <oldbundledfile> [<newbundledfile>]\n\n",prog);
  fputs(
    "The <oldbundledfile> is replaced with the <newbundledfile> in the new format.\n",
    status?stderr:stdout);
  exit(status);
}

int
main(int argc,char *argv[],char *[])
{
  FILE *log=0;
  int bundled_only=0;
  int status=1;
  struct stat statbuf;
  const char *prog=argv[0];
  while(argc>1)
  {
    if(!strcmp(argv[1],"-verbose")||!strcmp(argv[1],"--verbose"))
    {
      log=stdout;
      argc--;
      argv++;
    }else if(!strcmp(argv[1],"--"))
    {
      argc--;
      argv++;
      break;
    }else
    {
      break;
    }
  }
  if(argc < 3) 
  {
    if(argc < 2)
    {
      fputs("Too few arguments\n",stderr);
      usage(prog,1);
    }
    bundled_only=1;
  }
  if(argc > 3) 
  {
    fputs("To many arguments.\n",stderr);
    usage(prog,1);
  }
  const char *oldindex=argv[1];
  if(stat(oldindex,&statbuf)<0)
  {
    perror("stat");
    fprintf(stderr,"Can not query %s\n",oldindex);
    usage(prog,1);
  }
  const char *newindex=argv[2];
  if(bundled_only)
  {
    newindex=argv[1];
  }else if(stat(newindex,&statbuf)>=0)
  {
    fprintf(stderr,"%s already exists\n",newindex);
    usage(prog,1);
    char *ext=strrchr(newindex,'.');
    if(!ext ||(!strcmp(ext,".djvu")&&!strcmp(ext,".djv")))
    {
      fprintf(stderr,"WARNING: The new index name %s does not have a .djvu extension.\n",newindex);
    }
  }
  TRY {
    GString file_name=oldindex;
    GMap<GURL,GString>filenames;
    GP<DjVuDocument> doc=new DjVuDocument;
    doc->init(GOS::filename_to_url(file_name),0,0,&filenames);
//    int pages=doc->get_pages_num();
    if((doc->get_doc_type()==DjVuDocument::BUNDLED)||(doc->get_doc_type()==DjVuDocument::INDIRECT))
    {
      fputs("This document is already in the new format.\n",stderr);
    }else if((!bundled_only)||(doc->get_doc_type() == DjVuDocument::OLD_BUNDLED))
    {
      TempDir tmp(newindex,TempDir::REMOVE_ALL);
      const char *basename=strrchr(newindex,'/');
      if(!basename++)
        basename=newindex;

      char *where=new char [strlen(tmp)+strlen(basename)+2];
      sprintf(where,"%s/%s",(const char *)tmp,basename);
      if(log) fprintf(log,"Saving new document as: %s\n",where);
      if(doc->get_doc_type() == DjVuDocument::OLD_BUNDLED)
      {
        doc->save_as(where,1);
      }else if(doc->get_doc_type() == DjVuDocument::OLD_INDEXED)
      {
        doc->save_as(where,0);
      }else
      {
        fprintf(stderr,"%s is an unrecognized format\n",oldindex);
        usage(prog,1);
      }
      delete [] where;
      doc=0;
      TempDir save(oldindex,TempDir::RENAME_ALL);
      for (GPosition i = filenames; i; ++i)
      {
        save.addtotmp(filenames[i],log);
      }
      status=((tmp.RenameAll(log)>=0)&&(save.RemoveAll(log)>=0))?0:1;
    }else
    {
      fputs("Too few arguments\n",stderr);
      usage(prog,1);
    }
  } 
  CATCH(ex)
  {
    ex.perror("Test");
  }
  ENDCATCH;
  exit(status);
}

