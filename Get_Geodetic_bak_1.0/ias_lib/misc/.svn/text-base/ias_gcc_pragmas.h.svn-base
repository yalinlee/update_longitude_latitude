
#ifndef IAS_GCC_PRAGMAS_H
#define IAS_GCC_PRAGMAS_H

/*****************************************************************************
  NAME: ias_gcc_pragmas.h

  PURPOSE: To define macros which specify GCC pragmas which manipulate
           compile time options.

  IAS_GCC_VERSION: This macro creates a version value that can be tested
                   against.

  IAS_DISABLE_OPTIMIZATION: This define will push the current options on the
                            stack and then turn off optimization.

  IAS_ENABLE_OPTIMIZATION: This define will pop the saved options off the
                           stack restoring the optimization setting.

  NOTES:
    The optimize pragma is only available in GCC 4.4 or later.  So the
    optimization defines will be empty for any prior versions of GCC or any
    other compiler.  The version macro will be set to 0 for any non-GCC
    compiler.

    The use of "_Pragma" is a c99 standard and is required for specifying
    pragma's using a #define macro.

*****************************************************************************/

#ifdef __GNUC__

    #define IAS_GCC_VERSION() (__GNUC__ * 10000 \
                               + __GNUC_MINOR__ * 100 \
                               + __GNUC_PATCHLEVEL__)

    #if (IAS_GCC_VERSION() >= 40400)

        #define IAS_DISABLE_OPTIMIZATION \
            _Pragma( "GCC push_options" ) \
            _Pragma( "GCC optimize (\"O0\")" )

        #define IAS_RESTORE_OPTIMIZATION \
            _Pragma( "GCC pop_options" )

    #else

        #define IAS_DISABLE_OPTIMIZATION
        #define IAS_RESTORE_OPTIMIZATION

    #endif

    #undef IAS_GCC_VERSION

#else

    #define IAS_DISABLE_OPTIMIZATION
    #define IAS_RESTORE_OPTIMIZATION

#endif

#endif /* IAS_GCC_PRAGMAS_H */

