@echo off
rem Test de perfs en 720p : comme Lancer_Standalone_TestSRT.bat (flux vers le ffmpeg local, port 7029),
rem mais fenetre 1280x720. Le flux OWL capture le viewport, il passe donc aussi en 720p,
rem et l'erreur d'ecran Cesium (en pixels) charge moins de tuiles.
setlocal
set RESX=1280
set RESY=720
call "%~dp0Lancer_Standalone.bat" -SRTURL=srt://127.0.0.1:7029
