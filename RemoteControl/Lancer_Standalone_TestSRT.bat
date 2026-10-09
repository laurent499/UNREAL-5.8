@echo off
rem Test de cadence : lance le jeu comme Lancer_Standalone.bat, mais envoie le flux SRT
rem a un ffmpeg en ecoute sur ce PC (port 7029) au lieu du recepteur habituel.
call "%~dp0Lancer_Standalone.bat" -SRTURL=srt://127.0.0.1:7029
