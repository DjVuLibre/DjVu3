//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: Template.h,v 1.14 2001-01-04 22:04:55 bcr Exp $
// $Name:  $

//T// This is a template for the header files in the
//T// DjVu reference library. It describes the general
//T// conventions as well as the documentation. 
//T// Comments prefixed with '//T//' explain the template
//T// features and should be removed.

#ifndef _TEMPLATE_H_
#define _TEMPLATE_H_

//T// GCC stuff reducing template overhead
#ifdef __GNUC__
#pragma interface
#endif

//T// Always include "DjVuGlobal.h"
#include "DjVuGlobal.h"

//T// Other include files
#include <string.h>
#include "GException.h"



/** @name Template.h
    
    Files #"Template.h"# and #"Template.cpp"# are not used for anything but
    the current programming and documentation standards in the DjVu reference
    library. This doc++ comment briefly describes the abstractions defined in
    this files.  It must mention all the files involved in implementing this
    features, as well as references to the main classes \Ref{classname}.  

    This comment may contain additional sections as follows:

    {\bf Algorithmic Remarks} --- Comments about the algorithms, their
    performance and their limitations.

    {\bf Historical Remarks} --- Comments about the successive revisions of
    this file and other anecdotical details. This is where we can amuse the
    reader with funny details.

    {\bf ToDo} --- Things that we have been thinking to do but did not
    fully implement yet. It should explain how we plan to modify the current
    code could be modified to implement these things. People who change this
    code thus should avoid jeopardizing these plans.
    
    {\bf Example} --- It would be cool to demonstrate how these functions
    can be used by providing a small segment of C/C++ code.
    \begin{verbatim}
       ExampleClass toto(3,4);
       toto.draw(mywin);
    \end{verbatim}

    This main doc++ comment is followed by a few doc++ entries.
    \begin{itemize}
    \item the "memo" field contains a single line description of the file.
    \item the "version" field contains a cvs magic expression.
    \item the author fields contains a list of authors and email addresses.
          (the #\\# termination breaks the line.)
    \end{itemize}

    @memo 
    Template header file
    @version 
    #$Id: Template.h,v 1.14 2001-01-04 22:04:55 bcr Exp $# 
    @author: 
    L\'eon Bottou <leonb@research.att.com> -- initial implementation \\
    Andrew Erofeev <eaf@geocities.com> -- implemented EXTERNAL_TEMPLATES */
//@{
//T// The magic doc++ comment above opens a doc++ context.



//T// Now comes the 'interface part' of the file.
//T// The c++ classes and public functions are defined there.
//T// Doc++ comments must be inserted for all functions
//T// intended to be used by other people. 
//T//
//T// Quite often c++ sucks and it is necessary to have public or external symbols
//T// that actually are only there for implementation purposes. 
//T// It is good to give a comment but this should not be a doc++ comment
//T// (see class GPool in GContainer.h).  All other 'public' and 'protected' 
//T// members should have a doc++ comment. There is no need to comment 'private'
//T// members, although a regular comment can be useful (not a doc++ comment). 



/** One-line class description.
    Long description. There is no need to repeat the class name in the
    one-line description. The long description should describe the abstraction
    and point the user to the main member functions.  An example could be
    inserted when this is informative and not redundant with the file
    description.  Templates should document which member functions are
    required for their type argument. The availability of non availabilty of a
    copy constructor/copy operator can be specified when appropriate. 
    See the doc++ documentation for available LaTeX constructs. 
*/

class ExampleClass
{
public:
  /** Virtual Destructor. */
  ~ExampleClass();
  /** Null Constructor. */
  ExampleClass();
  /** Copy Constructor. */
  ExampleClass(ExampleClass &ref);
  /** Copy operator. */
  ExampleClass& operator=(ExampleClass &ref);
  /** Example of member function. The first sentence of the member
      function description must be a short single line description.
      The rest can be more verbose. Excerpts of C or C++ text should 
      be surrounded by dieze characters (as in #win#).  The doc++ #@param#
      construct should be used when there is a need for additional details
      about the arguments. In that case all the arguments must be documented
      with a #@param# directive.
      @param win drawing window.
      This window must be created with #CreateWindow# and must be visible when
      function #draw# is called.
   */
  void draw(Window win);
protected:
  /** Minimal x-coordinate. */
  int xmin;
  /** Maximal x-coordinate. */
  int xmax;
private:
  int whatever;
  float encode;
};


/** One-line function description.
    Long description. Public external functions should be documented
    as classes. Note that a family of public external function can be 
    introduced by a generic entry (see below) */

ExampleClass combine_example_classes(const ExampleClass&, 
                                     const ExampleClass &b);


/** @name Generic entry.
    Long description. there is sometimes a need to add documentation
    entries for grouping things which are not connected by the C++ 
    syntax (a family of functions, a family of defines, etc...).
    The description starts with a very short name (introduced with #@name#)
    followed by a long description.  Because of doc++ limitations,
    the one-line description must appear after the long description
    in a #@memo# entry.
    @memo One-line description
*/

//T// The following comments should be used when
//T// the preceding generic entry contains sub-entries
//@{
//T// Sub-entries (both DOC++ and C++) should be declared there.
//@}






//@}
//T// The magic doc++ comment above closes the doc++ file context.
//T// The rest of the file only contains implementation stuff.

// ------------ CLASSEXAMPLE INLINES
//T// This is where all the inline/template functions should be written
//T// This part of the file is segmented with comments.

inline void 
ClassExample::width()
{
  return xmax-xmin;
}



// ------------ THE END
#endif
//T// Terminates the multiple inclusion #ifndef
      
      
             

    
