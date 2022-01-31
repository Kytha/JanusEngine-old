@echo off

setlocal
pushd
echo Generating Janus Engine project files...

call "Scripts\Windows\CallBuildTool.bat" -genproject %*
if errorlevel 1 goto BuildToolFailed

popd
echo Finished!
pause
exit /B 0

:BuildToolFailed
echo Janus.Build tool failed.
pause
goto Exit

:Exit
popd
exit /B 1