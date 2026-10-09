@echo off
rem Test audio : flux vers ffmpeg local (port 7029) avec la piste audio OWL activee (-SRTAudio).
rem Argument optionnel : format audio OWL (71, Downmix ou Stereo). Ex. : Lancer_Standalone_TestSRT_Audio.bat 71
set LAYOUT=%1
if "%LAYOUT%"=="" set LAYOUT=Stereo
call "%~dp0Lancer_Standalone.bat" -SRTURL=srt://192.168.88.129:7029 -SRTAudio -SRTAudioLayout=%LAYOUT%
