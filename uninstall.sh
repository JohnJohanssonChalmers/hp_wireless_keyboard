#!/bin/bash

# Stop and disable the service. Tolerate errors so a partially-installed
# or not-running service doesn't abort the rest of the uninstall.
sudo systemctl stop hp_keyboard.service || true
sudo systemctl disable hp_keyboard.service || true
sudo systemctl daemon-reload || true

# Remove installed files (-f so missing files aren't an error)
sudo rm -f /etc/systemd/system/hp_keyboard.service
sudo rm -f /usr/local/bin/hp_keyboard.sh
sudo rm -f /usr/local/bin/kbdusb

# Remove local build artifact
rm -f kbdusb
