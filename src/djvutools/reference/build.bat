@echo off
if NOT EXIST ..\..\libapi\libapi.dsw goto nojpeg
cd ..\..\libapi
msdev libapi.dsw /MAKE %1 %3 %4 %5 %6 %7 %8 %9 
exit
:nojpeg
msdev reference.dsw /MAKE %2 %3 %4 %5 %6 $7 $8 $9
exit
