/* 
 * File:   keypad.h
 * Author: tavish
 *
 * Created on January 16, 2013, 4:10 PM
 */

#ifndef KEYPAD_H
#define	KEYPAD_H

#include <Compiler.h>

#ifdef	__cplusplus
extern "C" {
#endif

//RB0-3, column 1-4,digital out, RB4-7, row 4-1 is digital in

#define R1 LATBbits.LATB7
#define R2 LATBbits.LATB6
#define R3 LATBbits.LATB5
#define R4 LATBbits.LATB4

#define C4 LATBbits.LATB3
#define C3 LATBbits.LATB2
#define C2 LATBbits.LATB1
#define C1 LATBbits.LATB0

extern unsigned char curr_state[4];
extern unsigned char last_state[4];

extern unsigned char pressed_keys[4];
extern unsigned char released_keys[4];

extern char keymap[4][4];

void keypad_init();
void keypad_read_state();
void keypad_update_last_state();//internal
void keypad_sense();

#ifdef	__cplusplus
}
#endif

#endif	/* KEYPAD_H */

