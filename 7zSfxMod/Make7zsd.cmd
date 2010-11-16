@echo off
setlocal enabledelayedexpansion
setlocal enableextensions

set DEVELOP_ROOT=C:\Mobile\Develop
set ENV_CPU=x86
set COMPILER=VC8

set TARGET_CPU=
set MODULE_SUBNAMES=

:Parse_Args

IF /I "%~1"=="/x86"        SET "TARGET_CPU=%TARGET_CPU% x86"           & SHIFT & GOTO Parse_Args
IF /I "%~1"=="/I386"       SET "TARGET_CPU=%TARGET_CPU% x86"           & SHIFT & GOTO Parse_Args
IF /I "%~1"=="/x64"        SET "TARGET_CPU=%TARGET_CPU% x64"           & SHIFT & GOTO Parse_Args
IF /I "%~1"=="/amd64"      SET "TARGET_CPU=%TARGET_CPU% x64"           & SHIFT & GOTO Parse_Args

IF /I "%~1"=="/deflate"    SET "MODULE_SUBNAMES=%MODULE_SUBNAMES% Deflate"   & SHIFT & GOTO Parse_Args
IF /I "%~1"=="/lzma"       SET "MODULE_SUBNAMES=%MODULE_SUBNAMES% LZMA"      & SHIFT & GOTO Parse_Args
IF /I "%~1"=="/lzma2"      SET "MODULE_SUBNAMES=%MODULE_SUBNAMES% LZMA2"     & SHIFT & GOTO Parse_Args
IF /I "%~1"=="/ppmd"       SET "MODULE_SUBNAMES=%MODULE_SUBNAMES% PPMd"      & SHIFT & GOTO Parse_Args
IF /I "%~1"=="/All"        SET "MODULE_SUBNAMES=%MODULE_SUBNAMES% All"      & SHIFT & GOTO Parse_Args
IF /I "%~1"=="/Dialogs"    SET "MODULE_SUBNAMES=%MODULE_SUBNAMES% LZMA_Dialogs"  & SHIFT & GOTO Parse_Args

IF /I "%~1"=="/size"       SET "OPTIMIZATION=METHODS_OPTIMIZATION=-O1"      & SHIFT & GOTO Parse_Args
IF /I "%~1"=="/speed"      SET "OPTIMIZATION=METHODS_OPTIMIZATION=-O2"      & SHIFT & GOTO Parse_Args
IF /I "%~1"=="/mapfile"	   SET "_MAPFILE=_MAPFILE=1" 			    & SHIFT & GOTO Parse_Args
IF /I "%~1"=="/crypto"	   SET "SFX_CRYPTO=SFX_CRYPTO=1" 		    & SHIFT & GOTO Parse_Args

IF /I "%~1"=="-h"          GOTO Error_Usage
IF /I "%~1"=="/?"          GOTO Error_Usage
IF /I "x%~1" == "x"	   GOTO process_make
SET NMAKE_CMDS=%NMAKE_CMDS% %1
SHIFT
goto Parse_args

:process_make
IF "x%TARGET_CPU%" == "x" SET TARGET_CPU=x86 x64
IF "x%MODULE_SUBNAMES%" == "x" SET MODULE_SUBNAMES=Deflate LZMA LZMA2 PPMd All LZMA_Dialogs

FOR %%c IN (%TARGET_CPU%) DO (
	FOR %%m IN (%MODULE_SUBNAMES%) DO (
		IF "%%m" == "All" SET "COMPRESSION_METHODS=COMPRESS_COPY=1 COMPRESS_BCJ=1 COMPRESS_BCJ2=1 COMPRESS_LZMA=1 COMPRESS_LZMA2=1 COMPRESS_DEFLATE=1 COMPRESS_PPMD=1"
		IF "%%m" == "Deflate" SET "COMPRESSION_METHODS=COMPRESS_COPY=1 COMPRESS_DEFLATE=1"
		IF "%%m" == "LZMA" SET "COMPRESSION_METHODS=COMPRESS_COPY=1 COMPRESS_BCJ=1 COMPRESS_BCJ2=1 COMPRESS_LZMA=1"
		IF "%%m" == "LZMA2" SET "COMPRESSION_METHODS=COMPRESS_COPY=1 COMPRESS_BCJ=1 COMPRESS_BCJ2=1 COMPRESS_LZMA=1 COMPRESS_LZMA2=1"
		IF "%%m" == "PPMd" SET "COMPRESSION_METHODS=COMPRESS_COPY=1 COMPRESS_PPMD=1"
		IF "%%m" == "LZMA_Dialogs" SET "COMPRESSION_METHODS=COMPRESS_COPY=1 COMPRESS_BCJ=1 COMPRESS_BCJ2=1 COMPRESS_LZMA=1 _SFXTOOLS=1"
		echo ===============================
		echo %%m-%%c
		echo ===============================

		PATH=%DEVELOP_ROOT%\Bin.%ENV_CPU%\%COMPILER%\%%c
		PATH=!PATH!;%DEVELOP_ROOT%\Bin.%ENV_CPU%\Common\%%c
		PATH=!PATH!;%SystemRoot%;%SystemRoot%\system32

		set Include=%DEVELOP_ROOT%\Include\SDK.v7
		set Include=!Include!;%DEVELOP_ROOT%\Include\%COMPILER%
		set Include=!Include!;%DEVELOP_ROOT%\Include\Common

		set Lib=%DEVELOP_ROOT%\Lib\SDK.v7\%%c
		set Lib=!Lib!;%DEVELOP_ROOT%\Lib\%COMPILER%\%%c

		nmake -nologo _SFX_MAINTAINER=1 CPU=%%c MODULE_SUBNAME=%%m !COMPRESSION_METHODS! %OPTIMIZATION% %_MAPFILE% %SFX_CRYPTO% %NMAKE_CMDS%
	)
)

exit 0

:Error_Usage
ECHO Usage: "Make7zsd [/x86|/x64] [/deflate|/lzma|/lzma2|/ppmd|/all] [/size|/speed] [-h | /?] [make_options]"
ECHO.
ECHO                 /x86     - Create 32-bit x86 applications
ECHO                 /x64     - Create 64-bit x64 applications
