

TARG= 	TestString TestException TestArray TestList TestMap\
	TestBitmap TestSmartPointer TestJB2

NTARG=  TestParser TestIFF TestDir TestHLO


LIB= 	  ../libdjvu++.a
SHELL=    /bin/sh
CC=       egcs
CXX=      eg++
OPT=      -g
CXXFLAGS= $(OPT) -I..
LDFLAGS=
LDLIBS=
THREADLIBS= # -lpthread

# Make all

all: $(TARG)
	echo Done.

$(TARG): $(LIB)

$(LIB): PHONY
	@echo ----- Making library
	( cd .. ; ${MAKE} libdjvu++.a )
	@echo ----- Done library



# Generic rules

clean: PHONY
	-@rm 2>/dev/null -rf $(TARG) *.o *.out ii_files
	-@echo Cleaned.

depend: PHONY
	sed -e '/^### DEPENDENCIES$$/,$$d' Makefile > Makefile.new
	echo "### DEPENDENCIES" >> Makefile.new
	for n in *.cpp ; do ${CXX} -MM $(CXXFLAGS) $$n >> Makefile.new ; done
	sed -e '1,$$s/^\(Test[A-Za-z0-9]*\)\.o:/\1:/g' < Makefile.new > Makefile
	rm Makefile.new

PHONY:


# Handle cpp files as C++
.SUFFIXES: .cpp
.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< 

.cpp:
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $< $(LIB) $(LDLIBS)
	@if test -f $@.exp ; then\
	   if $@ > $@.out 2>&1  &&  cmp -s $@.out $@.exp  ;\
	     then echo -- $@ passed ;\
	     else echo -- $@ begin ; cat $@.out ; echo -- $@ end ;\
           fi ;\
	   else echo -- $@ compiled ;\
	 fi

TestThreads:
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o TestThreads TestThreads.cpp $(LIB) $(THREADLIBS)
	@if test -f $@.exp ; then\
	   if $@ 2>&1 | sort > $@.out 2>&1  &&  cmp -s $@.out $@.exp  ;\
	     then echo -- $@ passed ;\
	     else echo -- $@ begin ; cat $@.out ; echo -- $@ end ;\
           fi ;\
	   else echo -- $@ compiled ;\
	 fi


### DEPENDENCIES
TestArray: TestArray.cpp ../GContainer.h ../DjVuGlobal.h \
 ../GException.h ../GString.h ../GSmartPointer.h
TestBitmap: TestBitmap.cpp ../GBitmap.h ../DjVuGlobal.h ../GRect.h \
 ../ByteStream.h ../GException.h ../GContainer.h ../GSmartPointer.h
TestException: TestException.cpp ../GException.h ../DjVuGlobal.h \
 ../GString.h ../GSmartPointer.h
TestJB2: TestJB2.cpp ../JB2Image.h ../DjVuGlobal.h ../GString.h \
 ../GException.h ../GSmartPointer.h ../GContainer.h ../ZPCodec.h \
 ../ByteStream.h ../GRect.h ../GBitmap.h
TestList: TestList.cpp ../GContainer.h ../DjVuGlobal.h \
 ../GException.h ../GString.h ../GSmartPointer.h
TestMap: TestMap.cpp ../GContainer.h ../DjVuGlobal.h ../GException.h \
 ../GString.h ../GSmartPointer.h
TestSmartPointer: TestSmartPointer.cpp ../GSmartPointer.h \
 ../DjVuGlobal.h
TestString: TestString.cpp ../GString.h ../DjVuGlobal.h \
 ../GException.h ../GSmartPointer.h
TestThreads: TestThreads.cpp ../GThreads.h ../DjVuGlobal.h \
 ../GException.h
