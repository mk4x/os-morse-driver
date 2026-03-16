#!/usr/bin/env sh

set -eu

echo "==> Building module..."
make clean && make

echo "==> Copying kernel modules to Pi..."
scp \
	./morse_dev.ko ./morse_table.ko ./gpio_handler.ko \
	mark@mark.local:/home/mark/project2/

echo "==> Reloading module on Pi..."
ssh mark@mark.local "cd ~/project2 && \
	sudo sh morse_unload >/dev/null 2>&1 || true; \
	sudo sh morse_load && \
	ls -l /dev/morse && dmesg | tail -20"

echo "==> Done!"
