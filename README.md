## Linux driver for HP Wireless Keyboard + Mouse Combo

This is a Linux driver hack to make the keyboard work for a HP wireless keyboard + mouse combo (specifically for HP Pavilion 800) manufactured by Primax. The keyboard problems experienced (keyboard works in BIOS, only a few special keys work in Linux) were very similar to problems reported by many many users for Lenovo wireless keyboard + mouse combos also manufactured by Primax, but the lenovo driver hack that this driver hack is a fork of did not work for the HP keyboard. Although the issue with the HP keyboard output was a little different than for the Lenovo keyboard, only minor changes of the code were needed.

### Compatibility
So far, this driver has been tested successfully with the following models:

* HP Pavilion Wireless Keyboard and Mouse 800 Nordic Layout (my model)
* Lenovo 510 Wireless Combo Keyboard & Mouse (@BorisKoval)

However, I have seen several reports of keyboards of other brands with similar problems, so it is possible that this fix could work for them. Feel free to test and please tell me if it worked or not. Linux used: Linux Mint 19.1 Cinnamon, Kernel 4.15.

### Prerequisites
This is a userspace program (not a kernel module). It reads raw USB HID
reports from the kernel's `usbmon` and re-emits them through `uinput`.
For it to work your system needs:

* The `uinput` kernel module loaded (usually automatic; `modprobe uinput`).
* The `usbmon` kernel module and a mounted `debugfs` — `hp_keyboard.sh`
  runs `modprobe usbmon` and reads `/sys/kernel/debug/usb/usbmon/*u`.
  This requires the kernel to be built with `CONFIG_USB_MON=y` (or the
  `usbmon` module available) and `debugfs` mounted (standard on most
  distros, typically at `/sys/kernel/debug`).
* `gcc` to build the program (`install.sh` handles compilation for you).
* Running as root (the systemd service runs as root and writes to
  `/dev/uinput`).

### Usage
Clone or download and install with:

```bash
./install.sh
```

and uninstall with:

```bash
./uninstall.sh
```

### Known issues
* Num pad keys behave strangely (for HP model). I kind of understand the issue, but don't know how to fix it.
* A couple of times in special circumstances, my terminal prompt has started filling with 0's as if I was holding down the 0 key. This stopped as soon as I pressed any key.
* At one point the driver stopped working. It seems the executable had disappeared somehow. I uninstalled and installed again to make it work again.

### Notes
* The `usbmon` capture is global: `hp_keyboard.sh` reads
  `/sys/kernel/debug/usb/usbmon/*u`, which includes traffic from **every**
  USB HID device. If another USB keyboard is connected, its reports are
  fed to the program too. If you see doubled or unexpected input with a
  second USB keyboard attached, that is the likely cause.
* Check the service logs with `journalctl -u hp_keyboard` if the driver
  stops responding.

If you find other issues or know how to fix something, please tell me.
