
                PDF to DjVu Conversion Utility

-------------------------------------------------------------------
                        README File Contents
-------------------------------------------------------------------

    I Install PDF to DjVu Conversion Utility
   II Contents of the Distribution
  III Release Notes
   IV Licenses and Trademarks.

-------------------------------------------------------------------
    I Install PDF to DjVu Conversion Utility 
-------------------------------------------------------------------
Perform the following procedure to install the PDF/PS to DjVu 
conversion utilities.  
 
Step 1:
        Download the pdftodjvu.tar.gz
Step 2:
        Untar and unzip the pdftodjvu.tar.gz file:
                gunzip -c < pdftodjvu.tar.gz|tar xvvf -
              
Step 3: Verify that if you have Ghostscript version 5.50 installed,
        with the command:
                gs --version
        If you do not have Ghostscript version 5.50 installed, download
        the following:
                gnu-gs-5.50.tar.gz
                gnu-gs-5.50jpeg.tar.gz
                gnu-gs-5.50libpng.tar.gz
                gnu-gs-5.50zlib.tar.gz
                gnu-gs-fonts-other-5.50.tar.gz
                gnu-gs-fonts-std-5.50.tar.gz
        
        GNU Ghostscript Source can also be obtained from the following
        locations:
                ftp://www.cs.wisc.edu/ghost/gnu
                http://www.cs.wisc.edu/~ghost/doc/mirror.htm

Step 4:
        Install either DjVu Enterprise for Linux (purchased from LizardTech, 
        Inc.) or build and install the Open Source DjVu Reference Library
        version 3.0, obtained from:

  http://www.lizardtech.com/products/djvu/referencelibrary/DjVuRefLib_3.0.html

Step 5:
        Execute the build-gnu-gs.sh script:
                ./build-gnu-gs.sh
        This script will extract the source and build Ghostscript.
        This script will also create an install.sh script.

Step 6:
        Run the ./install.sh to install Ghostscript:
                ./install.sh <prefix>
        where <prefix> is a somewhere in your search path such as:
                ./install.sh /usr
        or
                ./install.sh /usr/local

Step 7:
        Verify all the commands installed in step 4 and step 6 have been
        installed in your search path.

Step 7:
        Execute the shell scripts (pdftodjvu.sh or pstodjvu.sh) to convert
        PDF/PS files to DjVu by using the following syntax:

DjVu Open Source
    Bundled output
        ./pdftodjvu.sh --free [cpaldjvu options] <inputfile> <outputfile>
        
    Indirect output
        ./pdftodjvu.sh --free [cpaldjvu options] <inputfile> <outputdir>

DjVu Enterprise
    Bundled output
        ./pdftodjvu.sh --profile=[profilename] [documenttodjvu options] \
            [--best] <inputfile> <outputfile>

    Indirect output
        ./pdftodjvu.sh --profile=[profilename] [documenttodjvu options] \
	    [--best] <inputfile> <outputdir>

Note: "--profile=clean300" is recommended for most documents with photos or
other complicated graphics.  "--free" is recommended for documents that are
mostly simple text.

-------------------------------------------------------------------
   II Contents of the Distribution
-------------------------------------------------------------------
    1. COPYING.txt      - The GNU Copyright Notice for these scripts.
    2. README.txt       - This file.
    3. build-gnu-gs.sh  - shell script to build Ghostscript
    3. pdftodjvu.sh     - shell script to convert PS and PDF files to DjVu                      
    4. pstodjvu.sh      - same script with a different name.
    
-------------------------------------------------------------------
  III Release Notes
-------------------------------------------------------------------

    1. Ghostscript version 5.50 supports PDF versions 1.0, 1.1, and 1.2
       file formats

    2. "--profile=clean300" is recommended for most documents with photos
       or other complicated graphics.

    3. "--free" is recommended for documents that are mostly simple text.

     
-------------------------------------------------------------------
    IV Licenses and Trademarks.
-------------------------------------------------------------------

Copyright (c) 2001 LizardTech, Inc.
All rights reserved. DjVu is a registered trademark of LizardTech.
Unauthorized use is prohibited by law.

This pdftodjvu.tar.gz software is made available by LizardTech 
subject to the GNU General Public License, in the COPYING.txt file.

