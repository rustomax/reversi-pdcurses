# Basic Windows Reversi game in C using PDCurses library

## Compile and run

```sh
git clone https://github.com/rustomax/reversi-pdcurses.git
cd reversi-pdcurses\PDCurses\win32a
<MinGW_root>\bin\mingw32-make -f mingwin32.mak
gcc -O4 -Wall -pedantic -I.. -mwindows -o../../reversi/reversi.exe ../../reversi/main.c pdcurses.a -lgdi32 -lcomdlg32 -std=c11
..\..\reversi\reversi
```

## Notes

- This program uses the Project Pluto's version of PDCurses. The main site is https://www.projectpluto.com/win32a.htm. GitHub repo https://github.com/Bill-Gray/PDCurses
- This program is designed to run on Windows, but since all IO is done through curses interface, it should be relatively easy to port to other platforms. If you are in the mood for that feel free to send me a pull request.
- This program is licensed under GPL. PDCurses uses public domain license. See LICENSE files in corresponding directories.
- This repo is streamlined for MinGW compiler on Windows with a lot of directories in PDCurses distribution removed. If you want to compile with different compilers for other targets, make sure to clone the full PDCurses repo from locations mentioned above.

## Screenshot
![Reversi for Windows Cosole](screenshot.png?raw=true)
