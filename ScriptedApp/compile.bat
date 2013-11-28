@ECHO OFF

SET CFLAGS=/GS /W3 /Zc:wchar_t /Fd"scripts\%1\vc120.pdb" /fp:precise /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "_WINDLL" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /Gd /nologo /EHsc /Fo"scripts\%1\script.obj" /Fp"scripts\%1\script.pch" scripts\%1\script.cpp

SET LFLAGS=/OUT:"scripts\%1\script.dll" /MANIFEST /NXCOMPAT /PDB:"scripts\%1\script.pdb" /DYNAMICBASE /IMPLIB:"scripts\%1\script.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" /DEBUG /DLL /SUBSYSTEM:WINDOWS /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /ManifestFile:"scripts\%1\script.dll.intermediate.manifest" /ERRORREPORT:PROMPT /NOLOGO /TLBID:1

IF "%2" == "DEBUG" (
SET CFLAGS=%CFLAGS% /D "_DEBUG" /RTC1 /Gm /Od /MDd
SET LFLAGS=%LFLAGS% /INCREMENTAL
IF "%3" == "x86" SET CFLAGS=%CFLAGS% /ZI
IF "%3" == "x64" SET CFLAGS=%CFLAGS% /Zi
)

IF "%2" == "RELEASE" (
SET CFLAGS=%CFLAGS% /D "NDEBUG" /GL /Gy /Zi /Gm- /O2 /Oi /MD
SET LFLAGS=%LFLAGS% /LTCG /OPT:REF /INCREMENTAL:NO /OPT:ICF
)

IF "%3" == "x86" (
SET CFLAGS=%CFLAGS% /analyze- /Oy-
SET LFLAGS=%LFLAGS% /MACHINE:X86
IF "%2" == "RELEASE" SET LFLAGS=%LFLAGS% /SAFESEH
CALL "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86 > nul 2> nul
)

IF "%3" == "x64" (
SET LFLAGS=%LFLAGS% /MACHINE:X64
CALL "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86_amd64 > nul 2> nul
)

cl %CFLAGS% /link %LFLAGS%