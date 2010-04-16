/*---------------------------------------------------------------------------*/
/* File:        langstrs.h                                                   */
/* Created:     Fri, 29 Jul 2005 23:10:00 GMT                                */
/*              by Oleg N. Scherbakov, mailto:oleg@7zsfx.info                */
/* Last update: Mon, 22 Mar 2010 11:19:08 GMT                                */
/*              by Oleg N. Scherbakov, mailto:oleg@7zsfx.info                */
/* Revision:    1697                                                         */
/*---------------------------------------------------------------------------*/
/* Revision:    1697                                                         */
/* Updated:     Mon, 22 Mar 2010 11:19:08 GMT                                */
/*              by Oleg N. Scherbakov, mailto:oleg@7zsfx.info                */
/* Description: New file stamp.                                              */
/*---------------------------------------------------------------------------*/
#ifndef _LANGSTRS_H_INCLUDED_
#define _LANGSTRS_H_INCLUDED_

#define STR_SFXVERSION				1
#define STR_TITLE					2
#define STR_ERROR_TITLE				3
#define STR_ERROR_SUFFIX			4
#define STR_EXTRACT_TITLE			5
#define ERR_MODULEPATHNAME			6
#define ERR_OPEN_ARCHIVE			7
#define ERR_NON7Z_ARCHIVE			8
#define ERR_READ_CONFIG				9
#define ERR_WRITE_CONFIG			10
#define ERR_CONFIG_DATA				11
#define ERR_CREATE_FOLDER			12
#define ERR_DELETE_FILE				13
#define ERR_AUTOINSTALL_NOTFOUND	14
#define ERR_NO_SETUP_EXE			15
#define ERR_EXECUTE					16
#define ERR_7Z_UNSUPPORTED_METHOD	17
#define ERR_7Z_CRC_ERROR			18
#define ERR_7Z_DATA_ERROR			19
#define ERR_7Z_INTERNAL_ERROR		20
#define STR_EXTRACT_PATH_TITLE		21
#define STR_EXTRACT_PATH_TEXT		22
#define STR_CANCEL_PROMPT			23
#define STR_DEFAULT_HELP_TEXT		24

#define STR_BUTTON_OK				25
#define STR_BUTTON_CANCEL			26
#define STR_BUTTON_YES				27
#define STR_BUTTON_NO				28
#define STR_SECONDS					29

// Added April 9, 2008
#define ERR_CREATE_FILE				30
#define ERR_OVERWRITE				31

// added September 8, 2008
#define ERR_CONFIG_CMDLINE			32

// added December 18, 2008
#define ERR_7Z_EXTRACT_ERROR1		33
#define ERR_7Z_EXTRACT_ERROR2		34

// added January 29, 2010
#define STR_BUTTON_BACK				35
#define STR_BUTTON_NEXT				36
#define STR_BUTTON_FINISH			37
#define STR_BUTTON_CANCEL2			38

#ifdef _SFX_USE_CUSTOM_EXCEPTIONS
	// added February 5, 2010
	#define ERR_SFX_EXCEPTION		39
#endif // _SFX_USE_CUSTOM_EXCEPTIONS

LPCWSTR GetLanguageString( UINT id );
void FreeLanguageStrings();

typedef struct tagLANGSTRING {
	UINT	id;
	LPCSTR	strPrimary;
	LPCSTR	strSecondary;
	LPWSTR	lpszUnicode;
} LANGSTRING, * PLANGSTRING, * LPLANGSTRING;

extern const UINT SfxSecondaryLangId;
extern LANGSTRING SfxLangStrings[];


#endif // _LANGSTRS_H_INCLUDED_
