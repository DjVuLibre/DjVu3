#C-
#C- DjVu® Reference Library (v. 3.5)
#C- 
#C- Copyright © 1999-2001 LizardTech, Inc. All Rights Reserved.
#C- The DjVu Reference Library is protected by U.S. Pat. No.
#C- 6,058,214 and patents pending.
#C- 
#C- This software is subject to, and may be distributed under, the
#C- GNU General Public License, Version 2. The license should have
#C- accompanied the software or you may obtain a copy of the license
#C- from the Free Software Foundation at http://www.fsf.org .
#C- 
#C- The computer code originally released by LizardTech under this
#C- license and unmodified by other parties is deemed the "LizardTech
#C- Original Code."
#C- 
#C- With respect to the LizardTech Original Code ONLY, and subject
#C- to any third party intellectual property claims, LizardTech
#C- grants recipient a worldwide, royalty-free, non-exclusive license
#C- under patent claims now or hereafter owned or controlled by
#C- LizardTech that are infringed by making, using, or selling
#C- LizardTech Original Code, but solely to the extent that any such
#C- patent(s) is/are reasonably necessary to enable you to make, have
#C- made, practice, sell, or otherwise dispose of LizardTech Original
#C- Code (or portions thereof) and not to any greater extent that may
#C- be necessary to utilize further modifications or combinations.
#C- 
#C- The LizardTech Original Code is provided "AS IS" WITHOUT WARRANTY
#C- OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
#C- TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
#C- MERCHANTIBILITY OR FITNESS FOR A PARTICULAR PURPOSE.
#
# $Id: Makefile.am,v 1.5 2001-08-24 21:50:10 docbill Exp $
# $Name:  $




SUBDIRS    =  

INCLUDES   =  $(all_includes)

# ------------------------------------------------------------------------------
#   Define Primary Variables for Targets, Sources, Headers, etc.
# ------------------------------------------------------------------------------

#
# Local primary variables
#

LDADD = $(LIBDJVUXX_LDPATH)/$(LIBDJVUXX)

core_check_programs = \
	TestArray \
	TestBitmap \
	TestException \
	TestJB2 \
	TestList \
	TestMap \
	TestSmartPointer \
	TestString

extra_check_programs = 	TestThreads

check_PROGRAMS = $(core_check_programs)

##
## Program sources.
##

TestArray_SOURCES = TestArray.cpp

TestBitmap_SOURCES = TestBitmap.cpp

TestException_SOURCES = TestException.cpp

TestJB2_SOURCES = TestJB2.cpp

TestList_SOURCES = TestList.cpp

TestMap_SOURCES = TestMap.cpp

TestSmartPointer_SOURCES = TestSmartPointer.cpp

TestString_SOURCES = TestString.cpp

# TestThreads_SOURCES = TestThreads.cpp

##
## Dependencies
##

TestArray_DEPENDENCIES = $(LIBDJVUXX_LDPATH)/$(LIBDJVUXX)

TestBitmap_DEPENDENCIES = $(LIBDJVUXX_LDPATH)/$(LIBDJVUXX)

TestException_DEPENDENCIES = $(LIBDJVUXX_LDPATH)/$(LIBDJVUXX)

TestList_DEPENDENCIES = $(LIBDJVUXX_LDPATH)/$(LIBDJVUXX)

TestSmartPointer_DEPENDENCIES = $(LIBDJVUXX_LDPATH)/$(LIBDJVUXX)

TestString_DEPENDENCIES = $(LIBDJVUXX_LDPATH)/$(LIBDJVUXX)

# TestThreads_DEPENDENCIES = $(LIBDJVUXX_LDPATH)/$(LIBDJVUXX)

#
# Local rules
#


$(LIBDJVUXX_LDPATH)/$(LIBDJVUXX) :
	cd $(LIBDJVUXX_BUILDDIR); make

check-local : run-tests-local

run-tests-local : $(check_PROGRAMS)
	@test_list=`echo $^`; \
	for a_test in $$test_list; do \
	  $(ECHO) $(ECHO_N)  "Running Test:  "; \
	  $(ECHO) $(ECHO_N) "$$a_test:  "; \
          if test -f $(srcdir)/$$a_test.exp ; then \
	     if ./$$a_test > $$a_test.out 2>&1 \
	          && cmp -s ./$$a_test.out $(srcdir)/$$a_test.exp; then \
	       $(ECHO) passed; \
	     else \
	       $(ECHO) failed; \
               $(ECHO) $$a_test begin; cat $$a_test.out; echo $$a_test end; \
	     fi; \
	     else \
	       $(ECHO) compiled; \
          fi ; \
	done;







#           if $@ > $@.out 2>&1  &&  cmp -s $@.out $(SRCDIR)/$@.exp  ;\
#             then echo -- $@ passed ;\
#             else echo -- $@ begin ; cat $@.out ; echo -- $@ end ;\
#            else echo -- $@ compiled ;\

# $(ECHO) $^

