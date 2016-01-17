@ECHO OFF
cls
ECHO =======TSPServer Begin====================================================
cd TSPServer > NUL

if exist objfre_wxp_x86\i386\ (
del /Q /F objfre_wxp_x86\i386\*.* > NUL
)

build
cd..  > NUL

rem tools\upx.exe -9 -q TSPServer\objfre_wxp_x86\i386\TSPServer.dll > NUL
tools\7z e TSPServer\objfre_wxp_x86\i386\TSPServer.dll TSPLoader\TSPServer.dll > NUL
tools\7z e MyLoad\Release\MyLoad.dll TSPLoader\MyLoad.dll > NUL

tools\7z e TSPServer\objfre_wxp_x86\i386\TSPServer.dll TSPLoader2\TSPServer.dll > NUL
tools\7z e MyLoad\Release\MyLoad.dll TSPLoader2\SbieDLL.dll > NUL

rem copy /y TSPServer\objfre_wxp_x86\i386\TSPServer.dll TSPLoader\TSPServer.dll  > NUL
ECHO =======TSPServer End======================================================


ECHO =======TSPLoader Begin======================================================
cd TSPLoader  > NUL

if exist objfre_wxp_x86\i386\ (
del /Q /F objfre_wxp_x86\i386\*.* > NUL
)

build
cd..  > NUL

if not exist Release\ (
md Release > NUL
)

copy /y TSPLoader\objfre_wxp_x86\i386\TSPLoader.exe Release\server.dat  > NUL
rem tools\upx.exe -9 -q Release\server.dat > NUL

if not exist Debug\ (
md Debug > NUL
)

copy /y TSPLoader\objfre_wxp_x86\i386\TSPLoader.exe Debug\server.dat  > NUL
rem tools\upx.exe -9 -q Debug\server.dat > NUL
ECHO =======TSPLoader End========================================================


ECHO =======TSPLoader2 Begin======================================================
cd TSPLoader2  > NUL

rem DataToText.exe TSPServer.dll server.h

if exist objfre_wxp_x86\i386\ (
del /Q /F objfre_wxp_x86\i386\*.* > NUL
)

build
cd..  > NUL

if not exist Release\ (
md Release > NUL
)

copy /y TSPLoader2\objfre_wxp_x86\i386\TSPLoader2.exe Release\linyiserver.dat  > NUL
rem tools\upx.exe -9 -q Release\server.dat > NUL

if not exist Debug\ (
md Debug > NUL
)

copy /y TSPLoader2\objfre_wxp_x86\i386\TSPLoader2.exe Debug\linyiserver.dat  > NUL
rem tools\upx.exe -9 -q Debug\server.dat > NUL
ECHO =======TSPLoader2 End========================================================