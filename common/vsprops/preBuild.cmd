::@echo off
:: This file GENERATES the automatic GIT revision/version tag.
:: It uses the git.exe program to create an "svnrev.h" file for whichever
:: project is being compiled, during the project's pre-build step.
::
:: The git.exe program is part of the msysgit installation.
::
:: MsysGit can be downloaded from http://msysgit.github.io/
:: 
:: Usage: preBuild.cmd ProjectSrcDir VspropsDir
::
::    ProjectSrcDir - $(ProjectDir)\.. - Top-level Directory of project source code.

SETLOCAL ENABLEDELAYEDEXPANSION ENABLEEXTENSIONS

IF EXIST "%ProgramFiles(x86)%\Git\bin\git.exe" SET "GITPATH=%ProgramFiles(x86)%\Git\bin"
IF EXIST "%ProgramFiles%\Git\bin\git.exe" SET "GITPATH=%ProgramFiles%\Git\bin"
IF EXIST "%ProgramW6432%\Git\bin\git.exe" SET "GITPATH=%ProgramW6432%\Git\bin"
IF DEFINED GITPATH SET "PATH=%PATH%;%GITPATH%"

FOR /F "tokens=* USEBACKQ" %%i IN (`git rev-parse --short HEAD`) DO (
  set GIT_HASH=%%i
echo #define GIT_HASH "%%i" >> "%CD%\svnrev.h"
)
 
ENDLOCAL
:: Always return an errorlevel of 0 -- this allows compilation to continue if SubWCRev failed.
exit /B 0