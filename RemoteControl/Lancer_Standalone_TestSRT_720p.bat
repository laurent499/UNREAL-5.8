@echo off
rem Test de perfs en 720p : comme Lancer_Standalone_TestSRT.bat (destination : -SRTURL),
rem mais en 1280x720.
rem -BroadcastResY=720 passe la camera de capture OWL (source du flux) en 1280x720 ; la fenetre en 720p
rem reduit aussi les tuiles Cesium (erreur d'ecran en pixels du viewport).
set UE="C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
set PROJ="%~dp0..\TrailSimulator.uproject"
start "" %UE% %PROJ% -game -windowed -resx=1280 -resy=720 -BroadcastResY=720 -RCWebControlEnable -StartSRT -NoSRTAudio -log -SRTURL=srt://192.168.88.129:7029 %*
