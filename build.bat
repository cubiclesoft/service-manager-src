@echo off
cls

if exist update.bat (
  call update.bat
)

if not exist convert_int.obj (
  cl /Ox /c convert\*.cpp
)

if not exist environment_appinfo.obj (
  cl /Ox /c environment\*.cpp
)

if not exist utf8_file_dir.obj (
  cl /Ox /c utf8\*.cpp
)

cl /Ox servicemanager.cpp /link /FILEALIGN:512 /OPT:REF /OPT:ICF /INCREMENTAL:NO shell32.lib advapi32.lib convert_int.obj environment_appinfo.obj utf8_util.obj utf8_appinfo.obj utf8_file_dir.obj /out:servicemanager.exe
