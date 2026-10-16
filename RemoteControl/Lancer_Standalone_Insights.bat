@echo off
rem Mesure de perfs Unreal Insights : comme Lancer_Standalone.bat (720p, SRT, Remote Control),
rem avec en plus un enregistrement de trace (CPU par thread, frames, GPU, compteurs).
rem La trace est ecrite dans Saved\Profiling\<date>_<heure>_<id>.utrace et se termine a la fermeture du jeu.
rem Jouer normalement 3 a 5 minutes (camera, meteo, coureurs), puis fermer le jeu.
rem -statnamedevents detaille les scopes (acteurs, timers, Blueprints) ; il ajoute un peu de surcout.
call "%~dp0Lancer_Standalone.bat" -trace=default,counters,stats -statnamedevents -tracefile %*
