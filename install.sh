#!/bin/bash
set -e

make clean
make
sudo cp kbdusb /usr/local/bin/.
sudo cp hp_keyboard.sh /usr/local/bin/.
sudo cp hp_keyboard.service /etc/systemd/system/.
sudo systemctl enable hp_keyboard.service
sudo systemctl daemon-reload
sudo systemctl start hp_keyboard.service
