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
//C- $Id: parseoptions.cpp,v 1.1 1999-11-02 21:44:58 bcr Exp $
#ifdef __GNUC__
#pragma implementation
#endif

#define _PARSEOPTIONS_H_IMPLEMENTATION
#include "parseoptions.h"
#ifdef THREADMODEL
#include "GThreads.h"
#endif THREADMODEL
#include <string.h>
#include <unistd.h>
#include <pwd.h>
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
static FILE *OpenConfigFile(const char[],char **filename);
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

  /* This is a wrapper for the DjVuParseOptions::ParseArguments function */
void
djvu_parse_arguments
(struct djvu_parse opts,int argc,const char *argv[],const struct djvu_option *lopts)
{
  ((DjVuParseOptions *)(opts.Private))->ParseArguments(argc,argv,lopts);
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
djvu_parse_perror(struct djvu_parse opts)
{
  return ((DjVuParseOptions *)(opts.Private))->perror();
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
{
  VarTokens=new DjVuTokenList;
  ProfileTokens=new DjVuTokenList;
  Configuration=new ProfileList;
  Arguments=new Profiles;
  Errors=new ErrorList;
  const char *progname=strrchr(prog,'/');
  progname=progname?(progname+1):prog;
  name=new char [strlen(progname)+1];
  strcpy(name,progname);
  defaultProfile=ReadConfig(default_string);
  currentProfile=ReadConfig(name);
}

// This is a simple copy constructor.
//
DjVuParseOptions::DjVuParseOptions
(DjVuParseOptions &Original)
: defaultProfile(Original.defaultProfile),
  currentProfile(Original.currentProfile),
  VarTokens(Original.VarTokens),
  ProfileTokens(Original.ProfileTokens),
  Configuration(Original.Configuration)
{
  Errors=new ErrorList;
  VarTokens->links++;
  ProfileTokens->links++;
  Configuration->links++;
  Arguments=new Profiles;
  name=new char [strlen(Original.name)+1];
  strcpy(name,Original.name);
}

// This is the corresponding destructor.
//
DjVuParseOptions::~DjVuParseOptions()
{
  if(!VarTokens->links--) delete VarTokens;
  if(!ProfileTokens->links--) delete ProfileTokens;
  if(!Configuration->links--) delete Configuration;
  delete Arguments;
  delete Errors;
  delete [] name;
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

// This does a simple strtol() conversion.  Any string beginning with 'T' or
// 't' is always returned as 1.  Any string starting with 'F', 'f', or '\0'
// is returned as 0.  Otherwise if strtol() is successfull a value is returned.
// In the even of failure, the errval is returned.
//
const int
DjVuParseOptions::GetInteger
(const int token,const int errval) const 
{
  const char * const str=GetValue(token);
  int retval;
  if(!str)
  {
    retval=0;
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

// This function parses the command line arguments
//
const int
DjVuParseOptions::ParseArguments
(const int argc,const char **argv,const djvu_option *opts)
{
  GetOpt args(this,argc,argv,opts);
  int i;
  while((i=args.getopt_long())>=0)
  {
    int v;
    if((opts[i].name)&&((v=VarTokens->GetToken(opts[i].name))>=0))
    {
      Arguments->Add(v,args.optarg?args.optarg:"TRUE");
    }
  }
  return args.optind;
}


const int DjVuParseOptions::HasError() const 
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

void DjVuParseOptions::perror()
{
  const char *s;
  while((s=Errors->GetError()))
  {
    fputs(s,stderr);
    putc('\n',stderr);
  }
}

// This is a private function for adding a new value to a the specified pair
// of profile and variable tokens.
//
void
DjVuParseOptions::Add
(const char filename[],const int line,const int profile,const int var,const char value[])
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
const int
DjVuParseOptions::ReadConfig
(const char prog[])
{
#ifdef THREADMODEL
#if THREADMODEL!=NOTHREADS
  static GCriticalSection locking();
  GCriticalSectionLock(locking);
#endif /* THREADMODEL */
#endif
  const char *xname=strrchr(prog,'/');
  xname=xname?(xname+1):prog;
  const int retval=ProfileTokens->SetToken(xname);
  int profile=retval;
	// First check and see if we have already read in this profile.
  if(Configuration->Grow(profile+1))
  {
    int line=1;
    char *filename;
    FILE *f=OpenConfigFile(xname,&filename);
    if(f)
    {
      ReadFile(filename,line,f,profile);
      fclose(f);
      delete [] filename;
    }
  }
  return retval;
}

// Reads in the specified configuration file if it hasn't been read yet.
// Again this is private, since we don't want to read files multiple times.
// 
const int
DjVuParseOptions::ReadNextConfig
(const char filename[],int &line,const char prog[],FILE *f)
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
    ReadFile(filename,line,f,profile);
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
(const char filename[],int &line,FILE *f,const int profile)
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
                    ReadNextConfig(filename,line,value,f);
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
                    Add(filename,startline,profile,var,value);
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
                  Add(filename,startline,profile,var,value);
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
          Add(filename,startline,profile,-1,r);
          delete [] r;
        }else
        {
          Add(filename,startline,profile,var,value);
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
        memcpy(NewValues,values,size);
        memset(NewValues+size,0,new_size-size);
        delete [] values;
      }else
      {
        memset(NewValues,0,new_size);
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

// This routine searches for a configure file in the user's home directory
// and then the root configuration directory and the and opens it.
// The opened FILE pointer is returned if successfull, otherwise NULL is
// returned.
//
static FILE *
OpenConfigFile
(const char config[],char **filename)
{
  FILE *retval=0;
  static char nill[]="";
  static char *home=0;
  if(config[0])
  {  // First, we find the user's home directory.
    if(!home)
    {
      home=nill;
      struct passwd *pw=getpwuid(getuid());
      if(pw && pw->pw_dir)
      {
        if(pw->pw_dir[0] == '/')
        {
          strcpy(home=new char [strlen(pw->pw_dir)+sizeof(LocalDjVuDir)],pw->pw_dir);
          strcat(home,LocalDjVuDir);
        }
      }else
      {
        char *envhome=getenv("HOME");
        if(envhome && envhome[0] == '/')
        {
          strcpy(home=new char [strlen(envhome)+sizeof(LocalDjVuDir)],envhome);
          strcat(home,LocalDjVuDir);
        }
      }
    }
    if(home != nill)
    {
      char *configfile=new char [strlen(home)+strlen(config)+sizeof(ConfigExt)];
      strcpy(configfile,home);
      strcat(configfile,config);
      strcat(configfile,ConfigExt);
      retval=fopen(configfile,"r");
      if(filename)
      {
        filename[0]=configfile;
      }else
      {
        delete [] configfile;
      }
    }
    if(!retval)
    {
      char *configfile=new char [sizeof(RootDjVuDir)+strlen(config)+sizeof(ConfigExt)];
      strcpy(configfile,RootDjVuDir);
      strcat(configfile,config);
      strcat(configfile,ConfigExt);
      retval=fopen(configfile,"r");
      if(filename)
      {
        filename[0]=configfile;
      }else
      {
        delete [] configfile;
      }
    }
  }
  return retval;
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
  const char **xargv,
  const djvu_option *lopts)
: optind(1),
  VarTokens(*(xopt->VarTokens)),
  Errors(*(xopt->Errors)),
  argc(xargc),
  nextchar(1),
  argv(xargv),
  name(xopt->name),
  optstring(0),
  long_opts(lopts),
  optarg(0)
{
  int i;
  for(i=0;long_opts[i].name;i++)
  {
    VarTokens.SetToken(long_opts[i].name);
  }
  char *s=(optstring=new char[2*i+1]);
  int j;
  for(j=0;j<i;j++)
  {
    const int val=long_opts[j].val;
    if((val>0)&&(val<0xff))
    {
      (s++)[0]=(char)(val&0xff);
      (s++)[0]=(long_opts[j].has_arg)?':':'?';
    }
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
  do 
  {
    if(argc <= optind || !argv[optind] || argv[optind][0] != '-')
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
	if(++optind >= argc || argv[optind][0] != '-')
	  return -1;
      }
    }
  }while(! argv[optind][nextchar]);
  if(nextchar == 1 && argv[optind][nextchar] == '-')
  {
    int s;
    const djvu_option *opts;
    if(!argv[optind][2])
    {
      optind++;
      return -1;
    }
    for(longindex=0,opts=long_opts;opts->name;++opts,++longindex)
    {
      if(!strcmp(opts->name,argv[optind]+2))
      {
        if(!opts->has_arg)
          optarg=0;
        else
          optarg=argv[++optind];
        optind++;
        nextchar=1;
        return longindex;
      }
    }
    for(longindex=0,opts=long_opts;opts->name;++opts,++longindex)
    {
      s=strlen(opts->name);
      if(!strncmp(opts->name,argv[optind]+2,s)
        &&(argv[optind][2+s] == '=')
      ) {
        if(!opts->has_arg)
          optarg=0;
        else
          optarg=argv[optind]+3+s;
        optind++;
        nextchar=1;
        return longindex;
      }
    }
    static const char emesg[]="Unrecognized option '%s'";
    char *ss=new char[sizeof(emesg)+strlen(argv[optind])];
    sprintf(ss,emesg,argv[optind]);
    Errors.AddError(ss);
    delete [] ss;
    return -1;
  }
  optptr=strchr(optstring,argv[optind][nextchar]);
  if(!optptr)
  {
    if(nextchar > 1 || argv[optind][nextchar] != '-')
    {
      static const char emesg[]="Unrecognized option -%c";
      char *s=new char[sizeof(emesg)];
      sprintf(s,emesg,argv[optind][nextchar]);
      Errors.AddError(s);
      delete [] s;
      return -1;
    }
    nextchar=1;
    optarg=&(argv[optind][nextchar]);
    return -1;
  }else
  {
    longindex=((int)(optptr-optstring))/2;
  }
  if(optptr[1] == ':')
  {
    if(argv[optind][++nextchar])
    {
      optarg=&(argv[optind][nextchar]);
    }else if(++optind<argc)
    {
      optarg=argv[optind];
    }else if(optptr[2] == ':')
    {
      optarg=NULL;
    }else
    {
      static const char emesg[]="Argument required for --%s option";
      const char * const xname=long_opts[longindex].name;
      char *s=new char[sizeof(emesg)+strlen(xname)];
      sprintf(s,emesg,xname);
      Errors.AddError(s);
      delete [] s;
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

