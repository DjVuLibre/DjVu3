//C-  Copyright � 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//
// $Id: XMLTags.cpp,v 1.1 2001-01-17 00:14:55 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "XMLTags.h"
#include "UnicodeByteStream.h"
#include <stdio.h>
#include <ctype.h>

lt_XMLContents::lt_XMLContents(void) {}

lt_XMLContents::lt_XMLContents(GP<lt_XMLTags> t)
{
  tag=t;
}

static GString
getargn(char const tag[], char const *&t)
{
  char const *s;
  for(s=tag;isspace(*s);s++);
  for(t=s;(*t)&&((*t)!='/')&&((*t)!='>')&&((*t)!='=')&&!isspace(*t);++t);
  return GString(s,t-s);
}

static GString
getargv(char const tag[], char const *&t)
{
  GString retval;
  if(tag && tag[0] == '=')
  {
    char const *s=t=tag+1;
    if((*t == '"')||(*t == '\47'))
    {
      char const q=*(t++);
      for(s++;(*t)&&((*t)!=q)&&((*t)!='>');++t);
      retval=GString(s,t-s);
      if (t[0] == q)
      {
        ++t;
      }
    }else
    {
      for(t=s;(*t)&&((*t)!='/')&&((*t)!='>')&&!isspace(*t);++t);
      retval=GString(s,t-s);
    }
  }else
  {
    t=tag;
  }
  return retval;
}

static GString
tagtoname(char const tag[],char const *&t)
{
  char const *s;
  for(s=tag;isspace(*s);s++);
  for(t=s;(*t)&&((*t)!='>')&&!isspace(*t);++t);
  return GString(s,t-s);
}

static inline GString
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
      if(!isspace(*s++))
      {
        retval=false;
        break;
      }
    }
  }
  return retval;
}

void
lt_XMLTags::ParseValues(char const *t, GMap<GString,GString> &args,bool downcase)
{
  GString argn;
  char const *tt;
  while((argn=getargn(t,tt)).length())
  {
    if(downcase)
      argn=argn.downcase();
    args[argn]=getargv(tt,t).toEscaped();
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
  XMLByteStream xmlbs(bs);
  init(xmlbs);
}

void
lt_XMLTags::init(char const name[])
{
  GP<ByteStream> bs=new StdioByteStream(name,"r");
  init(bs);
}

void
lt_XMLTags::init(XMLByteStream &xmlbs)
{
  if(!get_count())
  {
    G_THROW("Not secured by a GP pointer");
  }
  GPList<lt_XMLTags> level;
  GMap<GString,GPList<lt_XMLTags> > allTags;
  GMap<GString,GMap<GString,GPList<lt_XMLTags> > > namedTags;
  GUnicode tag,raw(xmlbs.gets(0,'<',false));
  if(!isspaces((unsigned long const *)raw))
  {
    GString mesg;
    mesg.format("Raw string '%s' found outside document scope.",(const char *)raw);
    G_THROW(mesg);
  }
  while((tag=xmlbs.gets(0,'>',true))[0])
  {
    int len=tag.length();
    while(tag[len-1] != '>')
    {
      GString mesg;
      mesg.format("Unterminated Tag %s",(const char *)tag);
      G_THROW(mesg);
    }
    switch(tag[1])
    {
      case '?':
      {
        while(len < 4 || tag.substr_nr(len-2,len).get_GString() != "?>")
        {
          GUnicode cont(xmlbs.gets(0,'>',true));
          if(!cont[0])
          { 
            GString mesg;
            mesg.format("Unterminated Processing Instruction: %s",(const char *)tag);
            G_THROW(mesg);
          }
          len=((tag+=cont).length());
        }
        GString xname=tagtoname(((const char *)tag)+2);
//        if(xname.downcase() == "xml")
//        {
//          printf("Got XMLDecl: %s",(const char *)tag);
//          
//        }else
//        {
//          printf("Got PI: %s",(const char *)tag);
//        }
        break;
      }
      case '!':
      {
        if(tag[2] == '-' && tag[3] == '-')
        {
          while((len < 7) || (tag.substr_nr(len-3,len).get_GString() != "-->"))
          {
            GUnicode cont(xmlbs.gets(0,'>',true));
            if(!cont[0])
            { 
              GString mesg;
              mesg.format("Unterminated Comment: %s",(const char *)tag);
              G_THROW(mesg);
            }
            len=((tag+=cont).length());
          }
//          printf("Got Comment: %s\n",(const char *)tag);
        }else
        {
//          printf("Got declaration: %s\n",(const char *)tag);
        }
        break;
      }
      case '/':
      {
        GString xname=tagtoname(((const char *)tag)+2);
        GPosition last=level.lastpos();
        if(last || level[last]->name != xname)
        {
//          printf("Got end tag: %s\n",(const char *)name);
          level.del(last);
        }else
        {
          GString mesg("Well formed errror.  Too many end tags.");
          G_THROW(mesg);
        }
        break;
      }
      default:
      {
//        printf("Got tag: %s\n",(const char *)tag);
//        printf("Got tag: %s\n",(const char *)xname);
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
          GString mesg("No body");
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
        level[last]->addraw(raw.get_GString());
//        printf("Got raw %s: %s\n",(const char *)(level[last]->name),(const char *)raw);
      }else if(!isspaces((unsigned long const *)raw))
      {
        GString mesg;
        mesg.format("Raw string '%s' found outside document scope.",
          (const char *)raw);
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
lt_XMLTags::getMaps(char const tagname[],char const argn[],GPList<lt_XMLTags> list, GMap<GString, GP<lt_XMLTags> > &map)
{
  for(GPosition pos=list;pos;++pos)
  {
    lt_XMLTags const * const tag=list[pos];
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
            GMap<GString,GString> &args=gtag->args;
            GPosition gpos;
            if((gpos=args.contains(argn)))
            {
              map[args[gpos]]=gtag;
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
    GString tag="<"+name;
    for(GPosition pos=args;pos;++pos)
    {
      tag+=GString(' ')+args.key(pos)+GString("=\42")+args[pos].toEscaped()+GString("\42");
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
