#ifdef __cplusplus
//C- This software may only be used by you under license from AT&T
//C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
//C- AT&T's Internet web site having the URL <http://www.djvu.att.com/open>.
//C- If you received this software without first entering into a license with
//C- AT&T, you have an infringing copy of this software and cannot use it
//C- without violating AT&T's intellectual property rights.
//C-
//C- $Id: parseoptions.h,v 1.24 2000-02-01 04:18:51 bcr Exp $

#endif /* __cplusplus */

#ifndef __DJVUPARSEOPTIONS_H__
#define __DJVUPARSEOPTIONS_H__

#ifdef __cplusplus
// First we include some C wrappers for our class.
// The purpose of the DjVuParseOptions class, is to give a standard
// way for all DjVu programs and API type functions to access values
// from the command line and from configuration files.
//
// The operations of ChangeProfile(), and the copy constructor are only
// thread safe if you define a THREADMETHOD.
//
#endif /* __cplusplus */

#include "c-wrappers/DjVu.h"

#ifdef __cplusplus
#ifdef __GNUC__
#pragma interface
#endif /* __GNUC */

#ifndef DJVUPARSEOPTIONS_STANDALONE
#include "DjVuGlobal.h"
#endif /* DJVUPARSEOPTIONS_STANDALONE */

#include <stdio.h>

/** @name parseoptions.h
   The idea is simply to have one object that we can use to parse arguments
   for all command line programs, and API type function calls.  The
   primary class implemented is \Ref{DjVuParseOptions} in the files
   #"parseoptions.h"# and #"parseoptions.cpp"#.  

   Normal usage is to first declare an array of \Ref{djvu_option} structures
   listing all the command line options.  The last element of the array must
   be
\begin{verbatim}
 {0,0,0,0}
\end{verbatim}
   Typically the djvu_option array is declared statically.

   The next step is to declare the \Ref{DjVuParseOptions} class, passing
   it the name of the default profile to read values from.  Profiles are
   normally contained in "/etc/DjVu" or "~/.DjVu" for UNIX.  The location
   is stored in the registry for Windows.  Legal profile variables can be
   any name beginning with a letter consisting of characters [-A-Za-z0-9].
   This includes all command long options of this format in the
   \Ref{djvu_option} structure.
   Finally, you can tell your new \Ref{DjVuParseOptions} object to parse
   the command line arguments and use the class methods to determine the
   value specified for each option.

   For a better understanding of this procedure continue on to the 
   \Ref{DjVuParseOptions Examples}.

   @memo Class used for parsing options and configuration files.
   @author Bill Riemers
   @version #$Id: parseoptions.h,v 1.24 2000-02-01 04:18:51 bcr Exp $#
 */

/*@{*/

/** @name DjVuParseOptions Examples
    The following #DjVuParseOptions examples# demonstrates how to use the
    \Ref{DjVuParseOptions} class in your code.

\begin{verbatim}                                        
        // First we define the command line arguments with the
        // normal option structure defined by getopt(3)
        // Only I didn't add support for any feature that requires
        // a non-constant structure.  So we can use a compile time
        // constant.
        static const char favorite_color_string[]="favorite-color";
        static const char pizza_topping_string[]="pizza-topping";
        static const char bignumber_string[]="bignumber";
        static const char redo_string[]="redo";
        static const char profile_string[]="profile";

   	static const djvu_option long_options[] = {
   	  {favorite_color_string,1,0,'f'},
   	  {pizza_topping_string,1,0,'P'},
   	  {bignumber_string,1,0,'n'},
   	  {redo_string,0,0,'r'},
   	  {profile_string,0,0,'p'},
   	  {0,0,0,0}
   	};
  
   	int
    	main(int argc,const char **argv,char **env)
        {
   	  char *profile,*topping,*color;
          int redo,bignumber;
   		// This will read in the default configuration file
   		// which is either ~/.DjVu/global.conf if it exists,
   		// otherwise /etc/DjVu/global.conf.
   		// Next the file MyConfigFile.conf will be read from
   		// either ~/.DjVu or /etc/DjVu
          DjVuParseOptions MyOptions("MyConfigFile");
   		// This parses the command line arguments.
          MyOptions.ParseArguments(argc,argv,long_options);
   		// We could for example, check the value of a profile
   		// option, and switch which profile we are using.
   	  profile=MyOptions.GetValue(profile_string);
          if(profile) MyOptions.ChangeProfile(profile);
   		// Now we can read in the variables.
   		// I find it is a good idea to only list the variable
   		// strings once, so I use constant strings.  But this
                // isn't required.
   	  topping=MyOptions.GetValue(pizza_topping_string);
   	  color=MyOptions.GetValue(favorite_color_string);
   		// We can also read in integers, and specify a default value
   		// to return in case the value is not an integer.  In this
   		// case I specified -1.
   	  bignumber=MyOptions.GetNumber(bignumber_string,-1);
   		// We can also check true/false type values.
   	  redo=MyOptions.GetInteger(redo_string,0);
   		// Now before we do anything with these values we might
   		// want to check for errors.
   	  if(MyOptions.HasError())
   	  {
   	     MyOptions.perror();
   	     usage();
   	  }
  
   	  ... Now we can do the real work of this program...
   	  exit(0);
   	}
\end{verbatim}
  
   Now the more complicated issue is using the same object for libraries
   with a minimal amount of changes.  The first issue to note is we don't
   want a library function to re-read the configuration file each time it
   is called.  And we don't want to use static variables, since that could
   causing threading problems.  So, I took the simplest solution, which
   is to assume a copy of the class object will always get once created
   in the first function.
  
   So, then the problem is if we pass by value, then if a function reads
   in it's own profile, each time it is recalled, it will have lost that
   information and need to read in the same profile again.  If we pass by
   references we run into problems that multiple functions can not parse
   argv[] arrays.
  
   The compromise I reached is the profiles, are stored in the class as
   references.  So when you pass by "value", the profile information is
   still passed by reference.  Consequently if you do something like:
\begin{verbatim}                                        
   	int foo(DjVuParseOptions);
   	DjVuParseOptions Opts("Config");
   	Opts.ParseArguments(argc,argv,long_opts,long_only);
   	foo(Opts);
\end{verbatim}
   the copy of Opts passed to foo will not contain any information from
   the argv[] array.  But it will contain the profile information.
   If you do a ChangeProfile() it will only effect the original to
   the extent if it does the same ChangeProfile() the configuration
   file will have already been parsed.
  
  
   The core functionality is also available via C wrappers.
  
   The configuration files themselves are fairly simple.  They simple
   listings  of the form:
\begin{verbatim}                                        
   	<varable>=<value>
\end{verbatim}
  
   e.g.
\begin{verbatim}                                        
   	foo="689234 ksdf"
   	fi=bar
   	test=TRUE
\end{verbatim}
  
   The one unique implementation detail, is alternate profiles may be
   specified  within a configuration file.  The rules is simple.  The
   default profile name is the file name.  To end the current profile
   and begin a new one, simply list the new profile name followed by
   a ':'.  e.g.
  
\begin{verbatim}                                        
   	name=fido
   	species=dog
   	birthday=today
   	
   	# Lets begin an alternate profile for mycat
  
   	cats:
   	name=fluffy
   	species=cat
   	birthday=tomorrow
   	favorite-food=mice
   	mice-eaten-this-week=5
\end{verbatim}
  
   You can use quotes the same as in a shell script.  If you don't use
   quotes, extra white spaces are stripped, and escape characters are used.
   All of the following are equivalent.
   e.g.
\begin{verbatim}                                        
   	test="This is a simple test.\nThis is only a test.\\"
  
   	test="This is a simple test.
   	This is only a test.\\"
  
   	test= This is a simple test.\ 
   	\nThis 		is only		a test.\\ 
  
   	test='This is a simple test.
   	This is only a test.\'
\end{verbatim}
  
   Be very careful of missing quotes...
   @memo Examples using the \Ref{DjVuParseOptions} class
 */

class DjVuTokenList;

/** Main class for parsing options.
    #DjVuParseOptions# is the only class you really need to declare.  This
    will handle all fo the details of parsing options from the command line
    and configuration files on disk. */

class DjVuParseOptions
{
private: // These are just class declarations.
  class ErrorList;
  class Profiles;
  class ProfileList;
  class GetOpt;
  friend GetOpt;

private:
  char *filename; // This is the name of the function.
  char *name; // This is the name of the function.
  int defaultProfile;
  int currentProfile;
  ErrorList *Errors;
  DjVuTokenList *VarTokens;
  DjVuTokenList *ProfileTokens;
  ProfileList *Configuration;
  Profiles *Arguments;
  int argc;
  char **argv;
  int optind;

public:
/** @name Creation/destruction.
 */
  /*@{*/

  /** Normal Destructor.  This uses reference counts to decide when        
      references should be destroyed. */
  ~DjVuParseOptions();

  /// This is the normal constructor.  A profile name must be specified.   
  DjVuParseOptions(const char []);

  /// This constructor is for using an alternate configuration file 
  DjVuParseOptions(const char [],const char [],DjVuTokenList *VarTokens=0);

  /// This reinitallizes with the above constructor 
  void init(const char [],const char []);

  /** This is a copy constructor.  Arguments, and ErrorLists are not       
      copied.   VarTokens, ProfileTokens, and Configuration are copied by  
      reference, not value. */
  DjVuParseOptions(DjVuParseOptions &);
  /*@}*/

/** @name Accessing options
 */
  /*@{*/
  /** If you wish to retrieve the same variable multiple times, or from    
      multiple profiles, we recommend retrieving the token value for that  
      variable, to avoid repeated lookups of the string.                   
      Negative values are returned for an unregistered variable name. */
  inline int GetVarToken(const char xname[]) const;

  /** This is the same as GetVarToken() except if a variable name is NOT    
      currently tokenized, a new token is created and returned.  If you are 
      Use of this is recommended when the token value is to be stored       
      statically. */
  inline int SetVarToken(const char xname[]) const;

  /** This is similar to GetVarToken(), but it looks up the token for       
      profile name instead. */
  inline int GetProfileToken(const char xname[]) const;

  /** This is similar to SetVarToken(), but it looks up the token for      
      profile name instead.   Care should be used when using this method.  
      Once a token is assigned to a profile, then the profile is assumed   
      to have been read.  Consequently, if you use this function, and the  
      profile did not exist.  An empty profile will be created...          
      Perhaps this method should be private instead. */
  inline int SetProfileToken(const char xname[]) const;

  /** This is the reverse transform of the GetVarToken() routine.   
      Given the token, it returns the var name.  This is handy for  
      creating readable messages for log files and such. */
  inline const char * const GetVarName(const int token) const;
  
  /** This is the reverse transform of the GetProfileToken() routine.   
      Given the token, it returns the profile name.  This is handy for  
      creating readable messages for log files and such. */
  inline const char * const GetProfileName(const int token) const;

  /** This is the primary lookup routine.  Input is the token as returned by 
      GetToken(), and the return value is the string associated with the     
      token.  Multiple tokens may be in an array of the specified listsize. */
  const char * const GetValue(const int token) const;

  /** Multiple tokens may be in an array of the specified listsize.  The   
      index of the token with a value of the highest precedence will be    
      is returned.  Command line arguments have the highest precedence.    
      Default profile values have the lowest precedence.  It is an error   
      to have two values f the same precedence. */
  int GetBest(const int listsize,const int tokens[],bool=false);

  /// Same as above, but -1 terminated 
  inline int GetBest(const int tokens[],bool=false);

  /** This is just a short cut, when a token value is only needed for one 
      lookup.  A list of tokens may be specified as well. */
  inline const char * const GetValue(const char xname[]) const;

  /** Multiple names may be in an array of the specified listsize.  The    
      index of the name with a value of the highest precedence will be     
      is returned.  Command line arguments have the highest precedence.    
      Default profile values have the lowest precedence.  It is an error   
      to have two values f the same precedence. */
  int GetBest(const int listsize,const char * const[],bool=false);

  /// Same as above, but NULL terminated 
  inline int GetBest(const char * const names[],bool=false);

  /** This just checks for TRUE, and if not does an atoi() conversion. 
      Anything beginning with [Tt] is returned as 1, [Ff] is returned 
      as 0, and anything else that is not a legal integer is returned 
      as errval. */
  int GetInteger(const int token,const int errval=0) const;

  /** This is just a short cut, when a token value is only needed for one 
      lookup. */
  inline int GetInteger(const char xname[],const int errval=0) const;

  /** This just checks for valid integer numbers only.  If the string
      contains something other than [-0-9] the supplied errval is returned.
      Otherwise an integer is returned. */
  int GetNumber(const int token,const int errval=0) const;

  /** This is just a short cut, when a token value is only needed for one 
      lookup. */
  inline int GetNumber(const char xname[],const int errval=0) const;

  /*@}*/

/** @name Error handling
 */
  /*@{*/
  /// This method allows us to check if any errors occurred.           
  int HasError() const;

  /** This allows us to retrieve and clear errors one at a time.  A NULL is 
      returned when all errors have been cleared. */
  const char *GetError();

  /** This deletes and recreates the Errors object, to clear all errors    
      without the need to do a GetError() loop. */
  void ClearError();

  /** This simple perror() type function prints all errors to stderr, with 
      a GetError() loop, so the errors are cleared. */
  void perror(const char *mesg=0);
  /*@}*/

/** @name Parsing profiles or arguments
 */
  /*@{*/
  /** This is the only method of reading a new profile, and changing it to 
      the current profile that will be used with the Get*() methods. 
      Returns true if the profile exists. */
  bool ChangeProfile(const char []);

  /// This is the primary function for reading command  line arguments.  
  int ParseArguments(const int,const char * const [],const djvu_option [],const int=0);
  /// These are the arguments sent to ParseArguments
  inline const char * const * get_argv(void) const;

  /// These are the arguments sent to ParseArguments
  inline int get_argc(void) const;

  /// These are the arguments sent to ParseArguments
  inline int get_optind(void) const;

  /// Get the name of the last configuration file corresponding to the profile 
  const char * const ConfigFilename(const char [],int);
 /*@}*/


private:
  void Add(const int,const int,const int,const char []);
  int ReadConfig(const char[],const char []);
  inline int ReadConfig(const char name[]) 
  { return ReadConfig(name,""); }
  int ReadNextConfig(int &,const char prog[],FILE *f);
  void ReadFile(int &,FILE *f,const int profile);
  void Init(const char[],const int,const char * const [],const djvu_option []);
  FILE *OpenConfig(const char prog[]);
  void AmbiguousOptions(const int,const char[],const int,const char[]);
};

 /** Token list.
     This is a class very similar to GMap, only it is limited much more
     limited scope.  It is an associative array "string" to integer.  But
     the integer is assigned uniquely by this class in sequential order.
     This is of use when you want to store items sequentially in an array
     without making the array to large.  This list is always sorted, so
     this class is also useful for creating a sorted unique list of words.

     At some point the TokenList class may be replaced by a wrapper to the
     GMap class.  We will have to evaluate CPU and memory usage to see if
     the GMap replacement would be adequate.

     The DjVuTokenList keeps track of string,integer pairs.  One unique     
     integer is assigned per string.  With the integer range stored from   
     zero to the number of strings present.  This is primarily intended to  
     allow a simple mapping between strings and a fixed size array. */

class DjVuTokenList
{
private:
  int ListSize;
  int NextToken;
  class Entries;
  Entries *Entry;
  char **Strings;
public:
  int links;
  /// Simple Constructor 
  DjVuTokenList() : ListSize(0),NextToken(0),Entry(0),Strings(0),links(0) {};
  /// Simple Destructor 
  ~DjVuTokenList();

  /// Lookup up a string given the token 
  inline const char * const GetString(const int token) const;

  /// Lookup up a token given a string 
  int GetToken(const char name[]) const;

  /// Assign a token if not already assigned and return it given the string 
  int SetToken(const char name[]);

  /// Everybody needs friends 
  friend void DjVuParseOptions::init(const char [],const char []);
};

//@}

// The following are the inline functions declared above:
//
inline int
DjVuParseOptions::GetVarToken
(const char xname[]) const
{return VarTokens->GetToken(xname);}

inline int
DjVuParseOptions::SetVarToken
(const char xname[]) const
{return VarTokens->SetToken(xname);}

inline int
DjVuParseOptions::GetProfileToken
(const char xname[]) const
{return ProfileTokens->GetToken(xname);}

inline int
DjVuParseOptions::SetProfileToken
(const char xname[]) const
{return ProfileTokens->SetToken(xname);}

inline const char * const
DjVuParseOptions::GetVarName
(const int token) const
{return VarTokens->GetString(token);}

inline const char * const
DjVuParseOptions::GetProfileName
(const int token) const
{return ProfileTokens->GetString(token);}

inline int
DjVuParseOptions::GetBest
(const int tokens[],bool requiretrue)
{int i;for(i=0;tokens[i]>=0;i++);return GetBest(i,tokens,requiretrue);}

inline const char * const
DjVuParseOptions::GetValue
(const char xname[]) const
{ return GetValue(GetVarToken(xname)); }

inline int
DjVuParseOptions::GetBest
(const char * const names[],bool requiretrue)
{int i;for(i=0;names[i];i++);return GetBest(i,names,requiretrue);}

inline int
DjVuParseOptions::GetInteger
(const char xname[],const int errval) const
{ return GetInteger(GetVarToken(xname),errval); }

inline int
DjVuParseOptions::GetNumber
(const char xname[],const int errval) const
{ return GetNumber(GetVarToken(xname),errval); }

inline const char * const
DjVuTokenList::GetString
(const int token) const
{ return (token<NextToken)?Strings[token]:0; }

inline const char * const *
DjVuParseOptions::get_argv(void) const
{ return argv; }

inline int
DjVuParseOptions::get_argc(void) const
{ return argc; }

inline int
DjVuParseOptions::get_optind(void) const
{ return optind; }


// The following are all classes which are private to the implementation
// of this class.
//

#ifdef _PARSEOPTIONS_H_IMPLEMENTATION_ 

#define DEFAULT_STRING "global"

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
  inline int HasError() const 
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
  Profiles();
  ~Profiles();
  void Add(const int,const char []);
  inline const char * const GetValue(const int token) const
  {return ((token<size)&&(token>=0))?values[token]:0;};
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
  ProfileList();
  ~ProfileList();
  void Add(const int,const int,const char []);
  int Grow(const int);
  inline const char *GetValue(const int profile ,const int var) const
  { 
      return ((profile<size) && (profile>=0))?(profiles[profile].GetValue(var)):0; 
  };
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
  const char * const *argv;
  const char *name;
  char *optstring;
  const djvu_option *long_opts;
  int long_only;  // This is useful for allowing single - in arguments.
public:
  const char *optarg;
  int getopt_long();
  GetOpt(DjVuParseOptions *,const int,const char * const [],const djvu_option[],const int=0);
  ~GetOpt();
};

class DjVuTokenList::Entries
{
public:
  int Token;
  char *Name;
  Entries() : Token(0),Name(0) {};
};
#endif /* _PARSEOPTIONS_H_IMPLEMENTATION_  */

#endif /* __cplusplus */

#endif /* __DJVUPARSEOPTIONS_H__ */


