@echo off
rem Lance le jeu PACKAGE (TrailSimulator.exe) avec les memes options que Lancer_Standalone.bat :
rem fenetre 1280x720, flux SRT OWL en 720p avec audio, serveur Remote Control actif (port 30010).
rem Le package est produit par BuildCookRun dans ..\..\TrailSimulator_Package (hors depot git).
rem Pas de -DisablePython : le jeu package ne charge pas Python.
rem -ini:Engine:... : ramasse-miettes toutes les 5 min au lieu de toutes les 60 s (comme en Standalone).
rem Les arguments passes au .bat sont ajoutes a la ligne de commande (ex. : Lancer_Package.bat -SRTURL=srt://127.0.0.1:7029).
set EXE="%~dp0..\..\TrailSimulator_Package\Windows\TrailSimulator.exe"
if not exist %EXE% (
  echo Jeu package introuvable : %EXE%
  pause
  exit /b 1
)
start "" %EXE% -windowed -resx=1280 -resy=720 -BroadcastResY=720 -RCWebControlEnable -ini:Engine:[ConsoleVariables]:gc.TimeBetweenPurgingPendingKillObjects=300 -StartSRT -SRTAudio -log %*
rem La page meteo est servie par le jeu : http://<IP de ce PC>:30010/meteo
timeout /t 20 /nobreak >nul
