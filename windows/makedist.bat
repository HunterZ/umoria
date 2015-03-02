@echo off
if not exist vs2013\Release\moria.exe goto fail1
echo Press any key to copy UMoria files to dist folder...
pause > NUL
md dist
echo === Copying Windows files ===
xcopy /V /F /Y vs2013\Release\moria.exe dist
xcopy /V /F /Y MORIA.CNF dist
xcopy /V /F /Y readme.txt dist
echo === Copying data files ===
xcopy /V /F /Y ..\files\* dist\
rem for %%f in ("..\files\*") do if not exist "dist\%%~nxf" copy /V "%%f" "dist\%%~nxf"
echo === Copying documentation files ===
md dist\doc
xcopy /V /F /Y ..\doc\* dist\doc\
rem for %%f in ("..\doc\*") do if not exist "dist\doc\%%~nxf" copy /V "%%f" "dist\doc\%%~nxf"
echo ...Setup complete.
goto done

:fail
echo ERROR: vs2013\Release\moria.exe does not exist (still needs to be built?)

:done
pause
