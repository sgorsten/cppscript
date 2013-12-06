@ECHO OFF
DEL /F /Q scripts\%1.cpp > nul 2> nul
DEL /F /Q scripts\%1.dll > nul 2> nul
DEL /F /Q scripts\%1.pdb > nul 2> nul
RMDIR /S /Q scripts\%1 > nul 2> nul
MKDIR scripts\%1 > nul 2> nul