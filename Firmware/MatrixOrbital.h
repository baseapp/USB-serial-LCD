//define the Matrix Orbital command set
#define MATRIX_ORBITAL_COMMAND 254 //0xfe
#define BACKLIGHT_ON 66 //0x42, 1 parameter (minutes 00=forever)
#define BACKLIGHT_OFF 70 //0x46
#define CLEAR 88
#define HOME 72
#define POSITION 71 //2 parameters (col, row)
#define UNDERLINE_CURSER_ON 74
#define UNDERLINE_CURSER_OFF 75
#define BLOCK_CURSER_ON 83
#define BLOCK_CURSER_OFF 84
#define BACKLIGHT_BRIGHTNESS_SET_SAVE 152 //1 parameter (brightness)
#define BACKLIGHT_BRIGHTNESS_SET 153  //1 parameter (brightness)
#define CONTRAST_SET 80
#define CONTRAST_SET_AND_SAVE 145
#define CUSTOM_CHARACTER 78 //9 parameters (character #, 8 byte bitmap)
#define NETWORK_CONFIG 153 //show node connection details (custom)
#define EXIT 154 //exit Matrix orbital mode and resume screeensaver

