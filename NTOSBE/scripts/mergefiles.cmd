@echo off
if "%3" == "" goto Usage
call perl %BEROOT%\scripts\MergeFiles.pl %1 %2 %3
goto end

:Usage
echo.
echo Usage: MergeFiles.cmd file1 file2 outfile
echo.

:end
