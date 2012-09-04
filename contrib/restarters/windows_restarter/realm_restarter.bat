@ECHO OFF
@title SkyFire One
CLS
ECHO Initializing Realm (Logon-Server)...
:1
start "SkyFire Realm Server" /B /MIN /WAIT authserver.exe -c authserver.conf
if %errorlevel% == 0 goto end
goto 1
:end