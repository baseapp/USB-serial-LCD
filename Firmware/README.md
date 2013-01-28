BaseApp USB-serial LCD/Keypad
-----------------------------

Supports HD44780 compatible LCDs(in 4-bit mode). Enumerates as a serial com port, and can be
used with software like LCD smartie for windows, or LCDproc etc. for
linux using matrixorbital commands. Also supports upto 4x4 matrix keypad.

List of supported matrix orbital commands supported is in MatrixOrbital.h

Code is based on http://dangerousprototypes.com/docs/USB_Universal_LCD_backpack

- This code is for PIC18F25K50, and can me modified easily for 18f45k50,
  both do not require external crystal for USB.
- No bootloader
- Keypad press/release output on serial

- Compiler - tested with mplabc18 v3.44
- Requires USB stack from microchip applications library