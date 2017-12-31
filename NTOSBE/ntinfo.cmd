@echo off

REM ///////////////////////////////////////////////////////////////////////////
REM  ntinfo.cmd
REM ///////////////////////////////////////////////////////////////////////////

REM //
REM // Verify that the script is running under the sizzle environment.
REM //

if [%NTROOT%] equ [] (
    echo You must run this script under the sizzle environment.
    goto End
)

REM //
REM // Display the environment information.
REM //

echo Repository Name        (NTREPO)        = %NTREPO%
echo Source Path            (NTROOT)        = %NTROOT%
echo Target Architecture    (NTARCH)        = %NTARCH%
echo Target Build           (NTBLD)         = %NTBLD%
echo Target                 (NTTARGET)      = %NTTARGET%
echo NTOSBE Root            (BEROOT)        = %BEROOT%
echo NT Binary Tree         (NTTREE)        = %NTTREE%

:End
