/*---------------------------------------------------------------------------*/
/* File:        langstrs.cpp                                                 */
/* Created:     Wed, 10 Jan 2007 23:44:00 GMT                                */
/*              by Oleg N. Scherbakov, mailto:oleg@7zsfx.info                */
/* Last update: Tue, 22 Nov 2011 09:52:03 GMT                                */
/*              by Oleg N. Scherbakov, mailto:oleg@7zsfx.info                */
/* Revision:    1777                                                         */
/*---------------------------------------------------------------------------*/
/* Revision:    1404                                                         */
/* Updated:     Sun, 14 Nov 2010 01:04:46 GMT                                */
/*              by Oleg N. Scherbakov, mailto:oleg@7zsfx.info                */
/* Description: Add 'password' strings                                       */
/*---------------------------------------------------------------------------*/
/* Revision:    1263                                                         */
/* Updated:     Sat, 26 Jun 2010 04:44:01 GMT                                */
/*              by Oleg N. Scherbakov, mailto:oleg@7zsfx.info                */
/* Description: Add 'copyright' strings                                      */
/*---------------------------------------------------------------------------*/
/* Revision:    1167                                                         */
/* Updated:     Mon, 22 Mar 2010 11:17:07 GMT                                */
/*              by Oleg N. Scherbakov, mailto:oleg@7zsfx.info                */
/* Description: New file stamp.                                              */
/*---------------------------------------------------------------------------*/
#include "stdafx.h"
#include "7zSfxModInt.h"
#include "langstrs.h"
#include "version.h"

/* RU-russian */
const UINT SfxSecondaryLangId = 0x419;

LANGSTRING SfxLangStrings[] = {
// ������ ������
	{ STR_SFXVERSION,		"SFX module - Copyright (c) 2005-2011 Oleg Scherbakov\n"
							"\t"VERSION_SFX_TEXT_EN"\n"
							"\n7-Zip archiver - Copyright (c) 1999-2011 Igor Pavlov\n"
							"\t"VERSION_7ZIP_TEXT_EN"\n"
							"\nSupported methods and filters, build options:\n\t",
						"SFX ������ - Copyright (c) 2005-2011 ���� ��������\n"
						"\t"VERSION_SFX_TEXT_RU"\n"
						"\n��������� 7-Zip - Copyright (c) 1999-2011 ����� ������\n"
						"\t"VERSION_7ZIP_TEXT_RU"\n"
						"\n�������������� ������ � �������, ����� ������:\n\t", NULL },
// ��������� ���� ��������� �� ���������, ������������, ����
// ������ �� ���� ���������� ��� exe-�����.
// ���� SFX ������ ���� ���������� ���� ��� (� ��� ����������� 100%), ������ ����
// ����� ��������� ��� exe-����� ��� ����������. ���, ���� ���� ������������
// ������� �������� � � ��� ������ "Title" - ���������� "Title"
	{ STR_TITLE,			"7z SFX",
							"7z SFX", NULL },
	{ STR_ERROR_TITLE,		"7z SFX: error",
							"7z SFX: ������", NULL },
// ������ ��������� ����� ������������ � ��������� ��������� ��� ���� ������
// �.�. ��������� ���� ����� ���-�� ����� "7-Zip SFX: error"
	{ STR_ERROR_SUFFIX,		": error",
							": ������", NULL },
// ��������� �� ��������� ��� ���� ����������
	{ STR_EXTRACT_TITLE,	"Extracting",
							"����������", NULL },
// ���������� �������� ��� ����� SFX ���������. ������������, ������� �� ��������,
// � ���� �� ����, ��� ������������ ��� ��������.
	{ ERR_MODULEPATHNAME,	"Could not get SFX filename.",
							"���������� �������� ��� SFX ������.", NULL },
// ���������� ������� ���� ������
// ����� ����������, ���� ���� ���������� ������ �����������, ������������ ���� � �.�.
// ������ ������ ����� ��������� ����� ��������� ������ �� �����, ������������ � ������� �� ���������
	{ ERR_OPEN_ARCHIVE,		"Could not open archive file \"%s\".",
							"���������� ������� ���� ������ \"%s\".", NULL },
// ����� ���������/�� ������� ���������, �.� ���������� ��������� �� ������������� ������� 7-Zip SFX
// ����� ���������� �� "�������" ��� ������������ 7-Zip SFX ����������
	{ ERR_NON7Z_ARCHIVE,	"Non 7z archive.",
							"���� �� �������� 7z �������.", NULL },
// ���������� ��������� ���� ������������ (��� �� �����������)
	{ ERR_READ_CONFIG,		"Could not read SFX configuration or configuration not found.",
							"���������� ��������� ��� ����������� ���� ������������ SFX ������.", NULL },
	{ ERR_WRITE_CONFIG,		"Could not write SFX configuration.",
							"���������� ��������� ���� ������������.", NULL },
// ������ � ����� ������������, ���������� ������������ �������, ��������������
// ������� ��������="��������", �������� ��������� �� � UTF8 � �.�.
// "������ � ������ � ������������"
	{ ERR_CONFIG_DATA,		"Error in line %d of configuration data:\n\n%s",
							"������ � ������ %d ����� ������������:\n\n%s", NULL },
// ���������� ������� ����� "�����-��"
// ������ ������ ����� ��������� ����� ��������� ������ �� �����, ������������ � ������� �� ���������
	{ ERR_CREATE_FOLDER,	"Could not create folder \"%s\".",
							"���������� ������� ����� \"%s\".", NULL },
// ���������� ������� ���� ��� ���������� "�����-��-�����-��"
// ������ ������ ����� ��������� ����� ��������� ������ �� �����, ������������ � ������� �� ���������
	{ ERR_DELETE_FILE,		"Could not delete file or folder \"%s\".",
							"���������� ������� ���� ��� ����� \"%s\".", NULL },
// ���������� ����� ������� ��� "AutoInstallX"
// ����� �������, ����� ������������ ������ � ��������� ������ -aiX ��� -ai,
// � AutoInstall � �������� X (��� ��� ������� ��� -ai) � ������� �� ������
	{ ERR_AUTOINSTALL_NOTFOUND,	"Could not find command for \"%s\".",
								"�������� \"%s\" �� ������ � ����� ������������.", NULL },
// �� ������� �� "RunProgram", �� "AutoInstall", � ���������� ������ ����������
// �� ��������� �����. � ���� ������-��? ����������� � �������?
	{ ERR_NO_SETUP_EXE,		"Could not find \"setup.exe\".",
							"���� \"setup.exe\" �� ������.", NULL },
// ��������� ������ ��� ���������� "�����-�� ���������"
// ������ ������ ����� ��������� ����� ��������� ������ �� �����, ������������ � ������� �� ���������
	{ ERR_EXECUTE,			"Error during execution \"%s\".",
							"��������� ������ ��� ���������� \"%s\".", NULL },
// ������ ���� ����������
	{ ERR_7Z_UNSUPPORTED_METHOD,	"7-Zip: Unsupported method.",
									"7-Zip: ����� �� ��������������.", NULL },
	{ ERR_7Z_CRC_ERROR,				"7-Zip: CRC error.",
									"7-Zip: ������ ����������� ����� (CRC).", NULL },
	{ ERR_7Z_DATA_ERROR,			"7-Zip: Data error.\nThe archive is corrupted"
#ifdef SFX_CRYPTO
									", or invalid password was entered"
#endif // SFX_CRYPTO
									".",
									"7-Zip: ������ ������.\n����� ���������"
#ifdef SFX_CRYPTO
									" ��� ������ �������� ������"
#endif // SFX_CRYPTO
									".",
									NULL },
	{ ERR_7Z_INTERNAL_ERROR,		"7-Zip: Internal error, code %u.",
									"7-Zip: ���������� ������. ��� ������ %u.", NULL },
	{ ERR_7Z_EXTRACT_ERROR1,		"7-Zip: Internal error, code 0x%08X.",
									"7-Zip: ���������� ������. ��� ������ 0x%08X.", NULL },
	{ ERR_7Z_EXTRACT_ERROR2,		"7-Zip: Extraction error.",
									"7-Zip: ������ ����������.", NULL },

	{ STR_EXTRACT_PATH_TITLE,		"Extraction path",
									"���� ����������", NULL },
	{ STR_EXTRACT_PATH_TEXT,		"Extraction path:",
									"���� ����������:", NULL },
	{ STR_CANCEL_PROMPT,			"Really cancel the installation?",
									"�� ������������� ������ �������� ���������?", NULL },
// ��� ���������� ������� (�������� "HelpText")
	{ STR_DEFAULT_HELP_TEXT,	"No \"HelpText\" in the configuration file.",
								"�������� \"HelpText\" �� ������ � ����� ������������.", NULL },
// ������ ��������
	{ STR_BUTTON_OK,			"OK",		"OK", NULL },
	{ STR_BUTTON_CANCEL,		"Cancel",	"������", NULL },
	{ STR_BUTTON_YES,			"Yes",		"��", NULL },
	{ STR_BUTTON_NO,			"No",		"���", NULL },
	{ STR_SECONDS,				" s",		" �", NULL },
// Added April 9, 2008
// ���������� ������� ���� "�����-��"
// ������ ������ ����� ��������� ����� ��������� ������ �� �����, ������������ � ������� �� ���������
	{ ERR_CREATE_FILE,		"Could not create file \"%s\".",
							"���������� ������� ���� \"%s\".", NULL },

	{ ERR_OVERWRITE,		"Could not overwrite file \"%s\".",
							"���������� ������������ ���� \"%s\".", NULL },
	{ ERR_CONFIG_CMDLINE,	"Error in command line:\n\n%s",
							"������ � ��������� ������:\n\n%s", NULL },
	{ STR_BUTTON_BACK,		"Back",		"�����", NULL },
	{ STR_BUTTON_NEXT,		"Next",		"�����", NULL },
	{ STR_BUTTON_FINISH,	"Finish",	"������", NULL },
	{ STR_BUTTON_CANCEL2,	"Cancel",	"��������", NULL },
#ifdef _SFX_USE_CUSTOM_EXCEPTIONS
	{ ERR_SFX_EXCEPTION,		"Application error:\n\nException code:\n\t0x%08x\nAddress:\n\t0x%08x\nException data:\n",
								"������ ����������:\n\n��� ����������:\n\t0x%08x\n�����:\n\t0x%08x\n������ ����������:\n" },
#endif // _SFX_USE_CUSTOM_EXCEPTIONS

// Added June 6, 2010: warnings dialogs
#ifdef _SFX_USE_WARNINGS
	{ STR_WARNING_TITLE,		"7z SFX: warning",
								"7z SFX: ��������������", NULL },
	{ STR_WARNING_SUFFIX,		": warning",
								": ��������������", NULL },
#ifdef _SFX_USE_CHECK_FREE_SPACE
	{ STR_DISK_FREE_SPACE,		"Not enough free space for extracting.\n\nDo you want to continue?",
								"�� ����� ������������ ����� ��� ����������.\n\n�� ������ ����������?", NULL },
#endif // _SFX_USE_CHECK_FREE_SPACE
#ifdef _SFX_USE_CHECK_RAM
	{ STR_PHYSICAL_MEMORY,		"Insufficient physical memory.\nExtracting may take a long time.\n\nDo you want to continue?",
								"������������ ���������� ������.\n���������� ����� ������ ��������������� �����.\n\n�� ������ ����������?", NULL },
#endif // _SFX_USE_CHECK_FREE_SPACE
#endif // _SFX_USE_WARNINGS
#ifdef SFX_CRYPTO
	{ STR_PASSWORD_TEXT,		"Enter password:",
								"������� ������:", NULL },
#endif // SFX_CRYPTO
	{ 0, "", "", NULL }
};
