@echo off

if not exist "Scripts\Windows\GetMSBuildPath.bat" goto Error_InvalidLocation

call "Scripts\Windows\GetMSBuildPath.bat"
if errorlevel 1 goto Error_NoVisualStudioEnvironment

if not exist "%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" goto Compile
for /f "delims=" %%i in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath') do (
	for %%j in (15.0, Current) do (
		if exist "%%i\MSBuild\%%j\Bin\MSBuild.exe" (
			set MSBUILD_PATH="%%i\MSBuild\%%j\Bin\MSBuild.exe"
			goto Compile
		)
	)
)

:Compile
md Cache\Intermediate >nul 2>nul
dir /s /b Janus.Build\*.cs >Cache\Intermediate\Janus.Build.Files.txt
fc /b Cache\Intermediate\Build\Flax.Build.Files.txt Cache\Intermediate\Build\Janus.Build.PrevFiles.txt >nul 2>nul
if not errorlevel 1 goto SkipClean
echo %MSBUILD_PATH%
copy /y Cache\Intermediate\Build\Janus.Build.Files.txt Cache\Intermediate\Build\Janus.Build.PrevFiles.txt >nul
%MSBUILD_PATH% /nologo /verbosity:quiet Janus.Build\Janus.Build.csproj /property:Configuration=Release /property:Platform=AnyCPU /target:Clean

:SkipClean
%MSBUILD_PATH% /nologo /verbosity:quiet Janus.Build\Janus.Build.csproj /property:Configuration=Release /property:Platform=AnyCPU /target:Build

if errorlevel 1 goto Error_CompilationFailed

Binaries\Janus.Build.exe %*

if errorlevel 1 goto Error_JanusBuildFailed
exit /B 0


:Error_InvalidLocation
echo CallBuildTool ERROR: The script is in invalid directory.
goto Exit
:Error_NoVisualStudioEnvironment
echo CallBuildTool ERROR: Missing Visual Studio 2015 or newer.
goto Exit
:Error_CompilationFailed
echo CallBuildTool ERROR: Failed to compile Janus.Build project.
goto Exit
:Error_JanusBuildFailed
echo CallBuildTool ERROR: Janus.Build tool failed.
goto Exit
:Exit
exit /B 1