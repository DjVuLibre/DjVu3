#ifdef __cplusplus
//C- This software may only be used by you under license from AT&T
//C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
//C- AT&T's Internet web site having the URL <http://www.djvu.att.com/open>.
//C- If you received this software without first entering into a license with
//C- AT&T, you have an infringing copy of this software and cannot use it
//C- without violating AT&T's intellectual property rights.
//C-
//C- $Id: parseoptions.h,v 1.1 1999-11-02 21:44:58 bcr Exp $

#endif /* __cplusplus */

#ifndef __DJVUPARSEOPTIONS_H__
#define __DJVUPARSEOPTIONS_H__

#ifdef __cplusplus
#ifdef __GNUC__
#pragma interface
#endif

#include "tokenlist.h"
#include <stdio.h>

#define DEFAULT_STRING "global"

//** @name parseoptions.h \begin{verbatim}                                        
//
// The idea is simply to have one object that we can use to parse arguments
// for all command line programs, and API type function calls.
// For implementing this we declare a DjVuParseOptions class, with C
// wrappers.  It is probably easier to show an example before describing
// the functions:
//
//      // First we define the command line arguments with the
//      // normal option structure defined by getopt(3)
//      // Only I didn't add support for any feature that requires
//      // a non-constant structure.  So we can use a compile time
//      // constant.
// 	static const djvu_option long_options[] = {
// 	  {"favorite-color",1,0,'f'},
// 	  {"pizza-topping",1,0,'P'},
// 	  {"bignumber",1,0,'n'},
// 	  {"redo",0,0,'r'},
// 	  {"profile",0,0,'p'}
// 	};
//
// 	int
//  	main(int argc,char **argv,char **env)
//      {
// 	  char *profile,*topping,*color;
//        int redo,bignumber;
// 		// This will read in the default configuration file
// 		// which is either ~/.DjVu/global.conf if it exists,
// 		// otherwise /etc/DjVu/global.conf.
// 		// Next the file MyConfigFile.conf will be read from
// 		// either ~/.DjVu or /etc/DjVu
//        DjVuParseOptions MyOptions("MyConfigFile");
// 		// This parses the command line arguments.
//        MyOptions.ParseArguments(argc,argv,long_options);
// 		// We could for example, check the value of a profile
// 		// option, and switch which profile we are using.
// 	  profile=MyOptions.GetValue("profile");
//        if(profile) MyOptions.ChangeProfile(profile);
// 		// Now we can read in the variables.
// 		// I find it is a good idea to only list the variable
// 		// strings once, so I'm passing them from the long_options
// 		// structure.  But this isn't necessary.
// 	  topping=MyOptions.GetValue(long_options[0].name);
// 	  color=MyOptions.GetValue(long_options[1].name);
// 		// We can also read in integers, and specify a default value
// 		// to return in case the value is not an integer.  In this
// 		// case I specified -1.
// 	  bignumber=MyOptions.GetInteger(long_options[2].name,-1);
// 		// We can also check true/false type values.
// 	  redo=MyOptions.GetInteger(long_options[2].name,0);
// 		// Now before we do anything with these values we might
// 		// want to check for errors.
// 	  if(MyOptions.HasError())
// 	  {
// 	     MyOptions.perror();
// 	     usage();
// 	  }
//
// 	  ... Now we can do the real work of this program...
// 	  exit(0);
// 	}
// 	
//
// Now the more complicated issue is using the same object for libraries
// with a minimal amount of changes.  The first issue to note is we don't
// want a library function to re-read the configuration file each time it
// is called.  And we don't want to use static variables, since that could
// causing threading problems.  So, I took the simplest solution, which
// is to assume a copy of the class object will always get once created
// in the first function.
//
// So, then the problem is if we pass by value, then if a function reads
// in it's own profile, each time it is recalled, it will have lost that
// information and need to read in the same profile again.  If we pass by
// references we run into problems that multiple functions can not parse
// argv[] arrays.
//
// The compromise I reached is the profiles, are stored in the class as
// references.  So when you pass by "value", the profile information is
// still passed by reference.  Consequently if you do something like:
// 	int foo(DjVuParseOptions);
// 	DjVuParseOptions Opts("Config");
// 	Opts.ParseArguments(argc,argv,long_opts);
// 	foo(Opts);
// the copy of Opts passed to foo will not contain any information from
// the argv[] array.  But it will contain the profile information.
// If you do a ChangeProfile() it will only effect the original to
// the extent if it does the same ChangeProfile() the configuration
// file will have already been parsed.
//
//
// The core functionality is also available via C wrappers.
//
// The configuration files themselves are fairly simple.  They simple
// listings  of the form:
// 	<varable>=<value>
//
// e.g.
//
// 	foo="689234 ksdf"
// 	fi=bar
// 	test=TRUE
//
// The one unique implementation detail, is alternate profiles may be
// specified  within a configuration file.  The rules is simple.  The
// default profile name is the file name.  To end the current profile
// and begin a new one, simply list the new profile name followed by
// a ':'.  e.g.
//
// 	name=fido
// 	species=dog
// 	birthday=today
// 	
// 	# Lets begin an alternate profile for mycat
//
// 	cats:
// 	name=fluffy
// 	species=cat
// 	birthday=tomorrow
// 	favorite-food=mice
// 	mice-eaten-this-week=5
//
// You can use quotes the same as in a shell script.  If you don't use
// quotes, extra white spaces are stripped, and escape characters are used.
// All of the following are equivalent.
// e.g.
// 	test="This is a simple test.\nThis is only a test.\\"
//
// 	test="This is a simple test.
// 	This is only a test.\\"
//
// 	test= This is a simple test.\ 
// 	\nThis 		is only		a test.\\ 
//
// 	test='This is a simple test.
// 	This is only a test.\'
//
// Be very careful of missing quotes...
//
// \end{verbatim} */
//** @memo parseoptions header file */
//** @version $Id: parseoptions.h,v 1.1 1999-11-02 21:44:58 bcr Exp $ */
//** @author: $Author: bcr $ */

// First we include some C wrappers for our class.
// The purpose of the DjVuParseOptions class, is to give a standard
// way for all DjVu programs and API type functions to access values
// from the command line and from configuration files.
//
// The operations of ChangeProfile(), and the copy constructor are only
// thread safe if you define a THREADMETHOD.
//
extern "C" {
#endif /* __cplusplus */
  /** This structure is identical to most systems options structure. */
  /** As defined for getopt_long(3).                                 */
  struct djvu_option {
    const char *name;
    int has_arg;
    int *flag;
    int val;
  };

    /* This is a wrapper for the C++ DjVuParseOptions class */
  struct djvu_parse
  {
    void *Private;
  };

    /* This is a wrapper for the C++ DjVuParseOptions constructor  */
  struct djvu_parse
  djvu_parse_init(const char []);

    /* This is a wrapper for the C++ DjVuParseOptions constructor  */
  struct djvu_parse
  djvu_parse_copy(const struct djvu_parse);

    /* This is a wrapper for the DjVuParseOptions::ChangeProfile function. */
  void
  djvu_parse_change_profile(struct djvu_parse,const char []);

    /* This is a wrapper for the DjVuParseOptions destructor */
  void
  djvu_parse_free(struct djvu_parse);

    /* This is a wrapper for the DjVuParseOptions::GetValue function */
  const char *
  djvu_parse_value(struct djvu_parse,const char []);

    /* This is a wrapper for the DjVuParseOptions::GetInteger function */
  int
  djvu_parse_integer(struct djvu_parse,const char [],const int);

    /* This is a wrapper for the DjVuParseOptions::ParseArguments function */
  void
  djvu_parse_arguments
  (struct djvu_parse,int,const char *[],const struct djvu_option *);

    /* This is a wrapper for the DjVuParseOptions::HasError function */
  int
  djvu_parse_haserror(struct djvu_parse);

    /* This is a wrapper for the DjVuParseOptions::GetError function */
  const char *
  djvu_parse_error(struct djvu_parse);

    /* This is a wrapper for the DjVuParseOptions::perror function */
  void
  djvu_parse_perror(struct djvu_parse);

#ifdef __cplusplus
};


//@{

class DjVuParseOptions
{
private: // These are just class declarations.
  class ErrorList;
  class Profiles;
  class ProfileList;
  class GetOpt;
  friend GetOpt;

private:
  char *name; // This is the name of the function.
  int defaultProfile;
  int currentProfile;
  ErrorList *Errors;
  DjVuTokenList *VarTokens,*ProfileTokens;
  ProfileList *Configuration;
  Profiles *Arguments;
public:
  //** Normal Destructor.  This uses reference counts to decide when        */
  //** references should be destroyed.                                      */
  ~DjVuParseOptions();

  //** This is the normal constructor.  A profile name must be specified.   */
  DjVuParseOptions(const char []); // This is the normal constructor.

  //** This is a copy constructor.  Arguments, and ErrorLists are not       */
  //** copied.   VarTokens, ProfileTokens, and Configuration are copied by  */
  //** reference, not value.                                                */
  DjVuParseOptions(DjVuParseOptions &); // This is the copy constructor.

  //** This is the only method of reading a new profile, and changing it to */
  //** the current profile that will be used with the Get*() methods.       */
  void ChangeProfile(const char []);

  //** If you wish to retrieve the same variable multiple times, or from    */
  //** multiple profiles, we recommend retrieving the token value for that  */
  //** variable, to avoid repeated lookups of the string.                   */
  //** Negative values are returned for an unregistered variable name.      */
  inline int GetVarToken(const char xname[]) const
  {return VarTokens->GetToken(xname);};

  //** This is the same as GetVarToken() except if a variable name is NOT    */
  //** currently tokenized, a new token is created and returned.  If you are */
  //** Use of this is recommended when the token value is to be stored       */
  //** statically.                                                           */
  inline int SetVarToken(const char xname[]) const
  {return VarTokens->SetToken(xname);};

  //** This is simular to GetVarToken(), but it looks up the token for       */
  //** profile name instead.                                                 */
  inline int GetProfileToken(const char xname[]) const
  {return ProfileTokens->GetToken(xname);};

  //** This is simular to SetVarToken(), but it looks up the token for      */
  //** profile name instead.   Care should be used when using this method.  */
  //** Once a token is assigned to a profile, then the profile is assumed   */
  //** to have been read.  Consequently, if you use this function, and the  */
  //** profile did not exist.  An empty profile will be created...          */
  //** Perhaps this method should be private instead.                       */
  inline int SetProfileToken(const char xname[]) const
  {return ProfileTokens->SetToken(xname);};

  //** This is the reverse transform of the GetVarToken() routine.   */
  //** Given the token, it returns the var name.  This is handy for  */
  //** creating readable messages for log files and such.            */
  inline const char * const GetVarName(const int token) const
  {return VarTokens->GetString(token);};
  
  //** This is the reverse transform of the GetProfileToken() routine.   */
  //** Given the token, it returns the profile name.  This is handy for  */
  //** creating readable messages for log files and such.                */
  inline const char * const GetProfileName(const int token) const
  {return ProfileTokens->GetString(token);};

  //** This is the primary lookup routine.  Input is the token as returned by */
  //** GetToken(), and the return value is the string associated with the     */
  //** token                                                                  */
  const char * const GetValue(const int token) const;

  //** This is just a short cut, when a token value is only needed for one */
  //** lookup.                                                             */
  inline const char * const GetValue(const char xname[]) const
  { return GetValue(GetVarToken(xname)); }

  //** This just checks for TRUE, and if not does an atoi() conversion. */
  //** Anything beginning with [Tt] is returned as 1, [Ff\0] is returned */
  //** as 0, and anything else that is not a legal integer is returned */
  //** as errval.                                                       */
  const int GetInteger(const int token,const int errval=0) const;

  //** This is just a short cut, when a token value is only needed for one */
  //** lookup.                                                             */
  const int GetInteger(const char xname[],const int errval=0) const
  { return GetInteger(GetVarToken(xname),errval); }

  //** This method allows us to check if any errors occurred.           */
  const int HasError() const;

  //** This allows us to retrieve and clear errors one at a time.  A NULL is */
  //** returned when all errors have been cleared.                           */
  const char *GetError();

  //** This deletes and recreates the Errors object, to clear all errors    */
  //** without the need to do a GetError() loop.                            */
  void ClearError();

  //** This simple perror() type function prints all errors to stderr, with */
  //** a GetError() loop, so the errors are cleared.                        */
  void perror();

  //** This is the primary function for reading command  line arguments.  */
  const int ParseArguments(int,const char *[],const djvu_option *);

private:
  void Add(const char [],const int,const int,const int,const char []);
  const int ReadConfig(const char prog[]);
  const int ReadNextConfig(const char filename[],int &,const char prog[],FILE *f);
  void ReadFile(const char [],int &,FILE *f,const int profile);
  void Init(const char[],const int,const char * const [],const djvu_option &);
  FILE *OpenConfig(const char prog[]);
};

//@}

// The following are all classes which are private to the implementation
// of this class.
//

#ifdef _PARSEOPTIONS_H_IMPLEMENTATION 
// This is a double linked list for appending error messages.  This
// is used to queue up error messages, so we can parse all the configuration
// files without interruption, or trying to figure out how to resume if 
// the user wishes to ignore the error.
//
class DjVuParseOptions::ErrorList
{  // This is a double linked list, that wraps around at the beginning.
private:
  class ErrorList *next,*prev;
  char *value,*retvalue;
  ErrorList(class ErrorList *,const char[]);
public:
  ErrorList() : next(0),prev(this),value(0),retvalue(0) {};
  ErrorList(const char[]);
  ~ErrorList();
  const char *AddError(const char value[]);
  const char *GetError(); // Each get removes the error message.
  inline const int HasError() const 
  { return !!value; };
};

// This is just a simple array of strings.  We could just use a structure,
// but some compilers incorrectly treat classes differently from all public
// classes...
//
class DjVuParseOptions::Profiles
{
public:
  int size;
  char **values;
  Profiles() : size(0), values(0) {};
  ~Profiles() {delete [] values;};
  void Add(const int,const char []);
  inline const char * const GetValue(const int token) const
  {return (token<size)?values[token]:0;};
};

// And this is a an array of array of strings...
//
class DjVuParseOptions::ProfileList
{
private:
  int size;
public:
  int links;
  Profiles *profiles;
  ProfileList() : size(0),profiles(0) {};
  ~ProfileList() {delete [] profiles;};
  void Add(const int,const int,const char []);
  int Grow(const int);
  inline const char *GetValue(const int profile ,const int var) const
  { return (profile<size)?(profiles[profile].GetValue(var)):0; };
};

// This is a class for implementing a getopt_long type function.
//
class DjVuParseOptions::GetOpt
{
public:
  int optind;
private:
  DjVuTokenList &VarTokens;
  ErrorList &Errors;
  int argc;
  int nextchar;
  const char **argv,*name;
  char *optstring;
  const djvu_option *long_opts;
public:
  const char *optarg;
  int getopt_long();
  GetOpt(DjVuParseOptions *,const int,const char **,const djvu_option *);
};
#endif _PARSEOPTIONS_H_IMPLEMENTATION 

#endif /* __cplusplus */

#endif /* __DJVUPARSEOPTIONS_H__ */


