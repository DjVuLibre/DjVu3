//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.5)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- The DjVu Reference Library is protected by U.S. Pat. No.
//C- 6,058,214 and patents pending.
//C- 
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C- 
//C- The computer code originally released by LizardTech under this
//C- license and unmodified by other parties is deemed the "LizardTech
//C- Original Code."
//C- 
//C- With respect to the LizardTech Original Code ONLY, and subject
//C- to any third party intellectual property claims, LizardTech
//C- grants recipient a worldwide, royalty-free, non-exclusive license
//C- under patent claims now or hereafter owned or controlled by
//C- LizardTech that are infringed by making, using, or selling
//C- LizardTech Original Code, but solely to the extent that any such
//C- patent(s) is/are reasonably necessary to enable you to make, have
//C- made, practice, sell, or otherwise dispose of LizardTech Original
//C- Code (or portions thereof) and not to any greater extent that may
//C- be necessary to utilize further modifications or combinations.
//C- 
//C- The LizardTech Original Code is provided "AS IS" WITHOUT WARRANTY
//C- OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//C- TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
//C- MERCHANTIBILITY OR FITNESS FOR A PARTICULAR PURPOSE.
// 
// $Id: parseoptions.cpp,v 1.96 2001-10-17 18:56:49 docbill Exp $
// $Name:  $

#include "DjVuGlobal.h"

#ifndef UNDER_CE

#define _PARSEOPTIONS_H_IMPLEMENTATION_ true
#include "parseoptions.h"
#if defined(BUILD_LIB) && defined(WIN32)
#include "DjVuOptions.h"
#endif
#include "DjVuMessage.h"
#include "GURL.h"
#ifdef THREADMODEL
#include "GThreads.h"
#endif //THREADMODEL
#include <string.h>
#if defined (UNIX) || defined (MAC)
#include <unistd.h>
#include <pwd.h>
#else 
#include <tchar.h>
#include <atlbase.h>
#include <windows.h>
#include <winreg.h>
#endif
#include <string.h>
#ifndef UNDER_CE
#include <sys/types.h>
#endif
#include <stdlib.h>
#include <ctype.h>
#include "GString.h"

static const char LocalDjVuDir[] ="/.DjVu/"; // appended to the home directory.
static const char RootDjVuDir[] ="/etc/DjVu/";
static const char ConfigExt[] =".conf";
static const unsigned int local_bufsize=256;
static const char default_string[]=DEFAULT_STRING;
static const char profile_token_string[]="profile:";
static const char profile_token_read_string[]="read";
static const char profile_token_default_string[]="default";

// These are some static functions we need.  They don't need access to class
// variables so, there is no need to mess up the class declarations with them.
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
  retval.Private=new DjVuParseOptions(GNativeString(name));
  return retval; // Return by reference.
}

struct djvu_parse
djvu_parse_config(const char config[],const char name[]) 
{
  struct djvu_parse retval;    
  retval.Private=new DjVuParseOptions(GNativeString(config),GNativeString(name));
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
int
djvu_parse_change_profile(struct djvu_parse opts,const char name[])
{
  return (((DjVuParseOptions *)(opts.Private))->ChangeProfile(GNativeString(name)))?1:0;
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
  return ((DjVuParseOptions *)(opts.Private))->GetValue(GNativeString(name));
}

  /* This is a wrapper for the DjVuParseOptions::GetInteger function */
int
djvu_parse_integer(struct djvu_parse opts,const char name[],const int errval)
{
  return ((DjVuParseOptions *)(opts.Private))->GetInteger(GNativeString(name),errval);
}

  /* This is a wrapper for the DjVuParseOptions::GetNumber function */
int
djvu_parse_number(struct djvu_parse opts,const char name[],const int errval)
{
  return ((DjVuParseOptions *)(opts.Private))->GetNumber(GNativeString(name),errval);
}

  /* This is a wrapper for the DjVuParseOptions::ParseArguments function */
int
djvu_parse_arguments(
  struct djvu_parse opts,
  const int argc,
  char const * const *argv,
  const struct djvu_option *lopts )
{
  DArray<GUTF8String> dargv(argc+1);
  for(int i=0;i<argc;++i)
    dargv[i]=argv[i];
  return ((DjVuParseOptions *)(opts.Private))->ParseArguments(dargv,lopts);
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
  ((DjVuParseOptions *)(opts.Private))->perror(GNativeString(mesg));
  return;
  
}

  /* This is a wrapper for the DjVUParseOptions::ConfigFilename funciton */
const char *
djvu_parse_configfile(struct djvu_parse opts,const char *name)
{
  return ((DjVuParseOptions *)(opts.Private))->ConfigFilename(GNativeString(name));
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
DjVuParseOptions::DjVuParseOptions(const GUTF8String &prog)
: optind(0)
{
  VarTokens=new DjVuTokenList;
  ProfileTokens=new DjVuTokenList;
  Configuration=new ProfileList;
  Arguments=new Profiles;
  profile_token=VarTokens->SetToken(profile_token_string);
  defaultProfile=ReadConfig(default_string);
  const GURL::Filename::UTF8 xurl(prog);
  name=xurl.is_valid()?xurl.fname().downcase():prog.downcase();
#ifdef  WIN32
//  if(namelen > 4 && (!strcmp(name+namelen-4,".exe") || !strcmp(name+namelen-4,".EXE")))
//    name[namelen-4]=0;
#endif
  currentProfile=ReadConfig(name);
}

DjVuParseOptions::DjVuParseOptions (
  const GUTF8String &readfilename,const GUTF8String &readasprofile,DjVuTokenList *Vars)
: optind(0)
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
  name=readasprofile;
  profile_token=VarTokens->SetToken(profile_token_string);
  defaultProfile=ReadConfig(default_string);
  currentProfile=ReadConfig(readfilename,readasprofile);
}

// This is a simple copy constructor.
//
DjVuParseOptions::DjVuParseOptions(
  DjVuParseOptions &Original)
: defaultProfile(Original.defaultProfile),
  currentProfile(Original.currentProfile),
  VarTokens(Original.VarTokens),
  ProfileTokens(Original.ProfileTokens),
  Configuration(Original.Configuration),
  optind(0)
{
  VarTokens->links++;
  ProfileTokens->links++;
  Configuration->links++;
  Arguments=new Profiles;
  name=Original.name;
  profile_token=VarTokens->SetToken(profile_token_string);
  if(Original.filename.length())
  {
    filename=Original.filename;
  }else
  {
    filename=GUTF8String();
  }
}

// This is the corresponding destructor.
//
DjVuParseOptions::~DjVuParseOptions()
{
  if(!VarTokens->links--) delete VarTokens;
  if(!ProfileTokens->links--) delete ProfileTokens;
  if(!Configuration->links--) delete Configuration;
  delete Arguments;
}

// This reinitializes the object by creating a new object, copying the
// arguments and error list into that object, and then stealing the data
// from the new object.  This is the only way to handle something similar
// to make's -f option.
//
void
DjVuParseOptions::init(
  const GUTF8String &readfilename,const GUTF8String &readasprofile)
{
  DjVuParseOptions tmp(readfilename,readasprofile,VarTokens);

  GUTF8String s;  // Append the error messages from the tmp object.
  while((s=tmp.GetError()).length())
  {
    Errors.append(s);
  }
    // Now we copy all the new profiles
  const int i_max=tmp.ProfileTokens->NextToken;
  for(int i=0;i<i_max;i++)
  {
    s=tmp.ProfileTokens->Entry[i].Name;
    int k=tmp.ProfileTokens->Entry[i].Token;
    int j=ProfileTokens->GetToken(s);
    if(j<0)
    {
      j=ProfileTokens->SetToken(s);
      (void)(Configuration->Grow(j+1));
      delete [] Configuration->profiles[j].values;
    }
    Configuration->profiles[j].size=tmp.Configuration->profiles[k].size;
    Configuration->profiles[j].values=tmp.Configuration->profiles[k].values;
    tmp.Configuration->profiles[k].size=0;
    tmp.Configuration->profiles[k].values=0;
  }
  name=tmp.name;
  filename=tmp.filename;
}

// This changes the current profile
//
bool
DjVuParseOptions::ChangeProfile(const GUTF8String &prog)
{
  GURL::Filename::UTF8 url(prog);
  if(url.is_valid())
  {
    name=url.fname();
  }else
  {
    name=prog;
  }
  currentProfile=ReadConfig(name);
  const char *s=GetValue(profile_token);
  return (s&&s[0]);
}

// This should be the most frequently used function of this class.
// This could be made inline...
const char * const
DjVuParseOptions::GetValue(
  const int token ) const 
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
DjVuParseOptions::AmbiguousOptions(
  const int token1, const GUTF8String &value1, const int token2, const GUTF8String &value2 )
{
  if(token1 != token2)
  {
    const char *name1=GetVarName(token1);
    const char *name2=GetVarName(token2);
    if(name1 && name2)
    {
      GUTF8String s;
      s.format( ERR_MSG("parseoptions.ambiguous") "\t%s\t%s\t%s\t%s", (const char *)name1,(const char *)value1,(const char *)name2,(const char *)value2 );
      Errors.append(s);
    }
  }
}

// This function is useful when the same option has multiple names.
// The variable index of the highest priority variable will be returned.
// Command line arguments have higher priority than current profile values,
// which are higher priority than default profile values.
int
DjVuParseOptions::GetBest(
  const int listsize, const int tokens[], bool requiretrue )
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
      if((r=Configuration->GetValue(currentProfile,tokens[i])))
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
        if((r=Configuration->GetValue(defaultProfile,tokens[i])))
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
DjVuParseOptions::GetBest(
  const int listsize, const char * const xname[], bool requiretrue )
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
// 't' is always returned as trueval.
// Any string starting with 'F', 'f', or '\0' is returned as falseval.
// Otherwise we parse for a number.  In the even of failure, the errval
// is returned.
//
int
DjVuParseOptions::GetInteger(
  const int token,const int errval,const int falseval,const int trueval) const 
{
  const char * const str=GetValue(token);
  int retval=errval;
  if(str)
  {
    if((str[0] == 'T')||(str[0] == 't'))
    {
      retval=trueval;
    }else if((!str[0])||(str[0] == 'F')||(str[0] == 'f'))
    {
      retval=falseval;
    }else 
    {  // We should try and detect errors.
      const char *s=str;
      const char *endptr=s;    // any non-null value will do
      if(s[0])
      {
        for(;isspace(s[0]);s++);
        if(s[0] == '+')
          s++;
        for(retval=(int)strtol(s,(char **)&endptr,10);isspace(endptr[0]);endptr++);
      }
      if(*endptr)
      {
        const_cast<GList<GUTF8String> &>(Errors).append(GUTF8String( ERR_MSG("parseoptions.bad_value") "\t") + str);
        retval=errval;
      }
    }
  } 
  return retval;
}

// This does a simple strtol() conversion.  If the string contains a legal
// number value, that value will be returned.  If the string contains anything
// else, excluding white space, the errval supplied will be returned.
//
int
DjVuParseOptions::GetNumber(
  const int token,const int errval) const 
{
  int retval=errval;
  const char * const str=GetValue(token);
  if(str)
  {
    const char *s=str;
    const char *endptr=s;
    if(s[0])
    {
      for(;isspace(s[0]);s++);
      if(s[0] == '+')
        s++;
      for(retval=(int)strtol(s,(char **)&endptr,10);isspace(endptr[0]);endptr++);
    }
    if(*endptr)
    {
      const_cast<GList<GUTF8String> &>(Errors).append(GUTF8String( ERR_MSG("parseoptions.bad_number") "\t") + str);
      retval=errval;
    }
  }
  return retval;
}

// This function parses the command line arguments
//
int
DjVuParseOptions::ParseArguments(
  const DArray<GUTF8String> &xargv,
  const djvu_option opts[],
  const int long_only
)
{
  const int xargc=xargv.hbound()+1;
  GetOpt args(this,xargv,opts,long_only);
  int i;
  while((i=args.getopt_long())>=0)
  {
    int v,j;
    const char *s=opts[i].name;
    if(s && (j=strlen(s)) && (v=VarTokens->GetToken(opts[i].name))>=0)
    {
#ifdef SUPPRESS_VERBOSE
      if( strcmp( "verbose", s ) != 0 )
#endif
      Arguments->Add(v,args.optarg?args.optarg:"TRUE");
    }
  }
  argv.empty();
  if(xargc)
  {
    argv.resize(0,xargc-1);
    optind=args.optind; 
    for(int argc=0,i=0;argc<xargc;)
    {
      argv[argc++]=xargv[i++];
    }
  }
  return args.optind;
}


int
DjVuParseOptions::HasError() const 
{
  return ((GPosition)Errors)?1:0;
}

const char *
DjVuParseOptions::GetError()
{
  GPosition pos=Errors;
  if(pos)
  {
    PrevError=Errors[pos];
    Errors.del(pos);
  }else
  {
    PrevError.empty();
  }
  return PrevError;
}

void DjVuParseOptions::ClearError()
{
  Errors.empty();
}

void DjVuParseOptions::perror(const GUTF8String &mesg)
{
  GUTF8String s;
  while((s=GetError()).length())
  {
    if(mesg.length())
    {
      DjVuPrintErrorUTF8("%s\n",(const char *)DjVuMessage::LookUpUTF8(mesg+"\v"+s));
    }else
    {
      DjVuPrintErrorUTF8("%s\n",(const char *)DjVuMessage::LookUpUTF8(s));
    }
  }
}

// This is a private function for adding a new value to a the specified pair
// of profile and variable tokens.
//
void
DjVuParseOptions::Add(
  const int line,const int profile,const int var,const GUTF8String &value)
{
  if(var<0)
  {
    static const char emesg1[]= ERR_MSG("parseoptions.illegal_profile");
    static const char emesg2[]= ERR_MSG("parseoptions.profile_error");
    const char *emesg;
    if(value.search(':')>= 0)
    {
      emesg=emesg1;
    }else
    {
      emesg=emesg2;
    }
    const char *p=GetProfileName(profile);
    Errors.append(GUTF8String(emesg) + "\t" + filename +
                                      "\t" + GUTF8String(line) + 
                                      "\t" + p + 
                                      "\t" + value);
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
DjVuParseOptions::ReadConfig(const GUTF8String &prog,
  const GUTF8String &readasprofile)
{
#ifdef THREADMODEL
#if THREADMODEL!=NOTHREADS
  static GCriticalSection func_lock;
  GCriticalSectionLock lk(&func_lock);
#endif /* THREADMODEL */
#endif
  int retval;
  int line=1;
  if(readasprofile.length())
  {
    filename=prog;
    retval=ProfileTokens->SetToken(readasprofile);
    (void)(Configuration->Grow(retval+1));
    GURL::Filename::UTF8 url(filename);
    FILE *f=0;
    if(url.is_valid() && !url.is_empty() && url.is_file() &&
  	  (f=fopen((const char *)url.NativeFilename(),"r")))
    {
      Configuration->Add(retval,profile_token,profile_token_default_string);
      ReadFile(line,f,retval);
      fclose(f);
    }else
    {
      Configuration->Add(retval,profile_token,"");
    }
  }else
  {
	const GURL::Filename::UTF8 xurl(prog);
    const GUTF8String xname((xurl.is_valid()&&!xurl.is_empty())?xurl.fname():prog);
    retval=ProfileTokens->GetToken(xname);
  // First check and see if we have already read in this profile.
    if(retval < 0)
    {
      retval=ProfileTokens->SetToken(xname);
      (void)(Configuration->Grow(retval+1));
      FILE *f=0;
      if(ConfigFilename(xname) && filename.length())
	  {
	    GURL::Filename::UTF8 url(filename);
	    if(url.is_valid() && !url.is_empty() && url.is_file() &&
	  	  (f=fopen((const char *)url.NativeFilename(),"r")))
		{
          Configuration->Add(retval,profile_token,profile_token_read_string);
          ReadFile(line,f,retval);
          fclose(f);
		}else
		{
          Configuration->Add(retval,profile_token,"");
		}
      }else
      {
        Configuration->Add(retval,profile_token,"");
      }
    }
  }
  return retval;
}

// Reads in the specified configuration file if it hasn't been read yet.
// Again this is private, since we don't want to read files multiple times.
// 
int
DjVuParseOptions::ReadNextConfig (
  int &line, const GUTF8String &profilename, FILE *f )
{
  const GURL::Filename::UTF8 xurl(profilename);
  const GUTF8String xname(xurl.is_valid()?xurl.fname():profilename);
  // First check and see if we have already read in this profile.
  int profile=ProfileTokens->GetToken(xname);
//  const char * const s=(profile>0)?Configuration->GetValue(profile,profile_token):0;

    // We parse the rest of the current line so we can "inherit"
    // values from other profiles.
  static const int buf_size=1024;
  char *buf=new char[buf_size];
  int cur_buf_size=buf_size;
  fgets(buf,buf_size,f);
  for(int i=strlen(buf),j=0;(i != j)&&(buf[i-1] != '\n');i=strlen(buf))
  {
    char *newbuf=new char[(cur_buf_size+=buf_size)];
    strcpy(newbuf,buf);
    delete [] buf;
    buf=newbuf;
    fgets(buf+i,buf_size,f);
    j=i;
  }
//  if(!s||strcmp(s,profile_token_read_string))
  {
    profile=ProfileTokens->SetToken(xname);
    (void)(Configuration->Grow(profile+1));
    Configuration->Add(profile,profile_token,profile_token_read_string);
      // First we inherit variables from known profiles.
    char *ptr;
    for(ptr=buf;ptr[0];)
    {
      for(;*ptr&&isspace(*ptr);ptr++);
      if(*ptr)
      {
        const char *name=ptr;
        for(;*ptr&&!isspace(*ptr);++ptr);
        if(*ptr)
          *ptr++=0;
        if(name[0])
        {
          const int inherit_profile=ProfileTokens->GetToken(name);
          if(inherit_profile >= 0)
          {
            int &oldsize=Configuration->profiles[profile].size;
            const int newsize=Configuration->profiles[inherit_profile].size;
            char **inherit_values=Configuration->profiles[inherit_profile].values;
            char **&values=Configuration->profiles[profile].values;
            if(oldsize < newsize)
            {
              char **NewValues=new char *[newsize];
              if(oldsize)
                memcpy(NewValues,values,oldsize*sizeof(char *));
              memset(NewValues+oldsize,0,(newsize-oldsize)*sizeof(char *));
              memset(values,0,oldsize*sizeof(char *));
              delete [] values;
              values=NewValues;
              oldsize=newsize;
            }
            for(int k=0;k<newsize;k++)
            {
              if(inherit_values[k])
              {
		            delete [] values[k];
                char *s=new char [strlen(inherit_values[k])+1];
                strcpy(s,inherit_values[k]);
                values[k]=s;
              }
            }
          }else
          {
            Errors.append(GUTF8String( ERR_MSG("parseoptions.cant_inherit") ) + 
                                                                    "\t" + filename +
                                                                    "\t" + GUTF8String(line) +
                                                                    "\t" + profilename +
                                                                    "\t" + name);
//            break;
          }
        }
      }
    }
    delete [] buf;
     // Now we can read the new variables.
//  if(!feof(f))
//    ReadFile(line,f,profile);
    if(feof(f))
      profile=(-1);
  }
//  else if(f)
//  {
//    delete [] buf;
//    profile=(-1);
//  }
  return profile;
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
//  10. You can use single quotes to make everything exactly as is.  e.g. "\\"
//      would remain "\\", and not be parsed as an escape character for a
//      single "\"
//  In otherwords, it is very similar to parsing a typical ~/.profile file
//  that only sets variable names.

void
DjVuParseOptions::ReadFile(int &line,FILE *f,int profile)
{
  int c;
  char *value=new char[local_bufsize];
  char *value_end=value+local_bufsize;
//  while((profile >= 0)&&((c=getc(f))!=EOF))
  while(((c=getc(f))!=EOF))
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
              for(;isalnum(c)||(c == '-')||(c == '.')||(c == '_');c=getc(f))
              {
                (s++)[0]=c;
                if(s==value_end) break;
              }
              if(s==value_end) break;
              s[0]=0; // Mark the end of the variable name.
                // Skip spaces...
              for(;(c!=EOF)&&isspace(c);c=getc(f))
              { 
                if(c=='\n')
                  line++;
              }
                // Test the first non-space value.
              switch(c)
              {
                case ':': // This indicates the start of a new profile...
                  if(value[0] &&(c!=EOF))
                  {
//                    ReadNextConfig(line,value,f);
                    profile=ReadNextConfig(line,value,f);
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
                  if(s != (s_end+1))
                  {
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


// These constructors and destructors are as simple as it gets.
DjVuParseOptions::ProfileList::ProfileList()
: size(0),links(0),profiles(0) {}

DjVuParseOptions::ProfileList::~ProfileList()
{delete [] profiles;};

// Makes sure the profile list is increased to the specified size.
// Returns 1 if this is a new profile, and 0 if it is an existing one.
//
int
DjVuParseOptions::ProfileList::Grow(const int newsize)
{
  int retval=0;
  if(newsize > size)
  {
    Profiles *NewProfiles=new Profiles[newsize];
    int i;
    for(i=0;i<size;i++)
    {
      NewProfiles[i].size=profiles[i].size;
      NewProfiles[i].values=profiles[i].values;
    }
    Profiles *OldProfiles=profiles;
    const int oldsize=size;
    profiles=NewProfiles;
    size=newsize;
    for(i=0;i<oldsize;i++)
    {
      OldProfiles[i].values=0,
      OldProfiles[i].size=0;
    }
    delete [] OldProfiles;
    retval=1;
  }
  return retval;
}

void
DjVuParseOptions::Profiles::Add(const int var,const GUTF8String &value)
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
    strcpy(values[var]=new char [value.length()+1],value);
  }
}

void
DjVuParseOptions::ProfileList::Add(
  const int profile,const int var,const GUTF8String &value)
{
  if((profile>=0) && (var>=0))
  {
    (void)Grow(profile+1);
    profiles[profile].Add(var,value);
  }
}

#ifdef WIN32
static LPSTR 
RegOpenReadConfig ( HKEY hParentKey )
{


   // To do:  This needs to be shared with SetProfile.cpp
  LPCTSTR path = TEXT("Software\\LizardTech\\DjVu\\Profile Path") ;

  HKEY hKey = NULL;
  if (RegOpenKeyEx(hParentKey, path, 0,
              KEY_READ, &hKey) == ERROR_SUCCESS )
  {
    // Success
    LPSTR szPathValue = new char[1024];
    LPCTSTR lpszEntry = TEXT("");
    DWORD dwCount = 100;
    DWORD dwType;

    LONG lResult = RegQueryValueEx(hKey, lpszEntry, NULL,
             &dwType, (LPBYTE) szPathValue, &dwCount);

    RegCloseKey(hKey);

    if ((lResult == ERROR_SUCCESS))
    {
        szPathValue[dwCount] = 0;
        return szPathValue ;
    }
    delete [] szPathValue;
    return 0;
  } 
  if (hKey)  RegCloseKey(hKey); 
  return 0;

}

static bool
is_dir(TCHAR *filename)
{
   DWORD           dwAttrib;       ;
   dwAttrib = GetFileAttributes(filename) ;
   if (dwAttrib != 0xFFFFFFFF)
    {
      if( dwAttrib & FILE_ATTRIBUTE_DIRECTORY )
      {
        return TRUE ;
      }
    }
   return FALSE;
}
#endif
const char * const
DjVuParseOptions::ConfigFilename(const GUTF8String &config)
{
  filename=GUTF8String();
  GUTF8String this_config(config.length()?(config+ConfigExt):
    (GUTF8String(default_string)+ConfigExt));
  GList<GURL> urls(DjVuMessage::GetProfilePaths());
  for(GPosition pos=urls;pos;++pos)
  {
    GURL::UTF8 url(this_config,urls[pos]);
    if(url.is_valid() && url.is_file())
	{
	  filename=url.UTF8Filename();
      break;
	}
  }
  return filename.length()?((const char *)filename):0;
}

static int
ReadEscape(FILE *f,int &line)
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

DjVuParseOptions::GetOpt::~GetOpt()
{
  delete [] optstring;
}

// Having a prepared optstring is actually a pain, since it makes it more
// difficult to compute longindex.
//
DjVuParseOptions::GetOpt::GetOpt(DjVuParseOptions *xopt,
                                 const DArray<GUTF8String> &xargv,
                                 const djvu_option lopts[],
                                 const int only)
: optind(1),
  VarTokens(*(xopt->VarTokens)),
  Errors(xopt->Errors),
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
//   -r180
// as
//  -1 -r 80
// This is too confusing to the user, and since it varies system to system.
// We couldn't even document it correctly in the manual page.
//

int
DjVuParseOptions::GetOpt::getopt_long()
{
  const int argc=argv.hbound()+1;
  int longindex=1+(strlen(optstring)/2);
  char *optptr;
  optarg=0;
  do 
  {
    if((argc <= optind)||(!argv[optind].length())||(argv[optind][0] != '-'))
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
      if(!strcmp(opts->name,((const char *)argv[optind])+nextchar))
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
      if(!strncmp(ss,((const char *)argv[optind])+nextchar,s)
        &&(argv[optind][nextchar+s] == '='))
      {
        if(opts->has_arg)
          optarg=((const char *)argv[optind])+nextchar+1+s;
        optind++;
        nextchar=1;
        return longindex;
      }
    }
    if(has_dash)
    {
      Errors.append(( ERR_MSG("parseoptions.unrecog_option") "\t")+argv[optind]);
      return -1;
    }
  }
  if((argv[optind][nextchar] == '-')||
    !(optptr=strchr(optstring,argv[optind][nextchar])))
  {
    if(nextchar > 1 || argv[optind][nextchar] != '-' ||
      (argv[optind][nextchar+1]&&argv[optind][nextchar+1]!='-'))
    {
      Errors.append(GUTF8String( ERR_MSG("parseoptions.unrecog_option_c") "\t")
                      + GUTF8String(argv[optind][nextchar]));
      return -1;
    }
    nextchar=1;
    optarg=((const char *)argv[optind])+nextchar;
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
      optarg=((const char *)argv[optind])+nextchar;
    }else if(++optind<argc)
    {
      optarg=argv[optind];
    }else if(opts.has_arg == 2)
    {
      optarg=0;
    }else
    {
      const char * const xname=opts.name;
      Errors.append(GUTF8String( ERR_MSG("parseoptions.missing_arg_option") "\t") + xname);
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
DjVuTokenList::GetToken( const GUTF8String &name ) const
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

DjVuTokenList::Entries::~Entries() {}

int
DjVuTokenList::SetToken( const GUTF8String &name )
{
  int retval;
  if((retval=GetToken(name))<0)
  {
    const int MinGuess=(-1-retval);
  // Allocate a larger buffer, if needed.
    if(NextToken == ListSize)
    {
      Entries *NewEntry=new Entries[(ListSize+=inc_size)];
      memset(&NewEntry[NextToken],0,inc_size*sizeof(char *));
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
      memset(NewStrings,0,ListSize*sizeof(char *));
      memcpy(NewStrings,Strings,NextToken*sizeof(char *));
      memset(Strings,0,NextToken*sizeof(char *));
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
    delete [] Strings[NextToken];
    strcpy((Strings[NextToken]=new char [strlen(name)+1]),name);
    Entry[MinGuess].Name=Strings[NextToken];
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

#endif /* UNDER_CE */

