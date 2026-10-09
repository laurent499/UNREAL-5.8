@echo off
rem Lance TrailSimulator hors editeur (Standalone) avec le serveur Remote Control actif
rem pour piloter la meteo depuis RemoteControl\Meteo.html (port 30010).
rem -StartSRT demarre le flux SRT OWL (destination : SRTStreamURL du RaceManager, ou -SRTURL=srt://hote:port).
rem -BroadcastResY=720 : flux en 1280x720 (24,6 i/s contre 23,3 en 1080p, mesure du 2026-10-09).
rem -SRTAudio ajoute la piste audio (sortie audio Windows stereo obligatoire, sinon audio coupe).
rem Les arguments passes au .bat sont ajoutes a la ligne de commande (ex. : Lancer_Standalone.bat -SRTURL=srt://127.0.0.1:7029).
set UE="C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
set PROJ="%~dp0..\TrailSimulator.uproject"
start "" %UE% %PROJ% -game -windowed -resx=1280 -resy=720 -BroadcastResY=720 -RCWebControlEnable -StartSRT -SRTAudio -log %*
rem La page est aussi servie par Unreal : http://<IP de ce PC>:30010/meteo
timeout /t 20 /nobreak >nul
