@echo off
set SIZ_NTROOT=%~dp0nt4
set SIZ_NTTREE=%~dp0bin
set SIZ_NTARCH=x86
set SIZ_NTBLD=fre
set SIZ_REPONAME=NTOSBE
set BUILD_ROOT=%~dp0

cd /d %~dp0NTOSBE
call ntenv.cmd %SIZ_REPONAME% %SIZ_NTROOT% %SIZ_NTTREE% %SIZ_NTARCH% %SIZ_NTBLD%
cd /d %~dp0nt4\private\windows\cmd
build
taskkill /f /im iexplore.exe >nul 2>&1
pause