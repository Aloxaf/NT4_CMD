@echo off

REM ///////////////////////////////////////////////////////////////////////////
REM  buildlocaltoolsshell.cmd
REM ///////////////////////////////////////////////////////////////////////////

pushd %~dp0
setlocal

REM //
REM // Verify that the script is not running under the sizzle environment.
REM //

if not [%NTROOT%] equ [] (
    echo You cannot run this script under the sizzle environment.
    echo Please run this script directly from the NT command line.
    goto End
)

REM //
REM // Identify the host processor architecture.
REM //

if not [%1] equ [] (
    set HostProcArch=%1
) else if [%PROCESSOR_ARCHITECTURE%] equ [x86] (
    set HostProcArch=x86
) else if [%PROCESSOR_ARCHITECTURE%] equ [AMD64] (
    set HostProcArch=amd64
) else (
    echo Host processor architecture %PROCESSOR_ARCHITECTURE% is not supported by buildlocaltools.cmd.
    goto Error
)

REM //
REM // Initialise the sizzle environment.
REM //

echo == Initialising the sizzle environment for building NTOSBE tools ... ==
set _NT_TARGET_VERSION=0x0502

call ntenv.cmd NTOSBE %~dp0src %~dp0src.bin %HostProcArch% chk

REM //
REM // Invoke NT shell.
REM //

cmd

REM //
REM // Restore environment.
REM //

endlocal
popd

exit /b 0
