//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2000 LizardTech, Inc. All Rights Reserved.
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
//C- 
// 
// $Id: GIFFManager.cpp,v 1.11 2000-11-09 20:15:06 jmw Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "GIFFManager.h"
#include "GException.h"
#include "debug.h"

void
GIFFChunk::set_name(const char * name)
{
   DEBUG_MSG("GIFFChunk::set_name(): name='" << name << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   const char * colon;
   if ((colon=strchr(name, ':')))
   {
      type=GString(name, colon-name);
      name=colon+1;
      if (strchr(name, ':'))
	 G_THROW("GIFFManager.one_colon");
   };

   DEBUG_MSG("auto-setting type to '" << type << "'\n");

   if (strpbrk(name, ".[]"))
      G_THROW("GIFFManager.bad_char");
   
   strncpy(GIFFChunk::name, name, 4); GIFFChunk::name[4]=0;
   for(int i=strlen(GIFFChunk::name);i<4;i++) GIFFChunk::name[i]=' ';
}

bool
GIFFChunk::check_name(const char * name)
{
   GString type;
   
   const char * colon;
   if ((colon=strchr(name, ':')))
   {
      type=GString(name, colon-name);
      name=colon+1;
   };
   
   GString sname=GString(name, 4);
   for(int i=sname.length();i<4;i++) sname.setat(i, ' ');

   DEBUG_MSG("GIFFChunk::check_name(): type='" << type << "' name='" << sname << "'\n");
   return (type==GIFFChunk::type || !type.length() && GIFFChunk::type=="FORM")
       && sname==GIFFChunk::name;
}

void
GIFFChunk::save(IFFByteStream & istr, bool use_trick)
{
   DEBUG_MSG("GIFFChunk::save(): saving chunk '" << get_full_name() << "'\n");
   DEBUG_MAKE_INDENT(3);

   if (is_container())
   {
      istr.put_chunk(get_full_name(), use_trick);
      if (chunks.size())
      {
	 GPosition pos;
	 for(pos=chunks;pos;++pos)
	    if (chunks[pos]->get_type()=="PROP")
	       chunks[pos]->save(istr);
	 for(pos=chunks;pos;++pos)
	    if (chunks[pos]->get_type()!="PROP")
	       chunks[pos]->save(istr);
      } else
      {
	 DEBUG_MSG("but it's empty => saving empty container.\n");
      };
      istr.close_chunk();
   } else
   {
      istr.put_chunk(get_name(), use_trick);
      istr.writall((const char *) data, data.size());
      istr.close_chunk();
   };
}

void
GIFFChunk::add_chunk(const GP<GIFFChunk> & chunk, int position)
{
   DEBUG_MSG("GIFFChunk::add_chunk(): Adding chunk to '" << get_name() <<
	     "' @ position=" << position << "\n");
   DEBUG_MAKE_INDENT(3);

   if (!type.length())
   {
      DEBUG_MSG("Converting the parent to FORM\n");
      type="FORM";
   };

   if (chunk->get_type()=="PROP")
   {
      DEBUG_MSG("Converting the parent to LIST\n");
      type="LIST";
   };

   GPosition pos;
   if (position>=0 && chunks.nth(position, pos))
      chunks.insert_before(pos, chunk);
   else chunks.append(chunk);
}

void
GIFFChunk::decode_name(const char * name, GString * short_name_ptr,
		       int * number_ptr)
{
   DEBUG_MSG("GIFFChunk::decode_name(): Checking brackets in name '" << name << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   if (strchr(name, '.')) G_THROW("GIFFManager.no_dots");

   int number=0;
   const char * obracket;
   if ((obracket=strchr(name, '[')))
   {
      const char * cbracket;
      if (!(cbracket=strchr(obracket+1, ']')))
	 G_THROW("GIFFManager.unmatched");
      number=atoi(GString(obracket+1, cbracket-obracket-1));
      if (cbracket[1]) G_THROW("GIFFManager.garbage");
   };

   GString short_name=GString(name, obracket-name);
   int colon=short_name.search(':');
   if (colon>=0) short_name=(const char *) short_name+(colon+1);
   for(int i=short_name.length();i<4;i++) short_name.setat(i, ' ');
   
   DEBUG_MSG("short_name='" << short_name << "'\n");
   DEBUG_MSG("number=" << number << "\n");
   
   if (number_ptr) *number_ptr=number;
   if (short_name_ptr) *short_name_ptr=short_name;
}

void
GIFFChunk::del_chunk(const char * name)
   // The name may contain brackets to specify the chunk number
{
   DEBUG_MSG("GIFFChunk::del_chunk(): Deleting chunk '" << name <<
	     "' from '" << get_name() << "'\n");
   DEBUG_MAKE_INDENT(3);

   GString short_name;
   int number;
   decode_name(name, &short_name, &number);

   int num=0;
   for(GPosition pos=chunks;pos;++pos)
      if (chunks[pos]->get_name()==short_name && num++==number)
      {
	 chunks.del(pos);
	 return;
      };
 
/*    Original preserved in case I screw things up      
   char * buffer=new char[256];
   sprintf(buffer, "There is no subchunk '%s' #%d in chunk '%s'.",
	   (const char *) short_name, number, (const char *) get_name());
   G_THROW(buffer);
*/
   G_THROW(GString("GIFFManager.no_chunk\t")+short_name+"\t"+GString(number)+"\t"+get_name());
}

GP<GIFFChunk>
GIFFChunk::get_chunk(const char * name, int * pos_ptr)
   // The name may contain brackets to specify the chunk number
{
   DEBUG_MSG("GIFFChunk::get_chunk(): Returning chunk '" << name <<
	     "' from '" << get_name() << "'\n");
   DEBUG_MAKE_INDENT(3);

   GString short_name;
   int number;
   decode_name(name, &short_name, &number);

   int num=0;
   int pos_num;
   GPosition pos;
   for(pos=chunks, pos_num=0;pos;++pos, pos_num++)
      if (chunks[pos]->get_name()==short_name && num++==number)
      {
	 if (pos_ptr) *pos_ptr=pos_num;
	 return chunks[pos];
      };

   return 0;
}

int
GIFFChunk::get_chunks_number(const char * name)
{
   DEBUG_MSG("GIFFChunk::get_chunks_number(): Returning number of chunks '" << name <<
	     "' in '" << get_name() << "'\n");
   DEBUG_MAKE_INDENT(3);

   if (!name) return chunks.size();
   
   if (strpbrk(name, "[]")) G_THROW("GIFFManager.no_brackets");
   
   GString short_name;
   int number;
   decode_name(name, &short_name, &number);
   
   int num=0;
   for(GPosition pos=chunks;pos;++pos)
      num+=(chunks[pos]->get_name()==short_name);
   return num;
}

//************************************************************************

void
GIFFManager::add_chunk(const char * parent_name, const GP<GIFFChunk> & chunk,
		       int pos)
      // parent_name is the fully qualified name of the PARENT
      //             IT MAY BE EMPTY
      // All the required chunks will be created
      // pos=-1 means to append the chunk
{
   DEBUG_MSG("GIFFManager::add_chunk(): Adding chunk to name='" << parent_name << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   if (!top_level->get_name().length())
   {
      if (!parent_name || !strlen(parent_name) || parent_name[0]!='.')
	 G_THROW("GIFFManager.no_top_name");
      if (!parent_name[1])
      {
	    // 'chunk' is actually the new top-level chunk
	 DEBUG_MSG("since parent_name=='.', making the chunk top-level\n");
	 if (!chunk->is_container()) G_THROW("GIFFManager.no_top_cont");
	 top_level=chunk;
	 return;
      };
      DEBUG_MSG("Setting the name of the top-level chunk\n");
      const char * next_dot=strchr(parent_name+1, '.');
      if (!next_dot) top_level->set_name(parent_name+1);
      else top_level->set_name(GString(parent_name+1, next_dot-parent_name-1));
   };

   DEBUG_MSG("top level chunk name='" << top_level->get_name() << "'\n");
   
   if (parent_name && strlen(parent_name) && parent_name[0]=='.')
   {
      const char * next_dot=strchr(parent_name+1, '.');
      if (!next_dot) next_dot=parent_name+strlen(parent_name);
      GString top_name=GString(parent_name+1, next_dot-parent_name-1);
      if (!top_level->check_name(top_name))
	 G_THROW("GIFFManager.wrong_name\t"+top_name);
      parent_name=next_dot;
   };

   GP<GIFFChunk> cur_sec=top_level;
   const char * start, * end=parent_name-1;
   do
   {
      for(start=++end;*end;end++)
	 if (*end=='.') break;
      if (end>start)
      {
	 char name[128];
	 char short_name[128];
	 strncpy(name, start, end-start);
	 name[end-start]=0;
	 strcpy(short_name, name);

	 int number=0;
	 char * obracket=strchr(short_name, '[');
	 if (obracket)
	 {
	    char * cbracket=strchr(obracket+1, ']');
	    if (!cbracket) G_THROW("GIFFManager.unbalanced");
	    number=atoi(GString(obracket+1, cbracket-obracket-1));
	    *obracket=0;
	 };

	 for(int i=cur_sec->get_chunks_number(short_name);i<number+1;i++)
	    cur_sec->add_chunk(new GIFFChunk(short_name));
	 cur_sec=cur_sec->get_chunk(name);
	 if (!cur_sec) G_THROW("GIFFManager.unknown\t"+GString(name));
      };
   } while(*end);
   
   cur_sec->add_chunk(chunk, pos);
}

void
GIFFManager::add_chunk(const char * name, const TArray<char> & data)
      // name is fully qualified name of the chunk TO BE INSERTED.
      //      it may contain brackets at the end to set the position
      // All the required chunks will be created
{
   DEBUG_MSG("GIFFManager::add_chunk(): adding plain chunk with name='" << name << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   const char * short_name=strrchr(name, '.');
   if (!short_name) short_name=name;
   else short_name++;

   int pos=-1;
   const char * obracket=strchr(short_name, '[');
   if (obracket)
   {
      const char * cbracket=strchr(obracket+1, ']');
      if (!cbracket) G_THROW("GIFFManager.unbalanced");
      pos=atoi(GString(obracket+1, cbracket-obracket-1));
      if (cbracket[1]) G_THROW("GIFFManager.garbage");
   };
   GString chunk_name=GString(short_name, obracket-short_name);
   DEBUG_MSG("Creating new chunk with name " << chunk_name << "\n");
   GP<GIFFChunk> chunk;
   chunk=new GIFFChunk(chunk_name, data);
   add_chunk(GString(name, short_name-name), chunk, pos);
}

void
GIFFManager::del_chunk(const char * name)
      // "name" should be fully qualified, that is contain dots.
      // It may also end with [] to set the chunk order number
{
   DEBUG_MSG("GIFFManager::del_chunk(): Deleting chunk '" << name << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   if (!name || !strlen(name))
      G_THROW("GIFFManager.del_empty");

   if (name[0]=='.')
   {
      char * next_dot=strchr(name+1, '.');
      if (!next_dot)
      {
	 if (top_level->check_name(name+1))
	 {
	    DEBUG_MSG("Removing top level chunk..\n");
	    top_level=new GIFFChunk();
	    return;
	 } else
	    G_THROW("GIFFManager.wrong_name2\t"+GString(name+1));
      };
      GString top_name=GString(name+1, next_dot-name-1);
      if (!top_level->check_name(top_name))
	 G_THROW("GIFFManager.wrong_name2\t"+top_name);
      name=next_dot+1;
   };
   
   GP<GIFFChunk> cur_sec=top_level;
   const char * start, * end=name-1;
   do
   {
      for(start=++end;*end;end++)
	 if (*end=='.') break;
      if (end>start && *end=='.')
	 cur_sec=cur_sec->get_chunk(GString(start, end-start));
      if (!cur_sec) G_THROW("GIFFManager.cant_find\t"+GString(name));
   } while(*end);
   
   if (!strlen(start))
   {
     /*     Original preserved in case I screw things up
      char * buffer=new char[128];
      sprintf(buffer, "Malformed chunk name '%s': ends with a dot.", name);
      G_THROW(buffer);
     */
     G_THROW(GString("GIFFManager.malformed\t")+name);
   };
   
   cur_sec->del_chunk(start);
}

GP<GIFFChunk>
GIFFManager::get_chunk(const char * name, int * pos_num)
      // "name" should be fully qualified, that is contain dots.
      // It may also end with [] to set the chunk order number
{
   DEBUG_MSG("GIFFManager::get_chunk(): Returning chunk '" << name << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   if (!name || !strlen(name))
      G_THROW("GIFFManager.get_empty");

   if (name[0]=='.')
   {
      char * next_dot=strchr(name+1, '.');
      if (!next_dot)
      {
	       if (top_level->check_name(name+1))
	       {
	          DEBUG_MSG("Returning top level chunk..\n");
	          return top_level;
	       } else
	          G_THROW("GIFFManager.wrong_name2\t"+GString(name+1));
      };
      GString top_name=GString(name+1, next_dot-name-1);
      if (!top_level->check_name(top_name))
	       G_THROW("GIFFManager.wrong_name2\t"+top_name);
      name=next_dot+1;
   };
   
   GP<GIFFChunk> cur_sec=top_level;
   const char * start, * end=(const char *) name-1;
   do
   {
      for(start=++end;*end;end++)
	 if (*end=='.') break;
      if (end>start) cur_sec=cur_sec->get_chunk(GString(start, end-start), pos_num);
      if (!cur_sec) return 0;
   } while(*end);
   
   return cur_sec;
}

int
GIFFManager::get_chunks_number(const char * name)
   // Returns the number of chunks with given fully qualified name
{
   DEBUG_MSG("GIFFManager::get_chunks_number(): name='" << name << "'\n");
   DEBUG_MAKE_INDENT(3);

   if (!name) return top_level->get_chunks_number();
   
   const char * last_dot=strrchr(name, '.');
   if (!last_dot) return top_level->get_chunks_number(name);
   else
   {
      if (last_dot==name) return top_level->get_name()==(name+1);

      GP<GIFFChunk> chunk=get_chunk(GString(name, last_dot-name));
      if (chunk) return chunk->get_chunks_number(last_dot+1);
      else return 0;
   };
}

void
GIFFManager::load_chunk(IFFByteStream & istr, GP<GIFFChunk> chunk)
{
   DEBUG_MSG("GIFFManager::load_chunk(): loading contents of chunk '" <<
	     chunk->get_name() << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   int chunk_size;
   GString chunk_id;
   while ((chunk_size=istr.get_chunk(chunk_id)))
   {
      if (istr.check_id(chunk_id))
      {
	 GP<GIFFChunk> ch=new GIFFChunk(chunk_id);
	 load_chunk(istr, ch);
	 chunk->add_chunk(ch);
      } else
      {
	 TArray<char> data(chunk_size-1);
	 istr.readall( (char*)data, data.size());
	 GP<GIFFChunk> ch=new GIFFChunk(chunk_id, data);
	 chunk->add_chunk(ch);
      };
      istr.close_chunk();
   };
}

void
GIFFManager::load_file(const TArray<char> & data)
{
   MemoryByteStream str(data, data.size());
   load_file(str);
}

void
GIFFManager::load_file(ByteStream & str)
{
   DEBUG_MSG("GIFFManager::load_file(): Loading IFF file.\n");
   DEBUG_MAKE_INDENT(3);
   
   IFFByteStream istr(str);
   GString chunk_id;
   if (istr.get_chunk(chunk_id))
   {
      if (strncmp(chunk_id, "FORM:", 5))
	       G_THROW("GIFFManager.cant_find2");
      set_name(chunk_id);
      load_chunk(istr, top_level);
      istr.close_chunk();
   };
}

void
GIFFManager::save_file(TArray<char> & data)
{
   MemoryByteStream str;
   save_file(str);

   data=str.get_data();
}

void
GIFFManager::save_file(ByteStream & str)
{
   IFFByteStream istr(str);
   
   top_level->save(istr, 1);
}
