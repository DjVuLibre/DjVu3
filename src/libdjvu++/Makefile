# Makefile for libDejaVu.a

OBJS= 	GString.o		\
	GException.o		\
	GContainer.o		\
	GRect.o			\
	GSmartPointer.o		\
	GThreads.o		\
	ByteStream.o		\
	ZPCodec.o		\
	GBitmap.o		\
	GPixmap.o		\
	IFFByteStream.o		\
	BSByteStream.o		\
	JB2Image.o		\
	GScaler.o               \
	IWImage.o		\
	DjVuImage.o             \
	GOS.o                   \
	debug.o	

SHELL=/bin/sh
TARG= libDejaVu.a
PROG= showiff c44 d44 djvutopnm makedjvu breakdjvu ppmcoco bzz

#CC=gcc
#CXX=g++
CC=egcs
CXX=eg++
OPT=-O3 -fomit-frame-pointer -funroll-all-loops
CXXFLAGS= $(OPT) -Wall -Wno-unused -DUNIX
CFLAGS= $(OPT) -Wall -Wno-unused -DUNIX
LDFLAGS=
SOFLAGS= $(LDFLAGS) -shared -Wl,-no_unresolved
LDLIBS=-lm
SOLIBS= $(LDLIBS) -lc -lgcc -lc
RANLIB= :
DOCXX= doc++

# Make all

all: $(TARG) $(PROG) 
	echo Done.

# Rule for building the library

$(TARG): $(OBJS) Makefile
	if [ -r $(TARG) ] ; then rm $(TARG) ; fi
	ar cq $(TARG) $(OBJS)
	$(RANLIB) $(TARG)

# Rules for building support programs

C44OBJS = c44.o
D44OBJS = d44.o
SHOWIFFOBJS = showiff.o
DEJAVUTOPNMOBJS = djvutopnm.o
MAKEDEJAVUOBJS = makedjvu.o
BREAKDEJAVUOBJS = breakdjvu.o
PPMCOCOOBJS = ppmcoco.o
ANNOTATEOBJS = annotate.o
BZZOBJS = bzz.o
LIBDJVUOBJS = libdjvu.o 

c44: $(C44OBJS) $(TARG) Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@  $(C44OBJS) -L. -lDejaVu $(LDLIBS)

d44: $(D44OBJS) $(TARG) Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@  $(D44OBJS) -L. -lDejaVu $(LDLIBS)

showiff: $(SHOWIFFOBJS) $(TARG) Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@  $(SHOWIFFOBJS) -L. -lDejaVu $(LDLIBS)

djvutopnm: $(DEJAVUTOPNMOBJS) $(TARG) Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@  $(DEJAVUTOPNMOBJS) -L. -lDejaVu $(LDLIBS)

makedjvu: $(MAKEDEJAVUOBJS) $(TARG) Makefile	
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@  $(MAKEDEJAVUOBJS) -L. -lDejaVu $(LDLIBS)

breakdjvu: $(BREAKDEJAVUOBJS) $(TARG) Makefile	
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@  $(BREAKDEJAVUOBJS) -L. -lDejaVu $(LDLIBS)

ppmcoco: $(PPMCOCOOBJS) $(TARG) Makefile	
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@  $(PPMCOCOOBJS) -L. -lDejaVu $(LDLIBS)

annotate: $(ANNOTATEOBJS) $(TARG) Makefile	
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@  $(ANNOTATEOBJS) -L. -lDejaVu $(LDLIBS)

bzz: $(BZZOBJS) $(TARG) Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@  $(BZZOBJS) -L. -lDejaVu $(LDLIBS)

# Documentation

html:
	-rm -rf @Doc
	-mkdir @Doc 2>/dev/null
	${DOCXX} -d @Doc -B libDejaVu.html libDejaVu.dxx

dvi:
	-@mkdir @Doc 2>/dev/null
	${DOCXX} -t -o @Doc/libDejaVu.tex libDejaVu.dxx
	( cd @Doc ; latex libDejaVu.tex ; latex libDejaVu.tex )

# Generic rules

clean:
	-@rm 2>/dev/null -rf $(PROG) *.o *.a *.out ii_files
	-@echo Cleaned.

depend:
	sed -e '/^### DEPENDENCIES$$/,$$d' Makefile > Makefile.new
	echo "### DEPENDENCIES" >> Makefile.new
	for n in *.cpp ; do ${CXX} -MM $(CXXFLAGS) $$n >> Makefile.new ; done
	sed -e '1,$$s/^\(Test[A-Za-z0-9]*\)\.o:/\1:/g' < Makefile.new > Makefile
	rm Makefile.new

PHONY: html dvi clean depend disk


# Handle cpp files as C++
.SUFFIXES: .cpp
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< 



### DEPENDENCIES
BSByteStream.o: BSByteStream.cpp BSByteStream.h DjVuGlobal.h \
 GException.h ByteStream.h GContainer.h GSmartPointer.h ZPCodec.h
ByteStream.o: ByteStream.cpp ByteStream.h DjVuGlobal.h GException.h \
 GContainer.h GSmartPointer.h
DjVuImage.o: DjVuImage.cpp DjVuImage.h GString.h DjVuGlobal.h \
 GException.h GSmartPointer.h GThreads.h ByteStream.h GContainer.h \
 IFFByteStream.h JB2Image.h ZPCodec.h GRect.h GBitmap.h IWImage.h \
 GPixmap.h GScaler.h
GBitmap.o: GBitmap.cpp GBitmap.h DjVuGlobal.h GRect.h ByteStream.h \
 GException.h GContainer.h GSmartPointer.h GString.h
GContainer.o: GContainer.cpp GContainer.h DjVuGlobal.h GException.h
GException.o: GException.cpp GException.h DjVuGlobal.h debug.h
GOS.o: GOS.cpp GException.h DjVuGlobal.h GThreads.h GOS.h GString.h \
 GSmartPointer.h
GPixmap.o: GPixmap.cpp GString.h DjVuGlobal.h GException.h \
 GSmartPointer.h ByteStream.h GContainer.h GRect.h GBitmap.h GPixmap.h \
 GThreads.h
GRect.o: GRect.cpp GRect.h DjVuGlobal.h GException.h
GScaler.o: GScaler.cpp GScaler.h GException.h DjVuGlobal.h GRect.h \
 GBitmap.h ByteStream.h GContainer.h GSmartPointer.h GPixmap.h
GSmartPointer.o: GSmartPointer.cpp GThreads.h DjVuGlobal.h \
 GException.h GSmartPointer.h
GString.o: GString.cpp GString.h DjVuGlobal.h GException.h \
 GSmartPointer.h
GThreads.o: GThreads.cpp GThreads.h DjVuGlobal.h GException.h
IFFByteStream.o: IFFByteStream.cpp IFFByteStream.h DjVuGlobal.h \
 GException.h GString.h GSmartPointer.h ByteStream.h GContainer.h
IWImage.o: IWImage.cpp GRect.h DjVuGlobal.h GException.h \
 GSmartPointer.h ZPCodec.h ByteStream.h GContainer.h IWImage.h \
 GBitmap.h GPixmap.h IFFByteStream.h GString.h
JB2Image.o: JB2Image.cpp JB2Image.h DjVuGlobal.h GString.h \
 GException.h GSmartPointer.h GContainer.h ZPCodec.h ByteStream.h \
 GRect.h GBitmap.h
ZPCodec.o: ZPCodec.cpp ZPCodec.h DjVuGlobal.h ByteStream.h \
 GException.h GContainer.h GSmartPointer.h
breakdjvu.o: breakdjvu.cpp GException.h DjVuGlobal.h ByteStream.h \
 GContainer.h GSmartPointer.h IFFByteStream.h GString.h
bzz.o: bzz.cpp GException.h DjVuGlobal.h ByteStream.h GContainer.h \
 GSmartPointer.h BSByteStream.h ZPCodec.h
c44.o: c44.cpp GString.h DjVuGlobal.h GException.h GSmartPointer.h \
 IWImage.h GRect.h GBitmap.h ByteStream.h GContainer.h GPixmap.h \
 IFFByteStream.h GOS.h
d44.o: d44.cpp GString.h DjVuGlobal.h GException.h GSmartPointer.h \
 IWImage.h GRect.h GBitmap.h ByteStream.h GContainer.h GPixmap.h \
 IFFByteStream.h GOS.h
debug.o: debug.cpp
djvutopnm.o: djvutopnm.cpp GException.h DjVuGlobal.h GSmartPointer.h \
 GRect.h GPixmap.h ByteStream.h GContainer.h GBitmap.h DjVuImage.h \
 GString.h GThreads.h IFFByteStream.h JB2Image.h ZPCodec.h IWImage.h \
 GOS.h
makedjvu.o: makedjvu.cpp GString.h DjVuGlobal.h GException.h \
 GSmartPointer.h DjVuImage.h GThreads.h ByteStream.h GContainer.h \
 IFFByteStream.h JB2Image.h ZPCodec.h GRect.h GBitmap.h IWImage.h \
 GPixmap.h
ppmcoco.o: ppmcoco.cpp ByteStream.h DjVuGlobal.h GException.h \
 GContainer.h GSmartPointer.h GPixmap.h GRect.h GBitmap.h GString.h
showiff.o: showiff.cpp GException.h DjVuGlobal.h GString.h \
 GSmartPointer.h ByteStream.h GContainer.h IFFByteStream.h DjVuImage.h \
 GThreads.h JB2Image.h ZPCodec.h GRect.h GBitmap.h IWImage.h GPixmap.h
