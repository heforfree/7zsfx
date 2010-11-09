/*---------------------------------------------------------------------------*/
/* File:        Helpers.cpp                                                  */
/* Created:     Sat, 30 Jul 2005 11:10:00 GMT                                */
/*              by Oleg N. Scherbakov, mailto:oleg@7zsfx.info                */
/* Last update: Mon, 08 Nov 2010 11:40:53 GMT                                */
/*              by Oleg N. Scherbakov, mailto:oleg@7zsfx.info                */
/* Revision:    1928                                                         */
/*---------------------------------------------------------------------------*/
/* Revision:    1697                                                         */
/* Updated:     Mon, 22 Mar 2010 11:16:07 GMT                                */
/*              by Oleg N. Scherbakov, mailto:oleg@7zsfx.info                */
/* Description: New file stamp.                                              */
/*---------------------------------------------------------------------------*/
#include "stdafx.h"
#include "7zSfxModInt.h"
#ifdef _SFX_USE_IMAGES
	#include <olectl.h>
	#include <comdef.h>
#endif // _SFX_USE_IMAGES


UString MultiByteToUnicodeString( const AString &srcString, UINT codePage )
{
	UString resultString;
	if( srcString.IsEmpty() == false )
	{
		int numChars = MultiByteToWideChar( codePage, 0, srcString, srcString.Length(),
											resultString.GetBuffer(srcString.Length()), 
											srcString.Length() + 1 );
		resultString.ReleaseBuffer(numChars);
	}
	return resultString;
}


AString UnicodeStringToMultiByte( const UString &srcString, UINT codePage )
{
	AString resultString;
	if( srcString.IsEmpty() == false )
	{
		int numRequiredBytes = srcString.Length() * 2;
		int numChars = WideCharToMultiByte(	codePage, 0, srcString, srcString.Length(),
											resultString.GetBuffer(numRequiredBytes), 
											numRequiredBytes + 1, NULL, NULL );
		resultString.ReleaseBuffer(numChars);
	}
	return resultString;
}

int DeleteUseOverwriteFlags( LPCWSTR lpwszPath )
{
	if( DeleteFileOrDirectoryAlways( lpwszPath ) == FALSE )
	{
		if( ::GetLastError() == ERROR_ACCESS_DENIED && (OverwriteFlags&OVERWRITE_FLAG_SKIP_LOCKED) != 0 )
			return SFX_OM_SKIP;
		return SFX_OM_ERROR;
	}
	return SFX_OM_OVERWRITE;
}

int GetOverwriteMode( LPCWSTR lpwszPath, FILETIME * fileTime )
{
	DWORD dwAttributes = ::GetFileAttributes( lpwszPath );
	if( dwAttributes == (DWORD)-1 )
		return SFX_OM_OVERWRITE;
	if( dwAttributes&FILE_ATTRIBUTE_DIRECTORY )
	{
		::SetLastError( ERROR_CURRENT_DIRECTORY );
		return SFX_OM_ERROR;
	}
	if( OverwriteMode == OVERWRITE_MODE_ALL )
		return DeleteUseOverwriteFlags( lpwszPath );

	// OverwriteMode: none, older or confirm
	if( OverwriteMode == OVERWRITE_MODE_OLDER )
	{
		WIN32_FIND_DATA	fd;
		HANDLE hFind = ::FindFirstFile( lpwszPath, &fd );
		if( hFind != INVALID_HANDLE_VALUE )
		{
			::FindClose( hFind );
			if( CompareFileTime( &fd.ftLastWriteTime, fileTime ) >= 0 )
				return SFX_OM_SKIP;
		}
		return DeleteUseOverwriteFlags( lpwszPath );
	}
	// OverwriteMode: none or confirm
	return SFX_OM_SKIP;
}

BOOL SfxCreateDirectory( LPCWSTR lpwszPath )
{
	if( ::CreateDirectory( lpwszPath, NULL ) == FALSE )
	{
		DWORD dwLastError;
		if( (dwLastError = ::GetLastError()) != ERROR_ALREADY_EXISTS )
		{
			::SetLastError( dwLastError );
			return FALSE;
		}
		DWORD	dwAttributes = GetFileAttributes( lpwszPath );
		if( dwAttributes != (DWORD)-1 && (dwAttributes&FILE_ATTRIBUTE_DIRECTORY) == 0 )
		{
			::SetLastError( dwLastError );
			return FALSE;
		}
	}
	return TRUE;
}

void ExpandEnvironmentStrings( UString & ustr )
{
	UString	dst;

	DWORD dwSize = ::ExpandEnvironmentStrings( ustr, dst.GetBuffer(1), 1 );
	if( dwSize == 0 )
		return;
	::ExpandEnvironmentStrings( ustr, dst.GetBuffer(dwSize+1), dwSize+1 );
	dst.ReleaseBuffer();
	ustr = dst;
}


bool ReadConfig( IInStream * inStream, LPCSTR startID, LPCSTR endID, AString &stringResult )
{
	inStream->Seek( 0, STREAM_SEEK_SET, NULL );
	stringResult.Empty();
	
	const int kBufferSize = (1 << 12);

	Byte buffer[kBufferSize];
	int signatureStartSize = lstrlenA(startID);
	int signatureEndSize = lstrlenA(endID);
  
	UInt32 numBytesPrev = 0;
	bool writeMode = false;
	UInt64 posTotal = 0;
	while( true )
	{
		if( posTotal > (1 << 20) )
			return ( stringResult.IsEmpty() == false );
		UInt32 numReadBytes = kBufferSize - numBytesPrev;
		UInt32 processedSize;
		if( inStream->Read(buffer + numBytesPrev, numReadBytes, &processedSize) != S_OK || processedSize == 0 )
			return false;
		UInt32 numBytesInBuffer = numBytesPrev + processedSize;
		UInt32 pos = 0;
		while( true )
		{ 
			if( writeMode != false )
			{
				if( pos > (numBytesInBuffer - signatureEndSize) )
					break;
				if( memcmp(buffer + pos, endID, signatureEndSize) == 0 )
					return true;
				char b = buffer[pos];
				if( b == 0 )
					return false;
				stringResult += b;
				pos++;
			}
			else
			{
				if( pos > (numBytesInBuffer - signatureStartSize) )
					break;
				if( memcmp(buffer + pos, startID, signatureStartSize) == 0 )
				{
					writeMode = true;
					pos += signatureStartSize;
				}
				else
					pos++;
			}
		}
		numBytesPrev = numBytesInBuffer - pos;
		posTotal += pos;
		memmove(buffer, buffer + pos, numBytesPrev);
	}
}

static int nCfgLineNumber = 1;

static bool IsDelimitChar( char c )
{
	if( c == 0x0A )
	{
		nCfgLineNumber++;
		return true;
	}
	return( c == ' ' || c == 0x0D || c == '\0' || c == '\t' );
}

static AString GetIDString( const char *string, int &finishPos )
{
	AString result;
	for( finishPos = 0; true; finishPos++ )
	{
		char c = string[finishPos];
		if( IsDelimitChar(c) || c == '=' )
			return result;
		result += c;
	}
}


static bool WaitNextLine( const AString &string, int &pos )
{
	for( ; pos < string.Length(); pos++ )
		if( string[pos] == 0x0A )
		{
			nCfgLineNumber++;
			return true;
		}
	return false;
}

static bool SkipSpaces(const AString &string, int &pos)
{
	for( ;pos < string.Length(); pos++ )
	{
		char c = string[pos];
		if( !IsDelimitChar(c) )
		{
			if( c != ';' )
				return true;
			if( !WaitNextLine(string, pos) )
				return false;
		}
	}
	return false;
}

#ifdef _SFX_USE_TEST
	void WriteStdoutChar( WCHAR wc )
	{
		DWORD	dwWrited;
		char	buf[3];
		buf[0] = (char)wc; buf[1] = '\r'; buf[2] = '\n';
		::WriteFile( ::GetStdHandle(STD_OUTPUT_HANDLE), buf, 3, &dwWrited, NULL );
	}
#endif // _SFX_USE_TEST

bool ReportCfgError( const AString &string, int pos, bool fromCmdLine )
{
#ifdef _SFX_USE_TEST
	if( nTestModeType == TMT_DIALOGS_TO_STDOUT )
	{
		WriteStdoutChar( TSD_ERROR );
		return false;
	}
#endif // _SFX_USE_TEST
	AString messageUTF8;
	for( int i = pos; string[i] != '\0' && string[i] != 0x0A && string[i] != 0x0D; i++ )
		messageUTF8 += string[i];
	UString messageUnicode = MultiByteToUnicodeString( messageUTF8, CP_UTF8 );
	if( fromCmdLine == false )
		SfxErrorDialog( FALSE, ERR_CONFIG_DATA, nCfgLineNumber, (LPCWSTR)messageUnicode );
	else
		SfxErrorDialog( FALSE, ERR_CONFIG_CMDLINE, (LPCWSTR)messageUnicode );
	return false;
}

CTextConfigPair * GetConfigPair( const CObjectVector<CTextConfigPair> &pairs, LPCWSTR id, int * pFrom )
{
	int nFrom = (pFrom == NULL) ? 0: (*pFrom);
	for( int  i = nFrom; i < pairs.Size(); i++ )
		if( lstrcmp( pairs[i].ID, id ) == 0 )
		{
			if( pFrom != NULL )
				*pFrom = i;
			return (CTextConfigPair *)(&(pairs[i]));
		}
		
	return NULL;
}

int GetHexChar( WCHAR sym )
{
	if( sym >= L'0' && sym <= L'9' )
		return sym-L'0';
	if( sym >= L'a' && sym <= L'z' )
		return sym-L'a'+10;
	if( sym >= L'A' && sym <= L'Z' )
		return sym-L'A'+10;
	return -1;
}

int GetHexSymbol( LPCWSTR lpszChars )
{
	int cc;
	if( (cc = GetHexChar(lpszChars[0])<<4) >= 0 && (cc |= GetHexChar(lpszChars[1])) >= 0 )
		return cc;
	return -1;
}

void ReplaceHexChars( UString& str )
{
	UString result;
	LPCWSTR src = str;
	int	c1, c2;
	while( *src != L'\0' )
	{
		if( src[0] == L'~' && src[1] == L'x' && (c1 = GetHexSymbol(src+2)) >= 0 )
		{
			// ANSI char
			AString atmp = (CHAR)c1;
			UString utmp;
			utmp = MultiByteToUnicodeString( atmp, CP_ACP );
			result += utmp;
			src += 4;
		}
		else
			if( src[0] == L'~' && src[1] == L'X' &&
					(c1 = GetHexSymbol(src+2)) >= 0 && (c2 = GetHexSymbol(src+4)) >= 0 )
			{
				// Unicode char
				WCHAR	wc = (WCHAR)((c1<<8) | c2);
				result += wc;
				src += 6;
			}
			else
			{
				result += *src;
				src++;
			}
	}
	str = result;
}

void DeleteParams( CObjectVector<CTextConfigPair> &pairs, LPCWSTR lpwszName  )
{
	for( int  i = 0; i < pairs.Size(); i++ )
	{
		if( lstrcmp( (LPCWSTR)(pairs[i].ID), lpwszName ) == 0 )
		{
			pairs.Delete( i );
			i--;
		}
	}
}

bool GetTextConfig( const AString &string, CObjectVector<CTextConfigPair> &pairs, bool fromCmdLine )
{
	static LPCWSTR MultipleParameters[] = {
		CFG_GUIFLAGS,
		CFG_MISCFLAGS,
		CFG_RUNPROGRAM,
		CFG_AUTOINSTALL,
		CFG_SHORTCUT,
		CFG_DELETE,
		CFG_EXECUTEFILE,
		CFG_SETENVIRONMENT,
		NULL
	};

	int pos = 0;

	/////////////////////
	// read strings
	AString message;
	while (true)
	{
		if( !SkipSpaces(string, pos) )
			break;
		CTextConfigPair pair;
		int startPos = pos;
		int finishPos;
		// ID name
		pair.ID = MultiByteToUnicodeString( GetIDString( ((const char *)string) + pos, finishPos ), CP_UTF8 );
		if( finishPos == 0 )
			return ReportCfgError( string, startPos, fromCmdLine );
		pos += finishPos;
		if( !SkipSpaces(string, pos) )
			return ReportCfgError( string, startPos, fromCmdLine );
		if (string[pos] != '=')
			return ReportCfgError( string, startPos, fromCmdLine );
		pos++;
		if( !SkipSpaces(string, pos) )
			return ReportCfgError( string, startPos, fromCmdLine );
		if( string[pos] != '\"' )
		{
			if( string[pos] == '-' )
			{
				DeleteParams( pairs, pair.ID );
				pos++;
				continue;
			}
			return ReportCfgError( string, startPos, fromCmdLine );
		}
		pos++;
		message.Empty();
#ifdef _SFX_USE_RTF_CONTROL
Loc_RTF:
		if( strncmp( string+pos, "{\\rtf", 5 ) == 0 )
		{
			while( string[pos] != '\0' && string[pos] != '\"' )
			{
				if( string[pos] == '\\' && string[pos+1] == '\"' )
					pos++;
				message += string[pos++];
			}
			if( string[pos] != '\0' )
				pos++;
		}
		else
#endif // _SFX_USE_RTF_CONTROL
		{
			while( true )
			{
#ifdef _SFX_USE_RTF_CONTROL
				if( strncmp( string+pos, "{\\rtf", 5 ) == 0 )
					goto Loc_RTF;
#endif // _SFX_USE_RTF_CONTROL
				if( pos >= string.Length() )
					return ReportCfgError( string, startPos, fromCmdLine );
				char c = string[pos++];
				if( c == '\"' )
					break;
				if( c == '\\' )
				{
					char c = string[pos++];
					switch(c)
					{
					case 'n':
						message += '\n';
						break;
					case 't':
						message += '\t';
						break;
					case '\\':
						message += '\\';
						break;
					case '\"':
						message += '\"';
						break;
					default:
						message += '\\';
						message += c;
						break;
					}
				}
				else
					message += c;
			}
		}
		pair.String = MultiByteToUnicodeString( message, CP_UTF8 );
		ReplaceHexChars( pair.String );
		if( lstrcmp( pair.ID, CFG_SETENVIRONMENT ) == 0 )
		{
			if( pair.String.Find( L'=') <= 0 )
				return ReportCfgError( string, startPos, fromCmdLine );
		}
		LPCWSTR * mp = MultipleParameters;
		while( *mp != NULL )
		{
			if( wcsncmp( pair.ID, *mp, lstrlen(*mp) ) == 0 )
				break;
			mp++;
		}
		if( *mp != NULL )
		{
			// Multiple parameter, add to list
			pairs.Add( pair );
		}
		else
		{
			// Single parameter
			CTextConfigPair * pPair;
			if( (pPair=GetConfigPair(pairs, pair.ID, NULL)) != NULL )
				pPair->String = (LPCWSTR)(pair.String);
			else
				pairs.Add( pair );
		}
	}
	return true;
}


LPCWSTR GetTextConfigValue( const CObjectVector<CTextConfigPair> &pairs, LPCWSTR id, int * pFrom )
{
	CTextConfigPair * pPair = GetConfigPair( pairs, id, pFrom );
	if( pPair != NULL )
		return pPair->String;
	return NULL;
}

BOOL CreateFolderTree( LPCWSTR lpwszPath )
{
	int nLength = ::lstrlen( lpwszPath );
	UString	Path2 = lpwszPath;
	LPWSTR lpwszPath2 = Path2.GetBuffer(0);
	if( lpwszPath[nLength-1] == L'\\' || lpwszPath[nLength-1] == '/' )
	{
		lpwszPath2[nLength-1] = L'\0';
		nLength--;
	}
	int i = nLength;
	while( TRUE )
	{
		if( SfxCreateDirectory( lpwszPath2 ) != FALSE )
			break;
		for( ; i > 0 && lpwszPath2[i] != L'/' && lpwszPath2[i] != L'\\'; i-- );
		if( i == 0 )
		{
			if( nLength == 2 &&
					(lpwszPath[0]|0x20) >= L'a' && (lpwszPath[0]|0x20) <= L'z' &&
						lpwszPath[1] == L':' )
				return TRUE;
			SfxErrorDialog( TRUE, ERR_CREATE_FOLDER, lpwszPath );
			return FALSE;
		}
		lpwszPath2[i] = L'\0';
	}
	
	FILETIME	ftTmp;
	::GetSystemTimeAsFileTime( &ftTmp );
	if( (GetFileAttributes(lpwszPath2)&FILE_ATTRIBUTE_DIRECTORY) == 0 &&
			GetOverwriteMode(lpwszPath2, &ftTmp ) != SFX_OM_OVERWRITE )
	{
		return FALSE;
	}
	
	if( SfxCreateDirectory( lpwszPath2 ) == FALSE )
	{
		SfxErrorDialog( TRUE, ERR_CREATE_FOLDER, lpwszPath2 );
		return FALSE;
	}
	
	while( i < nLength )
	{
		memcpy( lpwszPath2+i, lpwszPath+i, nLength-i+1 );
		if( lpwszPath2[i] != L'\0' )
			i++;
		while( lpwszPath2[i] != L'\0' && lpwszPath2[i] != L'\\' && lpwszPath2[i] != L'/' ) i++;
		lpwszPath2[i] = L'\0';
		if( SfxCreateDirectory( lpwszPath2 ) == FALSE )
		{
			SfxErrorDialog( TRUE, ERR_CREATE_FOLDER, lpwszPath2 );
			return FALSE;
		}
	}

	return TRUE;
}

BOOL DeleteDirectoryWithSubitems( LPCWSTR path )
{
	WIN32_FIND_DATA	fd;
	HANDLE			hFind;
	UString			path2 = path;
	path2 += L"\\*";
	if( (hFind = ::FindFirstFile( path2, &fd )) != INVALID_HANDLE_VALUE )
	{
		do {
			path2 = path; path2 += L'\\'; path2 += fd.cFileName;
			if( (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 )
			{
				// directory
				if( lstrcmp(fd.cFileName, L".") != 0 && lstrcmp(fd.cFileName, L"..") != 0 )
				{
					if( DeleteDirectoryWithSubitems( path2 ) == FALSE )
						return FALSE;
				}
			}
			else
			{
				// file
				if( ::SetFileAttributes( path2, 0 ) == FALSE || ::DeleteFile( path2 ) == FALSE )
					return FALSE;
			}
		} while( ::FindNextFile( hFind, &fd ) != FALSE );
		::FindClose( hFind );
	}
	if( ::SetFileAttributes( path, 0 ) == FALSE || RemoveDirectory( path ) == FALSE )
		return FALSE;
	return TRUE;
}

BOOL DeleteFileOrDirectoryAlways( LPCWSTR lpwszPathName )
{
#ifdef _SFX_USE_TEST
	if( nTestModeType != 0 )
		return TRUE;
#endif // _SFX_USE_TEST
	DWORD dwAttributes;
	if( (dwAttributes = GetFileAttributes(lpwszPathName)) == (DWORD)-1 )
		return TRUE;
	if( (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0 )
	{
		// file
		if( ClearFileAttributes(lpwszPathName) != FALSE )
			return DeleteFile( lpwszPathName );
		return FALSE;
	}
	// delete folder tree
	return DeleteDirectoryWithSubitems( lpwszPathName );
}

// Shortcut=<���>,<�������>,<���������>,<��������>,<��������>,<���������>
LPCTSTR FetchShortcutSubstring( LPCWSTR src, UString& result )
{
	result.Empty();
	while( *src != L'\0' && ((unsigned)(*src)) <= L' ' ) src++;
	if( *src == L',' ) src++;
	while( *src != L'{' )
	{
		if( *src == L'\0' || *src == L',' )
			return src;
		src++;
	}
	src++;
	while( *src != L'\0' )
	{
		switch( *src )
		{
		case L'{':
			if( src[1] == L'{' )
				src++;
			break;
		case L'}':
			if( src[1] == L'}' )
			{
				src++;
				break;
			}
			return ++src;
		}
		result += *src++;
	}
	return src;
}

BOOL CreateShortcut( LPCTSTR lpszShortcutData )
{
	int nFolder;
	if( *lpszShortcutData >= L'0' && *lpszShortcutData <= L'9' )
		nFolder = StringToLong( lpszShortcutData );
	else
	{
		switch( lpszShortcutData[0]|0x20 )
		{
		case 'd':
			// desktop
			if( (lpszShortcutData[1]|0x20) == L'u' )
				nFolder = CSIDL_DESKTOPDIRECTORY;
			else
				nFolder = CSIDL_COMMON_DESKTOPDIRECTORY;
			break;
		case 's':
			// "Start" menu
			if( (lpszShortcutData[1]|0x20) == L'u' )
				nFolder = CSIDL_STARTMENU;
			else
				nFolder = CSIDL_COMMON_STARTMENU;
			break;
		case 'p':
			// "Start"->"Programs"
			if( (lpszShortcutData[1]|0x20) == L'u' )
				nFolder = CSIDL_PROGRAMS;
			else
				nFolder = CSIDL_COMMON_PROGRAMS;
			break;
		case 't':
			// "Start"->"Startup"
			if( (lpszShortcutData[1]|0x20) == L'u' )
				nFolder = CSIDL_STARTUP;
			else
				nFolder = CSIDL_COMMON_STARTUP;
			break;
		default:
			return FALSE;
		}
	}
	while( *lpszShortcutData != L',' )
	{
		if( *lpszShortcutData == L'\0' )
			return FALSE;
		lpszShortcutData++;
	}

	WCHAR wszPath[MAX_PATH+1];
	if( ::SHGetSpecialFolderPath(NULL,wszPath,nFolder,FALSE) == FALSE )
		return FALSE;

	// Work with the special folder's path (contained in szPath)
	UString	shortcutPath = wszPath;
	UString	srcFile;
	UString	commandLine;
	UString	dstFolder;
	UString	description;
	UString	shortcutName;
	UString	workFolder;
	UString iconLocation;
	UString	iconIndex;

	lpszShortcutData = FetchShortcutSubstring( lpszShortcutData, srcFile );
	if( srcFile.IsEmpty() != false )
		return FALSE;
	lpszShortcutData = FetchShortcutSubstring( lpszShortcutData, commandLine );
	lpszShortcutData = FetchShortcutSubstring( lpszShortcutData, dstFolder );
	lpszShortcutData = FetchShortcutSubstring( lpszShortcutData, description );
	lpszShortcutData = FetchShortcutSubstring( lpszShortcutData, shortcutName );
	lpszShortcutData = FetchShortcutSubstring( lpszShortcutData, workFolder );
	lpszShortcutData = FetchShortcutSubstring( lpszShortcutData, iconLocation );
	FetchShortcutSubstring( lpszShortcutData, iconIndex );
	long nIconIndex = StringToLong( iconIndex );
	if( shortcutName.IsEmpty() != false )
	{
		int nPos = GetDirectorySeparatorPos( srcFile ); // srcFile.ReverseFind( L'\\' );
		shortcutName = ((LPCTSTR)srcFile)+nPos+1;
		nPos = shortcutName.ReverseFind( L'.' );
		if( nPos >= 0 )
			shortcutName.ReleaseBuffer( nPos );
	}
	shortcutPath += L'\\';
	if( dstFolder.IsEmpty() == false )
	{
		shortcutPath += dstFolder; shortcutPath += L'\\';
	}
	if( CreateFolderTree( (LPCWSTR)shortcutPath ) == FALSE )
		return FALSE;
	shortcutPath += shortcutName;
	shortcutPath += L".lnk";

	HRESULT hres;
	IShellLink * psl = NULL;
 
	// Get a pointer to the IShellLink interface.
	hres = CoCreateInstance( CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void **)&psl );
	if( SUCCEEDED(hres) )
	{
		IPersistFile *	ppf = NULL;
 
		// Set the path to the shortcut target
		psl->SetPath( srcFile );
		if( description.IsEmpty() == false )
			psl->SetDescription( description );
		if( commandLine.IsEmpty() == false )
			psl->SetArguments( commandLine );
		if( workFolder.IsEmpty() == false )
			psl->SetWorkingDirectory( workFolder );
		if( iconLocation.IsEmpty() == false )
			psl->SetIconLocation( iconLocation, nIconIndex );

 		// Query IShellLink for the IPersistFile interface for saving the shortcut in persistent storage. 
		hres = psl->QueryInterface( IID_IPersistFile, (void **)&ppf );
		if( SUCCEEDED(hres) )
		{
			// Save the link by calling IPersistFile::Save.
			hres = ppf->Save( shortcutPath, TRUE );
			ppf->Release();
		}
		psl->Release();
	}
	
	return FALSE;
}

void ReplaceWithExtractPath( UString& str, UString &extractPath )
{
	UString tmp;
	
	tmp = extractPath; tmp += L"\\"; str.Replace( L"%%T\\", tmp );
	tmp = extractPath; tmp += L"/"; str.Replace( L"%%T/", tmp );
	str.Replace( L"%%T", extractPath );
}

void ReplaceWithArchivePath( UString& str, UString &archivePath )
{
	UString tmp;
	
	tmp = archivePath; tmp += L"\\"; str.Replace( L"%%S\\", tmp );
	tmp = archivePath; tmp += L"/"; str.Replace( L"%%S/", tmp );
	str.Replace( L"%%S", archivePath );
}

void ReplaceWithArchiveName( UString& str, UString &archiveName )
{
	UString tmp;
	
	tmp = archiveName; tmp += L"\\"; str.Replace( L"%%M\\", tmp );
	tmp = archiveName; tmp += L"/"; str.Replace( L"%%M/", tmp );
	str.Replace( L"%%M", archiveName );
}

int MyStrincmp( LPCWSTR str1, LPCWSTR str2, int nLength )
{
	int j;
	for( j = 0; j < nLength && str1[j] != L'\0' && str2[j] != L'\0'; j++ )
	{
		if( MyCharUpper(str1[j]) != MyCharUpper(str2[j]) )
			return MyCharUpper(str1[j]) - MyCharUpper(str2[j]);
	}
	return (j == nLength) ? 0 : 1;
}

BOOL checkAlloc(size_t size)
{
	if (size == 0)
		return FALSE;
#ifdef _SFX_USE_CHECK_RAM
	MEMORYSTATUSEX	ms;
	ms.dwLength = sizeof(ms);
#ifdef _DEBUG
	if( size > 20*1024*1024 )
#else
	if( (MiscFlags&MISCFLAGS_NO_CHECK_RAM) == 0 &&
			::GlobalMemoryStatusEx(&ms) != FALSE && ms.ullAvailPhys < size )
#endif // _DEBUG
	{
		if( ShowSfxWarningDialog( GetLanguageString(STR_PHYSICAL_MEMORY) ) != IDOK )
		{
			MiscFlags = -1;
			return FALSE;
		}
		MiscFlags |= MISCFLAGS_NO_CHECK_RAM;
	}
#endif // _SFX_USE_CHECK_RAM
	return TRUE;
}

// from alloc.c
extern "C" void *BigAlloc(size_t size)
{
	if( checkAlloc(size) == FALSE )
		return NULL;
	return VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);
}

extern "C" void *MidAlloc(size_t size)
{
	if( checkAlloc(size) == FALSE )
		return NULL;
	return VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);
}
extern "C" void *MyAlloc(size_t size)
{
	if( checkAlloc(size) == FALSE )
		return NULL;
	return malloc(size);
}

extern "C" void BigFree(void *address)	{ if( address != NULL ) VirtualFree(address, 0, MEM_RELEASE); }
extern "C" void MidFree(void *address)	{ if( address != NULL ) VirtualFree(address, 0, MEM_RELEASE); }
extern "C" void MyFree(void *address)	{ if( address != NULL ) free(address); }

int GetDirectorySeparatorPos( UString& ustrPath )
{
	int nPos1 = ustrPath.ReverseFind( L'\\' );
	int nPos2 = ustrPath.ReverseFind( L'/' );
	if( nPos2 > nPos1 ) nPos1 = nPos2;
	return nPos1;
}

void CreateDummyWindow()
{
	HWND hwnd = CreateWindow( WC_STATIC, L"", 0,-10,-10,5,5, NULL, NULL, GetModuleHandle(NULL), NULL );
	if( hwnd )
	{
		SetTimer( hwnd, 1, 1, NULL );
		MSG	msg;
		::GetMessage(&msg,NULL,0,0);
		::DispatchMessage( &msg );
		::KillTimer( hwnd, 1 );
		::DestroyWindow( hwnd );
	}
}

UString GetWindowUString( HWND hwnd )
{
	UString result;
	int nLength = GetWindowTextLength( hwnd );
	if( nLength > 0 )
	{
		GetWindowText( hwnd, result.GetBuffer(nLength+1), nLength+1 );
		result.ReleaseBuffer( nLength );
	}
	return result;
}

BOOL GetChildRect( HWND hwnd, LPRECT rc )
{
	HWND hwndParent =  GetParent( hwnd );
	if( hwndParent == NULL )
		return FALSE;
	::GetWindowRect( hwnd, rc );
	::ScreenToClient( hwndParent, (LPPOINT)&(rc->left) );
	::ScreenToClient( hwndParent, (LPPOINT)&(rc->right) );
	return TRUE;
}

BOOL ReplaceVariablesInWindow( HWND hwnd )
{
	UString	ustrText = GetWindowUString( hwnd );
	ReplaceVariablesEx( ustrText );
	ustrText.Replace( L"%^", L"%" );
	return ::SetWindowText( hwnd, (LPCWSTR)ustrText );
}

#ifdef _SFX_USE_RTF_CONTROL

HWND RecreateAsRichEdit( HWND hwndStatic )
{
	char szClassName[64];
	if( GetClassNameA(hwndStatic,szClassName,64) == 0 || lstrcmpiA(szClassName,"STATIC") != 0 )
		return NULL;
	if( (GetWindowLong(hwndStatic,GWL_STYLE)&SS_BITMAP) != 0 )
		return NULL;

	UString wndText = GetWindowUString( hwndStatic );
	if( MyStrincmp(wndText,L"{\\rtf",5 ) != 0 )
		return NULL;

	HWND hwndParent =  GetParent( hwndStatic );
	if( hwndParent == NULL )
		return NULL;
	LoadLibraryA( "riched20" );
	RECT rcClient;
	GetChildRect( hwndStatic, &rcClient );
	HMENU hMenu = GetMenu( hwndStatic );
	SetThreadLocale( 1049 );
	HWND hwndRichEdit = ::CreateWindowExW( 0, RICHEDIT_CLASSW, L"",WS_VISIBLE|WS_CHILD|ES_READONLY|ES_MULTILINE,
		rcClient.left, rcClient.top, rcClient.right-rcClient.left, rcClient.bottom-rcClient.top,
		hwndParent, hMenu, NULL, NULL );
	if( hwndRichEdit == NULL )
		return NULL;
	::DestroyWindow( hwndStatic );
	::SendMessage( hwndRichEdit, EM_SETTEXTMODE, TM_RICHTEXT|TM_MULTICODEPAGE, 0 );
	::SendMessage( hwndRichEdit, EM_SETBKGNDCOLOR, 0, GetSysColor(COLOR_3DFACE) );
	SETTEXTEX stex;
	stex.codepage = CP_UTF8;
	stex.flags = ST_DEFAULT;
	AString utf8WndText = UnicodeStringToMultiByte( wndText, CP_UTF8 );
	::SendMessage( hwndRichEdit, EM_SETTEXTEX, (WPARAM)&stex, (LPARAM)((LPCSTR)utf8WndText) );
	return hwndRichEdit;
}

#endif // _SFX_USE_RTF_CONTROL

UString MyGetEnvironmentVariable( LPCWSTR lpwszName )
{
	TCHAR tmp[2];
	DWORD dwChars = GetEnvironmentVariableW( lpwszName, tmp, 1 );
	UString	result;
	result.GetBuffer( dwChars+2 );
	GetEnvironmentVariableW( lpwszName, result.GetBuffer( dwChars+2 ), dwChars+1 );
	result.ReleaseBuffer();
	return result;
}

// Languages part
LANGID idSfxLang = 0;

#ifdef _SFX_USE_LANG

LPVOID LoadInterfaceResource( LPCSTR lpType, LPCSTR lpName, size_t * lpSize )
{
	static BOOL fLangIdSetted = FALSE;
	HMODULE hModule = ::GetModuleHandle( NULL );
	HRSRC hRsrc = ::FindResourceExA( hModule, lpType, lpName, (WORD)idSfxLang );
	if( hRsrc == NULL )
		hRsrc = ::FindResourceExA( hModule, lpType, lpName, MAKELANGID(LANG_ENGLISH,SUBLANG_DEFAULT) );
	if( hRsrc != NULL )
	{
		if( lpSize != NULL )
			*lpSize = SizeofResource( hModule, hRsrc );
		HGLOBAL hGlobal = ::LoadResource( hModule, hRsrc );
		if( hGlobal != NULL )
			return ::LockResource( hGlobal );
	}
	if( fLangIdSetted == FALSE )
	{
		fLangIdSetted = TRUE;
		typedef BOOL (WINAPI * SET_PREFERRED_UI_LANGUAGES_PROTOTYPE ) ( DWORD, PCWSTR, PULONG );

		WCHAR wszLanguages[32];
		SET_PREFERRED_UI_LANGUAGES_PROTOTYPE pLangProc = (SET_PREFERRED_UI_LANGUAGES_PROTOTYPE)GetProcAddress( LoadLibraryA("kernel32"), "SetProcessPreferredUILanguages" );
		wsprintfW( wszLanguages, L"%04X%c%04X%c", idSfxLang, 0, MAKELANGID(LANG_ENGLISH,SUBLANG_DEFAULT), 0 );
		if( pLangProc == NULL )
			pLangProc = (SET_PREFERRED_UI_LANGUAGES_PROTOTYPE)GetProcAddress( LoadLibraryA("kernel32"), "SetThreadPreferredUILanguages" );
		if( pLangProc != NULL )
			pLangProc( MUI_LANGUAGE_ID, wszLanguages, NULL );

	}
	return NULL;
}

#endif // _SFX_USE_LANG

#ifdef _SFX_USE_IMAGES

HBITMAP CopyResizeBmp( HBITMAP hBmp )
{
	BITMAP	bmp;
	HDC hDC = GetWindowDC(NULL);
	int	dpix = GetDeviceCaps( hDC, LOGPIXELSX );
	int	aspectratioMul = 1;
	int aspectratioDiv = 1;

	if( dpix < 1 ) dpix = 96;
	int percent = MulDiv( dpix, 100, 96 );
	if( percent >= 118 && percent <= 145 )
	{
		aspectratioMul = 4;
		aspectratioDiv = 3;
	}
	else
		if ( percent > 145 )
		{
			aspectratioMul = 3;
			aspectratioDiv = 2;
		}
		if( aspectratioMul != 1 )
		{
			GetObject( hBmp,sizeof(BITMAP),&bmp );
			int newcx = MulDiv( bmp.bmWidth, aspectratioMul, aspectratioDiv );
			int newcy = MulDiv( bmp.bmHeight, aspectratioMul, aspectratioDiv );
			HDC hdcSrc = CreateCompatibleDC( hDC );
			HDC hdcDst = CreateCompatibleDC( hDC );
			HBITMAP hOldSrcBmp = (HBITMAP)SelectObject( hdcSrc, hBmp );
			HBITMAP hNewBmp = CreateCompatibleBitmap( hDC, newcx, newcy );
			HBITMAP hOldDstBmp = (HBITMAP)SelectObject( hdcDst, hNewBmp );
			SetStretchBltMode( hdcDst, HALFTONE );
			StretchBlt( hdcDst, 0,0, newcx, newcy, hdcSrc,0,0,bmp.bmWidth,bmp.bmHeight,SRCCOPY );
			hNewBmp = (HBITMAP)GetCurrentObject( hdcDst, OBJ_BITMAP );
			SelectObject( hdcSrc, hOldSrcBmp );
			SelectObject( hdcDst, hOldDstBmp );
			DeleteDC( hdcSrc );
			DeleteDC( hdcDst );
			ReleaseDC( NULL, hDC );
			return hNewBmp;
		}
		ReleaseDC( NULL, hDC );
		return (HBITMAP)CopyImage( hBmp,IMAGE_BITMAP,0,0,0 );
}

BOOL SetDlgControlImage( HWND hwndControl )
{
	// Use IPicture stuff to use JPG / GIF files
	IPicture *	p;
	IStream *	s;
	HBITMAP		hbm;

	char szClassName[64];
	if( GetClassNameA(hwndControl,szClassName,64) == 0 || lstrcmpiA(szClassName,"STATIC") != 0 )
		return FALSE;
	if( (GetWindowLong(hwndControl,GWL_STYLE)&SS_BITMAP) == 0 )
		return FALSE;

	void * data;
	size_t sz;
#ifdef _SFX_USE_LANG
	data = LoadInterfaceResource( "IMAGES", MAKEINTRESOURCEA(GetMenu(hwndControl)), &sz );
#else
	HRSRC res = FindResourceA(GetModuleHandle(NULL),MAKEINTRESOURCEA(GetMenu(hwndControl)),"IMAGES");
	if( res == NULL )
		return FALSE;
	sz = SizeofResource( GetModuleHandle(NULL), res );
	HGLOBAL mem = LoadResource( GetModuleHandle(NULL), res );
	if( mem == NULL )
		return FALSE;
	data = LockResource( mem );
#endif // _SFX_USE_LANG
	if( data == NULL || sz < 16 )
		return FALSE;

	HGLOBAL hGlobal = GlobalAlloc(GPTR, sz);
	memcpy((void *)hGlobal,data,sz);
	CoInitialize(NULL);
	if( ::CreateStreamOnHGlobal( hGlobal, FALSE, &s ) != S_OK || s == NULL )
	{
		::GlobalFree( hGlobal );
		return FALSE;
	}

	::OleLoadPicture( s, 0, FALSE, IID_IPicture, (void**)&p );
	s->Release();
	::GlobalFree( hGlobal );

	if( p == NULL )
		return FALSE;

	hbm = NULL;
	p->get_Handle( (OLE_HANDLE *)(&hbm) );
	if( hbm != NULL )
	{
		hbm = CopyResizeBmp( hbm );
		BITMAP	bmp;
		GetObject( hbm,sizeof(BITMAP),&bmp );
		SetWindowPos( hwndControl, NULL, 0,0, bmp.bmWidth,bmp.bmHeight, SWP_NOMOVE|SWP_NOZORDER );
		SendMessage( hwndControl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbm );
	}
	p->Release();

	return TRUE;
}

#endif // _SFX_USE_IMAGES

UINT GetUILanguage()
{
	if( idSfxLang == 0 )
	{
		idSfxLang = GetUserDefaultUILanguage();

		if( PRIMARYLANGID(idSfxLang) != LANG_ENGLISH /* not english */ )
			return idSfxLang;
		if( GetSystemDefaultUILanguage() == MAKELANGID(LANG_ENGLISH,SUBLANG_DEFAULT) /* english */ &&
				GetSystemDefaultLCID() == SfxSecondaryLangId )
		{
			idSfxLang = SfxSecondaryLangId;
		}
	}

	return idSfxLang;
}

LPCWSTR GetLanguageString( UINT id )
{
	UINT i;
	WCHAR	wszEnvName[sizeof(SFX_STRING_FORMAT)/sizeof(WCHAR)+32];
	LPWSTR	lpwszEnvValue;

	for( i = 0; SfxLangStrings[i].id != 0; i++ )
	{
		if( SfxLangStrings[i].id == id )
			break;
	}
	if( SfxLangStrings[i].id == 0 )
		return L"";

	DWORD dwCurrentLastError = ::GetLastError();
	wsprintf( wszEnvName, SFX_STRING_FORMAT, id );
	DWORD dwSize = GetEnvironmentVariable( wszEnvName, NULL, 0 );
	if( GetLastError() == NO_ERROR )
	{
		// use environment value
		lpwszEnvValue = new WCHAR[dwSize+2];
		if( GetEnvironmentVariable( wszEnvName, lpwszEnvValue, dwSize+1 ) <= dwSize && GetLastError() == NO_ERROR )
		{
			if( SfxLangStrings[i].lpszUnicode != NULL )
			{
				if( lstrcmpi(SfxLangStrings[i].lpszUnicode,lpwszEnvValue) != 0 )
				{
					delete SfxLangStrings[i].lpszUnicode;
					SfxLangStrings[i].lpszUnicode = lpwszEnvValue;
				}
				else
				{
					delete lpwszEnvValue;
				}
			}
			else
				SfxLangStrings[i].lpszUnicode = lpwszEnvValue;
		}
		else
			delete lpwszEnvValue;
	}
	::SetLastError( dwCurrentLastError );

	if( SfxLangStrings[i].lpszUnicode != NULL )
		return SfxLangStrings[i].lpszUnicode;
	
	LPCSTR	lpszReturn = SfxLangStrings[i].strPrimary;
	
	if( SfxLangStrings[i].strSecondary != NULL && GetUILanguage() == SfxSecondaryLangId )
		lpszReturn = SfxLangStrings[i].strSecondary;
	
	int nLength = lstrlenA(lpszReturn)+1;
	SfxLangStrings[i].lpszUnicode = new WCHAR[nLength+2];
	static UINT uACP = (UINT)-1;
	if( uACP == (UINT)-1 )
	{
		WCHAR acp_text[32];
		uACP = CP_ACP;
		if( GetLocaleInfo( idSfxLang, LOCALE_IDEFAULTANSICODEPAGE, acp_text, (sizeof(acp_text)/sizeof(acp_text[0]))-1 ) > 0 )
			uACP = StringToLong( acp_text );
	}
	::MultiByteToWideChar( uACP, 0, lpszReturn, nLength, SfxLangStrings[i].lpszUnicode, nLength+1 );
	
	return SfxLangStrings[i].lpszUnicode;
}

void FreeLanguageStrings()
{
	LANGSTRING * p = SfxLangStrings;
	while( p->id != 0 )
	{
		if( p->lpszUnicode != NULL )
		{
			delete p->lpszUnicode;
			p->lpszUnicode = NULL;
		}
		p++;
	}
}

class CLangStrings
{
public:
	~CLangStrings() { FreeLanguageStrings(); };
};

static CLangStrings __lsf;

#if defined(_SFX_USE_PREFIX_PLATFORM) || defined(_SFX_USE_CONFIG_PLATFORM)
	DWORD GetPlatform()
	{
		typedef void (WINAPI * GetNativeSystemInfo_Proc)( __out  LPSYSTEM_INFO lpSystemInfo );
		GetNativeSystemInfo_Proc pfnGetNativeSystemInfo;
		if( (pfnGetNativeSystemInfo = (GetNativeSystemInfo_Proc)::GetProcAddress( ::LoadLibraryA("kernel32"), "GetNativeSystemInfo" )) != NULL )
		{
			SYSTEM_INFO si;
			pfnGetNativeSystemInfo( &si );
			return si.wProcessorArchitecture;
		}
		return PROCESSOR_ARCHITECTURE_INTEL;
	}
	LPCWSTR GetPlatformName()
	{
		switch( GetPlatform() )
		{
		case PROCESSOR_ARCHITECTURE_AMD64:
			return L"x64";
		default:
			return L"x86";
		}
	}
#endif // defined(_SFX_USE_PREFIX_PLATFORM) || defined(_SFX_USE_CONFIG_PLATFORM)


#ifdef _SFX_USE_PREFIX_PLATFORM
	#if defined(_WIN64) && defined(_M_X64)
		/* exclude x86 (i386) prefixes */
		BOOL SfxPrepareExecute( int nPlatform )
		{
			switch( nPlatform )
			{
			case SFX_EXECUTE_PLATFORM_ANY:
			case SFX_EXECUTE_PLATFORM_AMD64:
				return TRUE;
			}
			return FALSE;
		}
	#elif defined(_WIN32) && defined(_M_IX86)
		typedef BOOL (WINAPI * Wow64DisableWow64FsRedirection_Proc)( __out  PVOID *OldValue );
		typedef BOOL (WINAPI * Wow64RevertWow64FsRedirection_Proc)( __in  PVOID OldValue );

		static BOOL SfxExecute_fFsRedirectionSwitched = FALSE;
		static PVOID SfxExecute_pFsRedirectionState = NULL;

		void SfxFinalizeExecute()
		{
			if( SfxExecute_fFsRedirectionSwitched != FALSE )
			{
				Wow64RevertWow64FsRedirection_Proc pfnWow64RevertWow64FsRedirection = 
					(Wow64RevertWow64FsRedirection_Proc)::GetProcAddress( ::LoadLibraryA("kernel32"), "Wow64RevertWow64FsRedirection" );
				if( pfnWow64RevertWow64FsRedirection != NULL )
				{
					pfnWow64RevertWow64FsRedirection( SfxExecute_pFsRedirectionState );
					SfxExecute_fFsRedirectionSwitched = FALSE;
				}
			}
		}

		void SfxDisableWow64FsRedirection()
		{
			SfxExecute_fFsRedirectionSwitched = FALSE;
			Wow64DisableWow64FsRedirection_Proc pfnWow64DisableWow64FsRedirection = 
				(Wow64DisableWow64FsRedirection_Proc)::GetProcAddress( ::LoadLibraryA("kernel32"), "Wow64DisableWow64FsRedirection" );
			if( pfnWow64DisableWow64FsRedirection != NULL )
			{
				pfnWow64DisableWow64FsRedirection( &SfxExecute_pFsRedirectionState );
				SfxExecute_fFsRedirectionSwitched = TRUE;
			}
		}

		BOOL SfxPrepareExecute( int nPlatform )
		{
			if( nPlatform == SFX_EXECUTE_PLATFORM_ANY )
				return TRUE;
			DWORD dwPlatform = GetPlatform();
			if( dwPlatform == PROCESSOR_ARCHITECTURE_AMD64 && nPlatform == SFX_EXECUTE_PLATFORM_AMD64 )
			{
				SfxDisableWow64FsRedirection();
				return TRUE;
			}
			if( dwPlatform == PROCESSOR_ARCHITECTURE_INTEL && nPlatform == SFX_EXECUTE_PLATFORM_I386 )
				return TRUE;
			return FALSE;
		}
	#endif

#endif // _SFX_USE_PREFIX_PLATFORM

#ifdef _SFX_USE_ELEVATION
	BOOL IsRunAsAdmin()
	{
		BOOL fIsRunAsAdmin = FALSE;
		PSID pAdministratorsGroup = NULL;

		// Allocate and initialize a SID of the administrators group.
		SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
		if( !AllocateAndInitializeSid( &NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
										0, 0, 0, 0, 0, 0, &pAdministratorsGroup) )
		{
			return FALSE;
		}

		// Determine whether the SID of administrators group is enabled in the primary access token of the process.
		CheckTokenMembership( NULL, pAdministratorsGroup, &fIsRunAsAdmin );

		FreeSid( pAdministratorsGroup );
		return fIsRunAsAdmin;
	}
#endif // _SFX_USE_ELEVATION

#if defined(_SFX_USE_CONFIG_PLATFORM) || defined(_SFX_USE_LANG)
void CreateConfigSignature(
#ifdef _SFX_USE_LANG
						   DWORD dwLangId,
#endif // _SFX_USE_LANG
#ifdef _SFX_USE_CONFIG_PLATFORM
						   LPCSTR lpszPlatformName,
#endif // _SFX_USE_CONFIG_PLATFORM
						   AString& strBegin, AString& strEnd )
{
	strBegin = kSignatureConfigStart;
	strEnd = kSignatureConfigEnd;
	strBegin = strBegin.Left( strBegin.Length()-1 );
	strEnd = strEnd.Left( strEnd.Length()-1 );

	CHAR Buf[100];

#ifdef _SFX_USE_CONFIG_PLATFORM
	if( lpszPlatformName != NULL && *lpszPlatformName != 0 )
	{
		wsprintfA( Buf, ":%hs", lpszPlatformName );
		strBegin += Buf;
		strEnd += Buf;
	}
#endif // _SFX_USE_CONFIG_PLATFORM

#ifdef _SFX_USE_LANG
	if( dwLangId != 0 )
	{
		wsprintfA( Buf, ":Language:%u", dwLangId );
		strBegin += Buf;
		strEnd += Buf;
	}
#endif // _SFX_USE_LANG

	strBegin += "!";
	strEnd += "!";
}

bool LoadConfigs( IInStream * inStream, AString& result )
{
#ifdef _SFX_USE_CONFIG_PLATFORM
	LPCSTR pPlatforms[3];
	#if defined(_WIN64) && defined(_M_X64)
		pPlatforms[0] = "x64";
		pPlatforms[1] = "amd64";
		pPlatforms[2] = NULL;
	#elif defined(_WIN32) && defined(_M_IX86)
		DWORD dwPlatform = GetPlatform();
		switch( dwPlatform )
		{
		case PROCESSOR_ARCHITECTURE_INTEL:
			pPlatforms[0] = "x86";
			pPlatforms[1] = "i386";
			pPlatforms[2] = NULL;
			break;
		case PROCESSOR_ARCHITECTURE_AMD64:
			pPlatforms[0] = "x64";
			pPlatforms[1] = "amd64";
			pPlatforms[2] = NULL;
			break;
		default:
			pPlatforms[0] = NULL;
		}
	#else
		#error "Unknown platform"
	#endif // Win32/Win64
#endif // _SFX_USE_CONFIG_PLATFORM

	AString sigBegin;
	AString sigEnd;
	AString config;
	bool	fResult = false;

	// Load default config
	CreateConfigSignature(
#ifdef _SFX_USE_LANG
		0,
#endif // _SFX_USE_LANG
#ifdef _SFX_USE_CONFIG_PLATFORM
		NULL,
#endif // _SFX_USE_CONFIG_PLATFORM
		sigBegin, sigEnd );
	if( ReadConfig( inStream, sigBegin, sigEnd, config ) != false )
		fResult = true;
	result = config;

#ifdef _SFX_USE_CONFIG_PLATFORM
	LPCSTR * platform = pPlatforms;
	while( *platform != NULL )
	{
		CreateConfigSignature(
#ifdef _SFX_USE_LANG
			0,
#endif // _SFX_USE_LANG
			*platform, sigBegin, sigEnd );
		if( ReadConfig( inStream, sigBegin, sigEnd, config ) != false )
		{
			if( result.Length() != 0 )
				result += "\r\n";
			result += config;
			fResult = true;
		}
		platform++;
	}
#endif // _SFX_USE_CONFIG_PLATFORM

#ifdef _SFX_USE_LANG
	// Selected language, all platforms
	CreateConfigSignature(
		idSfxLang,
	#ifdef _SFX_USE_CONFIG_PLATFORM
		NULL,
	#endif // _SFX_USE_CONFIG_PLATFORM
		sigBegin, sigEnd );
	if( ReadConfig( inStream, sigBegin, sigEnd, config ) != false )
		fResult = true;
	result += config;

	#ifdef _SFX_USE_CONFIG_PLATFORM
		platform = pPlatforms;
		while( *platform != NULL )
		{
			CreateConfigSignature( idSfxLang, *platform, sigBegin, sigEnd );
			if( ReadConfig( inStream, sigBegin, sigEnd, config ) != false )
			{
				if( result.Length() != 0 )
					result += "\r\n";
				result += config;
				fResult = true;
			}
			platform++;
		}
	#endif // _SFX_USE_CONFIG_PLATFORM
#endif // _SFX_USE_LANG

	return fResult;
}
#endif // defined(_SFX_USE_CONFIG_PLATFORM) || defined(_SFX_USE_LANG)

