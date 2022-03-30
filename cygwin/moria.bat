@echo off
set HOME=%cd%
set TERM=pcansi
set TERMINFO=.\terminfo
set PROMPT=Enter 'moria' to play again, or 'exit' (or close the window) to quit^> 
echo Starting Moria...
echo Don't forget to activate Num Lock if you haven't already!
echo Save data will be written to %HOME%\moria-save by default.
pause
start cmd /c moria.exe
