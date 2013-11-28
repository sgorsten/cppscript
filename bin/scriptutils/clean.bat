@ECHO OFF
RMDIR /S /Q scripts\%1 > nul 2> nul
MKDIR scripts\%1 > nul 2> nul