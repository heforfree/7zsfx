/*---------------------------------------------------------------------------*/
/* File:        SfxTest.cpp                                                  */
/* Created:     Sun, 25 Feb 2007 09:13:00 GMT                                */
/*              by Oleg N. Scherbakov, mailto:oleg@7zsfx.info                */
/* Last update: Sun, 06 Jun 2010 09:05:47 GMT                                */
/*              by Oleg N. Scherbakov, mailto:oleg@7zsfx.info                */
/* Revision:    1197                                                         */
/*---------------------------------------------------------------------------*/
/* Revision:    1197                                                         */
/* Updated:     Sun, 06 Jun 2010 09:05:47 GMT                                */
/*              by Oleg N. Scherbakov, mailto:oleg@7zsfx.info                */
/* Description: Add one more test for warning dialog                         */
/*---------------------------------------------------------------------------*/
/* Revision:    1122                                                         */
/* Updated:     Mon, 22 Mar 2010 11:32:53 GMT                                */
/*              by Oleg N. Scherbakov, mailto:oleg@7zsfx.info                */
/* Description: New file stamp.                                              */
/*---------------------------------------------------------------------------*/
#include "stdafx.h"
#include "7ZSfxModInt.h"
#include "SfxDialogs.h"

#ifdef _SFX_USE_TEST

UString	TSD_Flags;
DWORD	TSD_ExtractTimeout = 0;
int		nTestModeType = 0;

int TestSfxDialogs( CObjectVector<CTextConfigPair>& config )
{
	LPCWSTR	lpwszValue;
	LPCWSTR	lpwszFlags = TSD_Flags;
	int	nExitCode = ERRC_NONE;

	while( *lpwszFlags != L'\0' )
	{
		switch( *lpwszFlags )
		{
		case TSD_BEGINPROMPT:
		case TSD_BEGINPROMPT_FORCE:
			if( (lpwszValue = GetTextConfigValue( config, CFG_BEGINPROMPT )) == NULL &&
					*lpwszFlags == TSD_BEGINPROMPT_FORCE )
			{
				lpwszValue = CFG_BEGINPROMPT;
			}
			if( lpwszValue == NULL ) nExitCode = ERRC_SFXTEST;
			SfxBeginPrompt( lpwszTitle, lpwszValue );
			break;
		case TSD_CANCELPROMPT:
		case TSD_CANCELPROMPT_FORCE:
			if( (GUIFlags&GUIFLAGS_CONFIRM_CANCEL) != 0 || *lpwszFlags == TSD_CANCELPROMPT_FORCE )
			{
				CSfxDialog_CancelPrompt	cancelPrompt;
				cancelPrompt.IsCancel( NULL );
			}
			else
				nExitCode = ERRC_SFXTEST;
			break;
		case TSD_EXTRACTPATH:
		case TSD_EXTRACTPATH_FORCE:
			if( (GUIFlags&(GUIFLAGS_EXTRACT_PATH1|GUIFLAGS_EXTRACT_PATH2)) == GUIFLAGS_EXTRACT_PATH2 ||
					*lpwszFlags == TSD_EXTRACTPATH_FORCE )
			{
				SfxExtractPathDialog( lpwszExtractPathTitle, lpwszExtractPathText );
			}
			else
				nExitCode = ERRC_SFXTEST;
			break;
		case TSD_EXTRACT:
		case TSD_EXTRACT_FORCE:
			if( GUIMode != GUIMODE_HIDDEN || *lpwszFlags == TSD_EXTRACT_FORCE )
				ExtractDialog();
			else
				nExitCode = ERRC_SFXTEST;
			break;
		case TSD_ERROR:
		case TSD_ERROR_FORCE:
			if( (lpwszValue = GetTextConfigValue( config, CFG_ERRORTITLE )) != NULL ||
					*lpwszFlags == TSD_ERROR_FORCE )
			{
				SfxErrorDialog( FALSE, ERR_NO_SETUP_EXE );
			}
			else
				nExitCode = ERRC_SFXTEST;
			break;
		case TSD_FINISHMESSAGE:
		case TSD_FINISHMESSAGE_FORCE:
			if( (lpwszValue = GetTextConfigValue( config, CFG_FINISHMESSAGE)) == NULL &&
					*lpwszFlags == TSD_FINISHMESSAGE_FORCE )
			{
				lpwszValue = CFG_FINISHMESSAGE;
			}
			if( FinishMessage < 0 )
				FinishMessage = 1;
			if( FinishMessage > 0 || *lpwszFlags == TSD_FINISHMESSAGE_FORCE )
			{
				CSfxDialog_FinishMessage	fmDialog;
				fmDialog.Show( SD_OK|SD_ICONINFORMATION, lpwszTitle, lpwszValue );
			}
			else
				nExitCode = ERRC_SFXTEST;
			break;
		case TSD_HELPTEXT:
		case TSD_HELPTEXT_FORCE:
			if( (lpwszValue = GetTextConfigValue( config, CFG_HELP_TEXT )) == NULL &&
					*lpwszFlags == TSD_HELPTEXT_FORCE )
			{
				lpwszValue = GetLanguageString( STR_DEFAULT_HELP_TEXT );
			}
			if( lpwszValue == NULL )
				nExitCode = ERRC_SFXTEST;
			{
				CSfxDialog_HelpText	dlg;
				dlg.Show( SD_OK|SD_ICONINFORMATION, lpwszTitle, lpwszValue );
			}
			break;
		case TSD_WARNING:
		case TSD_WARNING_FORCE:
			if( (lpwszValue = GetTextConfigValue( config, CFG_WARNINGTITLE)) != NULL ||
				*lpwszFlags == TSD_WARNING_FORCE )
			{
				ShowSfxWarningDialog( GetLanguageString(STR_DISK_FREE_SPACE) );
			}
			else
				nExitCode = ERRC_SFXTEST;
			break;
		}
		lpwszFlags++;
	}

	return nExitCode;
}

int TestSfxDialogsToStdout( CObjectVector<CTextConfigPair>& config )
{
	// TSD_BEGINPROMPT:
	if( GetTextConfigValue( config, CFG_BEGINPROMPT ) != NULL )
		WriteStdoutChar( TSD_BEGINPROMPT );
	// TSD_CANCELPROMPT:
	if( (GUIFlags&GUIFLAGS_CONFIRM_CANCEL) != 0 )
		WriteStdoutChar( TSD_CANCELPROMPT );
	// TSD_EXTRACTPATH:
	if( (GUIFlags&(GUIFLAGS_EXTRACT_PATH1|GUIFLAGS_EXTRACT_PATH2)) == GUIFLAGS_EXTRACT_PATH2 )
		WriteStdoutChar( TSD_EXTRACTPATH );
	// TSD_EXTRACT:
	if( GUIMode != GUIMODE_HIDDEN )
		WriteStdoutChar( TSD_EXTRACT );
	// TSD_ERROR:
	if( GetTextConfigValue( config, CFG_ERRORTITLE ) != NULL )
		WriteStdoutChar( TSD_ERROR );
	// TSD_FINISHMESSAGE:
	if( GetTextConfigValue( config, CFG_FINISHMESSAGE ) != NULL )
		WriteStdoutChar( TSD_FINISHMESSAGE );
	// TSD_HELPTEXT:
	if( GetTextConfigValue( config, CFG_HELP_TEXT ) != NULL )
		WriteStdoutChar( TSD_HELPTEXT );
	if( GetTextConfigValue( config, CFG_WARNINGTITLE) != NULL )
		WriteStdoutChar( TSD_WARNING );
	return ERRC_NONE;
}

#endif // _SFX_USE_TEST