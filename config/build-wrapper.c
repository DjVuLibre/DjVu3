#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>

#ifndef RESERVEDLENGTH
#define RESERVEDLENGTH 256
#endif
static const int filenamelen=RESERVEDLENGTH;
#ifndef MAXPREFIXLENGTH
#define MAXPREFIXLENGTH 256
#endif
static const int maxlength=MAXPREFIXLENGTH;

static int testpath(const char *path)
{
  struct stat statbuf;
  int retval=1;
  if((filenamelen>0)&&(maxlength> 0)
    && (strlen(path)>(maxlength+filenamelen)))
  {
    retval=0;
  }else if(stat(path,&statbuf) < 0)
  {
    symlink(".",path);
    if(stat(path,&statbuf) < 0)
    {
      retval=0;
    }
  }
  return retval;
}

char linkpath_base[]="Dj1EdOgYxD9gKCrMiR4GBw";

static char *
get_linkpath(int i)
{
  char *linkpath;
  linkpath=(char *)malloc(sizeof(linkpath_base)+12);
  sprintf(linkpath,"%s%u",linkpath_base,i);
  return linkpath;
}

static void
changepath(const char *prefix,char *buf,int buflen)
{
  if(prefix)
  {
    int i=0;
    char *linkpath=(char *)malloc(sizeof(linkpath_base)+12);
    sprintf(linkpath,"%s%%u",linkpath_base);
    for(i=0;i<buflen;i++)
    {
      int ii=i;
      unsigned int j;
      if((buf[ii] == linkpath[0]) && sscanf(buf+ii,linkpath,&j) >= 1)
      {
        int k=ii,n;
        char *linkpath=(char *)malloc(sizeof(linkpath_base)+12);
        int linkpath_len;
        sprintf(linkpath,"%s%u",linkpath_base,j);
        linkpath_len=strlen(linkpath);
        for(k+=linkpath_len;buf[k] == '/' && !strncmp(buf+k+1,linkpath,linkpath_len);
          k+=linkpath_len+1);
        if((k-ii)<strlen(prefix))
        {
          fprintf(stderr,"Prefix '%s' is too long\n",prefix);
          exit(1);
        }
        for(n=k;buf[n]&&n<buflen;n++) {}
        strcpy(buf+ii,prefix);
        for(ii+=strlen(prefix);k<=n;k++)
        {
          buf[ii++]=buf[k];
        }
        while(ii<=n)
        {
          buf[ii++]=0;
        }
      }
    }
  }
}

static int
copyfile(const char *prefix,char *src,char *dest)
{
  int retval=1;
  int fd=open(src,O_RDONLY);
  if(fd>0)
  {
    struct stat statbuf;
    if(fstat(fd,&statbuf) >= 0)
    {
      char *buf=(char *)malloc(statbuf.st_size+1);
      if(buf)
      {
        int i=0;
        while(i<statbuf.st_size)
        {
          int j=read(fd,buf+i,statbuf.st_size-i);
          if(j<0)
            break;
          i+=j;
        }
        buf[statbuf.st_size]=0;
        close(fd);
        changepath(prefix,buf,statbuf.st_size);
        if((i == statbuf.st_size) && (fd=open(dest,O_WRONLY|O_CREAT,0x600)) >= 0)
        {
          for(i=0;i<statbuf.st_size;)
          {
            int j=write(fd,buf+i,statbuf.st_size-i);
            if(j<0)
              break;
            i+=j;
          }
          fchmod(fd,statbuf.st_mode);
          retval=0;
          close(fd);
        }
        free(buf);
      }
    }
  }
  return retval;
}

static char *
make_path(
  const char linkpath[],
  const char *realpath)
{
  char *path=0,*newpath=0;
  int len=0;
  size_t linkpath_len=strlen(linkpath)+1; 
  if(realpath)
  {
    symlink(realpath,linkpath);
  }
  do
  {
    if(newpath)
    {
      if(path)
        free(path);
      len+=linkpath_len;
      path=newpath;
    }
    if(!(newpath=(char *)malloc(len+linkpath_len)))
    {
      break;
    }
    if(path)
    {
      strcpy(newpath,path);
      strcat(newpath,"/");
      strcat(newpath,linkpath);
    }else
    {
      strcpy(newpath,linkpath);
    }
  } while (testpath(newpath));
  if(newpath)
  {
    free(newpath);
  }
  if(! path)
  {
    unlink(linkpath);
  }else
  {
    int i=len-filenamelen;
    if((maxlength>0) && (i>maxlength))
    {
      i=maxlength;
    }
    for(;i>linkpath_len;--i)
    {
      if(path[i]=='/')
      {
        path[i]=0;
        break;
      } 
    }
    if(i<=linkpath_len)
    {
      unlink(path);
      free(path);
      unlink(linkpath);
      path=0;
    }
  }
  return path;
}

int
main(int argc,char *argv[],char *env[])
{
#ifndef COMMAND
  if(argc > 2)
  {
    if(!strcmp(argv[1],"-prefix"))
    {
      if(argc<5)
      {
        fprintf(stderr,"Usage: %s -prefix <prefix> <input> <output>\n"
          ,argv[0]);
        exit(1);
      }else
      {
        exit(copyfile(argv[2],argv[3],argv[4]));
      }
    }else
#endif
    { 
      int status=1;
      pid_t pid=(-1); 
      char **linkpaths=(char **)malloc(argc*sizeof(char *));
      char **paths=(char **)malloc(argc*sizeof(char *));
      char **argv2=(char **)malloc((2*argc+1)*sizeof(char *));
      int argc2=0;
      int i=1,k=0;
#ifndef COMMAND
      argv2[argc2++]=argv[i++];
#else
      argv2[argc2++]=COMMAND;
#endif
      linkpaths[k]=get_linkpath(k);
      if((paths[k]=make_path(linkpaths[k],0)))
      {
        static const char def_opt[]="-DLT_DEFAULT_PREFIX=\"";
        argv2[argc2]=(char *)malloc(strlen(paths[k])+sizeof(def_opt)+3);
        strcpy(argv2[argc2],def_opt);
        strcpy(argv2[argc2]+sizeof(def_opt)-1,paths[k]);
        strcat(argv2[argc2++],"\"");
        k++;
      }
      for(;i<argc;i++,argc2++)
      {
        if(argv[i][0] == '-' && argv[i][1] == 'L')
        {
          static const char rpath_opt[]="-Wl,-rpath,";
          linkpaths[k]=get_linkpath(k);
          if((paths[k]=make_path(linkpaths[k],argv[i]+2)))
          {
            argv2[argc2]=(char *)malloc(strlen(paths[k])+sizeof(rpath_opt));
            strcpy(argv2[argc2],rpath_opt);
            strcpy(argv2[argc2++]+sizeof(rpath_opt)-1,paths[k]);
            argv2[argc2]=(char *)malloc(strlen(paths[k])+3);
            argv2[argc2][0]=argv[i][0];
            argv2[argc2][1]=argv[i][1];
            strcpy(argv2[argc2]+2,paths[k]);
            k++;
          }else
          {
            free(linkpaths[k]);
            argv2[argc2]=strdup(argv[i]);
          }
        }else
        {
          argv2[argc2]=strdup(argv[i]);
        }
      }
      argv2[argc2]=0;
      pid=fork();
      if(! pid)
      {
        execve(argv2[0],argv2,env);
//        execve(argv2[0],argv+1,env);
        perror(argv2[0]);
        exit(1);
      }else if((pid > 0)&& (waitpid(pid,&status,0) == pid))
      {
        if(WIFSIGNALED(status))
        {
          kill(getpid(),WTERMSIG(status));
        }
        status=(WIFEXITED(status))?(WEXITSTATUS(status)):0;
      }
      for(i=1;i<argc2;i++)
      {
        free(argv2[i]);
      }
      for(i=0;i<k;++i)
      {
        unlink(paths[i]);
        free(paths[i]);
        unlink(linkpaths[i]);
        free(linkpaths[i]);
      }
      exit(status);
    }    
#ifndef COMMAND
  }else
  {
    fprintf(stderr,"Usage: %s <command> <arguments>\n",argv[0]);
    fprintf(stderr,"-or- Usage: %s -prefix <input> <output>\n",argv[0]);
    exit(1);
  }
#endif
}
  
  
