@echo off
setlocal
cd /d "%~dp0EasyAntiCheat"
echo [EAC] Installing Easy Anti-Cheat service...
EasyAntiCheat_EOS_Setup.exe uninstall a91729130eb949eaa22ef2e3bd7ea5cb
echo [EAC] Installation complete.
exit
