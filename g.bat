@echo off

rem "C:\MinGW700x\bin\g++.exe" -O9 -s -ILib -fwhole-program -fno-exceptions -fno-rtti -static stegdict.cpp -o stegdict

"C:\MinGW700\bin\g++.exe" -m32 -O9 -s -ILib -fwhole-program -fno-exceptions -fno-rtti -static stegdict.cpp -o stegdict
