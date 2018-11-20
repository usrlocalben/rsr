@echo off
%LOCALAPPDATA%\Programs\Python\Python36\python.exe -m pip install virtualenv
%LOCALAPPDATA%\Programs\Python\Python36\python.exe -m virtualenv wvenv
wvenv\scripts\pip install -r requirements.txt
echo ..\..\..\src\python > wvenv\lib\site-packages\rqdq.pth
