// debug.h
//      Debugging facilities for Nachos.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef DEBUG_H
#define DEBUG_H

#include "copyright.h"

// Debugging macros
extern bool debugFlags[];

#define DEBUG(flag, format, args...) \
    if (debugFlags[flag]) { printf(format, ##args); fflush(stdout); }

void DebugInit(char *flags);

#endif // DEBUG_H
