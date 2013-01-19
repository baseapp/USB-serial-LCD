#include "keypad.h"

unsigned char curr_state[4];
unsigned char last_state[4];

unsigned char pressed_keys[4];
unsigned char released_keys[4];

char keymap[4][4]=
{
    {'a', 'b', 'c', 'd'},
    {'e', 'f', 'g', 'h'},
    {'i', 'j', 'k', 'l'},
    {'m', 'n', 'o', 'p'}
};

void keypad_read_state()
{
    keypad_update_last_state();

    C1=0;
    delayUS(5);
    curr_state[0]=PORTB>>4;
    C1=1;

    C2=0;
    delayUS(5);
    curr_state[1]=PORTB>>4;
    C2=1;

    C3=0;
    delayUS(5);
    curr_state[2]=PORTB>>4;
    C3=1;

    C4=0;
    delayUS(5);
    curr_state[3]=PORTB>>4;
    C4=1;

    keypad_sense();
}

void keypad_update_last_state()
{
    last_state[0]= curr_state[0];
    last_state[1]= curr_state[1];
    last_state[2]= curr_state[2];
    last_state[3]= curr_state[3];
}

void keypad_init()
{
    CM1CON0bits.C1ON=0;
    CM2CON0bits.C2ON=0;
    INTCONbits.IOCIE=0;
    LATB=0x0;
    ANSELB=0x00;
    PSTR1CONbits.STR1D=0;
    ADCON0bits.ADON=0;
    TRISB=0xF0;//RB0-3, column 1-4,digital out, RB4-7, row 4-1 is digital in
    WPUB=0xFF;//pull ups on RB4-7/row 4-1 input
    INTCON2bits.RBPU=0;
    C1=1;C2=1;C3=1;C4=1;
}

void keypad_sense()
{
    char i;
    for(i=0;i<4;i++)
    {
        pressed_keys[i] = ((last_state[i])&(~curr_state[i]))&0x0F;
        released_keys[i] = ((~last_state[i])&(curr_state[i]))&0x0F;
    }
}