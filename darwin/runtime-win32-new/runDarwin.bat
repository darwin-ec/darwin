@echo off
rem Batch file for DARWIN version 1.0 and later
rem
rem Creates essential environment variables for MSWindows and
rem runs the executable
rem
rem 11/18/2005 -- First Beta Release Version 1.0 -- JHS

set DARWINHOME=%CD%
cd %DARWINHOME%\system\bin
.\darwin.exe
cd %DARWINHOME%
