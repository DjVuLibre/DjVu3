//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: cin_data.cpp,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "cin_data.h"
#include "BSByteStream.h"

CINData::Item::Item(const char * name_, const char * data_, int length_)
{
   name=name_;
   data.resize(length_-1);
   memcpy(data, data_, length_);
}

GPList<CINData::Item>	* CINData::items;

GPList<CINData::Item> *
CINData::get_items(void)
{
   if (!items) items=new GPList<Item>();
   return items;
}

GP<ByteStream>
CINData::get(const char * name)
{
   GPList<Item> * items=get_items();

   for(GPosition pos=*items;pos;++pos)
   {
      GP<Item> item=(*items)[pos];
      if (item->name==name)
      {
	    // Decompress data and put it into a MemoryByteStream
	 GP<ByteStream> mem_str_in=ByteStream::create((const char *) item->data, item->data.size());
	 GP<ByteStream> bs_str=BSByteStream::create(mem_str_in);
	 
	 GP<ByteStream> mem_str_out=ByteStream::create();
	 mem_str_out->copy(*bs_str);
	 mem_str_out->seek(0);
	 return mem_str_out;	 
      }
   }
   return 0;
}

void
CINData::add(const char * name, const char * data, int length)
{
   GP<Item> new_item=new Item(name, data, length);
   
   GPList<Item> * items=get_items();

   for(GPosition pos=*items;pos;++pos)
   {
      GP<Item> item=(*items)[pos];
      if (item->name==name)
      {
	 *item=*new_item;
	 return;
      }
   }

   items->append(new_item);
}
