//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//
// $Id: UnicodeByteStream.cpp,v 1.4 2001-04-23 18:14:22 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "UnicodeByteStream.h"
#include "ByteStream.h"

UnicodeByteStream::UnicodeByteStream(const UnicodeByteStream &uni)
: encodetype(uni.encodetype), linesread(0)
{
  bs=uni.bs;
  buffer=uni.buffer;
}

UnicodeByteStream::UnicodeByteStream(
  GP<ByteStream> ibs,const GUnicode::EncodeType et)
: encodetype(et), linesread(0)
{
  bs=ibs;
}

UnicodeByteStream::~UnicodeByteStream()
{}

static int
CountLines(const unsigned long *ptr,int len)
{
  int retval=0;
  while(len-- && ptr[0])
  {
    static const unsigned long lf='\n';
    if(ptr++[0] == lf)
    {
      retval++;
    }
  }
  return retval;
}

GUnicode 
UnicodeByteStream::gets(
  size_t const t,unsigned long const stopat,bool const inclusive)
{
  GUnicode retval;
  const unsigned int len=buffer.length();
  if(len)
  {
    if(buffer[0] == stopat)
    {
      if(inclusive)
      {
        retval=buffer.substr_nr(0,1);
        linesread+=CountLines(retval,1);
        buffer=buffer.substr(1,len);
      }
    }else
    {
      int i=1;
      for(;;)
      {
        if(i == (int)t)
        {
          retval=buffer.substr_nr(0,i);
          linesread+=CountLines(retval,i);
          buffer=buffer.substr(i,len);
          break;
        }else if(i == (int)len) 
        {
          retval=buffer.substr_nr(0,i);
          linesread+=CountLines(retval,i);
          buffer=buffer.substr(i,len);
          retval+=gets(t?(t-i):0,stopat,inclusive);
          break;
        } else if(buffer[i++] == stopat)
        {
          if(!inclusive) 
            --i;
          retval=buffer.substr_nr(0,i);
          linesread+=CountLines(retval,i);
          buffer=buffer.substr(i,len);
          break;
        }
      }
    }
  }else
  {
    unsigned char buf[1024];
    int i=read(buf,sizeof(buf));
    if(i > 0)
    {
      retval=gets(t,stopat,inclusive);
    }
  }
  return retval;
}

XMLByteStream::XMLByteStream(UnicodeByteStream &uni)
: UnicodeByteStream(uni) {}

XMLByteStream::XMLByteStream(GP<ByteStream> &ibs) 
: UnicodeByteStream(ibs,GUnicode::OTHER)
{}

GP<XMLByteStream>
XMLByteStream::create(GP<ByteStream> ibs) 
{
  XMLByteStream *xml=new XMLByteStream(ibs);
  GP<XMLByteStream> retval=xml;
  xml->init();
  return retval;
}

void
XMLByteStream::init(void)
{
  unsigned char buf[4];
  GP<ByteStream> ibs=bs;
  bs->readall(buf,sizeof(buf));
  const unsigned int i=(buf[0]<<8)+buf[1];
  switch(i)
  {
    case 0x0000:
    {
      const unsigned int j=(buf[2]<<8)+buf[3];
      switch(j)
      {
        case 0x003C:
        {
          encodetype=GUnicode::UCS4BE;
          buffer=GUnicode(buf,sizeof(buf),encodetype);
          break;
        }
        case 0x3C00:
        {
          encodetype=GUnicode::UCS4_2143;
          buffer=GUnicode(buf,sizeof(buf),encodetype);
          break;
        }
        case 0xFEFF:
        {
          encodetype=GUnicode::UCS4BE;
          buffer=GUnicode();
          break;
        }
        case 0xFFFE:
        {
          encodetype=GUnicode::UCS4_2143;
          buffer=GUnicode();
          break;
        }
        default:
        {
          encodetype=GUnicode::UTF8;
          buffer=GUnicode(buf,sizeof(buf),encodetype);
          break;
        }
      }
    }
    case 0x003C:
    {
      const unsigned int j=(buf[2]<<8)+buf[3];
      switch(j)
      {
        case 0x0000:
          encodetype=GUnicode::UCS4_3412;
          break;
        case 0x003F:
          encodetype=GUnicode::UTF16BE;
          break;
        default:
          encodetype=GUnicode::UTF8;
          break;
      }
      buffer=GUnicode(buf,sizeof(buf),encodetype);
      break;
    }
    case 0x3C00:
    {
      const unsigned int j=(buf[2]<<8)+buf[3];
      switch(j)
      {
        case 0x0000:
          encodetype=GUnicode::UCS4LE;
          break;
        case 0x3F00:
          encodetype=GUnicode::UTF16LE;
          break;
        default:
          encodetype=GUnicode::UTF8;
          break;
      }
      buffer=GUnicode(buf,sizeof(buf),encodetype);
      break;
    }
    case 0x4C6F:
    {
      const unsigned int j=(buf[2]<<8)+buf[3];
      encodetype=(j == 0xA794)?GUnicode::EBCDIC:GUnicode::UTF8;
      buffer=GUnicode(buf,sizeof(buf),encodetype);
      break;
    }
    case 0xFFFE:
    {
      encodetype=GUnicode::UTF16LE;
      buffer=GUnicode(buf+2,sizeof(buf)-2,encodetype);
      break;
    }
    case 0xFEFF:
    {
      encodetype=GUnicode::UTF16BE;
      buffer=GUnicode(buf+2,sizeof(buf)-2,encodetype);
      break;
    }
    case 0xEFBB:
    {
      encodetype=GUnicode::UTF8;
      buffer=((buf[2] == 0xBF)
        ?GUnicode(buf+3,sizeof(buf)-3,encodetype)
        :GUnicode(buf,sizeof(buf),encodetype));
      break;
    }
    case 0x3C3F:
    default:
    {
      encodetype=GUnicode::UTF8;
      buffer=GUnicode(buf,sizeof(buf),GUnicode::UTF8);
    }
  }
  bs=ibs;
}

XMLByteStream::~XMLByteStream()
{}



