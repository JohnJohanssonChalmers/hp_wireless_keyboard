#include <asm-generic/errno-base.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DEBUG 0
#define TEST_BIT(var, bitval) ((var & bitval) == bitval)

/*
 * USB HID usage code -> Linux input key code.
 * Indexed by the raw HID usage byte; any entry not listed below is 0
 * (unmapped), which callers treat as "no key". Because 0 == KEY_0, an
 * unmapped key is never emitted (see the `code != 0` guards in main).
 */
static const int keymap[256] = {
  [0x04] = KEY_A,     [0x05] = KEY_B,     [0x06] = KEY_C,
  [0x07] = KEY_D,     [0x08] = KEY_E,     [0x09] = KEY_F,
  [0x0a] = KEY_G,     [0x0b] = KEY_H,     [0x0c] = KEY_I,
  [0x0d] = KEY_J,     [0x0e] = KEY_K,     [0x0f] = KEY_L,
  [0x10] = KEY_M,     [0x11] = KEY_N,     [0x12] = KEY_O,
  [0x13] = KEY_P,     [0x14] = KEY_Q,     [0x15] = KEY_R,
  [0x16] = KEY_S,     [0x17] = KEY_T,     [0x18] = KEY_U,
  [0x19] = KEY_V,     [0x1a] = KEY_W,     [0x1b] = KEY_X,
  [0x1c] = KEY_Y,     [0x1d] = KEY_Z,
  [0x1e] = KEY_1,     [0x1f] = KEY_2,     [0x20] = KEY_3,
  [0x21] = KEY_4,     [0x22] = KEY_5,     [0x23] = KEY_6,
  [0x24] = KEY_7,     [0x25] = KEY_8,     [0x26] = KEY_9,
  [0x27] = KEY_0,
  [0x29] = KEY_ESC,       [0x2d] = KEY_MINUS,   [0x2e] = KEY_EQUAL,
  [0x2a] = KEY_BACKSPACE, [0x2b] = KEY_TAB,
  [0x2f] = KEY_LEFTBRACE, [0x30] = KEY_RIGHTBRACE,
  [0x28] = KEY_ENTER,
  [0x33] = KEY_SEMICOLON, [0x34] = KEY_APOSTROPHE, [0x35] = KEY_GRAVE,
  [0x32] = KEY_BACKSLASH,
  [0x36] = KEY_COMMA, [0x37] = KEY_DOT, [0x38] = KEY_SLASH,
  [0x55] = KEY_KPASTERISK,
  [0x2c] = KEY_SPACE,
  [0x39] = KEY_CAPSLOCK,
  [0x3a] = KEY_F1,  [0x3b] = KEY_F2,  [0x3c] = KEY_F3,
  [0x3d] = KEY_F4,  [0x3e] = KEY_F5,  [0x3f] = KEY_F6,
  [0x40] = KEY_F7,  [0x41] = KEY_F8,  [0x42] = KEY_F9,
  [0x43] = KEY_F10, [0x44] = KEY_F11, [0x45] = KEY_F12,
  [0x53] = KEY_NUMLOCK,
  [0x47] = KEY_SCROLLLOCK,
  [0x5f] = KEY_KP7, [0x60] = KEY_KP8, [0x61] = KEY_KP9,
  [0x56] = KEY_KPMINUS,
  [0x5c] = KEY_KP4, [0x5d] = KEY_KP5, [0x5e] = KEY_KP6,
  [0x57] = KEY_KPPLUS,
  [0x59] = KEY_KP1, [0x5a] = KEY_KP2, [0x5b] = KEY_KP3,
  [0x62] = KEY_KP0, [0x63] = KEY_KPDOT,
  [0x58] = KEY_KPENTER,
  [0x54] = KEY_KPSLASH,
  [0x46] = KEY_SYSRQ,
  [0x4a] = KEY_HOME,  [0x52] = KEY_UP,   [0x4b] = KEY_PAGEUP,
  [0x50] = KEY_LEFT,  [0x4f] = KEY_RIGHT,
  [0x4d] = KEY_END,   [0x51] = KEY_DOWN, [0x4e] = KEY_PAGEDOWN,
  [0x49] = KEY_INSERT, [0x4c] = KEY_DELETE, [0x48] = KEY_PAUSE,
  [0x64] = KEY_102ND,
};

static int mapkey(const unsigned char keypressed) {
  if (DEBUG) printf("mapkey keypressed=%i, ", keypressed);
  int keysent = keymap[keypressed];
  if (DEBUG) printf("mapkey keysent=%i\n", keysent);
  return keysent;
}

/* Emit a single key press (value=1) or release (value=0). */
static void send_key(int fd, int code, int value) {
  struct input_event ev;
  memset(&ev, 0, sizeof(ev));
  ev.type = EV_KEY;
  ev.code = code;
  ev.value = value;
  write(fd, &ev, sizeof(ev));
}

/*
 * Modifier bits in HID report byte 0 -> Linux key codes.
 * Order matches the original code so the emit order is unchanged.
 */
struct modifier {
  int mask;
  int code;
};
static const struct modifier modifiers[] = {
  {1,  KEY_LEFTCTRL},
  {16, KEY_RIGHTCTRL},
  {2,  KEY_LEFTSHIFT},
  {32, KEY_RIGHTSHIFT},
  {4,  KEY_LEFTALT},
  {64, KEY_RIGHTALT},
  {8,  KEY_LEFTMETA},
};

int main() {
  int ret;
  unsigned char buffer[80];
  unsigned char keypressed[8];
  unsigned char keypressed_prec[8];

  const int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK | O_DSYNC);
  if (fd < 0) {
    perror("open /dev/uinput");
    return 1;
  }
  ret = ioctl(fd, UI_SET_EVBIT, EV_KEY);
  ret = ioctl(fd, UI_SET_EVBIT, EV_SYN);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_A); /* Auth KEY_A */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_B); /* Auth KEY_B */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_C); /* Auth KEY_C */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_D); /* Auth KEY_D */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_E); /* Auth KEY_E */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_F); /* Auth KEY_F */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_G); /* Auth KEY_G */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_H); /* Auth KEY_H */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_I); /* Auth KEY_I */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_J); /* Auth KEY_J */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_K); /* Auth KEY_K */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_L); /* Auth KEY_L */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_M); /* Auth KEY_M */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_N); /* Auth KEY_N */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_O); /* Auth KEY_O */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_P); /* Auth KEY_P */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_Q); /* Auth KEY_Q */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_R); /* Auth KEY_R */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_S); /* Auth KEY_S */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_T); /* Auth KEY_T */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_U); /* Auth KEY_U */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_V); /* Auth KEY_V */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_W); /* Auth KEY_W */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_X); /* Auth KEY_X */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_Y); /* Auth KEY_Y */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_Z); /* Auth KEY_Z */
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_0);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_1);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_2);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_3);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_4);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_5);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_6);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_7);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_8);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_9);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_ESC);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_MINUS);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_EQUAL);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_BACKSPACE);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_TAB);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_LEFTBRACE);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_RIGHTBRACE);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_ENTER);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_SEMICOLON);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_APOSTROPHE);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_GRAVE);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_LEFTSHIFT);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_BACKSLASH);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_COMMA);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_DOT);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_SLASH);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_RIGHTSHIFT);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_KPASTERISK);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_LEFTALT);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_SPACE);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_CAPSLOCK);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_F1);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_F2);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_F3);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_F4);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_F5);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_F6);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_F7);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_F8);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_F9);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_F10);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_F11);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_F12);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_NUMLOCK);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_SCROLLLOCK);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_KP7);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_KP8);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_KP9);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_KPMINUS);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_KP4);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_KP5);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_KP6);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_KPPLUS);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_KP1);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_KP2);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_KP3);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_KP0);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_KPDOT);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_KPENTER);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_RIGHTCTRL);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_LEFTCTRL);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_KPSLASH);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_SYSRQ);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_RIGHTALT);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_HOME);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_UP);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_PAGEUP);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_LEFT);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_RIGHT);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_END);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_DOWN);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_PAGEDOWN);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_INSERT);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_DELETE);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_MUTE);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_VOLUMEDOWN);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_VOLUMEUP);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_CALC);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_LEFTMETA);
  ret = ioctl(fd, UI_SET_KEYBIT, KEY_102ND);

  struct uinput_user_dev uidev;
  memset(&uidev, 0, sizeof(uidev));
  snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-sample");
  uidev.id.bustype = BUS_USB;
  uidev.id.vendor = 0x1234;
  uidev.id.product = 0xfedc;
  uidev.id.version = 1;
  ret = write(fd, &uidev, sizeof(uidev));
  ret = ioctl(fd, UI_DEV_CREATE);
  if (ret < 0) {
    perror("ioctl UI_DEV_CREATE");
    return 1;
  }
  if (DEBUG) printf("uinput all set\n");

  memset(&buffer, 0, 80);
  FILE *in;
  in = fopen("/dev/stdin", "r");
  if (!in) {
    perror("fopen /dev/stdin");
    return 1;
  }
  if (DEBUG) printf("stdin is open\n");

  while (fgets(buffer, 79, in)) {
    if (DEBUG) printf("read: %s", buffer);
    char bytes_reported[2];
    strncpy(bytes_reported, buffer, 1);
    bytes_reported[1] = 0; /* null-terminate (strncpy with n=1 does not) */
    int ibytes_reported = atoi(bytes_reported);

    if (DEBUG) printf("Num bytes reported: %i\n", ibytes_reported);

    if (ibytes_reported == 8) { /* a regular key, 8 bytes */
      /* start at 2 to skip length and = */
      for (int i = 2; i < ibytes_reported * 2; i += 2) {
        char c1[2], c2[2];
        c1[0] = buffer[i];
        c2[0] = buffer[i + 1];
        c1[1] = 0;
        c2[1] = 0;
        keypressed[i / 2 - 1] =
            (unsigned char)strtol(c1, NULL, 16) * 16 + strtol(c2, NULL, 16);
        if (DEBUG)
          printf("keypressed[%i]: %i\n", i / 2 - 1, keypressed[i / 2 - 1]);
      }

      /*
       * Modifier keys (report byte 0). Emit all releases first, then all
       * presses, matching the original emit order.
       */
      for (size_t m = 0; m < sizeof(modifiers) / sizeof(modifiers[0]); m++) {
        if (TEST_BIT(keypressed_prec[0], modifiers[m].mask) &&
            !TEST_BIT(keypressed[0], modifiers[m].mask)) {
          if (DEBUG) printf("modifier 0x%x released\n", modifiers[m].mask);
          send_key(fd, modifiers[m].code, 0);
        }
      }
      for (size_t m = 0; m < sizeof(modifiers) / sizeof(modifiers[0]); m++) {
        if (!TEST_BIT(keypressed_prec[0], modifiers[m].mask) &&
            TEST_BIT(keypressed[0], modifiers[m].mask)) {
          if (DEBUG) printf("modifier 0x%x pressed\n", modifiers[m].mask);
          send_key(fd, modifiers[m].code, 1);
        }
      }

      /* Regular keys (report bytes 2..7): detect releases and new presses. */
      int i, j;
      for (i = 2; i < 8; i++) {
        if (keypressed_prec[i] != 0) {
          /* still pressed? */
          for (j = 2; j < 8; j++) {
            if (keypressed_prec[i] == keypressed[j]) break;
          }
          if (j == 8) {
            /* key is released */
            int code = mapkey(keypressed_prec[i]);
            if (code != 0) send_key(fd, code, 0); /* skip unmapped (0 == KEY_0) */
          }
        }
        if (keypressed[i] != 0) {
          /* wasn't pressed? */
          for (j = 2; j < 8; j++) {
            if (keypressed_prec[j] == keypressed[i]) break;
          }
          if (j == 8) {
            /* key is newly pressed */
            int code = mapkey(keypressed[i]);
            if (code != 0) send_key(fd, code, 1); /* skip unmapped (0 == KEY_0) */
          }
        }
      }

      struct input_event ev;
      memset(&ev, 0, sizeof(ev));
      ev.type = EV_SYN;
      ev.code = SYN_REPORT;
      ev.value = 1;
      ret = write(fd, &ev, sizeof(ev));

      memcpy(keypressed_prec, keypressed, 8);
      if (DEBUG) printf("\n");
    }
  }

  return 0;
}
