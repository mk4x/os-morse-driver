@echo off
echo ==> Building module...
wsl -e bash -c "cd ~/project2 && make clean && make"
if %errorlevel% neq 0 (
    echo ==> Build failed, aborting.
    exit /b 1
)

echo ==> Copying to Pi...
scp Z:\home\mark\project2\morse_dev.ko mark@192.168.137.234:/home/mark/project2/

echo ==> Reloading module on Pi...
ssh mark@192.168.137.234 "sudo rmmod morse_dev 2>/dev/null; cd ~/project2 && sudo sh morse_load && dmesg | tail -20"

echo ==> Done!