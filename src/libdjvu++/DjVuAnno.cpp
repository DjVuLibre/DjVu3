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
//C- $Id: DjVuAnno.cpp,v 1.1.1.1 1999-10-22 19:29:23 praveen Exp $


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

#include "GContainer.h"
#include "GString.h"
#include "GSmartPointer.h"
#include "GException.h"
#include "IFFByteStream.h"
#include "BSByteStream.h"

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

GLParser::GLParser(void) 
{
}

GLParser::~GLParser(void) 
{
}

GPList<GLObject> &
GLParser::get_list(void) 
{ 
  return list; 
}

GLParser::GLParser(const char * str) 
{
  parse(str); 
}


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
   if (!cur_pos) { cur_pos=new int; *cur_pos=0; aldel_cur_pos=1; }
   
   char buffer[1024];
   GTArray<char> buffer_str;
   const char * to_print=0;
   switch(type)
   {
      case NUMBER:
	 sprintf(buffer, "%d", number);
	 to_print=buffer;
	 break;
      case STRING:
	 {
	    int src=0, dst=0;
	    buffer_str.resize(5+string.length()*2);
	    buffer_str[dst++]='"';
	    for(src=0;src<(int)string.length();src++)
	    {
	       char ch=string[src];
	       if (ch=='"') buffer_str[dst++]='\\';
	       buffer_str[dst++]=ch;
	    }
	    buffer_str[dst++]='"';
	    buffer_str[dst++]=0;
	    to_print=buffer_str;
	 }
	 break;
      case SYMBOL:
	 sprintf(buffer, "%s", (const char *) symbol);
	 to_print=buffer;
	 break;
      case LIST:
	 sprintf(buffer, "(%s", (const char *) name);
	 to_print=buffer;
	 break;
   }
   if (!compact && *cur_pos+strlen(to_print)>70)
   {
      char ch='\n';
      str.write(&ch, 1);
      ch=' ';
      for(int i=0;i<indent;i++) str.write(&ch, 1);
      *cur_pos=indent;
   }
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
   }
   
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
   }
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
   }
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
   }
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
   }
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
   }
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
   }
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
   char c = *start;
   if (c == '(')
     {
       start++;
       return GLToken(GLToken::OPEN_PAR, 0);
     }
   else if (c == ')')
     {
       start++;
       return GLToken(GLToken::CLOSE_PAR, 0);
     }
   else if (c=='-' || (c>='0' && c<='9'))
     {
       return GLToken(GLToken::OBJECT,
                      new GLObject(strtol(start, (char **) &start, 10)));
     }
   else if (c=='"')
     {
       GString str;
       start++;
       while(1)
	 {
           char ch=*start++;
           if (!ch) THROW("EOF");
           if (ch=='"')
             {
	       if (str.length()>0 && str[(int)str.length()-1]=='\\')
                 str.setat(str.length()-1, '"');
	       else break;
             } else str+=ch;
	 }
	 return GLToken(GLToken::OBJECT, new GLObject(GLObject::STRING, str));
     }
   else
     {
       GString str;
       while(1)
	 {
           char ch=*start++;
           if (!ch) THROW("EOF");
           if (ch==')') { start--; break; }
           if (isspace(ch)) break;
           str+=ch;
	 }
       return GLToken(GLToken::OBJECT, new GLObject(GLObject::SYMBOL, str));
     }
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
	 }
	 
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
	    }
	    THROW(buffer);
	 }
	 
	 // OK. Get the object contents
	 GPList<GLObject> new_list;
	 TRY
           {
             parse(object->get_symbol(), new_list, start);
           } 
         CATCH(exc)
           {
             if (strcmp(exc.get_cause(), "EOF"))
               RETHROW;
           } 
         ENDCATCH;
	 list.append(new GLObject(object->get_symbol(), new_list));
	 continue;
      }
      if (token.type==GLToken::CLOSE_PAR) 
        return;
      list.append(token.object);
   }
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
//********************************** ANT ************************************
//***************************************************************************

#include "DjVuAnno.h"
#include "debug.h"

#include <ctype.h>

#define PNOTE_TAG	"pnote"
#define BACKGROUND_TAG	"background"
#define ZOOM_TAG	"zoom"
#define MODE_TAG	"mode"
#define ALIGN_TAG	"align"

DjVuANT::DjVuANT()
{
   bg_color=0xffffffff;
   zoom=0;
   mode=MODE_UNSPEC;
   hor_align=ver_align=ALIGN_UNSPEC;
}

DjVuANT::~DjVuANT()
{
}

GString
DjVuANT::read_raw(ByteStream & str)
{
   GString raw;
   char buffer[1024];
   int length;
   while((length=str.read(buffer, 1024)))
      raw+=GString(buffer, length);
   return raw;
}

void
DjVuANT::decode(class GLParser & parser)
{
   bg_color=get_bg_color(parser);
   zoom=get_zoom(parser);
   mode=get_mode(parser);
   hor_align=get_hor_align(parser);
   ver_align=get_ver_align(parser);
   map_areas=get_map_areas(parser);
}

void 
DjVuANT::decode(ByteStream & str)
{
   GLParser parser(read_raw(str));
   decode(parser);
}

void
DjVuANT::merge(ByteStream & str)
{
   GLParser parser(encode_raw());
   GString add_raw=read_raw(str);
   parser.parse(add_raw);
   decode(parser);
}

void
DjVuANT::encode(ByteStream &bs)
{
  GString raw=encode_raw();
  bs.writall((const char*) raw, raw.length());
}

unsigned int 
DjVuANT::get_memory_usage() const
{
  return sizeof(DjVuANT);
}

unsigned char
DjVuANT::decode_comp(char ch1, char ch2)
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
      }
      return dig1;
   }
   return 0;
}

u_int32
DjVuANT::cvt_color(const char * color, u_int32 def)
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

      // Do the fourth byte
   end=color+strlen(color)-6; start=end-2;
   if (start<color) start=color;
   if (end>start)
      color_rgb|=decode_comp(start[0], start+1<end ? start[1] : 0) << 24;
   
   return color_rgb;
}

u_int32
DjVuANT::get_bg_color(GLParser & parser)
{
   DEBUG_MSG("DjVuANT::get_bg_color(): getting background color ...\n");
   DEBUG_MAKE_INDENT(3);
   TRY
   {
      GP<GLObject> obj=parser.get_object(BACKGROUND_TAG);
      if (obj && obj->get_list().size()==1)
      {
	 GString color=(*obj)[0]->get_symbol();
	 DEBUG_MSG("color='" << color << "'\n");
	 return cvt_color(color, 0xffffff);
      } else { DEBUG_MSG("can't find any.\n"); }
   } CATCH(exc) {} ENDCATCH;
   DEBUG_MSG("resetting color to 0xffffffff (UNSPEC)\n");
   return 0xffffffff;
}

int
DjVuANT::get_zoom(GLParser & parser)
      // Returns:
      //   <0 - special zoom (like ZOOM_STRETCH)
      //   =0 - not set
      //   >0 - numeric zoom (%%)
{
   DEBUG_MSG("DjVuANT::get_zoom(): getting zoom factor ...\n");
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
      } else { DEBUG_MSG("can't find any.\n"); }
   } CATCH(exc) {} ENDCATCH;
   DEBUG_MSG("resetting zoom to 0 (UNSPEC)\n");
   return ZOOM_UNSPEC;
}

int
DjVuANT::get_mode(GLParser & parser)
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
      } else { DEBUG_MSG("can't find any.\n"); }
   } CATCH(exc) {} ENDCATCH;
   DEBUG_MSG("resetting mode to MODE_UNSPEC\n");
   return MODE_UNSPEC;
}

int
DjVuANT::get_hor_align(GLParser & parser)
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
      } else { DEBUG_MSG("can't find any.\n"); }
   } CATCH(exc) {} ENDCATCH;
   DEBUG_MSG("resetting alignment to ALIGN_UNSPEC\n");
   return ALIGN_UNSPEC;
}

int
DjVuANT::get_ver_align(GLParser & parser)
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
      } else { DEBUG_MSG("can't find any.\n"); }
   } CATCH(exc) {} ENDCATCH;
   DEBUG_MSG("resetting alignment to ALIGN_UNSPEC\n");
   return ALIGN_UNSPEC;
}

GPList<GMapArea>
DjVuANT::get_map_areas(GLParser & parser)
{
   DEBUG_MSG("DjVuANT::get_map_areas(): forming and returning back list of map areas\n");
   DEBUG_MAKE_INDENT(3);

   GPList<GMapArea> map_areas;
   
   GPList<GLObject> list=parser.get_list();
   for(GPosition pos=list;pos;++pos)
   {
      GLObject & obj=*list[pos];
      if (obj.get_type()==GLObject::LIST && obj.get_name()==MAPAREA_TAG)
      {
	 TRY {
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

	    GLObject * shape=obj[2];
	    GP<GMapArea> map_area;
	    if (shape->get_type()==GLObject::LIST)
	    {
	       if (shape->get_name()==RECT_TAG)
	       {
		  DEBUG_MSG("it's a rectangle.\n");
		  GRect grect((*shape)[0]->get_number(),
			      (*shape)[1]->get_number(),
			      (*shape)[2]->get_number(),
			      (*shape)[3]->get_number());
		  map_area=new GMapRect(grect);
	       } else if (shape->get_name()==POLY_TAG)
	       {
		  DEBUG_MSG("it's a polygon.\n");
		  int points=shape->get_list().size()/2;
		  GTArray<int> xx(points-1), yy(points-1);
		  for(int i=0;i<points;i++)
		  {
		     xx[i]=(*shape)[2*i]->get_number();
		     yy[i]=(*shape)[2*i+1]->get_number();
		  }
		  map_area=new GMapPoly(xx, yy, points);
	       } else if (shape->get_name()==OVAL_TAG)
	       {
		  DEBUG_MSG("it's an ellipse.\n");
		  GRect grect((*shape)[0]->get_number(),
			      (*shape)[1]->get_number(),
			      (*shape)[2]->get_number(),
			      (*shape)[3]->get_number());
		  map_area=new GMapOval(grect);
	       }
	    }

	    if (map_area)
	    {
	       map_area->url=url;
	       map_area->target=target;
	       map_area->comment=comment;
	       for(int obj_num=3;obj_num<obj.get_list().size();obj_num++)
	       {
		  GLObject * el=obj[obj_num];
		  if (el->get_type()==GLObject::LIST)
		  {
		     const GString & name=el->get_name();
		     if (name==BORDER_AVIS_TAG)
			map_area->border_always_visible=true;
		     else if (name==HILITE_TAG)
		     {
			GLObject * obj=el->get_list()[el->get_list().firstpos()];
			if (obj->get_type()==GLObject::SYMBOL)
			   map_area->hilite_color=cvt_color(obj->get_symbol(), 0xff);
		     } else
		     {
			int border_type=
				   name==NO_BORDER_TAG ? GMapArea::NO_BORDER :
				   name==XOR_BORDER_TAG ? GMapArea::XOR_BORDER :
				   name==SOLID_BORDER_TAG ? GMapArea::SOLID_BORDER :
				   name==SHADOW_IN_BORDER_TAG ? GMapArea::SHADOW_IN_BORDER :
				   name==SHADOW_OUT_BORDER_TAG ? GMapArea::SHADOW_OUT_BORDER :
				   name==SHADOW_EIN_BORDER_TAG ? GMapArea::SHADOW_EIN_BORDER :
				   name==SHADOW_EOUT_BORDER_TAG ? GMapArea::SHADOW_EOUT_BORDER : -1;
			if (border_type>=0)
			{
			   map_area->border_type=(GMapArea::BorderType) border_type;
			   for(GPosition pos=el->get_list();pos;++pos)
			   {
			      GLObject * obj=el->get_list()[pos];
			      if (obj->get_type()==GLObject::SYMBOL)
				 map_area->border_color=cvt_color(obj->get_symbol(), 0xff);
			      if (obj->get_type()==GLObject::NUMBER)
				 map_area->border_width=obj->get_number();
			   }
			}
		     }	    
		  } // if (el->get_type()==...)
	       } // for(int obj_num=...)
	       map_areas.append(map_area);
	    } // if (map_area) ...
	 } CATCH(exc) {} ENDCATCH;
      } // if (...get_name()==MAPAREA_TAG)
   } // while(item==...)

   return map_areas;
}

void
DjVuANT::del_all_items(const char * name, GLParser & parser)
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
DjVuANT::encode_raw(void) const
{
   char buffer[512];
   GLParser parser;

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
      if (mode==MODE_COLOR) strcpy(buffer, "(" MODE_TAG " color)");
      else if (mode==MODE_FORE) strcpy(buffer, "(" MODE_TAG " fore)");
      else if (mode==MODE_BACK) strcpy(buffer, "(" MODE_TAG " back)");
      else if (mode==MODE_BW) strcpy(buffer, "(" MODE_TAG " bw)");
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
   
      //*** Mapareas
   del_all_items(MAPAREA_TAG, parser);
   for(GPosition pos=map_areas;pos;++pos)
      parser.parse(map_areas[pos]->print());

   MemoryByteStream str;
   parser.print(str, 1);
   GString ans;
   int size = str.size();
   str.seek(0);
   str.read(ans.getbuf(size), size);
   return ans;
}

bool
DjVuANT::is_empty(void) const
{
   GString raw=encode_raw();
   for(int i=raw.length()-1;i>=0;i--)
      if (isspace(raw[i])) raw.setat(i, 0);
      else break;
   return raw.length()==0;
}

GP<DjVuANT>
DjVuANT::copy(void) const
{
   GP<DjVuANT> ant=new DjVuANT;

      // First: copy all primitives
   *ant=*this;

      // Now process the list of hyperlinks.
   ant->map_areas.empty();
   for(GPosition pos=map_areas;pos;++pos)
      ant->map_areas.append(map_areas[pos]->get_copy());

   return ant;
}




//***************************************************************************
//******************************** DjVuTXT **********************************
//***************************************************************************


const char DjVuTXT::end_of_column    = 013;      // VT: Vertical Tab
const char DjVuTXT::end_of_region    = 035;      // GS: Group Separator
const char DjVuTXT::end_of_paragraph = 037;      // US: Unit Separator
const char DjVuTXT::end_of_line      = 012;      // LF: Line Feed

const int DjVuTXT::Zone::version  = 0;

DjVuTXT::Zone::Zone()
  : ztype(DjVuTXT::PAGE), text_start(0), text_length(0)
{
}

DjVuTXT::Zone *
DjVuTXT::Zone::append_child()
{
  Zone empty;
  empty.ztype = ztype;
  empty.text_start = 0;
  empty.text_length = 0;
  children.append(empty);
  return & children[children.lastpos()];
}

void
DjVuTXT::Zone::cleartext()
{
  text_start = 0;
  text_length = 0;
  for (GPosition i=children; i; ++i)
    children[i].cleartext();
}

void
DjVuTXT::Zone::normtext(const char *instr, GString &outstr)
{
  if (text_length == 0)
    {
      // Descend collecting text below
      text_start = outstr.length();
      for (GPosition i=children; i; ++i)
        children[i].normtext(instr, outstr);
      text_length = outstr.length() - text_start;
      // Ignore empty zones
      if (text_length == 0)
        return;
    }
  else
    {
      // Collect text at this level
      int new_start = outstr.length();
      outstr = outstr + GString(instr+text_start, text_length);
      text_start = new_start;
      // Clear textual information on lower level nodes
      for (GPosition i=children; i; ++i)
        children[i].cleartext();
    }
  // Determine standard separator
  char sep;
  switch (ztype)
    {
    case COLUMN:
      sep = end_of_column; break;
    case REGION:
      sep = end_of_region; break;
    case PARAGRAPH: 
      sep = end_of_paragraph; break;
    case LINE:
      sep = end_of_line; break;
    default:
      return;
    }
  // Add separator if not present yet.
  if (outstr[text_start+text_length-1] != sep)
    {
      outstr = outstr + GString(&sep, 1);
      text_length += 1;
    }
}

unsigned int 
DjVuTXT::Zone::memuse() const
{
  int memuse = sizeof(*this);
  for (GPosition i=children; i; ++i)
    memuse += children[i].memuse();
  return memuse;
}


#ifndef NEED_DECODER_ONLY
void 
DjVuTXT::Zone::encode(ByteStream &bs) const
{
  // Encode type
  bs.write8(ztype);
  // Encode rectangle
  bs.write24(rect.xmin);
  bs.write24(rect.xmax);
  bs.write24(rect.ymin);
  bs.write24(rect.ymax);
  // Encode text info
  bs.write24(text_start);
  bs.write24(text_length);
  // Encode number of children
  bs.write24(children.size());
  // Encode all children
  for (GPosition i=children; i; ++i)
    children[i].encode(bs);
}
#endif

void 
DjVuTXT::Zone::decode(ByteStream &bs, int maxtext)
{
  // Decode type
  ztype = (ZoneType) bs.read8();
  if ( ztype<PAGE || ztype>CHARACTER )
    THROW("Corrupted text zone hierarchy");
  // Decode rectangle
  rect.xmin = bs.read24();
  rect.xmax = bs.read24();
  rect.ymin = bs.read24();
  rect.ymax = bs.read24();
  // Decode text info
  text_start = bs.read24();
  text_length = bs.read24();
  // Get children size
  int size = bs.read24();
  // Checks
  if (rect.isempty() || text_start<0 || text_start+text_length>maxtext )
    THROW("Corrupted text zone hierarchy");
  // Process children
  children.empty();
  while (size-- > 0) {
    Zone *z = append_child();
    z->decode(bs, maxtext);
  }
}

void 
DjVuTXT::normalize_text()
{
  GString newtextUTF8;
  main.normtext( (const char*)textUTF8, newtextUTF8 );
  textUTF8 = newtextUTF8;
}

int 
DjVuTXT::has_valid_zones() const
{
  if (!textUTF8)
    return false;
  if (main.children.isempty() || main.rect.isempty()) 
    return false;
  return true;
}


#ifndef NEED_DECODER_ONLY
void 
DjVuTXT::encode(ByteStream &bs) const
{
  if (! textUTF8 )
    THROW("No text information to encode");
  // Encode text
  int textsize = textUTF8.length();
  bs.write24( textsize );
  bs.writall( (void*)(const char*)textUTF8, textsize );
  // Encode zones
  if (has_valid_zones())
    {
      bs.write8(Zone::version);
      main.encode(bs);
    }
}
#endif

void 
DjVuTXT::decode(ByteStream &bs)
{
  // Read text
  textUTF8.empty();
  int textsize = bs.read24();
  char *buffer = textUTF8.getbuf(textsize);
  int readsize = bs.read(buffer,textsize);
  buffer[readsize] = 0;
  if (readsize < textsize)
    THROW("Corrupted TXT chunk");
  // Try reading zones
  unsigned char version;
  if ( bs.read( (void*) &version, 1 ) == 1) 
    {
      if (version != Zone::version)
        THROW("Unsupported version tag in text zone information");
      main.decode(bs, textsize);
    }
}

GP<DjVuTXT> 
DjVuTXT::copy(void) const
{
  return new DjVuTXT(*this);
}

unsigned int 
DjVuTXT::get_memory_usage() const
{
  return sizeof(*this) + textUTF8.length() + main.memuse() - sizeof(main); 
}



//***************************************************************************
//******************************** DjVuAnno *********************************
//***************************************************************************

void
DjVuAnno::decode(ByteStream &bs)
{
   GString chkid;
   IFFByteStream iff(bs);
   while( iff.get_chunk(chkid) )
   {
     if (chkid == "ANTa")
       {
         if (ant) {
           ant->merge(iff);
         } else {
           ant=new DjVuANT;
           ant->decode(iff);
         }
       }
     else if (chkid == "ANTz")
       {
	 BSByteStream bsiff(iff);
	 if (ant) {
           ant->merge(bsiff);
         } else {
           ant=new DjVuANT;
           ant->decode(bsiff);
         }
       } else if (chkid == "TXTa")
       {
         if (txt)
           THROW("Duplicate TXT annotation");
         txt = new DjVuTXT;
         txt->decode(iff);
       }
     else if (chkid == "TXTz")
       {
         if (txt)
           THROW("Duplicate TXT annotation");
         txt = new DjVuTXT;
         BSByteStream bsiff(iff);
         txt->decode(bsiff);
       }
     // Add decoding of other chunks here
     iff.close_chunk();
   }
}

void
DjVuAnno::encode(ByteStream &bs)
{
  IFFByteStream iff(bs);
  if (ant)
    {
      iff.put_chunk("ANTa");
      ant->encode(iff);
      iff.close_chunk();/*
      iff.put_chunk("ANTz");
      {
	BSByteStream bsiff(iff, 50);
	ant->encode(bsiff);
      }
      iff.close_chunk();*/
    }
  if (txt)
    {
      iff.put_chunk("TXTz");
      {
	BSByteStream bsiff(iff,50);
	txt->encode(bsiff);
      }
      iff.close_chunk();
    }
  // Add encoding of other chunks here
}


GP<DjVuAnno>
DjVuAnno::copy(void) const
{
   GP<DjVuAnno> anno= new DjVuAnno;
      // Copy any primitives (if any)
   *anno=*this;
      // Copy each substructure
   if (ant) anno->ant = ant->copy();
   if (txt) anno->txt = txt->copy();
   return anno;
}

unsigned int 
DjVuAnno::get_memory_usage() const
{
  int memuse = 0;
  if (ant) memuse += ant->get_memory_usage();
  if (txt) memuse += txt->get_memory_usage();
  return memuse;
}
