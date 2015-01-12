//*************************************************************************
// File:     ompt-openmp.c
//
// Purpose:  This file contains a function that calls and a function in the 
//           OpenMP API. This forces initialization of an OpenMP runtime 
//           without entering a parallel region.
//
// Notes:    Using Intel's v.13.0.0 compiler and compiling a main function
//           with icpc put a call to __kmp_begin before **everything** in
//           the body of main. This subverted the need of regression tests
//           to perform some initialization of their own before OpenMP 
//           would get initialized. For that reason, this file must not
//           contain main and the file containing main must not be compiled
//           with an OpenMP flag.
//*************************************************************************

//*************************************************************************
// system includes
//*************************************************************************

#include <omp.h>



//*************************************************************************
// local includes
//*************************************************************************
#include <ompt-openmp.h>



//*************************************************************************
// interface functions
//*************************************************************************

int
openmp_init()
{
  return omp_get_max_threads();
}
