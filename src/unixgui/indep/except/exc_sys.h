//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: exc_sys.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifndef HDR_EXC_SYS
#define HDR_EXC_SYS

#ifdef __GNUC__
#pragma interface
#endif

#include "exc_base.h"

#define SYSTEM_ERROR(func, msg) SystemError(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(SystemError, "SystemError", Exception);

#define EXC_PERM(func, msg) ExcPERM(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcPERM, "ExcPERM", SystemError);

#define EXC_NOENT(func, msg) ExcNOENT(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcNOENT, "ExcNOENT", SystemError);

#define EXC_SRCH(func, msg) ExcSRCH(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcSRCH, "ExcSRCH", SystemError);

#define EXC_INTR(func, msg) ExcINTR(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcINTR, "ExcINTR", SystemError);

#define EXC_IO(func, msg) ExcIO(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcIO, "ExcIO", SystemError);

#define EXC_NXIO(func, msg) ExcNXIO(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcNXIO, "ExcNXIO", SystemError);

#define EXC_2BIG(func, msg) Exc2BIG(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(Exc2BIG, "Exc2BIG", SystemError);

#define EXC_NOEXEC(func, msg) ExcNOEXEC(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcNOEXEC, "ExcNOEXEC", SystemError);

#define EXC_BADF(func, msg) ExcBADF(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcBADF, "ExcBADF", SystemError);

#define EXC_CHILD(func, msg) ExcCHILD(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcCHILD, "ExcCHILD", SystemError);

#define EXC_AGAIN(func, msg) ExcAGAIN(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcAGAIN, "ExcAGAIN", SystemError);

#define EXC_NOMEM(func, msg) ExcNOMEM(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcNOMEM, "ExcNOMEM", SystemError);

#define EXC_ACCES(func, msg) ExcACCES(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcACCES, "ExcACCES", SystemError);

#define EXC_FAULT(func, msg) ExcFAULT(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcFAULT, "ExcFAULT", SystemError);

#define EXC_NOTBLK(func, msg) ExcNOTBLK(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcNOTBLK, "ExcNOTBLK", SystemError);

#define EXC_BUSY(func, msg) ExcBUSY(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcBUSY, "ExcBUSY", SystemError);

#define EXC_EXIST(func, msg) ExcEXIST(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcEXIST, "ExcEXIST", SystemError);

#define EXC_XDEV(func, msg) ExcXDEV(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcXDEV, "ExcXDEV", SystemError);

#define EXC_NODEV(func, msg) ExcNODEV(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcNODEV, "ExcNODEV", SystemError);

#define EXC_NOTDIR(func, msg) ExcNOTDIR(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcNOTDIR, "ExcNOTDIR", SystemError);

#define EXC_ISDIR(func, msg) ExcISDIR(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcISDIR, "ExcISDIR", SystemError);

#define EXC_INVAL(func, msg) ExcINVAL(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcINVAL, "ExcINVAL", SystemError);

#define EXC_NFILE(func, msg) ExcNFILE(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcNFILE, "ExcNFILE", SystemError);

#define EXC_MFILE(func, msg) ExcMFILE(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcMFILE, "ExcMFILE", SystemError);

#define EXC_NOTTY(func, msg) ExcNOTTY(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcNOTTY, "ExcNOTTY", SystemError);

#define EXC_TXTBSY(func, msg) ExcTXTBSY(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcTXTBSY, "ExcTXTBSY", SystemError);

#define EXC_FBIG(func, msg) ExcFBIG(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcFBIG, "ExcFBIG", SystemError);

#define EXC_NOSPC(func, msg) ExcNOSPC(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcNOSPC, "ExcNOSPC", SystemError);

#define EXC_SPIPE(func, msg) ExcSPIPE(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcSPIPE, "ExcSPIPE", SystemError);

#define EXC_ROFS(func, msg) ExcROFS(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcROFS, "ExcROFS", SystemError);

#define EXC_MLINK(func, msg) ExcMLINK(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcMLINK, "ExcMLINK", SystemError);

#define EXC_PIPE(func, msg) ExcPIPE(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcPIPE, "ExcPIPE", SystemError);

#define EXC_DOM(func, msg) ExcDOM(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcDOM, "ExcDOM", SystemError);

#define EXC_RANGE(func, msg) ExcRANGE(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcRANGE, "ExcRANGE", SystemError);

#define EXC_NOMSG(func, msg) ExcNOMSG(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcNOMSG, "ExcNOMSG", SystemError);

#define EXC_DEADLK(func, msg) ExcDEADLK(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcDEADLK, "ExcDEADLK", SystemError);

#define EXC_NOLCK(func, msg) ExcNOLCK(func, msg, 0, __FILE__, __LINE__)
DECLARE_EXCEPTION(ExcNOLCK, "ExcNOLCK", SystemError);

#endif
