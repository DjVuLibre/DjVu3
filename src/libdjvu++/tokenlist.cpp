
#ifdef __GNUC__
#pragma implementation
#endif

#define _DJVUTOKENLIST_H_IMPLEMENTATION_ true
#include "tokenlist.h"
#include <string.h>

static const int inc_size=256;  // This is how large we grow the list each time.

DjVuTokenList::~DjVuTokenList()
{
  int i;
  for(i=0;i<NextToken;i++)
  {
    delete [] Strings[i];
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

