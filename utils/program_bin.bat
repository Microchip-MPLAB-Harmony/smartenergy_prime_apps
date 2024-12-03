@echo off
echo JLink script launcher for pic32cx2051mtg binaries
echo.
"C:/Program Files/SEGGER/JLink/JLink.exe" -device pic32cx2051mtg -JTAGConf -1,-1 -CommanderScript flasher_bin.jlink >j_link.log
IF ERRORLEVEL 2 GOTO LabelError
GOTO LabelEnd
:LabelError
echo.
echo IMPORTANT: Edit the path to JLink.exe according to your installation folder
echo.
:LabelEnd
echo.
echo JLink script launcher done.
echo.
