//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: XMLTags.cpp,v 1.16 2001-04-21 00:16:58 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "XMLTags.h"
#include "UnicodeByteStream.h"
//#include <stdio.h>
#include <ctype.h>
#include <wctype.h>

lt_XMLContents::lt_XMLContents(void) {}

lt_XMLContents::lt_XMLContents(GP<lt_XMLTags> t)
{
  tag=t;
}

static GUTF8String
getargn(char const tag[], char const *&t)
{
  char const *s;
  for(s=tag;isspace(*s);s++);
  for(t=s;(*t)&&((*t)!='/')&&((*t)!='>')&&((*t)!='=')&&!isspace(*t);++t);
  return GUTF8String(s,t-s);
}

static GUTF8String
getargv(char const tag[], char const *&t)
{
  GUTF8String retval;
  if(tag && tag[0] == '=')
  {
    char const *s=t=tag+1;
    if((*t == '"')||(*t == '\47'))
    {
      char const q=*(t++);
      for(s++;(*t)&&((*t)!=q)&&((*t)!='>');++t);
      retval=GUTF8String(s,t-s);
      if (t[0] == q)
      {
        ++t;
      }
    }else
    {
      for(t=s;(*t)&&((*t)!='/')&&((*t)!='>')&&!isspace(*t);++t);
      retval=GUTF8String(s,t-s);
    }
  }else
  {
    t=tag;
  }
  return retval;
}

static GUTF8String
tagtoname(char const tag[],char const *&t)
{
  char const *s;
  for(s=tag;isspace(*s);s++);
  for(t=s;(*t)&&((*t)!='>')&&!isspace(*t);++t);
  return GUTF8String(s,t-s);
}

static inline GUTF8String
tagtoname(char const tag[])
{
  char const *t;
  return tagtoname(tag,t);
}

#if 0
static bool
isspaces(char const *s)
{
  bool retval=true;
  if(s)
  {
    while(*s)
    {
      if(!isspace(*s++))
      {
        retval=false;
        break;
      }
    }
  }
  return retval;
}
#endif

static bool
isspaces(unsigned long const *s)
{
  bool retval=true;
  if(s)
  {
    while(*s)
    {
      if(!iswspace((wint_t)*s++))
      {
        retval=false;
        break;
      }
    }
  }
  return retval;
}

void
lt_XMLTags::ParseValues(char const *t, GMap<GUTF8String,GUTF8String> &args,bool downcase)
{
  GUTF8String argn;
  char const *tt;
  while((argn=getargn(t,tt)).length())
  {
    if(downcase)
      argn=argn.downcase();
    args[argn]=getargv(tt,t).fromEscaped();
  }
}

lt_XMLTags::~lt_XMLTags() {}

lt_XMLTags::lt_XMLTags(void) {}

lt_XMLTags::lt_XMLTags(const char n[])
{
  char const *t;
  name=tagtoname(n,t);
  ParseValues(t,args);
}

void
lt_XMLTags::init(GP<ByteStream> &bs)
{
  GP<XMLByteStream> gxmlbs=XMLByteStream::create(bs);
  init(*gxmlbs);
}

void
lt_XMLTags::init(const GURL &url)
{
  GP<ByteStream> bs=ByteStream::create(url,"r");
  init(bs);
}

void
lt_XMLTags::init(XMLByteStream &xmlbs)
{
  if(!get_count())
  {
    G_THROW( ERR_MSG("XMLTags.no_GP") );
  }
  GPList<lt_XMLTags> level;
  GMap<GUTF8String,GPList<lt_XMLTags> > allTags;
  GMap<GUTF8String,GMap<GUTF8String,GPList<lt_XMLTags> > > namedTags;
  GUnicode tag,raw(xmlbs.gets(0,'<',false));
  if(!isspaces((unsigned long const *)raw))
  {
    GUTF8String mesg;
    mesg.format( ERR_MSG("XMLTags.raw_string") "\t%s",(const char *)raw);
    G_THROW(mesg);
  }
  while((tag=xmlbs.gets(0,'>',true))[0])
  {
    int len=tag.length();
    while(tag[len-1] != '>')
    {
      GUTF8String mesg;
      mesg.format( ERR_MSG("XMLTags.bad_tag") "\t%s",(const char *)tag);
      G_THROW(mesg);
    }
    switch(tag[1])
    {
      case '?':
      {
        while(len < 4 || tag.substr_nr(len-2,len).get_GUTF8String() != "?>")
        {
          GUnicode cont(xmlbs.gets(0,'>',true));
          if(!cont[0])
          { 
            GUTF8String mesg;
            mesg.format( ERR_MSG("XMLTags.bad_PI") "\t%s",(const char *)tag);
            G_THROW(mesg);
          }
          len=((tag+=cont).length());
        }
        GUTF8String xname=tagtoname(((const char *)tag)+2);
//        if(xname.downcase() == "xml")
//        {
//          DjVuPrintMessage("Got XMLDecl: %s",(const char *)tag);
//          
//        }else
//        {
//          DjVuPrintMessage("Got PI: %s",(const char *)tag);
//        }
        break;
      }
      case '!':
      {
        if(tag[2] == '-' && tag[3] == '-')
        {
          while((len < 7) || (tag.substr_nr(len-3,len).get_GUTF8String() != "-->"))
          {
            GUnicode cont(xmlbs.gets(0,'>',true));
            if(!cont[0])
            { 
              GUTF8String mesg;
              mesg.format( ERR_MSG("XMLTags.bad_comment") "\t%s",(const char *)tag);
              G_THROW(mesg);
            }
            len=((tag+=cont).length());
          }
//          DjVuPrintMessage("Got Comment: %s\n",(const char *)tag);
        }else
        {
//          DjVuPrintMessage("Got declaration: %s\n",(const char *)tag);
        }
        break;
      }
      case '/':
      {
        GUTF8String xname=tagtoname(((const char *)tag)+2);
        GPosition last=level.lastpos();
        if(last || level[last]->name != xname)
        {
//          DjVuPrintMessage("Got end tag: %s\n",(const char *)name);
          level.del(last);
        }else
        {
          GUTF8String mesg( ERR_MSG("XMLTags.bad_form") );
          G_THROW(mesg);
        }
        break;
      }
      default:
      {
//        DjVuPrintMessage("Got tag: %s\n",(const char *)tag);
//        DjVuPrintMessage("Got tag: %s\n",(const char *)xname);
        GPosition last=level.lastpos();
        GP<lt_XMLTags> t;
        if(last)
        {
          t=new lt_XMLTags(((char const *)tag)+1);
          level[last]->addtag(t);
          if(tag[len-2] != '/')
          {
            level.append(t);
          }
        }else if(tag[len-2] != '/')
        {
          char const *n;
          name=tagtoname(((char const *)tag)+1,n);
          ParseValues(n,args);
          t=this;
          level.append(t);
        }else
        {
          GUTF8String mesg( ERR_MSG("XMLTags.no_body") );
          G_THROW(mesg);
        }
        allTags[t->name].append(t);
        for(GPosition pos=t->args;pos;++pos)
        {
          namedTags[t->args.key(pos)][t->args[pos]].append(t);
        }
        break;
      }
    }
    if((raw=xmlbs.gets(0,'<',false))[0])
    { 
      GPosition last=level.lastpos();
      if(last)
      {
        level[last]->addraw(raw.get_GUTF8String());
//        DjVuPrintMessage("Got raw %s: %s\n",(const char *)(level[last]->name),(const char *)raw);
      }else if(!isspaces((unsigned long const *)raw))
      {
        GUTF8String mesg;
        mesg.format( ERR_MSG("XMLTags.raw_string") "\t%s", (const char *)raw);
        G_THROW(mesg);
      }
    }
  }
}

GPList<lt_XMLTags>
lt_XMLTags::getTags(char const tagname[]) const
{
  GPosition pos=allTags.contains(tagname);
  GPList<lt_XMLTags> retval;
  return (pos?allTags[pos]:retval);
}

void
lt_XMLTags::getMaps(char const tagname[],char const argn[],GPList<lt_XMLTags> list, GMap<GUTF8String, GP<lt_XMLTags> > &map)
{
  for(GPosition pos=list;pos;++pos)
  {
    GP<lt_XMLTags> &tag=list[pos];
    if(tag)
    {
      GPosition loc;
      if((loc=tag->contains(tagname)))
      {
        GPList<lt_XMLTags> maps=(GPList<lt_XMLTags> &)((*tag)[loc]);
        for(GPosition mloc=maps;mloc;++mloc)
        {
          GP<lt_XMLTags> gtag=maps[mloc];
          if(gtag)
          {
            GMap<GUTF8String,GUTF8String> &args=gtag->args;
            GPosition gpos;
            if((gpos=args.contains(argn)))
            {
              map[args[gpos]]=gtag;
//              DjVuPrintError("Inserting %s\n",(const char *)(args[gpos]));
            }
          }
        }
      }
    }
  }
}

void
lt_XMLTags::write(ByteStream &bs,bool const top) const
{
  if(name.length())
  {
    GUTF8String tag="<"+name;
    for(GPosition pos=args;pos;++pos)
    {
      tag+=GUTF8String(' ')+args.key(pos)+GUTF8String("=\42")+args[pos].toEscaped()+GUTF8String("\42");
    }
    GPosition tags=content;
    if(tags||raw.length()) 
    {
      tag+=">";
      bs.writall((const char *)tag,tag.length());
      tag="</"+name+">";
      if(raw.length())
      {
        bs.writall((char const *)raw,raw.length());
      }
      for(;tags;++tags)
      {
        content[tags].write(bs);
      }
    }else if(!raw.length())
    {
      tag+="/>";
    }
    bs.writall((const char *)tag,tag.length());
  }
  if(top)
  {
     bs.writall("\n",1);
  }
}

void
lt_XMLContents::write(ByteStream &bs) const
{
  if(tag)
  {
    tag->write(bs,false);
  }
  if(raw.length())
  {
    bs.writall((char const *)raw,raw.length());
  } 
}

