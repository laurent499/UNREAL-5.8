@echo off
rem Test de perfs en 720p : comme Lancer_Standalone_TestSRT.bat (flux vers le ffmpeg local, port 7029),
rem mais fenetre 1280x720 et sans piste audio (memes conditions que les mesures 1080p de reference).
rem Le flux OWL capture le viewport : il passe donc aussi en 720p, et l'erreur d'ecran Cesium
rem (en pixels) charge moins de tuiles.
set UE="C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe"
set PROJ="%~dp0..\TrailSimulator.uproject"
start "" %UE% %PROJ% -game -windowed -resx=1280 -resy=720 -RCWebControlEnable -StartSRT -log -SRTURL=srt://127.0.0.1:7029 %*
