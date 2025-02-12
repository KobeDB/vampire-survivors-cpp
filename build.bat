set SRC_FILES=main.cpp resources.cpp

::cl /EHsc /Zi /Od %SRC_FILES% /I"C:\raylib\include" /MD /link /LIBPATH:"C:\raylib\lib" "C:\raylib\lib\raylib.lib" opengl32.lib kernel32.lib user32.lib shell32.lib gdi32.lib winmm.lib msvcrt.lib

cl /EHsc /O2 %SRC_FILES% /I"C:\raylib\include" /MD /link /LIBPATH:"C:\raylib\lib" "C:\raylib\lib\raylib.lib" opengl32.lib kernel32.lib user32.lib shell32.lib gdi32.lib winmm.lib msvcrt.lib