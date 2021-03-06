/* abi_common.h */

/*
 * Copyright (C) 2008-2020 ABINIT Group (MG)
 *
 * This file is part of the ABINIT software package. For license information,
 * please see the COPYING file in the top-level directory of the ABINIT source
 * distribution.
 *
 */

#ifndef _ABINIT_COMMON_H
#define _ABINIT_COMMON_H

/*
 * Language standards requires the existance of pre-defined macros
 * Microsoft Visual C++ does not define __STDC__,
 * Sun Workshop 4.2 supports C94 without setting __STDC_VERSION__ to the proper value
 */

#if defined (__STDC__)
# define PREDEF_STANDARD_C_1989    /** ANSI X3.159-1989 **/
# if defined (__STDC_VERSION__)
#  define PREDEF_STANDARD_C_1990   /** ISO/IEC 9899:1990 **/
#  if (__STDC_VERSION__ >= 199409L)
#   define PREDEF_STANDARD_C_1994  /** ISO/IEC 9899-1:1994 **/
#  endif
#  if (__STDC_VERSION__ >= 199901L)
#   define PREDEF_STANDARD_C_1999  /** ISO/IEC 9899:1999 **/
#  endif
# endif
#endif

#if defined(HAVE_FC_LONG_LINES) || defined(__INTEL_COMPILER) || defined(FC_NAG) || !defined(HAVE_FC_MACRO_NEWLINE)
# define NEWLINE ;
#else
# define NEWLINE \newline
#endif

/*
 * These macros are used to pass information on the file and the line to the children if the compiler supports long lines.
 * Remember, indeed, that ISO doesn't define any standard and __FILE__ could expand to the full path name.
 * This can lead to compilation errors if the compiler does not accept statements longer than 132 columns.
 */
#ifdef HAVE_FC_LONG_LINES 
#define _FILE_LINE_ARGS_    ,file=__FILE__, line=__LINE__
#define _FILE_ABIFUNC_LINE_ARGS_    ,file=__FILE__, line=__LINE__
#else
#define _FILE_LINE_ARGS_ 
#define _FILE_ABIFUNC_LINE_ARGS_ 
#endif

/** this does not work with gfort, pgi, **/
#if defined (FC_GNU) || defined (FC_PGI)
#define QUOTE(x)     'x'
#else
#define QUOTE(x)     #x
#endif


/** Token concatenation
 1) ANSI CPP
 2) Traditional CPP
**/

#if defined (FC_INTEL)
#define CONCAT(x,y) x ## y
#else
#define CONCAT(x,y) x/**/y
#endif

#define BYTE_SIZE(array)  PRODUCT(SHAPE(array)) * DBLE(KIND(array))

/*
 * ABI_  abinit macros.
 * DBG_  macros for debugging. Defined only if abinit is compiled in DEBUG_MODE.
 * MSG_  macros for logging.
*/

#define ABI_CHECK(expr, str) if (.not.(expr)) call assert(.FALSE., str _FILE_LINE_ARGS_)
#define ABI_CHECK_MPI(ierr, msg) call check_mpi_ierr(ierr, msg _FILE_LINE_ARGS_)

/* This macro is used in low-level MPI wrappers to return mpierr to the caller
 * so that one can locate the problematic call. Use it wisely since it may cause memory leaks.
 * #define ABI_HANDLE_MPIERR(mpierr) ABI_CHECK_MPI(mpierr,"ABI_HANDLE: Fatal error")
 */
#define ABI_HANDLE_MPIERR(mpierr) if (mpierr/=MPI_SUCCESS) RETURN

/* Macros for memory checking and profiling
 * TODO
 * Add option to check automatically the exit status of allocate so that we can abort gracefully if oom.
 * and abipy can detect the problem.
 *
 *   ABI_ALLOCATE   --> Allocate memory for intrinsic datatypes (real, integer, complex).
 *   ABI_CALLOC     --> Clear alloc: same as ABI_ALLOCATE but initializes memory with zeros
 *   ABI_DEALLOCATE --> Free memory allocated by ABI_ALLOCATE
 *
 * To allocate/deallocate arrays of user-defined type use:
 *
 *   ABI_DT_ALLOCATE
 *   ABI_DT_DEALLOCATE
 *
 * To allocate scalars, use:
 *
 *  ABI_MALLOC_SCALAR
 *  ABI_FREE_SCALAR
 *
 #define HAVE_MEM_PROFILING
*/

#ifdef HAVE_MEM_PROFILING

/* 
 These macros are used to get the memory address of the objet and the memory allocated.

   - loc returns the address and is a language extension supported by gfortran and ifort.
   - storage_size was introduced in F2003 and returns the size in bits.

 Both loc and storage_size are polymorphic so one can use it with intrinsic types as well 
 as user-defined datatypes. scalar types require a special treatment (MALLOC_SCALAR, FREE_SCALAR)
 because shape == 0 thus it's not possible to discern with _MEM between zero-sized arrays and scalar
*/
#  define _LOC(x)  int(loc(x), kind=8) 
#  define _MEM(arr) product(int(shape(arr), kind=8)) * storage_size(arr, kind=8)

/* and now the debugging macros */
#  define ABI_ALLOCATE(ARR, SIZE) \
   allocate(ARR SIZE) NEWLINE \
   call abimem_record(0, QUOTE(ARR), _LOC(ARR), "A", _MEM(ARR),  __FILE__, __LINE__)

#  define ABI_MALLOC_SCALAR(scalar) \
   allocate(scalar) NEWLINE \
   call abimem_record(0, QUOTE(scalar), _LOC(scalar), "A", storage_size(scalar, kind=8),  __FILE__, __LINE__)

#  define ABI_FREE_SCALAR(scalar) \
   call abimem_record(0, QUOTE(scalar), _LOC(scalar), "D", -storage_size(scalar, kind=8),  __FILE__, __LINE__) NEWLINE \
   deallocate(scalar)

#  define ABI_DEALLOCATE(ARR) \
   call abimem_record(0, QUOTE(ARR), _LOC(ARR), "D", - _MEM(ARR), __FILE__,  __LINE__) NEWLINE \
   deallocate(ARR) 

#  define ABI_STAT_ALLOCATE(ARR,SIZE,ierr) \
   allocate(ARR SIZE, stat=ierr) NEWLINE \
   call abimem_record(0, QUOTE(ARR), _LOC(ARR), "A", _MEM(ARR),  __FILE__, __LINE__)

#  define ABI_DATATYPE_ALLOCATE(ARR,SIZE) \
   allocate(ARR SIZE) NEWLINE \
   call abimem_record(0, QUOTE(ARR), _LOC(ARR), "A", _MEM(ARR),  __FILE__, __LINE__)

#  define ABI_DATATYPE_DEALLOCATE(ARR)  \
   call abimem_record(0, QUOTE(ARR), _LOC(ARR), "D", - _MEM(ARR), __FILE__, __LINE__) NEWLINE \
   deallocate(ARR) 

#  define ABI_MALLOC_OR_DIE(ARR,SIZE,ierr) \
   allocate(ARR SIZE, stat=ierr) NEWLINE \
   call abimem_record(0, QUOTE(ARR), _LOC(ARR), "A", _MEM(ARR),  __FILE__, __LINE__) NEWLINE \
   ABI_CHECK(ierr == 0, "out-of-memory")

#  define ABI_MOVE_ALLOC(from, to) \
   call abimem_record(0, QUOTE(from), _LOC(from), "D", -_MEM(from),  __FILE__, __LINE__) NEWLINE \
   if (allocated(to)) call abimem_record(0, QUOTE(to), _LOC(to), "D", -_MEM(to),  __FILE__, __LINE__) NEWLINE \
   call move_alloc(from, to) NEWLINE \
   call abimem_record(0, QUOTE(to), _LOC(to), "A", _MEM(to),  __FILE__, __LINE__)


/* Allocate a polymophic scalar 
 * allocate(datatype:: scalar) */
#  define ABI_DATATYPE_ALLOCATE_SCALAR(type, scalar)                    \
  allocate(type::scalar) NEWLINE                                        \
    call abimem_record(0, QUOTE(scalar), _LOC(scalar), "A", storage_size(scalar, kind=8),  __FILE__, __LINE__)

#  define ABI_DATATYPE_DEALLOCATE_SCALAR(scalar)                        \
  call abimem_record(0, QUOTE(scalar), _LOC(scalar), "D", -storage_size(scalar, kind=8), __FILE__, __LINE__) NEWLINE \
    deallocate(scalar) 



#else
/* macros used in production */
#  define ABI_ALLOCATE(ARR,SIZE) allocate(ARR SIZE)
#  define ABI_DEALLOCATE(ARR)  deallocate(ARR)
#  define ABI_STAT_ALLOCATE(ARR,SIZE,ierr) allocate(ARR SIZE, stat=ierr)
#  define ABI_DATATYPE_ALLOCATE(ARR,SIZE)  allocate(ARR SIZE)
#  define ABI_DATATYPE_DEALLOCATE(ARR)   deallocate(ARR)
#  define ABI_MALLOC_OR_DIE(ARR,SIZE,ierr) \
        allocate(ARR SIZE, stat=ierr) NEWLINE \
        ABI_CHECK(ierr == 0, "out-of-memory")
#  define ABI_MALLOC_SCALAR(scalar) allocate(scalar)
#  define ABI_FREE_SCALAR(scalar) deallocate(scalar)
#  define ABI_MOVE_ALLOC(from, to) call move_alloc(from, to)


#  define ABI_DATATYPE_ALLOCATE_SCALAR(type,scalar)  allocate(type::scalar)
#  define ABI_DATATYPE_DEALLOCATE_SCALAR(scalar)   deallocate(scalar)


#endif


/* Macros to allocate zero-initialized arrays.
 * defined in terms of previous macros */
#define ABI_CALLOC(ARR, SIZE) ABI_ALLOCATE(ARR, SIZE) NEWLINE ARR = zero
#define ABI_ICALLOC(ARR, SIZE) ABI_ALLOCATE(ARR, SIZE) NEWLINE ARR = 0
#define ABI_CALLOC_OR_DIE(ARR,SIZE,ierr) ABI_MALLOC_OR_DIE(ARR, SIZE, ierr) NEWLINE ARR = zero

/* Shorthand versions */
#define ABI_MALLOC(ARR,SIZE) ABI_ALLOCATE(ARR,SIZE)

#define ABI_FREE(ARR) ABI_DEALLOCATE(ARR)

#define ABI_STAT_MALLOC(ARR,SIZE,ierr) ABI_STAT_ALLOCATE(ARR,SIZE,ierr)

#define ABI_DT_MALLOC(ARR,SIZE)  ABI_DATATYPE_ALLOCATE(ARR,SIZE)
#define ABI_DT_FREE(ARR)         ABI_DATATYPE_DEALLOCATE(ARR)

/* Macro used to deallocate memory allocated by Fortran libraries that do not use m_profiling_abi.F90
 * or allocate arrays before calling MOVE_ALLOC.
   In this case, indeed, we should not count the deallocation 
#define ABI_MALLOC_NOCOUNT(arr, size) allocate(arr size)
*/
#define ABI_FREE_NOCOUNT(arr) deallocate(arr)

/*
 * Macros to allocate/deallocate depending on the allocation (association) status of the variable (pointer).
 * SFREE stands for SAFE FREE
 * Caveat: pointers must use ABI_PTR_FREE_IF
 *
*/
#define ABI_MALLOC_IFNOT(ARR, SIZE) if (.not. allocated(ARR)) then NEWLINE ABI_MALLOC(ARR, SIZE) NEWLINE endif
#define ABI_SFREE(ARR) if (allocated(ARR)) then NEWLINE ABI_FREE(ARR) NEWLINE endif
#define ABI_SFREE_PTR(PTR) if (associated(PTR)) then NEWLINE ABI_FREE(PTR) NEWLINE endif
#define ABI_REMALLOC(ARR, SIZE) ABI_SFREE(ARR) NEWLINE ABI_MALLOC(ARR, SIZE)
#define ABI_RECALLOC(ARR, SIZE) ABI_SFREE(ARR) NEWLINE ABI_CALLOC(ARR, SIZE)

/* Macros used in debug mode */
#ifdef DEBUG_MODE

#  define ASSERT(expr) if (.not.expr) call assert((expr), "Assertion failed" _FILE_LINE_ARGS_)
#  define ASSERT_IF(condition, expr) if (condition) call assert((expr), "Assertion failed" _FILE_LINE_ARGS_)

#  define DBG_CHECK(expr,str) if (.not.expr) call assert((expr), str  _FILE_LINE_ARGS_)
#  define DBG_ENTER(mode) call sentinel(1,mode _FILE_ABIFUNC_LINE_ARGS_)
#  define DBG_EXIT(mode)  call sentinel(2,mode _FILE_ABIFUNC_LINE_ARGS_)
/* Stop if two arrays have different shape */
#  define DBG_EQSHAPE(arr1, arr2) if (any(shape(arr1)/=shape(arr2))) MSG_ERROR("Different shape")

#else
/* nops */
#  define ASSERT(expr)
#  define ASSERT_IF(condition, expr)

#  define DBG_CHECK(expr,str)
#  define DBG_ENTER(mode)
#  define DBG_EXIT(mode)
#  define DBG_EQSHAPE(arr1, arr2)
#endif

/* Macro for basic messages */
#define MSG_COMMENT(msg) call msg_hndl(msg,"COMMENT", "PERS" _FILE_LINE_ARGS_)
#define MSG_WARNING(msg) call msg_hndl(msg,"WARNING", "PERS" _FILE_LINE_ARGS_)
#define MSG_COMMENT_UNIT(msg, unt) call msg_hndl(msg,"COMMENT", "PERS" _FILE_LINE_ARGS_, unit=unt)
#define MSG_WARNING_UNIT(msg, unt) call msg_hndl(msg,"WARNING", "PERS" _FILE_LINE_ARGS_, unit=unt)
#define MSG_ERROR(msg) call msg_hndl(msg,"ERROR", "PERS" _FILE_LINE_ARGS_)
#define MSG_ERROR_CLASS(msg, cls) call msg_hndl(msg, cls , "PERS" _FILE_LINE_ARGS_)
#define MSG_BUG(msg) call msg_hndl(msg,"BUG", "PERS" _FILE_LINE_ARGS_)
#define MSG_STOP(msg) call msg_hndl(msg,"STOP", "PERS")

#define MSG_ERROR_NODUMP(msg) call msg_hndl(msg, "ERROR", "PERS", NODUMP=.TRUE. _FILE_LINE_ARGS_)
#define MSG_ERROR_NOSTOP(msg, ierr) \
   ierr=ierr+1; call msg_hndl(msg, "ERROR", "PERS", NOSTOP=.TRUE. _FILE_LINE_ARGS_)
#define MSG_ERROR_NOSTOP_IF(condition, msg, ierr) \
   if (condition)  then NEWLINE MSG_ERROR_NOSTOP(msg, ierr) NEWLINE endif

#define ETSF_WARN(lstat,Error_data) call abietsf_warn(lstat,Error_data,"PERS" _FILE_LINE_ARGS_)

#define ETSF_CHECK_ERROR(lstat,Error_data) if (.not. lstat) call abietsf_msg_hndl(lstat,Error_data,"PERS" _FILE_LINE_ARGS_)
#define NCF_CHECK(ncerr) if (ncerr/=nf90_noerr) call netcdf_check(ncerr,"No msg from caller" _FILE_LINE_ARGS_)
#define NCF_CHECK_MSG(ncerr,msg) if (ncerr/=nf90_noerr) call netcdf_check(ncerr,msg _FILE_LINE_ARGS_)

#define NOT_IMPLEMENTED_ERROR() MSG_ERROR("Not Implemented Error")

/* Macro to deprecate particular features. */
#define MSG_DEPRECATE(msg) call wrtout(ab_out, msg, "COLL")
/*
#define MSG_DEPRECATE(msg) MSG_ERROR(msg)
*/

/* Macro for clean exit */
/*
#define ABI_EXIT(exit_status) call abi_abort("COLL",exit_status=exit_status,print_config=.False.)
*/

/* Macros used for stopping the code if external libraries have not been enabled */
#define NETCDF_NOTENABLED_ERROR() MSG_ERROR("netcdf is not activated. Use configure --enable-netcdf")

#ifdef HAVE_FC_LONG_LINES
#define BIGDFT_NOTENABLED_ERROR() call bigdft_lib_error(__FILE__, __LINE__)
#else
#define BIGDFT_NOTENABLED_ERROR() call bigdft_lib_error()
#endif

/* Write a warning if condition  */
#define MSG_WARNING_IF(expr, msg) if ((expr)) MSG_WARNING(msg)

/* Dummy use of unused arguments to silence compiler warnings */
#define ABI_UNUSED(var) if (.FALSE.) call unused_var(var)
/* 
 * The method above work for basic types (integer, real, etc...). 
 * For types, arrays, we can use associate (F03 feature)
 * Does not work for character(*) with gfortran <=5.x (>7.x is fine. No 6.x data)
 * character with fixed length is fine.
 * */ 
#define ABI_UNUSED_A(var) associate( var => var ) NEWLINE end associate 

/* Macro to set the default the value of a local variable when optional arguments are used.
Use if statement instead of Fortran merge. See https://software.intel.com/en-us/forums/intel-fortran-compiler/topic/640598
*/

#define ABI_DEFAULT(value, optional, default) value = default; if (present(optional)) value = optional

#ifdef HAVE_PAPI
#  define XPAPI_CHECK(check,msg) if (check/=PAPI_OK) call xpapi_handle_error(check, msg _FILE_LINE_ARGS_)
#else
#  define XPAPI_CHECK(check,msg)
#endif

/* Portable support for /dev/null */
#if defined HAVE_OS_WINDOWS
#define NULL_FILE "NUL"
#else
#define NULL_FILE "/dev/null"
#endif


/* F2003 support  */
#define ABI_CHECK_CNULL(cptr,msg) if (.not.C_ASSOCIATED(cptr)) MSG_ERROR(msg)

#ifdef HAVE_FC_ASYNC
#define ABI_ASYNC ,asynchronous
#else
#define ABI_ASYNC 
#endif

#ifdef HAVE_FC_PRIVATE
#define ABI_PRIVATE ,private
#else
#define ABI_PRIVATE
#endif

#ifdef HAVE_FC_PROTECTED
#define ABI_PROTECTED ,protected
#else
#define ABI_PROTECTED
#endif


/* F2008 support  */
#ifdef HAVE_FC_CONTIGUOUS
#define ABI_CONTIGUOUS contiguous,
#else
#define ABI_CONTIGUOUS
#endif

/* 
 * Temporary trick used to pass contiguous array descriptors to F90 routines 
 * Mainly used to avoid copy-in copy-out in m_abi_linalg
#define DEV_CONTARRD contiguous,
*/
#define DEV_CONTARRD

/* OpenMP support */
#ifndef HAVE_OMP_COLLAPSE
#define COLLAPSE(x)
#endif

/* DFTI macros (should be declared in m_dfti but build-sys tests complain */
#define DFTI_CHECK(status) if (status/=0) call dfti_check_status(status _FILE_LINE_ARGS_)


/* Macros used in the GW code */
#ifdef HAVE_GW_DPC
#  define GWPC_CONJG(cvar)  DCONJG(cvar)
#  define GWPC_CMPLX(re,im) DCMPLX(re,im)
#else
#  define GWPC_CONJG(cvar)  CONJG(cvar)
#  define GWPC_CMPLX(re,im) CMPLX(re,im)
#endif

/* Macros used for particular slaves of the Abinit tests farm */

/* 
 * IBM6 has a very problematic IO. Flushing the IO buffer helps avoid segmentation fault 
 * To disable these tricks, comment the three lines below.
 * */
#ifdef FC_IBM
#define HAVE_IBM6
#endif

#ifdef HAVE_IBM6
#define _IBM6(message) call wrtout(std_out,message,"COLL",do_flush=.True.)
#else
#define _IBM6(message)
#endif

#endif
/* _ABINIT_COMMON_H */
