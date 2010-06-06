/*---------------------------------------------------------------------------*/
/* File:        config.h                                                     */
/* Created:     Sat, 23 Jan 2010 11:11:00 GMT                                */
/*              by Oleg N. Scherbakov, mailto:oleg@7zsfx.info                */
/* Last update: Sat, 05 Jun 2010 23:58:49 GMT                                */
/*              by Oleg N. Scherbakov, mailto:oleg@7zsfx.info                */
/* Revision:    134                                                          */
/*---------------------------------------------------------------------------*/
/* Revision:    55                                                           */
/* Updated:     Thu, 18 Mar 2010 14:04:32 GMT                                */
/*              by Oleg N. Scherbakov, mailto:oleg@7zsfx.info                */
/* Description: New file stamp.                                              */
/*---------------------------------------------------------------------------*/
#define _SFX_USE_CUSTOM_MSVCRT			/* msvcrt.dll */
										/* require msvcrt.lib from old VC6 for x86 */
										/* or new for x64 from the latest DDK */
										/* see realization in 'config.cpp' */
#define _SFX_USE_WIN7_PROGRESSBAR		/* Windows 7 progress bar on taskbar button */
										/* require latest SDK */
#define _SFX_USE_RTF_CONTROL			/* Enable RTF in texts */
#define _SFX_USE_IMAGES					/* Enable images in custom dialogs */
#define _SFX_OPTIMIZE_SIZE				/* section optimizations, was used in VC6 */

#define _SFX_USE_TEST					/* internal use */
#define _SFX_USE_LANG					/* use -sfxlang:XXXX command line switch */
#define _SFX_USE_PREFIX_PLATFORM		/* use x86 & x64 prefixes for execution parameters */
#define _SFX_USE_CUSTOM_EXCEPTIONS		/* use custom realization of exceptions handlers */
										/* see realization in 'config.cpp' */

// check RAM before decompression: feature, may be later
#define _SFX_USE_CHECK_RAM				/* check available physical memory to prevent swap */
#define _SFX_USE_CHECK_FREE_SPACE		/* check available free space on destination drive */

#ifdef _7ZSFX_LITE
	#undef _SFX_USE_RTF_CONTROL
	#undef _SFX_USE_IMAGES
	#undef _SFX_USE_LANG
	#undef _SFX_USE_PREFIX_PLATFORM
#endif // _7ZSFX_LITE

#ifndef _SFX_USE_WARNINGS
	#if defined(_SFX_USE_CHECK_FREE_SPACE) || defined(_SFX_USE_CHECK_RAM)
		#define _SFX_USE_WARNINGS
	#endif // defined(_SFX_USE_CHECK_FREE_SPACE) || defined(_SFX_USE_CHECK_RAM)
#endif // 

//#define _SFX_FEATURES_TEST
