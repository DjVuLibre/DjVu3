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
//C- $Id: DjVuAnno.cpp,v 1.2 1999-05-25 19:42:27 eaf Exp $


#ifdef __GNUC__
#pragma implementation
#endif

// GLParser.h and GLParser.cpp used to be separate files capable to decode
// that weird ANTa chunk format into C++ structures and lists. But since
// its implementation is temporary and is used only in this file (DjVuAnno.cpp)
// it appears reasonable to build it in here.

//***************************************************************************
//****************************** GLParser.h *********************************
//***************************************************************************

#include "GPContainer.h"
#include "GString.h"
#include "GSmartPointer.h"
#include "GException.h"
#include "ByteStream.h"

class GLObject : public GPEnabled
{
public:
   enum GLObjectType { NUMBER, STRING, SYMBOL, LIST };

   GLObject(int _number=0);
   GLObject(GLObjectType type, const char * str);
   GLObject(const char * name, const GPList<GLObject> & list);
   virtual ~GLObject(void);
   
   int		get_number(void) const;
   GString	get_string(void) const;
   GString	get_symbol(void) const;
   GPList<GLObject>	& get_list(void);
   GP<GLObject>	operator[](int n) const;
   
   GLObjectType	get_type(void) const;
   GString	get_name(void) const;
   void		print(ByteStream & str, int compact=1, int indent=0, int * cur_pos=0) const;
private:
   GLObjectType	type;
   GString	name;
   
   int		number;
   GString	string;
   GString	symbol;
   GPList<GLObject>	list;
};

inline GLObject::GLObjectType
GLObject::get_type(void) const { return type; }

inline
GLObject::~GLObject(void) {}

class GLToken
{
public:
   enum GLTokenType { OPEN_PAR, CLOSE_PAR, OBJECT };
   GLTokenType	type;
   GP<GLObject>	object;
   
   GLToken(GLTokenType type, const GP<GLObject> & object);
};

inline
GLToken::GLToken(GLTokenType xtype, const GP<GLObject> & xobject) :
      type(xtype), object(xobject) {}

class GLParser
{
public:
   void		parse(const char * str);
   GPList<GLObject>	& get_list(void);
   GP<GLObject>		get_object(const char * name);
   void		print(ByteStream & str, int compact=1);

   GLParser(void);
   GLParser(const char * str);
   ~GLParser(void);
private:
   GPList<GLObject>	list;

   void		skip_white_space(const char * & start);
   GLToken	get_token(const char * & start);
   void		parse(const char * cur_name, GPList<GLObject> & list,
		      const char * & start);
};

inline GPList<GLObject> &
GLParser::get_list(void) { return list; }

inline
GLParser::GLParser(void) {}

inline
GLParser::GLParser(const char * str) { parse(str); }

inline
GLParser::~GLParser(void) {}

//***************************************************************************
//***************************** GLParser.cpp ********************************
//***************************************************************************

#include "debug.h"

#include <ctype.h>

GLObject::GLObject(int xnumber) : type(NUMBER), number(xnumber) {}

GLObject::GLObject(GLObjectType xtype, const char * str) : type(xtype)
{
   if (type!=STRING && type!=SYMBOL)
      THROW("GLObject(): Wrong object type passed. Should be either STRING or SYMBOL.");
   if (type==STRING) string=str;
   else symbol=str;
}

GLObject::GLObject(const char * xname, const GPList<GLObject> & xlist) :
      type(LIST), name(xname), list(xlist) {}

void
GLObject::print(ByteStream & str, int compact, int indent, int * cur_pos) const
{
   int aldel_cur_pos=0;
   if (!cur_pos) { cur_pos=new int; *cur_pos=0; aldel_cur_pos=1; };
   
   char buffer[256];
   TArray<char> buffer_str;
   const char * to_print=0;
   switch(type)
   {
      case NUMBER:
	 sprintf(buffer, "%d", number);
	 to_print=buffer;
	 break;
      case STRING:
	 if (1)
	 {
	    unsigned int src=0, dst=0;
	    buffer_str.resize(string.length()*2);
	    buffer_str[dst++]='"';
	    for(src=0;src<string.length();src++)
	    {
	       char ch=string[src];
	       if (ch=='"') buffer_str[dst++]='\\';
	       buffer_str[dst++]=ch;
	    };
	    buffer_str[dst++]='"';
	    buffer_str[dst++]=0;
	    to_print=buffer_str;
	 };
	 break;
      case SYMBOL:
	 sprintf(buffer, "%s", (const char *) symbol);
	 to_print=buffer;
	 break;
      case LIST:
	 sprintf(buffer, "(%s", (const char *) name);
	 to_print=buffer;
	 break;
   };
   if (!compact && *cur_pos+strlen(to_print)>70)
   {
      char ch='\n';
      str.write(&ch, 1);
      ch=' ';
      for(int i=0;i<indent;i++) str.write(&ch, 1);
      *cur_pos=indent;
   };
   str.write(to_print, strlen(to_print));
   char ch=' ';
   str.write(&ch, 1);
   *cur_pos+=strlen(to_print)+1;
   if (type==LIST)
   {
      int indent=*cur_pos-strlen(to_print);
      for(GPosition pos=list;pos;++pos)
	 list[pos]->print(str, compact, indent, cur_pos);
      str.write(") ", 2);
      *cur_pos+=2;
   };
   
   if (aldel_cur_pos) delete cur_pos;
}

GString
GLObject::get_string(void) const
{
   if (type!=STRING)
   {
      char * buffer=new char[256], buffer1[256]; buffer1[0]=0;
      sprintf(buffer, "Invalid object type. Can't convert ");
      if (type==NUMBER) sprintf(buffer1, "number %d", number);
      else if (type==SYMBOL) sprintf(buffer1, "symbol '%s'", (const char *) symbol);
      else if (type==LIST) sprintf(buffer1, "compound object '%s'", (const char *) name);
      strcat(buffer, buffer1); strcat(buffer, " to string.");
      THROW(buffer);
   };
   return string;
}

GString
GLObject::get_symbol(void) const
{
   if (type!=SYMBOL)
   {
      char * buffer=new char[256], buffer1[256]; buffer1[0]=0;
      sprintf(buffer, "Invalid object type. Can't convert ");
      if (type==NUMBER) sprintf(buffer1, "number %d", number);
      else if (type==STRING) sprintf(buffer1, "string '%s'", (const char *) string);
      else if (type==LIST) sprintf(buffer1, "compound object '%s'", (const char *) name);
      strcat(buffer, buffer1); strcat(buffer, " to symbol.");
      THROW(buffer);
   };
   return symbol;
}

int
GLObject::get_number(void) const
{
   if (type!=NUMBER)
   {
      char * buffer=new char[256], buffer1[256];
      sprintf(buffer, "Invalid object type. Can't convert ");
      if (type==STRING) sprintf(buffer1, "string '%s'", (const char *) string);
      else if (type==SYMBOL) sprintf(buffer1, "symbol '%s'", (const char *) symbol);
      else if (type==LIST) sprintf(buffer1, "compound object '%s'", (const char *) name);
      strcat(buffer, buffer1); strcat(buffer, " to integer.");
      THROW(buffer);
   };
   return number;
}

GString
GLObject::get_name(void) const
{
   if (type!=LIST)
   {
      char * buffer=new char[256], buffer1[256];
      sprintf(buffer, "Invalid object type. Can't convert ");
      if (type==NUMBER) sprintf(buffer1, "number '%d'", number);
      else if (type==STRING) sprintf(buffer1, "string '%s'", (const char *) string);
      else if (type==SYMBOL) sprintf(buffer1, "symbol '%s'", (const char *) symbol);
      strcat(buffer, buffer1); strcat(buffer, " to compound object.");
      THROW(buffer);
   };
   return name;
}

GP<GLObject>
GLObject::operator[](int n) const
{
   if (type!=LIST)
   {
      char * buffer=new char[256], buffer1[256];
      sprintf(buffer, "Invalid object type. Can't convert ");
      if (type==NUMBER) sprintf(buffer1, "number '%d'", number);
      else if (type==STRING) sprintf(buffer1, "string '%s'", (const char *) string);
      else if (type==SYMBOL) sprintf(buffer1, "symbol '%s'", (const char *) symbol);
      strcat(buffer, buffer1); strcat(buffer, " to compound object.");
      THROW(buffer);
   };
   if (n>=list.size()) THROW("Too few elements in '"+name+"'");
   int i;
   GPosition pos;
   for(i=0, pos=list;i<n && pos;i++, ++pos);
   return list[pos];
}

GPList<GLObject> &
GLObject::get_list(void)
{
   if (type!=LIST)
   {
      char * buffer=new char[256], buffer1[256];
      sprintf(buffer, "Invalid object type. Can't convert ");
      if (type==NUMBER) sprintf(buffer1, "number '%d'", number);
      else if (type==STRING) sprintf(buffer1, "string '%s'", (const char *) string);
      else if (type==SYMBOL) sprintf(buffer1, "symbol '%s'", (const char *) symbol);
      strcat(buffer, buffer1); strcat(buffer, " to compound object.");
      THROW(buffer);
   };
   return list;
}

//********************************** GLParser *********************************

void
GLParser::skip_white_space(const char * & start)
{
   while(*start && isspace(*start)) start++;
   if (!*start) THROW("EOF");
}

GLToken
GLParser::get_token(const char * & start)
{
   skip_white_space(start);
   switch(*start)
   {
      case '(':
	 start++;
	 return GLToken(GLToken::OPEN_PAR, 0);
      case ')':
	 start++;
	 return GLToken(GLToken::CLOSE_PAR, 0);
      case '-':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
	 return GLToken(GLToken::OBJECT,
			new GLObject(strtol(start, (char **) &start, 10)));
      case '"':
      {
	 GString str;
	 start++;
	 while(1)
	 {
	    char ch=*start++;
	    if (!ch) THROW("EOF");
	    if (ch=='"')
	    {
	       if (str.length()>0 && str[str.length()-1]=='\\')
		  str.setat(str.length()-1, '"');
	       else break;
	    } else str+=ch;
	 };
	 return GLToken(GLToken::OBJECT, new GLObject(GLObject::STRING, str));
      }
      default:
      {
	 GString str;
	 while(1)
	 {
	    char ch=*start++;
	    if (!ch) THROW("EOF");
	    if (ch==')') { start--; break; };
	    if (isspace(ch)) break;
	    str+=ch;
	 };
	 return GLToken(GLToken::OBJECT, new GLObject(GLObject::SYMBOL, str));
      }
   };
} 

void
GLParser::parse(const char * cur_name, GPList<GLObject> & list,
		const char * & start)
{
   DEBUG_MSG("GLParse::parse(): Parsing contents of object '" << cur_name << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   while(1)
   {
      GLToken token=get_token(start);
      if (token.type==GLToken::OPEN_PAR)
      {
	 if (isspace(*start))
	 {
	    char * buffer=new char[128];
	    sprintf(buffer, "Error occurred when parsing contents of object '%s':\n"
		    "'(' must be IMMEDIATELY followed by an object name.",
		    (const char *) cur_name);
	    THROW(buffer);
	 };
	 
	 GLToken tok=get_token(start);
	 GP<GLObject> object=tok.object;	// This object should be SYMBOL
	 					// We will convert it to LIST later
	 if (tok.type!=GLToken::OBJECT || object->get_type()!=GLObject::SYMBOL)
	 {
	    char * buffer=new char[512];
	    sprintf(buffer, "Error occurred when parsing contents of object '%s':\n"
		    "'(' must be followed by object name, not by ",
		    (const char *) cur_name);
	    if (tok.type==GLToken::OPEN_PAR ||
		tok.type==GLToken::CLOSE_PAR) strcat(buffer, "parenthesis.");
	    if (tok.type==GLToken::OBJECT)
	    {
	       GLObject::GLObjectType type=object->get_type();
	       if (type==GLObject::NUMBER) strcat(buffer, "number.");
	       else if (type==GLObject::STRING) strcat(buffer, "string.");
	    };
	    THROW(buffer);
	 };
	 
	 // OK. Get the object contents
	 GPList<GLObject> new_list;
	 int eof_caught=0;
	 TRY
	 {
	    parse(object->get_symbol(), new_list, start);
	 } CATCH(exc)
	 {
	    if (!strcmp(exc.get_cause(), "EOF")) eof_caught=1;
	    else RETHROW;
	 } ENDCATCH;
	 if (eof_caught)
	 {
	    char * buffer=new char[128];
	    sprintf(buffer, "Unexpectedly got EOF when parsing contents of object '%s'.",
		    (const char *) object->get_symbol());
	    THROW(buffer);
	 };
	 
	 list.append(new GLObject(object->get_symbol(), new_list));
	 continue;
      };
      
      if (token.type==GLToken::CLOSE_PAR) return;

      list.append(token.object);
   };
}

void
GLParser::parse(const char * str)
{
   DEBUG_MSG("GLParser::parse(): parsing string contents\n");
   DEBUG_MAKE_INDENT(3);
   
   TRY
   {
      parse("toplevel", list, str);
   } CATCH(exc)
   {
      if (strcmp(exc.get_cause(), "EOF")) RETHROW;
   } ENDCATCH;
}

void
GLParser::print(ByteStream & str, int compact)
{
   for(GPosition pos=list;pos;++pos)
      list[pos]->print(str, compact);
}

GP<GLObject>
GLParser::get_object(const char * name)
{
   for(GPosition pos=list;pos;++pos)
   {
      GP<GLObject> obj=list[pos];
      if (obj->get_type()==GLObject::LIST &&
	  obj->get_name()==name) return obj;
   }
   return 0;
}

//***************************************************************************
//***************************** DjVuAnno.cpp ********************************
//***************************************************************************

#include "DjVuAnno.h"
#include "debug.h"

#include <ctype.h>

#define PNOTE_TAG	"pnote"
#define BACKGROUND_TAG	"background"
#define ZOOM_TAG	"zoom"
#define MODE_TAG	"mode"
#define ALIGN_TAG	"align"

DjVuAnno::DjVuAnno()
{
   bg_color=0xffffffff;
   zoom=0;
   mode=MODE_UNSPEC;
   hor_align=ver_align=ALIGN_UNSPEC;
}

DjVuAnno::~DjVuAnno()
{
}

GString
DjVuAnno::read_raw(ByteStream & str)
{
   GString raw;
   char buffer[1024];
   int length;
   while((length=str.read(buffer, 1024)))
      raw+=GString(buffer, length);
   return raw;
}

void
DjVuAnno::decode(class GLParser & parser)
{
   GCriticalSectionLock lock(&class_lock);
   
   bg_color=get_bg_color(parser);
   zoom=get_zoom(parser);
   mode=get_mode(parser);
   hor_align=get_hor_align(parser);
   ver_align=get_ver_align(parser);
   get_hlinks(parser, rect_hlinks, poly_hlinks, oval_hlinks);
}

void 
DjVuAnno::decode(ByteStream & str)
{
   GCriticalSectionLock lock(&class_lock);

   raw=read_raw(str);
   GLParser parser(raw);
   decode(parser);
}

void
DjVuAnno::merge(ByteStream & str)
{
   GCriticalSectionLock lock(&class_lock);
   
   GString raw1=encode_raw();
   GLParser parser(raw);
   GString raw2=read_raw(str);
   parser.parse(raw2);
   decode(parser);
   raw=encode_raw();
}

void
DjVuAnno::encode(ByteStream &bs)
{
  GCriticalSectionLock lock(&class_lock);
  
  raw=encode_raw();
  bs.writall((const void*)raw, raw.length());
}

unsigned int 
DjVuAnno::get_memory_usage() const
{
  return sizeof(DjVuAnno) + raw.length();
}

unsigned char
DjVuAnno::decode_comp(char ch1, char ch2)
{
   unsigned char dig1=0;
   if (ch1)
   {
      ch1=toupper(ch1);
      if (ch1>='0' && ch1<='9') dig1=ch1-'0';
      if (ch1>='A' && ch1<='F') dig1=10+ch1-'A';
      
      unsigned char dig2=0;
      if (ch2)
      {
	 ch2=toupper(ch2);
	 if (ch2>='0' && ch2<='9') dig2=ch2-'0';
	 if (ch2>='A' && ch2<='F') dig2=10+ch2-'A';
	 return (dig1 << 4) | dig2;
      };
      return dig1;
   };
   return 0;
}

u_int32
DjVuAnno::cvt_color(const char * color, u_int32 def)
{
   if (color[0]!='#') return def;

   u_int32 color_rgb=0;
   color++;
   const char * start, * end;
   
   // Do blue
   end=color+strlen(color); start=end-2;
   if (start<color) start=color;
   if (end>start)
      color_rgb|=decode_comp(start[0], start+1<end ? start[1] : 0);
   
   // Do green
   end=color+strlen(color)-2; start=end-2;
   if (start<color) start=color;
   if (end>start)
      color_rgb|=decode_comp(start[0], start+1<end ? start[1] : 0) << 8;
   
   // Do red
   end=color+strlen(color)-4; start=end-2;
   if (start<color) start=color;
   if (end>start)
      color_rgb|=decode_comp(start[0], start+1<end ? start[1] : 0) << 16;
   return color_rgb;
}

u_int32
DjVuAnno::get_bg_color(GLParser & parser)
{
   DEBUG_MSG("DjVuAnno::get_bg_color(): getting background color ...\n");
   DEBUG_MAKE_INDENT(3);
   TRY
   {
      GP<GLObject> obj=parser.get_object(BACKGROUND_TAG);
      if (obj && obj->get_list().size()==1)
      {
	 GString color=(*obj)[0]->get_symbol();
	 DEBUG_MSG("color='" << color << "'\n");
	 return cvt_color(color, 0xffffff);
      } else { DEBUG_MSG("can't find any.\n"); };
   } CATCH(exc) {} ENDCATCH;
   DEBUG_MSG("resetting color to 0xffffffff (UNSPEC)\n");
   return 0xffffffff;
}

int
DjVuAnno::get_zoom(GLParser & parser)
      // Returns:
      //   <0 - special zoom (like ZOOM_STRETCH)
      //   =0 - not set
      //   >0 - numeric zoom (%%)
{
   DEBUG_MSG("DjVuAnt::get_zoom(): getting zoom factor ...\n");
   DEBUG_MAKE_INDENT(3);
   TRY
   {
      GP<GLObject> obj=parser.get_object(ZOOM_TAG);
      if (obj && obj->get_list().size()==1)
      {
	 GString zoom=(*obj)[0]->get_symbol();
	 DEBUG_MSG("zoom='" << zoom << "'\n");

	 if (zoom=="stretch") return ZOOM_STRETCH;
	 else if (zoom=="one2one") return ZOOM_ONE2ONE;
	 else if (zoom=="width") return ZOOM_WIDTH;
	 else if (zoom=="page") return ZOOM_PAGE;
	 else if (zoom[0]!='d') THROW("Illegal zoom specification");
	 else return atoi((const char *) zoom+1);
      } else { DEBUG_MSG("can't find any.\n"); };
   } CATCH(exc) {} ENDCATCH;
   DEBUG_MSG("resetting zoom to 0 (UNSPEC)\n");
   return ZOOM_UNSPEC;
}

int
DjVuAnno::get_mode(GLParser & parser)
{
   DEBUG_MSG("DjVuAnt::get_mode(): getting default mode ...\n");
   DEBUG_MAKE_INDENT(3);
   TRY
   {
      GP<GLObject> obj=parser.get_object(MODE_TAG);
      if (obj && obj->get_list().size()==1)
      {
	 GString mode=(*obj)[0]->get_symbol();
	 DEBUG_MSG("mode='" << mode << "'\n");

	 if (mode=="color") return MODE_COLOR;
	 else if (mode=="fore") return MODE_FORE;
	 else if (mode=="back") return MODE_BACK;
	 else if (mode=="bw") return MODE_BW;
      } else { DEBUG_MSG("can't find any.\n"); };
   } CATCH(exc) {} ENDCATCH;
   DEBUG_MSG("resetting mode to MODE_UNSPEC\n");
   return MODE_UNSPEC;
}

int
DjVuAnno::get_hor_align(GLParser & parser)
{
   DEBUG_MSG("DjVuAnt::get_hor_align(): getting hor page alignemnt ...\n");
   DEBUG_MAKE_INDENT(3);
   TRY
   {
      GP<GLObject> obj=parser.get_object(ALIGN_TAG);
      if (obj && obj->get_list().size()==2)
      {
	 GString align=(*obj)[0]->get_symbol();
	 DEBUG_MSG("hor_align='" << align << "'\n");

	 if (align=="left") return ALIGN_LEFT;
	 else if (align=="center") return ALIGN_CENTER;
	 else if (align=="right") return ALIGN_RIGHT;
      } else { DEBUG_MSG("can't find any.\n"); };
   } CATCH(exc) {} ENDCATCH;
   DEBUG_MSG("resetting alignment to ALIGN_UNSPEC\n");
   return ALIGN_UNSPEC;
}

int
DjVuAnno::get_ver_align(GLParser & parser)
{
   DEBUG_MSG("DjVuAnt::get_ver_align(): getting vert page alignemnt ...\n");
   DEBUG_MAKE_INDENT(3);
   TRY
   {
      GP<GLObject> obj=parser.get_object(ALIGN_TAG);
      if (obj && obj->get_list().size()==2)
      {
	 GString align=(*obj)[1]->get_symbol();
	 DEBUG_MSG("ver_align='" << align << "'\n");

	 if (align=="top") return ALIGN_TOP;
	 else if (align=="center") return ALIGN_CENTER;
	 else if (align=="bottom") return ALIGN_BOTTOM;
      } else { DEBUG_MSG("can't find any.\n"); };
   } CATCH(exc) {} ENDCATCH;
   DEBUG_MSG("resetting alignment to ALIGN_UNSPEC\n");
   return ALIGN_UNSPEC;
}

void
DjVuAnno::get_hlinks(GLParser & parser, GPList<GHLRect> & rect_hlinks,
		     GPList<GHLPoly> & poly_hlinks,
		     GPList<GHLOval> & oval_hlinks)
{
   DEBUG_MSG("DjVuAnno::get_hlinks(): forming and returning back list of hyperlinks\n");
   DEBUG_MAKE_INDENT(3);

   rect_hlinks.empty(); poly_hlinks.empty(); oval_hlinks.empty();
   
   GPList<GLObject> list=parser.get_list();
   for(GPosition pos=list;pos;++pos)
   {
      GLObject & obj=*list[pos];
      if (obj.get_type()==GLObject::LIST && obj.get_name()==MAPAREA_TAG)
      {
	 TRY
	 {
	    // Getting the url
	    GString url;
	    GString target="_self";
	    GLObject & url_obj=*(obj[0]);
	    if (url_obj.get_type()==GLObject::LIST)
	    {
	       if (url_obj.get_name()!="url") THROW("Invalid URL specification");
	       url=(url_obj[0])->get_string();
	       target=(url_obj[1])->get_string();
	    } else url=url_obj.get_string();
	    
	    // Getting the comment
	    GString comment=(obj[1])->get_string();
	    
	    DEBUG_MSG("found maparea '" << comment << "' (" <<
		      url << ":" << target << ")\n");
	    
	    GHLObject::HLType hltype;
	    GString hlcolor="blue";
	    int shadow_thick=3;
	    GLObject * hl=obj[3];
	    if (hl->get_type()==GLObject::LIST)
	    {
	       const GString & str=hl->get_name();
	       hltype=
		  str==NONE_TAG ? GHLObject::NONE :
		  str==XOR_TAG ? GHLObject::XOR :
		  str==BORDER_TAG ? GHLObject::BORDER :
		  str==SHADOW_IN_TAG ? GHLObject::SHADOW_IN :
		  str==SHADOW_OUT_TAG ? GHLObject::SHADOW_OUT :
		  str==SHADOW_EIN_TAG ? GHLObject::SHADOW_EIN :
		  str==SHADOW_EOUT_TAG ? GHLObject::SHADOW_EOUT : GHLObject::XOR;
	       for(GPosition pos=hl->get_list();pos;++pos)
	       {
		  GLObject * obj=hl->get_list()[pos];
		  if (obj->get_type()==GLObject::SYMBOL) hlcolor=obj->get_symbol();
		  if (obj->get_type()==GLObject::NUMBER) shadow_thick=obj->get_number();
	       };
	    };
	    
	    u_int32 hlcolor_rgb=cvt_color(hlcolor, 0xff);
	    
	    GLObject * shape=obj[2];
	    GHLObject * maparea=0;
	    if (shape->get_type()==GLObject::LIST)
	    {
	       if (shape->get_name()==RECT_TAG)
	       {
		  DEBUG_MSG("it's a rectangle.\n");
		  GRect grect((*shape)[0]->get_number(),
			      (*shape)[1]->get_number(),
			      (*shape)[2]->get_number(),
			      (*shape)[3]->get_number());
		  GP<GHLRect> rect=new GHLRect(grect);
		  rect_hlinks.append(rect);
		  maparea=rect;
	       } else if (shape->get_name()==POLY_TAG)
	       {
		  DEBUG_MSG("it's a polygojn.\n");
		  int points=shape->get_list().size()/2;
		  TArray<int> xx(points-1), yy(points-1);
		  for(int i=0;i<points;i++)
		  {
		     xx[i]=(*shape)[2*i]->get_number();
		     yy[i]=(*shape)[2*i+1]->get_number();
		  };
		  GP<GHLPoly> poly=new GHLPoly(xx, yy, points);
		  poly_hlinks.append(poly);
		  maparea=poly;
	       } else if (shape->get_name()==OVAL_TAG)
	       {
		  DEBUG_MSG("it's an ellipse.\n");
		  GRect grect((*shape)[0]->get_number(),
			      (*shape)[1]->get_number(),
			      (*shape)[2]->get_number(),
			      (*shape)[3]->get_number());
		  GP<GHLOval> oval=new GHLOval(grect);
		  oval_hlinks.append(oval);
		  maparea=oval;
	       };
	       if (maparea)
	       {
		  maparea->url=url;
		  maparea->target=target;
		  maparea->comment=comment;
		  maparea->hltype=hltype;
		  maparea->hlcolor_rgb=hlcolor_rgb;
		  maparea->shadow_thick=shadow_thick;
	       } else DEBUG_MSG("hlink type is not recognized\n");
	    };
	 } CATCH(exc)
	 {
	 } ENDCATCH;
      }; // if (...get_name()==MAPAREA_TAG)
   }; // while(item==...)
}

void
DjVuAnno::del_all_items(const char * name, GLParser & parser)
{
   GPList<GLObject> & list=parser.get_list();
   GPosition pos=list;
   while(pos)
   {
      GLObject & obj=*list[pos];
      if (obj.get_type()==GLObject::LIST &&
	  obj.get_name()==name)
      {
	 GPosition this_pos=pos;
	 ++pos;
	 list.del(this_pos);
      } else ++pos;
   }
}

GString
DjVuAnno::encode_raw(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   
   char buffer[512];
   GLParser parser(raw);

      //*** Background color
   del_all_items(BACKGROUND_TAG, parser);
   if (bg_color!=0xffffffff)
   {
      sprintf(buffer, "(" BACKGROUND_TAG " #%02X%02X%02X)",
	      (bg_color & 0xff0000) >> 16,
	      (bg_color & 0xff00) >> 8,
	      (bg_color & 0xff));
      parser.parse(buffer);
   }

      //*** Zoom
   del_all_items(ZOOM_TAG, parser);
   if (zoom!=ZOOM_UNSPEC)
   {
      if (zoom==ZOOM_STRETCH) strcpy(buffer, "(" ZOOM_TAG " stretch)");
      else if (zoom==ZOOM_ONE2ONE) strcpy(buffer, "(" ZOOM_TAG " one2one)");
      else if (zoom==ZOOM_WIDTH) strcpy(buffer, "(" ZOOM_TAG " width)");
      else if (zoom==ZOOM_PAGE) strcpy(buffer, "(" ZOOM_TAG " page)");
      else sprintf(buffer, "(" ZOOM_TAG " d%d)", zoom);
      parser.parse(buffer);
   }

      //*** Mode
   del_all_items(MODE_TAG, parser);
   if (mode!=MODE_UNSPEC)
   {
      if (mode==MODE_COLOR) strcpy(buffer, "(" ZOOM_TAG " color)");
      else if (mode==MODE_FORE) strcpy(buffer, "(" ZOOM_TAG " fore)");
      else if (mode==MODE_BACK) strcpy(buffer, "(" ZOOM_TAG " back)");
      else if (mode==MODE_BW) strcpy(buffer, "(" ZOOM_TAG " bw)");
      parser.parse(buffer);
   }

      //*** Alignment
   del_all_items(ALIGN_TAG, parser);
   if (hor_align!=ALIGN_UNSPEC || ver_align!=ALIGN_UNSPEC)
   {
      strcpy(buffer, "(" ALIGN_TAG " ");
      if (hor_align==ALIGN_LEFT) strcat(buffer, "left ");
      else if (hor_align==ALIGN_CENTER) strcat(buffer, "center ");
      else if (hor_align==ALIGN_RIGHT) strcat(buffer, "right ");
      else strcat(buffer, "default ");
      if (ver_align==ALIGN_TOP) strcat(buffer, "top)");
      else if (ver_align==ALIGN_CENTER) strcat(buffer, "center)");
      else if (ver_align==ALIGN_BOTTOM) strcat(buffer, "bottom)");
      else strcat(buffer, "default)");
      parser.parse(buffer);
   }
   
      //*** Hyperlinks
   del_all_items(MAPAREA_TAG, parser);
   GPosition pos;
   for(pos=rect_hlinks;pos;++pos)
      parser.parse(rect_hlinks[pos]->print());
   for(pos=poly_hlinks;pos;++pos)
      parser.parse(poly_hlinks[pos]->print());
   for(pos=oval_hlinks;pos;++pos)
      parser.parse(oval_hlinks[pos]->print());

   MemoryByteStream str;
   parser.print(str, 1);
   TArray<char> data=str.get_data();
   return GString(data, data.size());
}

bool
DjVuAnno::is_empty(void) const
{
   GString raw=encode_raw();
   return raw.length()==0;
}
