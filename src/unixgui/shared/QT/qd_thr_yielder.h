//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_thr_yielder.h,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifndef HDR_QD_THR_YIELDER
#define HDR_QD_THR_YIELDER

#ifdef __GNUC__
#pragma interface
#endif

class QDThrYielder
{
private:
   static int		tasks;
   static class Helper	* helper;
public:
   static void	schedulingCB(int);
   
   static bool	isInitialized(void);
   static void	initialize(void);
   static int	getTasksNum(void);
};

#endif
