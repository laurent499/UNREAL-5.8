@echo off
rem Package TrailSimulator (Win64 Development) dans ..\..\TrailSimulator_Package, hors depot git.
rem   Packager.bat        : compile + cook incremental (seuls les assets modifies) + pak
rem   Packager.bat code   : modif C++ seulement, pas de cook (reprend le contenu deja cuit), ~1-2 min
rem L'editeur peut rester ouvert : -nocompileeditor evite de recompiler ses DLL.
rem Le premier packaging est long (shaders de tous les materiaux) ; ensuite ils sont en cache.
rem Le jeu lit Meteo.html, Flore.json et BancTest.json dans <ProjectDir>\RemoteControl :
rem ils sont recopies a cote de l'executable package apres le packaging.
set UAT="C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat"
set PROJ=%~dp0..\TrailSimulator.uproject
set OUT=%~dp0..\..\TrailSimulator_Package
set COOK=-cook -iterate
if /i "%1"=="code" set COOK=-skipcook
call %UAT% BuildCookRun -project="%PROJ%" -platform=Win64 -clientconfig=Development -build %COOK% -stage -pak -archive -archivedirectory="%OUT%" -nocompileeditor -unattended -utf8output -nop4
if errorlevel 1 (
  echo Echec du packaging.
  pause
  exit /b 1
)
set DEST=%OUT%\Windows\TrailSimulator\RemoteControl
if not exist "%DEST%" mkdir "%DEST%"
copy /y "%~dp0Meteo.html" "%DEST%\" >nul
copy /y "%~dp0Flore.json" "%DEST%\" >nul
copy /y "%~dp0BancTest.json" "%DEST%\" >nul
echo Package pret : %OUT%\Windows\TrailSimulator.exe
pause
