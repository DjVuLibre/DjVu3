//C-  -*- C++ -*-
//C-
//C- Copyright (c) 1999 AT&T Corp.  All rights reserved.
//C-
//C- This software may only be used by you under license from AT&T
//C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
//C- AT&T's Internet website having the URL <http://www.djvu.att.com/open>.
//C- If you received this software without first entering into a license with
//C- AT&T, you have an infringing copy of this software and cannot use it
//C- without violating AT&T's intellectual property rights.
//C-
//C- $Id: parseoptions.cpp,v 1.25 2000-01-11 21:09:38 eaf Exp $
#ifdef __GNUC__
#pragma implementation
#endif

#define _PARSEOPTIONS_H_IMPLEMENTATION_ true
#include "parseoptions.h"
#include <string.h>
#ifdef THREADMODEL
#include "GThreads.h"
#endif THREADMODEL
#ifndef WIN32
#include <unistd.h>
#include <pwd.h>
#else 
#include <windows.h>
#include <winreg.h>
#endif
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>

static const char LocalDjVuDir[] ="/.DjVu/"; // appended to the home directory.
static const char RootDjVuDir[] ="/etc/DjVu/";
static const char ConfigExt[] =".conf";
static const unsigned int local_bufsize=256;
static const char default_string[]=DEFAULT_STRING;

// These are some static functions we need.  They don't need access to class
// variables so, there is no need to messup the class declarations with them.
//
static int ReadEscape(FILE *f,int &line);

//////////////////////////////////////////////////////////////////////////
// First some C wrapper functions
//////////////////////////////////////////////////////////////////////////

  /* This is a wrapper for the C++ DjVuParseOptions constructor  */
struct djvu_parse
djvu_parse_init(const char name[]) 
{
  struct djvu_parse retval;    
  retval.Private=new DjVuParseOptions(name);
  return retval; // Return by reference.
}

struct djvu_parse
djvu_parse_config(const char config[],const char name[]) 
{
  struct djvu_parse retval;    
  retval.Private=new DjVuParseOptions(config,name);
  return retval; // Return by reference.
}

  /* This is a wrapper for the C++ DjVuParseOptions constructor  */
struct djvu_parse
djvu_parse_copy(const struct djvu_parse opts)
{
  struct djvu_parse retval;    
  retval.Private=new DjVuParseOptions(*(DjVuParseOptions *)(opts.Private));
  return retval;
}

  /* This is a wrapper for the DjVuParseOptions::ChangeProfile function. */
void
djvu_parse_change_profile(struct djvu_parse opts,const char name[])
{
  ((DjVuParseOptions *)(opts.Private))->ChangeProfile(name);
}

  /* This is a wrapper for the DjVuParseOptions destructor */
void
djvu_parse_free(struct djvu_parse opts)
{
  delete (DjVuParseOptions *)(opts.Private);
}

  /* This is a wrapper for the DjVuParseOptions::GetValue function */
const char *
djvu_parse_value(struct djvu_parse opts,const char name[])
{
  return ((DjVuParseOptions *)(opts.Private))->GetValue(name);
}

  /* This is a wrapper for the DjVuParseOptions::GetInteger function */
int
djvu_parse_integer(struct djvu_parse opts,const char name[],const int errval)
{
  return ((DjVuParseOptions *)(opts.Private))->GetInteger(name,errval);
}

  /* This is a wrapper for the DjVuParseOptions::GetNumber function */
int
djvu_parse_number(struct djvu_parse opts,const char name[],const int errval)
{
  return ((DjVuParseOptions *)(opts.Private))->GetNumber(name,errval);
}

  /* This is a wrapper for the DjVuParseOptions::ParseArguments function */
int
djvu_parse_arguments
(struct djvu_parse opts,int argc,const char * const argv[],const struct djvu_option lopts[])
{
  return ((DjVuParseOptions *)(opts.Private))->ParseArguments(argc,argv,lopts);
}

  /* This is a wrapper for the DjVuParseOptions::HasError function */
int
djvu_parse_haserror(struct djvu_parse opts)
{
  return ((DjVuParseOptions *)(opts.Private))->HasError();
}

  /* This is a wrapper for the DjVuParseOptions::GetError function */
const char *
djvu_parse_error(struct djvu_parse opts)
{
  return ((DjVuParseOptions *)(opts.Private))->GetError();
}

  /* This is a wrapper for the DjVuParseOptions::perror function */
void
djvu_parse_perror(struct djvu_parse opts,const char *mesg)
{
  ((DjVuParseOptions *)(opts.Private))->perror(mesg);
  return;
  
}

  /* This is a wrapper for the DjVUParseOptions::ConfigFilename funciton */
const char *
djvu_parse_configfile(struct djvu_parse opts,const char *name,int which)
{
  return ((DjVuParseOptions *)(opts.Private))->ConfigFilename(name,which);
}


//////////////////////////////////////////////////////////////////////////
// This is the implimentation for the DjVuParseOptions class.
//////////////////////////////////////////////////////////////////////////
// The purpose is to read the various profiles from the configuration
// files, and to keep them available for access until all copies of
// the class is destroyed, so we don't have to parse the files multiple
// times.
//
// Simple constructor
DjVuParseOptions::DjVuParseOptions 
(const char prog[])
: argc(0),argv(0),optind(0)
{
	filename = 0;
  VarTokens=new DjVuTokenList;
  ProfileTokens=new DjVuTokenList;
  Configuration=new ProfileList;
  Arguments=new Profiles;
  Errors=new ErrorList;
  filename=0;
  defaultProfile=ReadConfig(default_string);
  const char *progname=strrchr(prog,'/');
  progname=progname?(progname+1):prog;
  int namelen=strlen(progname),i;
  name=new char [namelen+1];
  for(i=0;i<namelen;i++)
  {
    name[i]=tolower(progname[i]);
  }
  name[i]=0;
#ifndef  WIN32
  if(namelen > 4 && !strcasecmp(name+namelen-4,".exe"))
#else
  if(namelen > 4 && !strcmp(name+namelen-4,".exe"))
#endif
  {
    name[namelen-4]=0;
  }
  currentProfile=ReadConfig(name);
}

#if 0
DjVuParseOptions::DjVuParseOptions()
: argc(0),argv(0),optind(0)
{
	filename = 0;
  VarTokens=new DjVuTokenList;
  ProfileTokens=new DjVuTokenList;
  Configuration=new ProfileList;
  Arguments=new Profiles;
  Errors=new ErrorList;
  name=new char[1];
  name[0]=0;
  filename=0;
  name=new char [sizeof(default_string)];
  strcpy(name,default_string));
  currentProfile=defaultProfile=ReadConfig(default_string);
}
#endif

DjVuParseOptions::DjVuParseOptions 
(const char readfilename[],const char readasprofile[],DjVuTokenList *Vars)
: argc(0),argv(0),optind(0)
{
  if(Vars)
  {
    (VarTokens=Vars)->links++;
  }else
  {
    VarTokens=new DjVuTokenList;
  }
  ProfileTokens=new DjVuTokenList;
  Configuration=new ProfileList;
  Arguments=new Profiles;
  Errors=new ErrorList;
  name=new char [strlen(readasprofile)+1];
  strcpy(name,readasprofile);
  filename=0;
  defaultProfile=ReadConfig(default_string);
  currentProfile=ReadConfig(readfilename,readasprofile);
}

// This is a simple copy constructor.
//
DjVuParseOptions::DjVuParseOptions
(DjVuParseOptions &Original)
: defaultProfile(Original.defaultProfile),
  currentProfile(Original.currentProfile),
  VarTokens(Original.VarTokens),
  ProfileTokens(Original.ProfileTokens),
  Configuration(Original.Configuration),
  argc(0),
  argv(0),
  optind(0)
{
  Errors=new ErrorList;
  VarTokens->links++;
  ProfileTokens->links++;
  Configuration->links++;
  Arguments=new Profiles;
  name=new char [strlen(Original.name)+1];
  strcpy(name,Original.name);
  if(Original.filename)
  {
    filename=new char [strlen(Original.filename)+1];
    strcpy(filename,Original.filename);
  }else
  {
    filename=0;
  }
}

// This is the corresponding destructor.
//
DjVuParseOptions::~DjVuParseOptions()
{
  if(argv)
  {
    int i;
    for(i=0;i<argc;i++)
    {
      delete argv[i];
      argv[i]=0;
    }
    delete [] argv;
  }
  if(!VarTokens->links--) delete VarTokens;
  if(!ProfileTokens->links--) delete ProfileTokens;
  if(!Configuration->links--) delete Configuration;
  delete Arguments;
  delete Errors;
  delete [] name;
  delete [] filename;
}

// This reiniallizes the object, by creating a new object, copying the
// arguments and error list into that object, and then stealing the data
// from the new object.  This is the only way to handle something simmular
// to make's -f option.
//
void
DjVuParseOptions::init
(const char readfilename[],const char readasprofile[])
{
  DjVuParseOptions tmp(readfilename,readasprofile,VarTokens);

  const char *s;  // Append the error messages from the tmp object.
  while((s=tmp.Errors->GetError()))
  {
    Errors->AddError(s);
  }
    // Now we copy all the new profiles
  int i;
  const int i_max=ProfileTokens->NextToken;
  for(i=0;i<i_max;i++)
  {
    s=tmp.ProfileTokens->Entry[i].Name;
    int j=ProfileTokens->SetToken(s);
    if(!(Configuration->Grow(j+1)))
    {
      delete [] Configuration->profiles[j].values;
    }
    Configuration->profiles[j].size=tmp.Configuration->profiles[i].size;
    Configuration->profiles[j].values=tmp.Configuration->profiles[i].values;
    tmp.Configuration->profiles[i].size=0;
    tmp.Configuration->profiles[i].values=0;
  }
  delete [] name;
  delete [] filename;
  name=tmp.name;
  filename=tmp.filename;
  tmp.name=0;
  tmp.filename=0;
}

// This changes the current profile
//
void
DjVuParseOptions::ChangeProfile(const char prog[])
{
  const char *progname=strrchr(prog,'/');
  progname=progname?(progname+1):prog;
  delete [] name;
  name=new char [strlen(progname)+1];
  strcpy(name,progname);
  currentProfile=ReadConfig(name);
}

// This should be the most frequently used function of this class.
// This could be made inline...
const char * const
DjVuParseOptions::GetValue
(const int token) const 
{
  const char *retval;
  if(!(retval=Arguments->GetValue(token)))
  {
    if(!(retval=Configuration->GetValue(currentProfile,token)))
    {
      retval=Configuration->GetValue(defaultProfile,token);
    }
  }
  return retval;
}

void
DjVuParseOptions::AmbiguousOptions
(const int token1,const char value1[],const int token2,const char value2[])
{
  if(token1 != token2)
  {
    const char *name1=GetVarName(token1);
    const char *name2=GetVarName(token2);
    if(name1 && name2)
    {
      const char emsg[]="Ambiguous options: '%s=%s' and '%s=%s' specified.";
      char *s=new char [sizeof(emsg)+strlen(name1)+strlen(name2)+strlen(value1)+strlen(value2)];
      sprintf(s,emsg,name1,value1,name2,value2);
      Errors->AddError(s);
      delete [] s;
    }
  }
}

// This function is usefull when the same option has multiple names.
// The variable index of the highest priority variable will be returned.
// Command line arguments have higher priority than current profile values,
// which are higher priority than default profile values.
int
DjVuParseOptions::GetBest
(const int listsize,const int tokens[],bool requiretrue)
{
  const char *r=0;
  int retval=(-1);
  int besttoken=(-1);
  int i;
  for(i=0;(i<listsize);i++)
  {
    if((r=Arguments->GetValue(tokens[i])))
    {
      if((!requiretrue)||(r[0]=='T')||(r[0]=='t')||(atoi(r)))
      {
        for(besttoken=tokens[(retval=i++)];i<listsize;i++)
        {
          const char *s=Arguments->GetValue(tokens[i]);
          if(s)
          {
            if((!requiretrue)||(s[0]=='T')||(s[0]=='t')||(atoi(s)))
            {
              AmbiguousOptions(besttoken,r,tokens[i],s);
            }
          }
        }
        break;
      }else
      {
        r=0;
      }
    }
  }
  if(!r)
  {
    for(i=0;(i<listsize);i++)
    {
      if((r=Arguments->GetValue(tokens[i])))
      {
        if((!requiretrue)||(r[0]=='T')||(r[0]=='t')||(atoi(r)))
        {
          for(besttoken=tokens[(retval=i++)];i<listsize;i++)
          {
            const char *s=Configuration->GetValue(currentProfile,tokens[i]);
            if(s)
            {
              if((!requiretrue)||(s[0]=='T')||(s[0]=='t')||(atoi(s)))
              {
                AmbiguousOptions(besttoken,r,tokens[i],s);
              }
            }
          }
          break;
        }else
        {
          r=0;
        }
      }
    }
    if(!r)
    {
      for(i=0;(i<listsize);i++)
      {
        if((r=Arguments->GetValue(tokens[i])))
        {
          if((!requiretrue)||(r[0]=='T')||(r[0]=='t')||(atoi(r)))
          {
            for(besttoken=tokens[(retval=i++)];i<listsize;i++)
            {
              const char *s=Configuration->GetValue(defaultProfile,tokens[i]);
              if(s)
              {
                if((!requiretrue)||(s[0]=='T')||(s[0]=='t')||(atoi(s)))
                {
                  AmbiguousOptions(besttoken,r,tokens[i],s);
                }
              }
            }
            break;
          }else
          {
            r=0;
          }
        }
      }
    }
  }
  return retval;
}

// This function is usefull when the same option has multiple names.
// The variable index of the highest priority variable will be returned.
// Command line arguments have higher priority than current profile values,
// which are higher priority than default profile values.
int
DjVuParseOptions::GetBest
(const int listsize,const char * const xname[],bool requiretrue)
{
  int retval=(-1);
  if(xname && listsize > 0)
  {
    int i,j;
    int *tokens=new int[listsize];
    for(i=j=0;i<listsize;i++)
    {
      tokens[j++]=xname[i]?GetVarToken(xname[i]):(-1);
    }
    retval=GetBest(j,tokens,requiretrue);
    delete [] tokens;
  }
  return retval;
}

// This does a simple strtol() conversion.  Any string beginning with 'T' or
// 't' is always returned as 1.  Any string starting with 'F', 'f', or '\0'
// is returned as 0.  Otherwise if strtol() is successfull a value is returned.
// In the even of failure, the errval is returned.
//
int
DjVuParseOptions::GetInteger
(const int token,const int errval) const 
{
  const char * const str=GetValue(token);
  int retval;
  if(!str)
  {
    retval=errval;
  }else if((str[0] == 'T')||(str[0] == 't'))
  {
    retval=1;
  }else if((!str[0])||(str[0] == 'F')||(str[0] == 'f'))
  {
    retval=0;
  }else 
  {  // We should try and detect errors.
    char *endptr;
    retval=(int)strtol(str,&endptr,10);
    if(errval&&(!retval)&&(*endptr)&&((endptr==str)||!isdigit(*(endptr-1))))
      retval=(errval);
  }
  return retval;
}

// This does a simple strtol() conversion.  If the string contains a legal
// number value, that value will be returned.  If the string contains anything
// else, excluding white space, the errval supplied will be returned.
//
int
DjVuParseOptions::GetNumber
(const int token,const int errval) const 
{
  const char mesg[]="'%1.10s' is not a number";
  int retval=errval;
  const char * const str=GetValue(token);
  if(str)
  {
    const char *s=str;
    const char *endptr=mesg;
    if(s[0])
    {
      for(;isspace(s[0]);s++);
      if(s[0] == '+')
        s++;
      for(retval=(int)strtol(s,(char **)&endptr,10);isspace(endptr[0]);endptr++);
    }
    if(*endptr)
    {
      char sbuf[sizeof(mesg)+10];
      sprintf(sbuf,mesg,str?str:"(NULL)");
      Errors->AddError(sbuf);
      retval=errval;
    }
  }
  return retval;
}

// This function parses the command line arguments
//
int
DjVuParseOptions::ParseArguments
(
  const int xargc,
  const char * const xargv[],
  const djvu_option opts[],
  const int long_only
)
{
  GetOpt args(this,xargc,xargv,opts,long_only);
  int i;
  while((i=args.getopt_long())>=0)
  {
    int v,j;
    const char *s=opts[i].name;
    if(s && (j=strlen(s)) && (v=VarTokens->GetToken(opts[i].name))>=0)
    {
      Arguments->Add(v,args.optarg?args.optarg:"TRUE");
    }
  }
  if(argv)
  {
    for(i=0;i<argc;i++)
    {
      delete argv[i];
      argv[i]=0;
    }
    delete [] argv;
  }
  if(xargc)
  {
    int j;
    if(argc)
    {
      char **oldargv=argv;
      argv=new char *[optind+xargc-1];
      for(i=0;i<optind;i++)
      {
        argv[i]=oldargv[i];
        oldargv[i]=0;
      }
      for(j=i;j<argc;j++)
      {
        delete [] oldargv[j];
        oldargv[j]=0;
      }
      delete [] oldargv;
      argc=optind+xargc-(j=1);
      optind+=args.optind-1;
    }else
    {
      i=j=0;
      argv=new char *[(argc=xargc)];
      optind=args.optind; 
    }
    while(i<argc)
    {
      argv[i]=new char[strlen(xargv[j])+1];
      strcpy(argv[i++],xargv[j++]);
    }
  }
  return args.optind;
}


int
DjVuParseOptions::HasError
() const 
{
  return Errors->HasError();
}

const char *DjVuParseOptions::GetError()
{
  return Errors->GetError();
}

void DjVuParseOptions::ClearError()
{
  delete Errors;
  Errors=new ErrorList;
}

void DjVuParseOptions::perror(const char *mesg)
{
  const char *s;
  while((s=Errors->GetError()))
  {
    if(mesg)
    {
      fprintf(stderr,"%s: %s\n",mesg,s);
    }else
    {
      fputs(s,stderr);
      putc('\n',stderr);
    }
  }
}

// This is a private function for adding a new value to a the specified pair
// of profile and variable tokens.
//
void
DjVuParseOptions::Add
(const int line,const int profile,const int var,const char value[])
{
  if(var<0)
  {
    static const char emesg[]="%s:Error in line %d of profile '%s'\n\t--> %s";
    const char *p=GetProfileName(profile);
    char *s=new char[22+sizeof(emesg)+strlen(p)+strlen(value)+strlen(filename)];
    sprintf(s,emesg,filename,line,p,value);
    Errors->AddError(s);
    delete [] s;
  }else if(profile >= 0)
  {
    Configuration->Add(profile,var,value);
  }
}

// Reads in the specified configuration file if it hasn't been read yet.
// Again this is private, since we don't want to read files multiple times.
// This routine is also non-thread safe, unless a THREADMODEL is defined.
// Since multiple threads may try to grow the profile at the same time. 
// Even if that doesn't happen, one thread may try to access the profile
// while it is still being read.
//
int
DjVuParseOptions::ReadConfig
(const char prog[],const char readasprofile[])
{
#ifdef THREADMODEL
#if THREADMODEL!=NOTHREADS
  static GCriticalSection func_lock;
  GCriticalSectionLock lk(&func_lock);
#endif /* THREADMODEL */
#endif
  int retval;
  int line=1;
  if(readasprofile[0])
  {
    delete [] filename;
    filename=new char [strlen(prog)+1];
    strcpy(filename,prog);
    FILE *f=fopen(filename,"r");
    retval=ProfileTokens->SetToken(readasprofile);
    (void)(Configuration->Grow(retval+1));
    if(f)
    {
      ReadFile(line,f,retval);
      fclose(f);
    }
  }else
  {
#ifndef WIN32
    const char *xname=strrchr(prog,'/');
#else
	const char *xname=strrchr(prog,'\\');
#endif
    xname=xname?(xname+1):prog;

    retval=ProfileTokens->SetToken(xname);
	// First check and see if we have already read in this profile.
    if(Configuration->Grow(retval+1))
    {
      FILE *f=0;
      if(((ConfigFilename(xname,0)&&(f=fopen(filename,"r"))))
         ||((ConfigFilename(xname,1)&&(f=fopen(filename,"r")))))
      {
        ReadFile(line,f,retval);
        fclose(f);
      }
    }
  }
  return retval;
}

// Reads in the specified configuration file if it hasn't been read yet.
// Again this is private, since we don't want to read files multiple times.
// 
int
DjVuParseOptions::ReadNextConfig
(int &line,const char prog[],FILE *f)
{
  const char *xname=strrchr(prog,'/');
  xname=xname?(xname+1):prog;
  const int retval=ProfileTokens->SetToken(xname);
  int profile=retval;
	// First check and see if we have already read in this profile.
  if(!Configuration->Grow(profile+1))
  {
    profile=(-1);
  }else if(f)
  {
    ReadFile(line,f,profile);
  }
  return retval;
}


// For the most part this is yet another configuration file parser.
// The syntax used is:
//    <var1>=<value>
//    <var2>=<value>
//    ...
//    <varN>=<value>
//  <profilename>:
//    <var1>=<value>
//    <var2>=<value>
//
// We use the following syntax rules.
//   1. The first character of a variable name must be a letter.
//   2. Legal variable names consist only of [-A-Za-z0-9] characters.
//   3. Comments begin with '#' and may start anywhere in the line.
//   4. White spaces before and after the variable names are ignored.
//   5. White spaces before the start of value and at the end of the
//      value are ignored.
//   6. White multiple white spaces between words of value are reduced to 1.
//   7. Double quotes, may be used to bypass white space stripping.
//   8. You can continue multiple lines within quotes.
//   9. The standard shell escape characters \n, \r, ... may be used.
//  10. You can use single quotes to make everything exactly as is.  e.g. \\  
//      would remain \\, and not be parsed as an escape character for a
//      single \  
//  In otherwords, it is very simmular to parsing a typical ~/.profile file
//  that only sets variable names.

void
DjVuParseOptions::ReadFile
(int &line,FILE *f,const int profile)
{
  int c;
  char *value=new char[local_bufsize];
  char *value_end=value+local_bufsize;
  while((c=getc(f))!=EOF)
  {
    // Skip spaces...
    if(isspace(c)) 
    {
      if(c == '\n') line++;
      for(c=getc(f);(c!=EOF)&&isspace(c);c=getc(f))
      {
        if(c == '\n')
          line++;
      }
      if(c == EOF) break;
    }
    if(c == '#')
    {
      for(c=getc(f);(c!=EOF);c=getc(f))
      {
        if(c == '\n')
        {
          line++;
          break;
        }
      }
    }else
    {
      char *s=value,*s_end=0;
      int var=(-1);
      int startline=line;
      enum {
        READ_HEAD,
        READ_NAME,
        READ_VALUE_SINGLE_QUOTE,
        READ_VALUE_DOUBLE_QUOTE,
        READ_VALUE,
        READ_EOL
      } state=READ_HEAD;
      value[0]=0;
      for(;c!=EOF;c=getc(f))
      {
        if(c) 
        {
          switch(state)
          {
            case READ_HEAD:
  	      if(isalpha(c))
              {
                state=READ_NAME;
                (s++)[0]=c;
              }else
              {
                state=READ_VALUE;
                value[0]=0;
                (s++)[0]=c;
              }
              break;
            case READ_NAME:
              for(;isalnum(c)||(c == '-');c=getc(f))
              {
                (s++)[0]=c;
                if(s==value_end) break;
              }
              if(s==value_end) break;
              s[0]=0; // Mark the end of the variable name.
                // Skip spaces...
              for(;(c!=EOF)&&isspace(c);c=getc(f)) {if(c=='\n') line++;}
                // Test the first non-space value.
              switch(c)
              {
                case ':': // This indicates the start of a new profile...
                  if(value[0] &&(c!=EOF))
                  {
                    ReadNextConfig(line,value,f);
                    value[0]=0;
                    s=value;
                    state=READ_EOL;
                  }
                  break;
                case '=': // This indicates the start of the value.
                  do{c=getc(f);}while((c!=EOF)&&(c!='\n')&&isspace(c));
                  var=VarTokens->SetToken(value);
                  if(c!=EOF)
                  {
                    state=READ_VALUE;
                    ungetc(c,f);
                  }else
                  {
                    var=(-1);
                    state=READ_EOL;
                    Add(startline,profile,var,value);
                  }
                  s=value;
                  s_end=0;
                  value[0]=0;
                  break;
                default:  // Syntax error
                  state=READ_VALUE;
                  ungetc(c,f);
                  s_end=0;
              }
              break;
            case READ_VALUE_SINGLE_QUOTE:
              if(c == '\47')
              {
                state=READ_VALUE;
                s_end=0;
              }else if(c != '\r')
              {
                if(c == '\n') line++;
                (s++)[0]=c;
              }
              break;
            case READ_VALUE_DOUBLE_QUOTE:
              switch(c)
              {
                case '"':
                  state=READ_VALUE;
                  s_end=0;
                  break;
                case '\r':
                  break;
                case '\\':
                  if((c=ReadEscape(f,line))&(~0xff)) break;
                  // fall through.
                default:
                  if(c == '\n') line++;
                  (s++)[0]=c;
                  break;
              }
              break;
            case READ_VALUE:
              switch(c)
              {
                case '"':
                  state=READ_VALUE_DOUBLE_QUOTE;
                  break;
                case '\47':
                  state=READ_VALUE_SINGLE_QUOTE;
                  break;
                case '#':
                  ungetc(c,f);
                  // Fall through.
                case '\n':
                  line++;
                  if(s == (s_end+1)) // Erase trailing spaces
                  {
                    s_end[0]=0;
                  }else
                  {
                    s[0]=0;
                  }
                  state=READ_EOL;
                  Add(startline,profile,var,value);
                  var=(-1);
                  value[0]=0;
                  break;
                case ' ':
                case '\t':
                    // If there are no quotes, we only keep one space
                    // and even that is never at the end of the line.
                  if(s != (s_end+1)) {
                    s_end=s;
                    (s++)[0]=' ';
                  }
                  break;
                case '\r':
                  break;
                case '\\':
                  if((c=ReadEscape(f,line))&(~0xff)) break;
                  if(c == '\n') break;
                  // fall through.
                default:
                  (s++)[0]=c;
                  break;
              }
            default:
              break;
          }
          if(s == value_end)
          {
            unsigned int value_size=(unsigned int)(value_end-value);
            char *value_new=new char [local_bufsize+value_size];
            strncpy(value_new,value,value_size);
            delete [] value;
            value=value_new;
            value_end=value+value_size+local_bufsize;
            s=value+value_size;
          }
          if(state == READ_EOL) break;
        }
      }
      if(var >= 0)
      {
        if(s == (s_end+1)) // Erase trailing spaces
        {
          s_end[0]=0;
        }else
        {
          s[0]=0;
        }
        if((state == READ_VALUE_SINGLE_QUOTE)||
          (state == READ_VALUE_DOUBLE_QUOTE))
        {
          const char *v=VarTokens->GetString(var);
          char *r=new char [strlen(v)+strlen(value)+2];
          sprintf(r,"%s=%s",v,value);
          Add(startline,profile,-1,r);
          delete [] r;
        }else
        {
          Add(startline,profile,var,value);
        }
      }
    }
  }
  delete [] value;
}


//////////////////////////////////////////////////////////////////////////
// Now we define the private ErrorList Class
//////////////////////////////////////////////////////////////////////////

// This is a private constructor, for adding elements to the linked
// list.
DjVuParseOptions::ErrorList::ErrorList
(ErrorList *old,const char mesg[]) : next(0),prev(old),retvalue(0) 
{
  old->next=this;
  strcpy(value=new char [strlen(mesg)+1],mesg);
}

// This destructor will destroy the everything in the linked list after
// it's location, as well as itself.
DjVuParseOptions::ErrorList::~ErrorList()
{
  delete [] retvalue;
  delete [] value;
  if(prev) prev->next=0;
  delete next;
}


// This is a function for adding more error messages.  Each error message
// is an element of the double linked list.  The new element will be add to
// the end of the list.  We can find this element quickly by using the 
// prev pointer of the first element.
//
const char *
DjVuParseOptions::ErrorList::AddError
(const char mesg[])
{
  char *retval;
  if(value)
  {
    prev=new ErrorList(prev,mesg);
    retval=(prev->value);
  }else
  {
    value=new char[strlen(mesg)+1];
    strcpy((retval=value),mesg);
  }
  return retval;
}

// This function is the reverse of AddError.  Only we peal off the error
// message from the top of the list, and AddError to the bottom.  So something
// like AddError(GetError()) would rotate the end of the list to the top.
//
const char *
DjVuParseOptions::ErrorList::GetError()
{
  delete [] retvalue;
  retvalue=value;
  if(next)
  {
    ErrorList *next_save=next;
    value=next->value;
    next->value=0;
    if((next=next->next))
    {
      next->prev=this;
    }
    next_save->prev=next_save->next=0;
    delete next_save;
  }else
  {
    value=0;
  }
  return retvalue;
}

// These constructors and destructors are as simple as it gets.
DjVuParseOptions::ProfileList::ProfileList()
: size(0),links(0),profiles(0) {}

DjVuParseOptions::ProfileList::~ProfileList()
{delete [] profiles;};

// Makes sure the profile list is increased to the specified size.
// Returns 1 if this is a new profile, and 0 if it is an existing one.
//
int
DjVuParseOptions::ProfileList::Grow
(const int newsize)
{
  int retval=0;
  if(newsize > size)
  {
    Profiles *NewProfiles=new Profiles[newsize];
    int i;
    for(i=0;i<size;i++) {
      NewProfiles[i].size=profiles[i].size;
      NewProfiles[i].values=profiles[i].values;
    }
    Profiles *OldProfiles=profiles;
    const int oldsize=size;
    profiles=NewProfiles;
    size=newsize;
    for(i=0;i<oldsize;i++) {
      OldProfiles[i].values=0,
      OldProfiles[i].size=0;
    }
    delete [] OldProfiles;
    retval=1;
  }
  return retval;
}

void
DjVuParseOptions::Profiles::Add
(const int var,const char value[])
{
  if(var >= 0)
  {
    if(var >= size)
    {
      const int new_size=((var+local_bufsize+1)/local_bufsize)*local_bufsize;
      char **NewValues=new char *[new_size];
      if(size)
      {
        memcpy(NewValues,values,size*sizeof(char *));
        memset(NewValues+size,0,(new_size-size)*sizeof(char *));
        delete [] values;
      }else
      {
        memset(NewValues,0,new_size*sizeof(char *));
      }
      values=NewValues;
      size=new_size;
    }else
    {
      delete [] values[var];
    }
    strcpy(values[var]=new char [strlen(value)+1],value);
  }
}

void
DjVuParseOptions::ProfileList::Add
(const int profile,const int var,const char value[])
{
  if((profile>=0) && (var>=0))
  {
    (void)Grow(profile+1);
    profiles[profile].Add(var,value);
  }
}

#ifdef WIN32
LPSTR RegOpenReadConfig ( HKEY hParentKey) {

  LPCSTR szSoftware = "Software";
  LPCSTR szCompany =  "AT&T";
  LPCSTR szProduct =  "DjVu";
  LPCSTR szProfilePath = "Profile Path";

  HKEY hSoftwareKey = NULL;
  HKEY hCompanyKey = NULL;
  HKEY hProductKey = NULL;
  HKEY hProfilePathKey = NULL;

  if (RegOpenKeyEx(hParentKey, szSoftware, 0,
                    KEY_READ, &hSoftwareKey) == ERROR_SUCCESS )
    if (RegOpenKeyEx(hSoftwareKey, szCompany, 0,
                      KEY_READ, &hCompanyKey) == ERROR_SUCCESS )
      if (RegOpenKeyEx(hCompanyKey, szProduct, 0,
		                KEY_READ, &hProductKey) == ERROR_SUCCESS )
        if (RegOpenKeyEx(hProductKey, szProfilePath, 0,
		                  KEY_READ, &hProfilePathKey) == ERROR_SUCCESS )
        {
          // Success
          LPSTR szPathValue = new char[100];
		  LPCSTR lpszEntry = "";
          DWORD dwCount = 100;
		  DWORD dwType;
          LONG lResult = RegQueryValueEx(hProfilePathKey, lpszEntry, NULL,
		                   &dwType, (LPBYTE) szPathValue, &dwCount);
		  RegCloseKey(hSoftwareKey);
          RegCloseKey(hCompanyKey) ;
          RegCloseKey(hProductKey);
          RegCloseKey(hProfilePathKey);
		  
		  if ( (lResult == ERROR_SUCCESS) && (dwType == REG_SZ)) {
			  return szPathValue ;
		  }
          return 0;
        }

  if (hSoftwareKey) RegCloseKey(hSoftwareKey);
  if (hCompanyKey)  RegCloseKey(hCompanyKey); 
  if (hProductKey)  RegCloseKey(hProductKey); 
  if (hProfilePathKey)  RegCloseKey(hProfilePathKey); 
  return 0;

}
#endif

const char * const
DjVuParseOptions::ConfigFilename
(const char config[],int level)
{
#ifndef WIN32
  const char *retval=0;
  const char *this_config=config[0]?config:default_string;
	if ( filename ) {
  	delete [] filename;
  	filename=0;
	}
  if(level<=0)
  {
    static const char *home=0;
    if(!home)
    {
      if(!((home=getenv("HOME"))&&(home[0]=='/')))
      {
        static const char nill[]="";
        home=nill;
        struct passwd *pw=getpwuid(getuid());
        if(pw && pw->pw_dir)
        {
          if(pw->pw_dir[0] == '/')
          {
            char *s=new char [strlen(pw->pw_dir)+1];
            strcpy(s,pw->pw_dir);
            home=s;
          }
        }
      }
    }
    if(home[0])
    {
      if(level<0)
      {
        retval=home;
      }else 
      {
        retval=filename=new char [strlen(home)+sizeof(LocalDjVuDir)+strlen(this_config)+sizeof(ConfigExt)];
        strcpy(filename,home);
        strcat(filename,LocalDjVuDir);
        strcat(filename,this_config);
        strcat(filename,ConfigExt);
      }
    }
  }else
  {
    static int rootlen=sizeof(RootDjVuDir)-1;
    static const char *root=0;
    if(!root)
    {
      root=getenv("DJVU_CONFIG_DIR");
      if(!(root&&(root[0]=='/')))
      {
        root=RootDjVuDir;
      }else
      {
        rootlen=strlen(root);
      }
    }
    retval=filename=new char [rootlen+strlen(this_config)+sizeof(ConfigExt)];
    strcpy(filename,root);
    strcat(filename,this_config);
    strcat(filename,ConfigExt);
  }
  return retval;
#else

	char * root;
	int rootlen;

	const char *retval=0;
	const char *this_config=config[0]?config:default_string;

	if ( filename ) {
  	delete [] filename;
  	filename=0;
	}

    if (level == 0 ) 
		root = (char *) RegOpenReadConfig (HKEY_CURRENT_USER);
	else
		root = (char *) RegOpenReadConfig (HKEY_LOCAL_MACHINE);
    if( !root )
    {
        char *defl="c:\\";
        retval = filename = new char [strlen(defl)+1];
        strcpy(filename, defl);
        return retval;
    }
	if (root && root[0]) {
		rootlen = strlen(root);
		retval=filename=new char [rootlen+strlen(this_config)+sizeof(ConfigExt)+1];
		strcpy(filename,root);
		strcat(filename, "\\");
		strcat(filename,this_config);
		strcat(filename,ConfigExt);
		return retval;        
	} else {
        const char emsg[]="SDK not installed properly, please install it again.\n";
        char *s=new char [sizeof(emsg)];
        sprintf(s,emsg);
        Errors->AddError(s);
        delete [] s;
		char tempfilename[] = "/windows/djvu";
		filename = new char[sizeof(tempfilename)];
		strcpy ( filename, tempfilename);
		return (retval = filename);
	}
#endif
}

static int
ReadEscape
(FILE *f,int &line)
{
  int c;
  for((c=getc(f));(!c)||(c=='\r');c=getc(f));
  if(c!=EOF) switch(c)
  {
    case 'a':
      c='\a';
      break;
    case 'b':
      c='\b';
      break;
    case 't':
      c='\t';
      break;
    case 'n':
      c='\n';
      break;
    case 'v':
      c='\v';
      break;
    case 'f':
      c='\f';
      break;
    case 'r':
      c='\r';
      break;
    case '1':
    case '2':
    case '3':
    {
      c-='0';
      int t=getc(f);
      if(t>='0'&&t<'8')
      {
        c=(c<<3)+(t-'0');
        t=getc(f);
        if(t>='0'&&t<'8')
        {
          c=(c<<3)+(t-'0');
        }else if(c!=EOF)
        {
          ungetc(c,f);
        }
      }else if(c!= EOF)
      {
        ungetc(c,f);
      }
      break;
    }
    case '4':
    case '5':
    case '6':
    case '7':
    {
      c-='0';
      int t=getc(f);
      if(t>='0'&&t<'8')
      {
        c=(c<<3)+(t-'0');
        t=getc(f);
      }else if(c!= EOF)
      {
        ungetc(c,f);
      }
      break;
    }
    case '\\':
      c='\\';
      break;
    case '\n':
      line++;
      break;
    default:
      break;
  }
  return c;
}

// Having a prepared optstring is actually a pain, since it makes it more
// difficult to compute longindex.
//
DjVuParseOptions::GetOpt::GetOpt
(DjVuParseOptions *xopt,
  const int xargc,
  const char * const xargv[],
  const djvu_option lopts[],
  const int only)
: optind(1),
  VarTokens(*(xopt->VarTokens)),
  Errors(*(xopt->Errors)),
  argc(xargc),
  nextchar(1),
  argv(xargv),
  name(xopt->name),
  optstring(0),
  long_opts(lopts),
  long_only(only),
  optarg(0)
{
  int i;
  const char *ss;
  const djvu_option *opts;
  for(i=0;(ss=long_opts[i].name);i++)
  {
    int j=strlen(ss);
    if(j)
    {
      VarTokens.SetToken(ss);
    }
  }
  char *s=(optstring=new char[2*i+1]);
  for(opts=long_opts;opts->name;opts++)
  {
    const int val=opts->val;
    (s++)[0]=((val>0)&&(val<256))?((char)(val&0xff)):'-';
  }
  (s++)[0]=0;
}


// We don't use the system getopt_long, because some versions of getopt_long
// will interpret 
// 	-r180
// as
//	-1 -r 80
// This is too confusing to the user, and since it varies system to system.
// We couldn't even document it correctly in the manual page.
//

int
DjVuParseOptions::GetOpt::getopt_long()
{
  int longindex=1+(strlen(optstring)/2);
  char *optptr;
  optarg=0;
  do 
  {
    if((argc <= optind)||(!argv[optind])||(argv[optind][0] != '-'))
    {
      return -1;
    }
    if(!argv[optind][nextchar])
    {
      if(nextchar == 1)
      {
        return -1;
      }else
      {
        nextchar=1;
	if((++optind >= argc)||(argv[optind][0] != '-'))
	  return -1;
      }
    }
  }while(! argv[optind][nextchar]);
  const int has_dash=(argv[optind][nextchar] == '-');
  if((nextchar == 1)&&(has_dash || long_only))
  {
    int s;
    if(has_dash) nextchar++;
    const djvu_option *opts;
    if(has_dash&&!argv[optind][2])
    {
      optind++;
      return -1;
    }
    for(longindex=0,opts=long_opts;opts->name;++opts,++longindex)
    {
      if(!strcmp(opts->name,argv[optind]+nextchar))
      {
        if((opts->has_arg)&&(opts->has_arg != 2))
        {
          if(++optind < argc)
          {
            optarg=argv[optind];
            optind++;
          }
        }else
        {
          optind++;
        }
        nextchar=1;
        return longindex;
      }
    }
    for(longindex=0,opts=long_opts;opts->name;++opts,++longindex)
    {
      const char *ss=opts->name;
      s=strlen(opts->name);
      if(!strncmp(ss,argv[optind]+nextchar,s)
        &&(argv[optind][nextchar+s] == '=')
      ) {
        if(opts->has_arg)
          optarg=argv[optind]+nextchar+1+s;
        optind++;
        nextchar=1;
        return longindex;
      }
    }
    if(has_dash)
    {
      static const char emesg[]="Unrecognized option '%1.1024s'";
      char ss[sizeof(emesg)+1024];
      sprintf(ss,emesg,argv[optind]);
      Errors.AddError(ss);
      return -1;
    }
  }
  if((argv[optind][nextchar] == '-')||
    !(optptr=strchr(optstring,argv[optind][nextchar])))
  {
    if(nextchar > 1 || argv[optind][nextchar] != '-' ||
      (argv[optind][nextchar+1]&&argv[optind][nextchar+1]!='-'))
    {
      static const char emesg[]="Unrecognized option -%c";
      char s[sizeof(emesg)];
      sprintf(s,emesg,argv[optind][nextchar]);
      Errors.AddError(s);
      return -1;
    }
    nextchar=1;
    optarg=&(argv[optind][nextchar]);
    return -1;
  }else
  {
    longindex=(int)(optptr-optstring);
  }
  const djvu_option &opts=long_opts[longindex];
  if(opts.has_arg)
  {
    if(opts.has_arg != 3)
    {
      nextchar++;
    }
    if(argv[optind][nextchar])
    {
      optarg=&(argv[optind][nextchar]);
    }else if(++optind<argc)
    {
      optarg=argv[optind];
    }else if(opts.has_arg == 2)
    {
      optarg=0;
    }else
    {
      static const char emesg[]="Argument required for --%1.1024s option";
      const char * const xname=opts.name;
      char s[sizeof(emesg)+1024];
      sprintf(s,emesg,xname);
      Errors.AddError(s);
      return -1;
    }
    optind++;
    nextchar=1;
  }else
  {
    nextchar++;
  }
  return longindex;
}

//////////////////////////////////////////////////////////////////////////////
//  The following is the tokenlist class implementation.
//////////////////////////////////////////////////////////////////////////////
static const int inc_size=256;  // This is how large we grow the list each time.

DjVuTokenList::~DjVuTokenList()
{
  int i;
  for(i=0;i<NextToken;i++)
  {
    delete [] Strings[i];
    Strings[i]=0;
  }
  delete [] Strings;
  delete [] Entry;
}

// This does a bilinear search for the given token, and if it doesn't find
// it, inserts it into the list with a new token value...
//
int
DjVuTokenList::GetToken
(const char name[]) const
{
  int MaxGuess=NextToken;
  int MinGuess=0;
  while(MinGuess<MaxGuess)
  {
    const int guess=MinGuess+((MaxGuess-MinGuess)/2);
    const int i=strcmp(name,Entry[guess].Name);
    if(i<0)
    {
      MaxGuess=guess;
    }else if(i)
    {
      MinGuess=guess+1;
    }else
    {
      return Entry[guess].Token;
    }
  }
  return (-1-MinGuess);
}

int
DjVuTokenList::SetToken
(const char name[])
{
  int retval;
  if((retval=GetToken(name))<0)
  {
    const int MinGuess=(-1-retval);
	// Allocate a larger buffer, if needed.
    if(NextToken == ListSize)
    {
      Entries *NewEntry=new Entries[(ListSize+=inc_size)];
	// Copy the lower entries.
      if(MinGuess)
      {
        memcpy(NewEntry,Entry,sizeof(Entries)*MinGuess);
      }
	// Copy the upper entries.
      if(MinGuess<NextToken)
      {
        memcpy(&(NewEntry[MinGuess+1]),&(Entry[MinGuess]),sizeof(Entries)*(NextToken-MinGuess));
      }
      delete [] Entry;
      Entry=NewEntry;
      char **NewStrings=new char *[ListSize];
      memcpy(NewStrings,Strings,sizeof(char *)*NextToken);
      delete [] Strings;
      Strings=NewStrings;
    }else
    {
      // Move the upper entries.
      if(MinGuess<NextToken)
      {
        memmove(&(Entry[MinGuess+1]),&(Entry[MinGuess]),sizeof(Entries)*(NextToken-MinGuess));
      }
    }
    strcpy((Strings[NextToken]=Entry[MinGuess].Name=new char [strlen(name)+1]),name);
    retval=Entry[MinGuess].Token=NextToken++;
  }
  return retval;
}

DjVuParseOptions::Profiles::Profiles()
: size(0), values(0) {}

DjVuParseOptions::Profiles::~Profiles()
{
  int i;
  for(i=0;i<size;i++)
  {
    delete [] values[i];
    values[i]=0;
  }
  delete [] values;
}


