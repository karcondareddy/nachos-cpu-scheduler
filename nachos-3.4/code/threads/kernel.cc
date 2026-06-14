// kernel.cc
//      Routines to manage the overall operation of the Nachos kernel.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "kernel.h"
#include "system.h"

//----------------------------------------------------------------------
// ThreadedKernel::ThreadedKernel
//      Constructor for the thread kernel.  Most initialization is done
//      in Initialize().
//----------------------------------------------------------------------

ThreadedKernel::ThreadedKernel(int argc, char **argv)
{
    // The constructor doesn't do much - most initialization is in Initialize()
}

//----------------------------------------------------------------------
// ThreadedKernel::~ThreadedKernel
//      Destructor - nothing to do here.
//----------------------------------------------------------------------

ThreadedKernel::~ThreadedKernel()
{
}

//----------------------------------------------------------------------
// ThreadedKernel::Initialize
//      Initialize the thread system.  This is called after the command
//      line arguments have been parsed.
//----------------------------------------------------------------------

void
ThreadedKernel::Initialize()
{
    // This is already handled in system.cc's Initialize() function
}

//----------------------------------------------------------------------
// ThreadedKernel::SelfTest
//      Test the thread system.
//----------------------------------------------------------------------

void
ThreadedKernel::SelfTest()
{
    // This is handled elsewhere
}

//----------------------------------------------------------------------
// ThreadedKernel::Run
//      Run the thread system.
//----------------------------------------------------------------------

void
ThreadedKernel::Run()
{
    // This is handled elsewhere
}
