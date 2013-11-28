@ECHO OFF

SET CFLAGS=/GS /W3 /Zc:wchar_t /Fd"scripts\%1\vc120.pdb" /fp:precise /D "WIN32" /D "_WINDOWS" /D "_USRDLL" /D "_WINDLL" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /Gd /nologo /EHsc /Fo"scripts\%1\script.obj" /Fp"scripts\%1\script.pch" scripts\%1\script.cpp
SET CFLAGS_D=/D "_DEBUG" /RTC1 /Gm /Od /MDd
SET CFLAGS_R=/D "NDEBUG" /GL /Gy /Zi /Gm- /O2 /Oi /MD

SET LFLAGS=/OUT:"scripts\%1\script.dll" /MANIFEST /NXCOMPAT /PDB:"scripts\%1\script.pdb" /DYNAMICBASE /IMPLIB:"scripts\%1\script.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" /DEBUG /DLL /SUBSYSTEM:WINDOWS /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /ManifestFile:"scripts\%1\script.dll.intermediate.manifest" /ERRORREPORT:PROMPT /NOLOGO /TLBID:1
SET LFLAGS_D=/INCREMENTAL
SET LFLAGS_R=/LTCG /OPT:REF /INCREMENTAL:NO /OPT:ICF

IF "%3" == "x86" CALL "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86 > nul 2> nul
IF "%3" == "x64" CALL "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x86_amd64 > nul 2> nul

IF "%2" == "DEBUG" IF "%3" == "x86" cl %CFLAGS% %CFLAGS_D% /analyze- /ZI /Oy- /link %LFLAGS% %LFLAGS_D% /MACHINE:X86
IF "%2" == "DEBUG" IF "%3" == "x64" cl %CFLAGS% %CFLAGS_D% /Zi /link %LFLAGS% %LFLAGS_D% /MACHINE:X64
IF "%2" == "RELEASE" IF "%3" == "x86" cl %CFLAGS% %CFLAGS_R% /analyze- /Oy- /link %LFLAGS% %LFLAGS_R% /MACHINE:X86 /SAFESEH
IF "%2" == "RELEASE" IF "%3" == "x64" cl %CFLAGS% %CFLAGS_R% /link %LFLAGS% %LFLAGS_R% /MACHINE:X64 