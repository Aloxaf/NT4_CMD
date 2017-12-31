@echo off

REM ///////////////////////////////////////////////////////////////////////////
REM  ntenv.cmd
REM ///////////////////////////////////////////////////////////////////////////

pushd %~dp0

REM //
REM // Validate parameters.
REM //

if [%1] equ []      goto Usage
if [%2] equ []      goto Usage
if [%3] equ []      goto Usage
if [%4] equ []      goto Usage
if [%5] equ []      goto Usage
if not [%6] equ []  goto Usage

set _RepoName=%1
set _NtRootPath=%2
set _BinRootPath=%3
set _BuildArch=%4
set _BuildType=%5

REM // Validate that NtRootPath exists.
if not exist "%_NtRootPath%" (
    echo Invalid NtRootPath.
    goto End
)

REM // Validate BuildArch.
if not [%_BuildArch%] equ [x86] (
if not [%_BuildArch%] equ [amd64] (
if not [%_BuildArch%] equ [arm] (
if not [%_BuildArch%] equ [mips] (
if not [%_BuildArch%] equ [alpha] (
if not [%_BuildArch%] equ [ppc] (
    echo Invalid BuildArch.
    goto End
))))))

REM // Validate BuildType.
if not [%_BuildType%] equ [chk] (
if not [%_BuildType%] equ [fre] (
    echo Invalid BuildType.
    goto End
))

REM //
REM // Set build environment root path variable.
REM //

set BEROOT=%~dp0
set BEROOT=%BEROOT:~0,-1%

REM //
REM // Set global environment variables.
REM //

set NTREPO=%_RepoName%
set NTROOT=%_NtRootPath%
set NTARCH=%_BuildArch%
set NTBLD=%_BuildType%
set NTTARGET=%NTARCH%%NTBLD%
set NTBINROOT=%_BinRootPath%
set NTTREE=%NTBINROOT%\%NTTARGET%
set NT%NTARCH%TREE=%NTTREE%

set VERIFY_SOURCES=1
set BUILD_PRODUCT=NT
set BUILD_DEFAULT=-D
set BUILD_DEFAULT_TARGETS=-%NTARCH%
set BUILD_MULTIPROCESSOR=1
set BUILD_ALT_DIR=%NTBLD%

REM // Use the repository configuration files whenever possible.
if exist "%NTROOT%\placefil.txt" (
    set BINPLACE_PLACEFILE=%NTROOT%\placefil.txt
) else (
    set BINPLACE_PLACEFILE=%BEROOT%\placefil.txt
)

if exist "%NTROOT%\coffbase.txt" (
    set COFFBASE_TXT_FILE=%NTROOT%\coffbase.txt
) else (
    set COFFBASE_TXT_FILE=%BEROOT%\coffbase.txt
)

REM //
REM // Initialise target profile.
REM //

call profiles\target\%NTTARGET%\init.cmd

REM //
REM // Initialise user profile. This is done only when the profile for the
REM // active user exists under profiles\user.
REM //

if exist "profiles\user\%COMPUTERNAME%\%USERNAME%" (
    call "profiles\user\%COMPUTERNAME%\%USERNAME%\init.cmd"
)

REM //
REM // Print environment configuration.
REM //

call ntinfo.cmd

REM //
REM // Store the original global path variable content if this is the first
REM // run. This is critical to make ntswitch.cmd work.
REM //

if "%DEFAULTPATH%"=="" (
    set "DEFAULTPATH=%PATH%"
) else (
    set "PATH=%DEFAULTPATH%"
)

REM //
REM // Set global path
REM //

path %PATH%;%BEROOT%;%BEROOT%\scripts

REM //
REM // Set tool paths. If PROCESSOR_ARCHITECTURE matches NTARCH, use native
REM // build environment. If not, use a cross build environment.
REM //

if /i [%PROCESSOR_ARCHITECTURE%] equ [%NTARCH%] (
REM // Native build environment
    set BETOOLS=%BEROOT%\tools\%NTARCH%
) else (
REM // Cross build environment
    set BETOOLS=%BEROOT%\tools\%PROCESSOR_ARCHITECTURE%_%NTARCH%
)

REM //
REM // Use x86 native build environment if the host processor architecture
REM // is AMD64.
REM //

if [%PROCESSOR_ARCHITECTURE%] equ [AMD64] (
    if not [%NTARCH%] equ [amd64] (
        if [%NTARCH%] equ [x86] (
            REM // Building for x86 from AMD64 host: use x86 native tools.
            set BETOOLS=%BEROOT%\tools\%NTARCH%
        ) else (
            REM // Building for non-x86 from AMD64 host: use x86 cross tools.
            set BETOOLS=%BEROOT%\tools\x86_%NTARCH%
        )
    )
)

if not exist "%BETOOLS%" (
    echo.
    echo == Unsupported build configuration ==
    echo Host: %PROCESSOR_ARCHITECTURE%
    echo Target: %NTARCH%
    goto End
)

set BEMSTOOLS=%BETOOLS%\mstools
set BEIDW=%BETOOLS%\idw
set BEREPOIDW=%BETOOLS%\idw\%_RepoName%
set BEPERL=%BETOOLS%\perl\bin

REM //
REM // Check if there exists a repository-specific IDW tools directory.
REM //

if not [%_RepoName%] equ [NTOSBE] (
if not exist "%BEREPOIDW%" (
    echo.
    echo WARNING: IDW tools not found for the provided repository name.
    echo          You must build the IDW tools with buildrepoidw.cmd before attempting
    echo          to build the repository.
))

REM //
REM // Use x86 Perl interpreter if the host processor architecture is
REM // AMD64.
REM //

if [%PROCESSOR_ARCHITECTURE%] equ [AMD64] (
    set BEPERL=%BEROOT%\tools\x86\perl\bin
)

path %PATH%;%BETOOLS%;%BEREPOIDW%;%BEIDW%;%BEMSTOOLS%;%BEPERL%

REM //
REM // Set tools16 path if the host processor architecture is x86. 16-bit
REM // application build is only available from 32-bit x86 build host.
REM //

if [%PROCESSOR_ARCHITECTURE%] equ [x86]    set TOOLS16=1
if [%PROCESSOR_ARCHITECTURE%] equ [AMD64]  set TOOLS16=1
if [%TOOLS16%] equ [1]                     path %PATH%;%BETOOLS%\tools16

REM //
REM // Load public and private command line shortcut aliases
REM //

if exist "%NTROOT%\cue.pub" (
    alias -f "%NTROOT%\cue.pub"
)

if exist "profiles\user\%COMPUTERNAME%\%USERNAME%\cue.pri" (
    alias -f "profiles\user\%COMPUTERNAME%\%USERNAME%\cue.pri"
)

REM //
REM // Clear the INCLUDE and LIB environment variables since the NTOS build
REM // environment always uses the full path to refer includes and libraries.
REM //

set INCLUDE=
set LIB=

REM //
REM // Set Razzle environment compatibility variables
REM //

set NTMAKEENV=%BEROOT%

goto End

REM //
REM // Print usage
REM //

:Usage

    echo ntenv.cmd RepoName NtRootPath BinRootPath BuildArch BuildType
    echo.
    echo Parameters:
    echo  RepoName    = Repository name
    echo  NtRootPath  = NT source tree path
    echo  BinRootPath = Binary output tree path
    echo  BuildArch   = { x86, amd64, arm, mips, alpha, ppc }
    echo  BuildType   = { fre, chk }
    echo.
    echo Example:
    echo  ntenv.cmd NT W:\nt W:\bin x86 fre

REM //
REM // End of ntenv script. Clean up all local environment configurations.
REM //

:End

popd
