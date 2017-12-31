@echo off

REM ///////////////////////////////////////////////////////////////////////////
REM  ntswitch.cmd
REM ///////////////////////////////////////////////////////////////////////////

REM //
REM // Verify that the script is running under the sizzle environment.
REM //

if [%NTROOT%] equ [] (
    echo You must run this script under the sizzle environment.
    goto End
)

REM //
REM // Verify arguments.
REM //

if [%1] equ [] (
    echo You must specify build arch.
    goto End
)

if [%2] equ [] (
    echo You must specify build type.
    goto End
)

REM //
REM // Re-initialise the sizzle environment.
REM //

call ntenv.cmd %NTREPO% %NTROOT% %NTBINROOT% %1 %2

REM //
REM // Set window and prompt style.
REM //

title NTOSBE: %NTTARGET% [%NTROOT%]
prompt [%COMPUTERNAME%: %USERNAME% // %NTTARGET%] $d$s$t$_$p$_$_$+$g

echo.

:End
