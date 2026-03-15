#!/usr/bin/env sh

set -eu

echo "==> Building module..."
make clean && make

echo "==> Copying to Pi..."
scp ./morse_dev.ko mark@mark.local:/home/mark/project2/

echo "==> Reloading module on Pi..."
ssh mark@mark.local "sudo rmmod morse_dev 2>/dev/null; cd ~/project2 && sudo sh morse_load && dmesg | tail -20"

echo "==> Done!"
