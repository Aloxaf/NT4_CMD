@echo off

set NTDBGFILES=1

REM //
REM // Set debug information
REM //

set NTDEBUG=ntsd
set NTDEBUGTYPE=windbg

REM //
REM // Set optimisation settings
REM //

set MSC_OPTIMIZATION=/Od

REM //
REM // Set environment shell colour
REM // Background = Red, Foreground = White
REM //

color 4F
