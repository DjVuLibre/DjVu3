//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 2000-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: DjVuMessage.cpp,v 1.52 2001-05-18 20:25:51 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuMessage.h"
#include "GOS.h"
#include "XMLTags.h"
#include "ByteStream.h"
#include "GURL.h"
#include "debug.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
// #include <stdio.h>
#ifdef WIN32
#include <tchar.h>
#include <atlbase.h>
#include <windows.h>
#include <winreg.h>
#endif
#ifdef UNIX
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#endif
#ifndef UNDER_CE
#include <locale.h>
#endif

static const char namestring[]="name";
static const char valuestring[]="value";
static const char numberstring[]="number";
static const char *failed_to_parse_XML=ERR_MSG("DjVuMessage.failed_to_parse_XML");
static const char bodystring[]="BODY";
static const char headstring[]="HEAD";
static const char includestring[]="INCLUDE";
static const char messagestring[]="MESSAGE";

#ifndef NO_DEBUG
#if defined(UNIX)
  // appended to the home directory.
static const char ModuleDjVuDir[] ="profiles";
  // appended to the home directory.
static const char DebugModuleDjVuDir[] ="../TOPDIR/SRCDIR/profiles";
#elif defined(WIN32)
  // appended to the home directory.
static const char DebugModuleDjVuDir[] ="../../profiles";
#endif
#endif

#ifdef WIN32
  // appended to the home directory.
static const char ModuleDjVuDir[] ="Profiles";
static const char RootDjVuDir[] ="C:/Program Files/LizardTech/Profiles";
static const TCHAR registrypath[]= TEXT("Software\\LizardTech\\DjVu\\Profile Path");
#else
// appended to the home directory.
static const char LocalDjVuDir[] =".DjVu";
static const char RootDjVuDir[] ="/etc/DjVu/";
#endif
static const char DjVuEnv[]="DJVU_CONFIG_DIR";
//  The name of the message file
static const char DjVuMessageFileName[] = "message";
static const char MessageFile[]="messages.xml";

static const char *lookup_id(GUTF8String language,const int start=0);

#if defined(WIN32)
static GURL
RegOpenReadConfig ( HKEY hParentKey )
{
  GURL retval;
   // To do:  This needs to be shared with SetProfile.cpp
  LPCTSTR path = registrypath;

  HKEY hKey = 0;
  // MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,argv[1],strlen(argv[1])+1,wszSrcFile,sizeof(wszSrcFile));
  if (RegOpenKeyEx(hParentKey, path, 0,
              KEY_READ, &hKey) == ERROR_SUCCESS )
  {
    TCHAR path[1024];
    // Success
    TCHAR *szPathValue = path;
    LPCTSTR lpszEntry = (LPCTSTR &)TEXT("");
    DWORD dwCount = (sizeof(path)/sizeof(TCHAR))-1;
    DWORD dwType;

    LONG lResult = RegQueryValueEx(hKey, lpszEntry, NULL,
             &dwType, (LPBYTE) szPathValue, &dwCount);

    RegCloseKey(hKey);

    if ((lResult == ERROR_SUCCESS))
    {
      szPathValue[dwCount] = 0;
      USES_CONVERSION;
      retval=GURL::Filename::Native(T2CA(path));
    }
  } 
//  if (hKey)  RegCloseKey(hKey); 
  return retval;
}

static GURL
GetModulePath( void )
{
  TCHAR path[1024];
  DWORD dwCount = (sizeof(path)/sizeof(TCHAR))-1;
  GetModuleFileName(0, path, dwCount);
  USES_CONVERSION;
  return GURL::Filename::Native(T2CA(path)).base();
}
#elif defined(UNIX) && !defined(NO_DEBUG)
extern char **environ;
static char **e=environ-1;
static GURL
GetModulePath( void )
{
  GURL retval;
  char **argv=e;
  int argc;
  for(argc=0;*(int *)&(argv[-1]) != argc;argc++,argv--)
    EMPTY_LOOP;
  if(argc>=1)
    retval=GURL::Filename::Native(argv[0]).base();
  return retval;
}
#endif

GList<GURL>
DjVuMessage::GetProfilePaths(void)
{
  static bool first=true;
  static GList<GURL> realpaths;
  if(first)
  {
    first=false;
    GList<GURL> paths;
    GURL path;
#ifndef UNDER_CE
    const GUTF8String envp(GOS::getenv(DjVuEnv));
    if(envp.length())
      paths.append((path=GURL::Filename::UTF8(envp)));
#endif
#if defined(WIN32) || (defined(UNIX) && !defined(NO_DEBUG))
    GURL mpath(GetModulePath());
    if(!mpath.is_empty() && mpath.is_dir())
    {
#ifndef NO_DEBUG
      path=GURL::UTF8(DebugModuleDjVuDir,mpath);
      if(!path.is_empty() && path.is_dir())
        paths.append(path);
#endif
      path=GURL::UTF8(ModuleDjVuDir,mpath);
      if(!path.is_empty() && path.is_dir())
        paths.append(path);
      path=mpath.base();
      if(path.pathname() != mpath.pathname())
      {
        path=GURL::UTF8(ModuleDjVuDir,path);
        if(!path.is_empty() && path.is_dir())
          paths.append(path);
      }
    }
#endif
#ifdef WIN32
    path=RegOpenReadConfig (HKEY_CURRENT_USER);
    if(!path.is_empty() && path.is_dir())
      paths.append(path);
    path=(RegOpenReadConfig (HKEY_LOCAL_MACHINE));
    if(!path.is_empty() && path.is_dir())
      paths.append(path);
#else
    GUTF8String home=GOS::getenv("HOME");
    struct passwd *pw=0;
    if(home.length())
    {
      pw=getpwuid(getuid());
      if(pw)
        home=GNativeString(pw->pw_dir);
    }
    if(home.length())
    {
      path=GURL::UTF8(LocalDjVuDir,GURL::Filename::UTF8(home));
      if(!path.is_empty() && path.is_dir())
        paths.append(path);
    }
#endif

    path=GURL::Filename::UTF8(RootDjVuDir);
    if(!path.is_empty() && path.is_dir())
      paths.append(path);

    GPosition pos;
#ifndef UNDER_CE
    const GUTF8String oldlocale(setlocale(LC_CTYPE,0));
    const GUTF8String defaultlocale((oldlocale.search((unsigned long)'_') < 0)
      ?setlocale(LC_CTYPE,""):(const char *)oldlocale);
    if(oldlocale != defaultlocale)
    {
      setlocale(LC_CTYPE,(const char *)oldlocale);
    }
    for(pos=paths;pos;++pos)
    {
      path=GURL::UTF8(defaultlocale,paths[pos]);
      if(path.is_dir())
      {
        realpaths.append(path);
      }
    }
    int underscore=defaultlocale.search((unsigned long)'_');
    if(underscore<0)
      underscore=defaultlocale.length();
    const GUTF8String lookuplocale(lookup_id(defaultlocale.substr(0,underscore)));
    if(lookuplocale != defaultlocale)
    {
      for(pos=paths;pos;++pos)
      {
        path=GURL::UTF8(lookuplocale,paths[pos]);
        if(path.is_dir())
        {
          realpaths.append(path);
        }
      }
    }
#endif
    for(pos=paths;pos;++pos)
    {
      realpaths.append(paths[pos]);
    }
  }
  return realpaths;
}

static GUTF8String
getbodies(
  GList<GURL> &paths,
  const GUTF8String &MessageFileName,
  GPList<lt_XMLTags> &body, 
  GMap<GUTF8String, void *> & map )
{
  GUTF8String errors;
  bool isdone=false;
  for(GPosition pos=paths;!isdone && pos;++pos)
  {
    const GURL::UTF8 url(MessageFileName,paths[pos]);
    if(url.is_file())
    {
      map[MessageFileName]=0;
      GP<lt_XMLTags> gtags=lt_XMLTags::create();
      lt_XMLTags &tags=*gtags;
      {
        GP<ByteStream> bs=ByteStream::create(url,"rb");
        G_TRY
        {
          tags.init(bs);
        }
        G_CATCH(ex)
        {
          GUTF8String mesg(failed_to_parse_XML+("\t"+url.get_string()));
          if(errors.length())
          {
            errors+="\n"+mesg;
          }else
          {
            errors=mesg;
          }
          errors+="\n"+GUTF8String(ex.get_cause());
        }
        G_ENDCATCH;
      }
      GPList<lt_XMLTags> Bodies=tags.getTags(bodystring);
      if(! Bodies.isempty())
      {
        isdone=true;
        for(GPosition pos=Bodies;pos;++pos)
        {
          body.append(Bodies[pos]);
        }
      }
      GPList<lt_XMLTags> Head=tags.getTags(headstring);
      if(! Head.isempty())
      {
        isdone=true;
        GMap<GUTF8String, GP<lt_XMLTags> > includes;
        lt_XMLTags::getMaps(includestring,namestring,Head,includes);
        for(GPosition pos=includes;pos;++pos)
        {
          const GUTF8String file=includes.key(pos);
          if(! map.contains(file))
          {
            const GUTF8String err2(getbodies(paths,file,body,map));
            if(err2.length())
            {
              if(errors.length())
              {
                errors+="\n"+err2;
              }else
              {
                errors=err2;
              }
            }
          }
        }
      }
    }
  }
  return errors;
}

static GUTF8String
parse(GMap<GUTF8String,GP<lt_XMLTags> > &retval)
{
  GUTF8String errors;
  GPList<lt_XMLTags> body;
  {
    GList<GURL> paths=GetProfilePaths();
    GMap<GUTF8String, void *> map;
    GUTF8String m(MessageFile);
    errors=getbodies(paths,m,body,map);
  }
  if(! body.isempty())
  {
    lt_XMLTags::getMaps(messagestring,namestring,body,retval);
  }
  return errors;
}


//  There is only object of class DjVuMessage in a program, and here it is:
const DjVuMessageLite &
DjVuMessage::create_full(void)
{
  GP<DjVuMessageLite> &static_message=getDjVuMessageLite();
  if(!static_message)
  {
    DjVuMessage *mesg=new DjVuMessage;
    static_message=mesg;
    mesg->init();
  }
  return DjVuMessageLite::create_lite();
}

void
DjVuMessage::use_locale(void)
{ 
  DjVuMessageLite::create=create_full; 
}


// Constructor
DjVuMessage::DjVuMessage( void ) {}

void
DjVuMessage::init(void)
{
  errors=parse(Map);
}

// Destructor
DjVuMessage::~DjVuMessage( )
{
}

static const char *
lookup_id(GUTF8String language, const int start)
{
  struct iso639
  {
    const char c1,c2,c3,c4;
    const unsigned long id;
    char const * const language;
    unsigned long get_id(void)
    { return id; }
    char const * get_name(void) const
    { return &c1; }
    iso639(const char x1, const char x2, const char x3,const char xlanguage[])
    : c1(x1),c2(x2),c3(x3),c4(0), id(((unsigned long)x1)|(((unsigned long)x2)<<8)|(((unsigned long)x3)<<16)), language(xlanguage) {}
  };
  static iso639 table[]={
  iso639('a','b',0,"abkhazian"),
  iso639('a','b','k',"abkhazian"),
  iso639('a','c','e',"achinese"),
  iso639('a','c','h',"acoli"),
  iso639('a','d','a',"adangme"),
  iso639('o','m',0,"afan"),
  iso639('a','a',0,"afar"),
  iso639('a','a','r',"afar"),
  iso639('a','f','h',"afrihili"),
  iso639('a','f',0,"afrikaans"),
  iso639('a','f','r',"afrikaans"),
  iso639('a','f','a',"afro-asiatic (other)"),
  iso639('a','k','a',"akan"),
  iso639('a','k','k',"akkadian"),
  iso639('a','l','b',"albanian"),
  iso639('s','q',0,"albanian"),
  iso639('s','q','i',"albanian"),
  iso639('a','l','e',"aleut"),
  iso639('a','l','g',"algonquian languages"),
  iso639('t','u','t',"altaic (other)"),
  iso639('e','n',0,"american"),
  iso639('a','m',0,"amharic"),
  iso639('a','m','h',"amharic"),
  iso639('a','p','a',"apache languages"),
  iso639('a','r',0,"arabic"),
  iso639('a','r','a',"arabic"),
  iso639('a','r','c',"aramaic"),
  iso639('a','r','p',"arapaho"),
  iso639('a','r','n',"araucanian"),
  iso639('a','r','w',"arawak"),
  iso639('a','r','m',"armenian"),
  iso639('h','y',0,"armenian"),
  iso639('h','y','e',"armenian"),
  iso639('a','r','t',"artificial (other)"),
  iso639('a','s',0,"assamese"),
  iso639('a','s','m',"assamese"),
  iso639('a','t','h',"athapascan languages"),
  iso639('m','a','p',"austronesian (other)"),
  iso639('a','v','a',"avaric"),
  iso639('a','v','e',"avestan"),
  iso639('a','w','a',"awadhi"),
  iso639('a','y',0,"aymara"),
  iso639('a','y','m',"aymara"),
  iso639('a','z',0,"azerbaijani"),
  iso639('a','z','e',"azerbaijani"),
  iso639('n','a','h',"aztec"),
  iso639('b','a','n',"balinese"),
  iso639('b','a','t',"baltic (other)"),
  iso639('b','a','l',"baluchi"),
  iso639('b','a','m',"bambara"),
  iso639('b','a','i',"bamileke languages"),
  iso639('b','a','d',"banda"),
  iso639('b','n',0,"bangla"),
  iso639('b','n','t',"bantu (other)"),
  iso639('b','a','s',"basa"),
  iso639('b','a',0,"bashkir"),
  iso639('b','a','k',"bashkir"),
  iso639('b','a','q',"basque"),
  iso639('e','u',0,"basque"),
  iso639('e','u','s',"basque"),
  iso639('b','e','j',"beja"),
  iso639('b','e','m',"bemba"),
  iso639('b','n',0,"bengali"),
  iso639('b','e','n',"bengali"),
  iso639('b','e','r',"berber (other)"),
  iso639('b','h','o',"bhojpuri"),
  iso639('d','z',0,"bhutani"),
  iso639('b','h',0,"bihari"),
  iso639('b','i','h',"bihari"),
  iso639('b','i','k',"bikol"),
  iso639('b','i','n',"bini"),
  iso639('b','i',0,"bislama"),
  iso639('b','i','s',"bislama"),
  iso639('b','r','a',"braj"),
  iso639('b','r',0,"breton"),
  iso639('b','r','e',"breton"),
  iso639('b','u','g',"buginese"),
  iso639('b','g',0,"bulgarian"),
  iso639('b','u','l',"bulgarian"),
  iso639('b','u','a',"buriat"),
  iso639('b','u','r',"burmese"),
  iso639('m','y',0,"burmese"),
  iso639('m','y','a',"burmese"),
  iso639('b','e',0,"byelorussian"),
  iso639('b','e','l',"byelorussian"),
  iso639('c','a','d',"caddo"),
  iso639('k','m',0,"cambodian"),
  iso639('c','a','r',"carib"),
  iso639('c','a',0,"catalan"),
  iso639('c','a','t',"catalan"),
  iso639('c','a','u',"caucasian (other)"),
  iso639('c','e','b',"cebuano"),
  iso639('c','e','l',"celtic (other)"),
  iso639('c','a','i',"central american indian (other)"),
  iso639('c','h','g',"chagatai"),
  iso639('c','h','a',"chamorro"),
  iso639('c','h','e',"chechen"),
  iso639('c','h','r',"cherokee"),
  iso639('c','h','y',"cheyenne"),
  iso639('c','h','b',"chibcha"),
  iso639('c','h','i',"chinese"),
  iso639('z','h',0,"chinese"),
  iso639('z','h','o',"chinese"),
  iso639('c','h','n',"chinook jargon"),
  iso639('c','h','o',"choctaw"),
  iso639('c','h','u',"church slavic"),
  iso639('c','h','v',"chuvash"),
  iso639('c','o','p',"coptic"),
  iso639('c','o','r',"cornish"),
  iso639('c','o',0,"corsican"),
  iso639('c','o','s',"corsican"),
  iso639('c','r','e',"cree"),
  iso639('m','u','s',"creek"),
  iso639('c','p','e',"creoles and pidgins, english-based (other)"),
  iso639('c','p','f',"creoles and pidgins, french-based (other)"),
  iso639('c','r','p',"creoles and pidgins (other)"),
  iso639('c','p','p',"creoles and pidgins, portuguese-based (other)"),
  iso639('h','r',0,"croatian"),
  iso639('c','u','s',"cushitic (other)"),
  iso639('c','e','s',"czech"),
  iso639('c','s',0,"czech"),
  iso639('c','z','e',"czech"),
  iso639('d','a','k',"dakota"),
  iso639('d','a',0,"danish"),
  iso639('d','a','n',"danish"),
  iso639('d','e','l',"delaware"),
  iso639('d','i','n',"dinka"),
  iso639('d','i','v',"divehi"),
  iso639('d','o','i',"dogri"),
  iso639('d','r','a',"dravidian (other)"),
  iso639('d','u','a',"duala"),
  iso639('d','u','t',"dutch"),
  iso639('d','u','m',"dutch, middle (ca. 1050-1350)"),
  iso639('n','l',0,"dutch"),
  iso639('n','l','a',"dutch"),
  iso639('d','y','u',"dyula"),
  iso639('d','z','o',"dzongkha"),
  iso639('e','f','i',"efik"),
  iso639('e','g','y',"egyptian (ancient)"),
  iso639('e','k','a',"ekajuk"),
  iso639('e','l','x',"elamite"),
  iso639('e','n',0,"english"),
  iso639('e','n','g',"english"),
  iso639('e','n','m',"english, middle (ca. 1100-1500)"),
  iso639('a','n','g',"english, old (ca. 450-1100)"),
  iso639('e','s','k',"eskimo (other)"),
  iso639('e','o',0,"esperanto"),
  iso639('e','p','o',"esperanto"),
  iso639('e','s','t',"estonian"),
  iso639('e','t',0,"estonian"),
  iso639('e','w','e',"ewe"),
  iso639('e','w','o',"ewondo"),
  iso639('f','o',0,"faeroese"),
  iso639('f','a','n',"fang"),
  iso639('f','a','t',"fanti"),
  iso639('f','a','o',"faroese"),
  iso639('f','i','j',"fijian"),
  iso639('f','j',0,"fiji"),
  iso639('f','i',0,"finnish"),
  iso639('f','i','n',"finnish"),
  iso639('f','i','u',"finno-ugrian (other)"),
  iso639('f','o','n',"fon"),
  iso639('f','r',0,"french"),
  iso639('f','r','a',"french"),
  iso639('f','r','e',"french"),
  iso639('f','r','m',"french, middle (ca. 1400-1600)"),
  iso639('f','r','o',"french, old (842- ca. 1400)"),
  iso639('f','r','y',"frisian"),
  iso639('f','y',0,"frisian"),
  iso639('f','u','l',"fulah"),
  iso639('g','d',0,"gaelic"),
  iso639('g','a','e',"gaelic (scots)"),
  iso639('g','d','h',"gaelic (scots)"),
  iso639('g','a','a',"ga"),
  iso639('g','l',0,"galician"),
  iso639('g','l','g',"gallegan"),
  iso639('l','u','g',"ganda"),
  iso639('g','a','y',"gayo"),
  iso639('g','e','z',"geez"),
  iso639('g','e','o',"georgian"),
  iso639('k','a',0,"georgian"),
  iso639('k','a','t',"georgian"),
  iso639('d','e',0,"german"),
  iso639('d','e','u',"german"),
  iso639('g','e','r',"german"),
  iso639('g','e','m',"germanic (other)"),
  iso639('g','m','h',"german, middle high (ca. 1050-1500)"),
  iso639('g','o','h',"german, old high (ca. 750-1050)"),
  iso639('g','i','l',"gilbertese"),
  iso639('g','o','n',"gondi"),
  iso639('g','o','t',"gothic"),
  iso639('g','r','b',"grebo"),
  iso639('g','r','c',"greek, ancient (to 1453)"),
  iso639('e','l',0,"greek"),
  iso639('e','l','l',"greek, modern (1453-)"),
  iso639('g','r','e',"greek, modern (1453-)"),
  iso639('k','a','l',"greenlandic"),
  iso639('k','l',0,"greenlandic"),
  iso639('g','n',0,"guarani"),
  iso639('g','r','n',"guarani"),
  iso639('g','u',0,"gujarati"),
  iso639('g','u','j',"gujarati"),
  iso639('h','a','i',"haida"),
  iso639('h','a',0,"hausa"),
  iso639('h','a','u',"hausa"),
  iso639('h','a','w',"hawaiian"),
  iso639('h','e','b',"hebrew"),
  iso639('i','w',0,"hebrew"),
  iso639('h','e','r',"herero"),
  iso639('h','i','l',"hiligaynon"),
  iso639('h','i','m',"himachali"),
  iso639('h','i',0,"hindi"),
  iso639('h','i','n',"hindi"),
  iso639('h','m','o',"hiri motu"),
  iso639('h','u',0,"hungarian"),
  iso639('h','u','n',"hungarian"),
  iso639('h','u','p',"hupa"),
  iso639('i','b','a',"iban"),
  iso639('i','c','e',"icelandic"),
  iso639('i','s',0,"icelandic"),
  iso639('i','s','l',"icelandic"),
  iso639('i','b','o',"igbo"),
  iso639('i','j','o',"ijo"),
  iso639('i','l','o',"iloko"),
  iso639('i','n','c',"indic (other)"),
  iso639('i','n','e',"indo-european (other)"),
  iso639('i','n',0,"indonesian"),
  iso639('i','n','d',"indonesian"),
  iso639('i','a',0,"interlingua"),
  iso639('i','n','a',"interlingua (international auxiliary language association)"),
  iso639('i','e',0,"interlingue"),
  iso639('i','n','e',"interlingue"),
  iso639('i','k','u',"inuktitut"),
  iso639('i','k',0,"inupiak"),
  iso639('i','p','k',"inupiak"),
  iso639('i','r','a',"iranian (other)"),
  iso639('g','a',0,"irish"),
  iso639('g','a','i',"irish"),
  iso639('i','r','i',"irish"),
  iso639('m','g','a',"irish, middle (900 - 1200)"),
  iso639('s','g','a',"irish, old (to 900)"),
  iso639('i','r','o',"iroquoian languages"),
  iso639('i','t',0,"italian"),
  iso639('i','t','a',"italian"),
  iso639('j','a',0,"japanese"),
  iso639('j','p','n',"japanese"),
  iso639('j','a','v',"javanese"),
  iso639('j','a','v',"javanese"),
  iso639('j','a','w',"javanese"),
  iso639('j','a','w',"javanese"),
  iso639('j','w',0,"javanese"),
  iso639('j','r','b',"judeo-arabic"),
  iso639('j','p','r',"judeo-persian"),
  iso639('k','a','b',"kabyle"),
  iso639('k','a','c',"kachin"),
  iso639('k','a','m',"kamba"),
  iso639('k','a','n',"kannada"),
  iso639('k','n',0,"kannada"),
  iso639('k','a','u',"kanuri"),
  iso639('k','a','a',"kara-kalpak"),
  iso639('k','a','r',"karen"),
  iso639('k','a','s',"kashmiri"),
  iso639('k','s',0,"kashmiri"),
  iso639('k','a','w',"kawi"),
  iso639('k','a','z',"kazakh"),
  iso639('k','k',0,"kazakh"),
  iso639('k','h','a',"khasi"),
  iso639('k','h','m',"khmer"),
  iso639('k','h','i',"khoisan (other)"),
  iso639('k','h','o',"khotanese"),
  iso639('k','i','k',"kikuyu"),
  iso639('k','i','n',"kinyarwanda"),
  iso639('r','w',0,"kinyarwanda"),
  iso639('k','i','r',"kirghiz"),
  iso639('k','y',0,"kirghiz"),
  iso639('r','n',0,"kirundi"),
  iso639('k','o','m',"komi"),
  iso639('k','o','n',"kongo"),
  iso639('k','o','k',"konkani"),
  iso639('k','o',0,"korean"),
  iso639('k','o','r',"korean"),
  iso639('k','p','e',"kpelle"),
  iso639('k','r','o',"kru"),
  iso639('k','u','a',"kuanyama"),
  iso639('k','u','m',"kumyk"),
  iso639('k','u',0,"kurdish"),
  iso639('k','u','r',"kurdish"),
  iso639('k','r','u',"kurukh"),
  iso639('k','u','s',"kusaie"),
  iso639('k','u','t',"kutenai"),
  iso639('l','a','d',"ladino"),
  iso639('l','a','h',"lahnda"),
  iso639('l','a','m',"lamba"),
  iso639('o','c','i',"langue d'oc (post 1500)"),
  iso639('l','a','o',"lao"),
  iso639('l','o',0,"laothian"),
  iso639('l','a',0,"latin"),
  iso639('l','a','t',"latin"),
  iso639('l','a','v',"latvian"),
  iso639('l','v',0,"latvian"),
  iso639('l','v',0,"lettish"),
  iso639('l','t','z',"letzeburgesch"),
  iso639('l','e','z',"lezghian"),
  iso639('l','i','n',"lingala"),
  iso639('l','n',0,"lingala"),
  iso639('l','i','t',"lithuanian"),
  iso639('l','t',0,"lithuanian"),
  iso639('l','o','z',"lozi"),
  iso639('l','u','b',"luba-katanga"),
  iso639('l','u','i',"luiseno"),
  iso639('l','u','n',"lunda"),
  iso639('l','u','o',"luo (kenya and tanzania)"),
  iso639('m','a','c',"macedonian"),
  iso639('m','a','k',"macedonian"),
  iso639('m','k',0,"macedonian"),
  iso639('m','a','d',"madurese"),
  iso639('m','a','g',"magahi"),
  iso639('m','a','i',"maithili"),
  iso639('m','a','k',"makasar"),
  iso639('m','g',0,"malagasy"),
  iso639('m','l','g',"malagasy"),
  iso639('m','a','l',"malayalam"),
  iso639('m','l',0,"malayalam"),
  iso639('m','a','y',"malay"),
  iso639('m','s',0,"malay"),
  iso639('m','s','a',"malay"),
  iso639('m','l','t',"maltese"),
  iso639('m','t',0,"maltese"),
  iso639('m','a','n',"mandingo"),
  iso639('m','n','i',"manipuri"),
  iso639('m','n','o',"manobo languages"),
  iso639('m','a','x',"manx"),
  iso639('m','a','o',"maori"),
  iso639('m','i',0,"maori"),
  iso639('m','r','i',"maori"),
  iso639('m','a','r',"marathi"),
  iso639('m','r',0,"marathi"),
  iso639('c','h','m',"mari"),
  iso639('m','a','h',"marshall"),
  iso639('m','w','r',"marwari"),
  iso639('m','a','s',"masai"),
  iso639('m','y','n',"mayan languages"),
  iso639('m','e','n',"mende"),
  iso639('m','i','c',"micmac"),
  iso639('m','i','n',"minangkabau"),
  iso639('m','i','s',"miscellaneous (other)"),
  iso639('m','o','h',"mohawk"),
  iso639('m','o',0,"moldavian"),
  iso639('m','o','l',"moldavian"),
  iso639('m','n',0,"mongolian"),
  iso639('m','o','n',"mongolian"),
  iso639('l','o','l',"mongo"),
  iso639('m','k','h',"mon-kmer (other)"),
  iso639('m','o','s',"mossi"),
  iso639('m','u','l',"multiple languages"),
  iso639('m','u','n',"munda languages"),
  iso639('n','a',0,"nauru"),
  iso639('n','a','u',"nauru"),
  iso639('n','a','v',"navajo"),
  iso639('n','d','e',"ndebele, north"),
  iso639('n','b','l',"ndebele, south"),
  iso639('n','d','o',"ndongo"),
  iso639('n','e',0,"nepali"),
  iso639('n','e','p',"nepali"),
  iso639('n','e','w',"newari"),
  iso639('n','i','c',"niger-kordofanian (other)"),
  iso639('s','s','a',"nilo-saharan (other)"),
  iso639('n','i','u',"niuean"),
  iso639('n','o','n',"norse, old"),
  iso639('n','a','i',"north american indian (other)"),
  iso639('n','o',0,"norwegian"),
  iso639('n','o','r',"norwegian"),
  iso639('n','n','o',"norwegian (nynorsk)"),
  iso639('n','u','b',"nubian languages"),
  iso639('n','y','m',"nyamwezi"),
  iso639('n','y','a',"nyanja"),
  iso639('n','y','n',"nyankole"),
  iso639('n','y','o',"nyoro"),
  iso639('n','z','i',"nzima"),
  iso639('o','c',0,"occitan"),
  iso639('o','j','i',"ojibwa"),
  iso639('o','r',0,"oriya"),
  iso639('o','r','i',"oriya"),
  iso639('o','m',0,"oromo"),
  iso639('o','r','m',"oromo"),
  iso639('o','s','a',"osage"),
  iso639('o','s','s',"ossetic"),
  iso639('o','t','o',"otomian languages"),
  iso639('p','a','l',"pahlavi"),
  iso639('p','a','u',"palauan"),
  iso639('p','l','i',"pali"),
  iso639('p','a','m',"pampanga"),
  iso639('p','a','g',"pangasinan"),
  iso639('p','a','n',"panjabi"),
  iso639('p','a','p',"papiamento"),
  iso639('p','a','a',"papuan-australian (other)"),
  iso639('f','a',0,"persian"),
  iso639('f','a','s',"persian"),
  iso639('p','e','o',"persian, old (ca 600 - 400 b.c.)"),
  iso639('p','e','r',"persian"),
  iso639('p','h','n',"phoenician"),
  iso639('p','l',0,"polish"),
  iso639('p','o','l',"polish"),
  iso639('p','o','n',"ponape"),
  iso639('p','o','r',"portuguese"),
  iso639('p','t',0,"portuguese"),
  iso639('p','r','a',"prakrit languages"),
  iso639('p','r','o',"provencal, old (to 1500)"),
  iso639('p','a',0,"punjabi"),
  iso639('p','s',0,"pashto"),
  iso639('p','s',0,"pushto"),
  iso639('p','u','s',"pushto"),
  iso639('q','u',0,"quechua"),
  iso639('q','u','e',"quechua"),
  iso639('r','a','j',"rajasthani"),
  iso639('r','a','r',"rarotongan"),
  iso639('r','m',0,"rhaeto-romance"),
  iso639('r','o','h',"rhaeto-romance"),
  iso639('r','o','a',"romance (other)"),
  iso639('r','o',0,"romanian"),
  iso639('r','o','n',"romanian"),
  iso639('r','u','m',"romanian"),
  iso639('r','o','m',"romany"),
  iso639('r','u','n',"rundi"),
  iso639('r','u',0,"russian"),
  iso639('r','u','s',"russian"),
  iso639('s','a','l',"salishan languages"),
  iso639('s','a','m',"samaritan aramaic"),
  iso639('s','m','i',"sami languages"),
  iso639('s','m',0,"samoan"),
  iso639('s','m','o',"samoan"),
  iso639('s','a','d',"sandawe"),
  iso639('s','a','g',"sango"),
  iso639('s','g',0,"sangro"),
  iso639('s','a',0,"sanskrit"),
  iso639('s','a','n',"sanskrit"),
  iso639('s','r','d',"sardinian"),
  iso639('g','d',0,"scots gaelic"),
  iso639('s','c','o',"scots"),
  iso639('s','e','l',"selkup"),
  iso639('s','e','m',"semitic (other)"),
  iso639('s','r',0,"serbian"),
  iso639('s','c','r',"serbo-croatian"),
  iso639('s','h',0,"serbo-croatian"),
  iso639('s','r','r',"serer"),
  iso639('s','t',0,"sesotho"),
  iso639('t','n',0,"setswana"),
  iso639('s','h','n',"shan"),
  iso639('s','n',0,"shona"),
  iso639('s','n','a',"shona"),
  iso639('s','i','d',"sidamo"),
  iso639('b','l','a',"siksika"),
  iso639('s','d',0,"sindhi"),
  iso639('s','n','d',"sindhi"),
  iso639('s','i',0,"singhalese"),
  iso639('s','i','n',"singhalese"),
  iso639('s','i','t',"sino-tibetan (other)"),
  iso639('s','i','o',"siouan languages"),
  iso639('s','s','w',"siswant"),
  iso639('s','s',0,"siswati"),
  iso639('s','l','a',"slavic (other)"),
  iso639('s','k',0,"slovak"),
  iso639('s','l','k',"slovak"),
  iso639('s','l','o',"slovak"),
  iso639('s','l',0,"slovenian"),
  iso639('s','l','v',"slovenian"),
  iso639('s','o','g',"sogdian"),
  iso639('s','o',0,"somali"),
  iso639('s','o','m',"somali"),
  iso639('s','o','n',"songhai"),
  iso639('w','e','n',"sorbian languages"),
  iso639('n','s','o',"sotho, northern"),
  iso639('s','o','t',"sotho, southern"),
  iso639('s','a','i',"south american indian (other)"),
  iso639('e','s',0,"spanish"),
  iso639('e','s','l',"spanish"),
  iso639('s','p','a',"spanish"),
  iso639('s','u',0,"sudanese"),
  iso639('s','u','n',"sudanese"),
  iso639('s','u','k',"sukuma"),
  iso639('s','u','x',"sumerian"),
  iso639('s','u','s',"susu"),
  iso639('s','w',0,"swahili"),
  iso639('s','w','a',"swahili"),
  iso639('s','s','w',"swazi"),
  iso639('s','v',0,"swedish"),
  iso639('s','v','e',"swedish"),
  iso639('s','w','e',"swedish"),
  iso639('s','y','r',"syriac"),
  iso639('t','g','l',"tagalog"),
  iso639('t','l',0,"tagalog"),
  iso639('t','a','h',"tahitian"),
  iso639('t','g',0,"tajik"),
  iso639('t','g','k',"tajik"),
  iso639('t','m','h',"tamashek"),
  iso639('t','a',0,"tamil"),
  iso639('t','a','m',"tamil"),
  iso639('t','a','t',"tatar"),
  iso639('t','t',0,"tatar"),
  iso639('t','e',0,"tegulu"),
  iso639('t','e','l',"telugu"),
  iso639('t','e','r',"tereno"),
  iso639('t','h',0,"thai"),
  iso639('t','h','a',"thai"),
  iso639('b','o',0,"tibetan"),
  iso639('b','o','d',"tibetan"),
  iso639('t','i','b',"tibetan"),
  iso639('t','i','g',"tigre"),
  iso639('t','i',0,"tigrinya"),
  iso639('t','i','r',"tigrinya"),
  iso639('t','e','m',"timne"),
  iso639('t','i','v',"tivi"),
  iso639('t','l','i',"tlingit"),
  iso639('t','o','g',"tonga (nyasa)"),
  iso639('t','o',0,"tonga"),
  iso639('t','o','n',"tonga (tonga islands)"),
  iso639('t','r','u',"truk"),
  iso639('t','s','i',"tsimshian"),
  iso639('t','s',0,"tsonga"),
  iso639('t','s','o',"tsonga"),
  iso639('t','s','n',"tswana"),
  iso639('t','u','m',"tumbuka"),
  iso639('o','t','a',"turkish, ottoman (1500 - 1928)"),
  iso639('t','r',0,"turkish"),
  iso639('t','u','r',"turkish"),
  iso639('t','k',0,"turkmen"),
  iso639('t','u','k',"turkmen"),
  iso639('t','y','v',"tuvinian"),
  iso639('t','w',0,"twi"),
  iso639('t','w','i',"twi"),
  iso639('u','g','a',"ugaritic"),
  iso639('u','i','g',"uighur"),
  iso639('u','k',0,"ukrainian"),
  iso639('u','k','r',"ukrainian"),
  iso639('u','m','b',"umbundu"),
  iso639('u','n','d',"undetermined"),
  iso639('u','r',0,"urdu"),
  iso639('u','r','d',"urdu"),
  iso639('u','z',0,"uzbek"),
  iso639('u','z','b',"uzbek"),
  iso639('v','a','i',"vai"),
  iso639('v','e','n',"venda"),
  iso639('v','i',0,"vietnamese"),
  iso639('v','i','e',"vietnamese"),
  iso639('v','o',0,"volapuk"),
  iso639('v','o','l',"volapük"),
  iso639('v','o','t',"votic"),
  iso639('w','a','k',"wakashan languages"),
  iso639('w','a','l',"walamo"),
  iso639('w','a','r',"waray"),
  iso639('w','a','s',"washo"),
  iso639('c','y',0,"welsh"),
  iso639('c','y','m',"welsh"),
  iso639('w','e','l',"welsh"),
  iso639('w','o',0,"wolof"),
  iso639('w','o','l',"wolof"),
  iso639('x','h',0,"xhosa"),
  iso639('x','h','o',"xhosa"),
  iso639('s','a','h',"yakut"),
  iso639('y','a','o',"yao"),
  iso639('y','a','p',"yap"),
  iso639('j','i',0,"yiddish"),
  iso639('y','i','d',"yiddish"),
  iso639('y','o',0,"yoruba"),
  iso639('y','o','r',"yoruba"),
  iso639('z','a','p',"zapotec"),
  iso639('z','e','n',"zenaga"),
  iso639('z','h','a',"zhuang"),
  iso639('z','u',0,"zulu"),
  iso639('z','u','l',"zulu"),
  iso639('z','u','n',"zuni") };
  static const int size=(int)(sizeof(table)/sizeof(iso639));
  language=language.downcase();
  int len=language.length();
  const char *xlanguage=language;
  const char *retval=xlanguage;
  if((len > 1)&&(len < 4))
  {
    const unsigned long id=((unsigned long)(xlanguage[0]))
     |((unsigned long)(xlanguage[1])<<8)
     |((unsigned long)(xlanguage[2])<<16);
    for(int i=start;i<size;++i)
    {
      if(id == table[i].get_id())
      {
        const char *language=table[i].language;
        const int k=i;
        for(int j=k-1;
          j>=0&&!strcmp(xlanguage,table[j].language);
          --j)
        {
          const char *jlanguage=table[j].language;
          if(strcmp(jlanguage,xlanguage))
            break;
          if(!table[j].get_name()[2])
          {
            i=j;
            len=2;
          }else if(len == 3)
          {
            i=j;
          }
        }
        if(len>2)
        {
          for(int j=k+1;j>0&&!strcmp(language,table[j].language);++j)
          {
            if(!table[j].get_name()[2])
            {
              i=j;
              len=2;
              break;
            }
          }
        }
        retval=table[i].get_name();
        break;
      }
    }
  }else
  {
    int minGuess=start;
    int maxGuess=size+1;
    while(minGuess<maxGuess)
    {
      const int guess=minGuess+((maxGuess-minGuess)/2);
      const int i=strcmp(xlanguage,table[guess].language);
      if(i<0)
      {
        maxGuess=guess;
      }else if(i)
      {
        minGuess=guess+1;
      }else
      {
        retval=lookup_id(table[guess].get_name(),guess);
        break;
      }
    }
  }
  return retval;
}

//  A C function to perform a message lookup. Arguments are a buffer to receiv
//  translated message, a buffer size (bytes), and a message_list. The transla
//  result is returned in msg_buffer encoded in UTF-8. In case of error, msg_b
//  empty (i.e., msg_buffer[0] == '\0').
void
DjVuMessage_LookUp( 
  char *msg_buffer, const unsigned int buffer_size, const char *message)
{
  const GUTF8String converted(DjVuMessage::LookUpUTF8( message ));
  if( converted.length() >= buffer_size )
    msg_buffer[0] = '\0';
  else
    strcpy( msg_buffer, converted );
}


void
DjVuFormatError( const char *fmt, ... )
{
  va_list args;
  va_start(args, fmt); 
  const GUTF8String message(fmt,args);
  DjVuWriteError( message );
}
