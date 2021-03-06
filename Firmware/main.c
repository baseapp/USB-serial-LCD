//Due to licensing issues, we can't redistribute the Microchip USB source. 
//You can get it from the Microchip website for free: 
//http://www.microchip.com/usb
//
//1.Download and install the USB source. These files install to c:\Microchip Soultions\ by default.
//2.Place the project folder in the Microchip source install directory (c:\Microchip Soultions\ by default)
//3.Copy usb_config.h & usb_descriptors.c from \Microchip Solutions\USB Device - CDC - Basic Demo\CDC - Basic Demo - Firmware3\ to the project folder.
//4. That's it. You've got the latest source and we're compliant with the license.
//
//Depending on the install location you may need to tweak the include paths under Project->build options.
#include "globals.h"
#include "config.h"
#include "MatrixOrbital.h"
#include "keypad.h"

//this struct buffers the USB input because the stack doesn't like 1 byte reads
#pragma udata
static struct _usbbuffer{
	unsigned char inbuf[64];
	unsigned char cnt;
	unsigned char rdptr;
} ubuf;

//USB output buffer
#define USB_OUT_BUF 64
unsigned char buf[USB_OUT_BUF];
unsigned char uartincnt=0;

char stringBuffer[40];

static void init(void);
void usbbufservice(void);
void usbbufflush(void);
unsigned char usbbufgetbyte(unsigned char* c);
unsigned char waitforbyte(void);
unsigned char checkforbyte(void);
void sendok(void);
void send(unsigned char c);

#pragma code
void main(void)
{  
    unsigned char i,cmd, param[9];

    init();			//setup the crystal, pins, usb

    HD44780_Reset();//setup the LCD
    HD44780_Init();
    LCD_Home();
    LCD_Clear();
    LCD_Backlight(0);

    usbbufflush();	//setup the USB byte buffer

    delayMS(10);

    keypad_read_state();
    
    while(1)
    {
        USBDeviceTasks();

        if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1))
            continue;
        usbbufservice();//load any USB data into byte buffer

        cmd=waitforbyte();//wait for a byte from USB

        if(cmd!=MATRIX_ORBITAL_COMMAND)
        {//assume text, if 254 then enter command mode

            LCD_WriteChar(cmd); //not a command, just write it to the display

        }else
        {//previous byte was 254, now get actual command
            switch(waitforbyte())
            {
                //switch on the command
                case BACKLIGHT_ON: //1 parameter (minutes 00=forever)
                    param[0]=waitforbyte();
                    LCD_Backlight(1);//turn it on, we ignore the parameter
                    break;
                case BACKLIGHT_OFF:
                    LCD_Backlight(0);//backlight off
                    break;
                case CLEAR:
                    LCD_Clear();
                    break;
                case HOME:
                    LCD_Home();
                    break;
                case POSITION: //2 parameters (col, row)
                    param[0]=waitforbyte();
                    param[1]=waitforbyte();
                    cmd=( ((param[1]-1)*20) + param[0] ); //convert to 20x4 layout (used defined lines, add rows...)
                    LCD_CursorPosition(cmd);
                    break;
                case UNDERLINE_CURSER_ON:
                    LCD_UnderlineCursor(1);
                    break;
                case UNDERLINE_CURSER_OFF:
                    LCD_UnderlineCursor(0);
                    break;
                case BLOCK_CURSER_ON:
                    LCD_BlinkCursor(1);
                    break;
                case BLOCK_CURSER_OFF:
                    LCD_BlinkCursor(0);
                    break;
                case BACKLIGHT_BRIGHTNESS_SET://1 parameter (brightness)
                case BACKLIGHT_BRIGHTNESS_SET_SAVE:
                    param[0]=waitforbyte();
                    LCD_Backlight(param[0]);
                    break;
                case CONTRAST_SET://1 parameter (brightness)
                case CONTRAST_SET_AND_SAVE:
                    param[0]=waitforbyte();
                    LCD_Contrast(param[0]);
                    break;
                case CUSTOM_CHARACTER: //9 parameters (character #, 8 byte bitmap)
                    LCD_WriteCGRAM(waitforbyte());//write character address
                    for(i=1; i<9; i++)
                    {
                        LCD_WriteRAM(waitforbyte()); //send 8 bitmap bytes
                    }
                    break;
                default: //error
                    break;
            }
        }
        CDCTxService();
    }
}//end main

void send(unsigned char c)
{
	unsigned char b[2];

  	if( mUSBUSARTIsTxTrfReady() ){ //it's always ready, but this could be done better
		b[0]=c;
		putUnsignedCharArrayUsbUsart(b,1);
	}	

}

void sendok(void)
{
	unsigned char b[2];

  	if( mUSBUSARTIsTxTrfReady() ){ //it's always ready, but this could be done better
		b[0]='1';//answer OK
		putUnsignedCharArrayUsbUsart(b,1);
	}	

}

unsigned char waitforbyte(void)
{
	unsigned char inbuf,togglebit=0;

        unsigned char temp;//calculate row number
        char output;
        char i; //loop var
        int len;// for  stringBuffer

	if(usbbufgetbyte(&inbuf)) return inbuf; //return data
	//wait for more USB data
	//services USB peripheral
	while(1)
        {            
            keypad_read_state();

            /* DO keypad stuff*/
            len=0;
            stringBuffer[0]=0x00;
            for(i=0;i<4;i++)
            {
                if((pressed_keys[i])!=0x0)
                {
                    temp=0;
                    while(pressed_keys[i]>>=1)
                        temp++;
                    output = keymap[i][temp]-'a'+'A';
                    stringBuffer[len]=output;len++;
                }
               
                if((released_keys[i])!=0x0)
                {
                    temp=0;
                    while(released_keys[i]>>=1)
                        temp++;
                    output = keymap[i][temp];
                    stringBuffer[len]=output;len++;
                }                
            }
            stringBuffer[len]=0x00;
            if(stringBuffer[0])
                putsUSBUSART(stringBuffer);

            USBDeviceTasks();
            if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1))
            {
                    return 0; //goto USB_loop_jump; //USB gone, fail out to main loop
            }
            usbbufservice();//load any USB data into byte buffer
            if(usbbufgetbyte(&inbuf))
                return inbuf; //wait for more data
	    CDCTxService();
	}

} 

unsigned char checkforbyte(void)
{
	unsigned char inbuf;

	if(usbbufgetbyte(&inbuf)) return 1; //return data
	//wait for more USB data
	//services USB peripheral
	USBDeviceTasks(); 
   	if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1)){
		return 1; //goto USB_loop_jump; //USB gone, fail out to main loop
	}
	usbbufservice();//load any USB data into byte buffer
	if(usbbufgetbyte(&inbuf)) return 1; //wait for more data
    CDCTxService();
	return 0;
} 

static void init(void)
{
	unsigned int cnt = 2048;
	
	//all pins digital
	//disable some defaults
        ADCON0bits.ADON=0;
        ADCON1 |= 0b1111;   	//all pins digital
	//CVRCON=0b00000000;

	//make sure everything is input (should be on startup, but just in case)
	TRISA=0xff;
        TRISB=0xff;
        TRISC=~0b0000111; //RC2 is pwm output, so TRIS=0,ANSEL=0, tris cleared later

        LCD_BL=0;//backlight off

        //pwm config for LCD_BL and LCD_VO
        PR2 = 63;//period
        CCP1CONbits.CCP1M=0b1111;//configure ccp module for pwm
        CCP2CONbits.CCP2M=0b1111;//configure ccp module for pwm
        //duty cycle is (CCPR1L:DC1B)/(4*(PR2+1))
        CCPR1L=0xff;
        CCPR2L=0xff;
        CCP1CONbits.DC1B=0x3;
        CCP1CONbits.DC1B=0x3;
        //start 8 bit timer2
        PIR1bits.TMR1IF=0;
        PIR2bits.TMR3IF=0;
        T2CONbits.T2CKPS=1;
        T2CONbits.TMR2ON=1;
        while(!PIR1bits.TMR2IF);
        TRISCbits.RC2=0;
        TRISCbits.RC1=0;
	
        OSCCONbits.IRCF0=1;
        OSCCONbits.IRCF1=1;
        OSCCONbits.IRCF2=1;

//        #if(USB_SPEED_OPTION == USB_FULL_SPEED)
            OSCTUNE = 0x80; //3X PLL ratio mode selected
            OSCCON = 0x70;  //Switch to 16MHz HFINTOSC
            OSCCON2 = 0x10; //Enable PLL, SOSC, PRI OSC drivers turned off

            ACTCONbits.ACTEN=0;
            ACTCONbits.ACTSRC=1;
            ACTCONbits.ACTEN=1;
            ACTCONbits.ACTUD=0;
//            *((unsigned char*)0xFB5) = 0x90;  //Enable active clock tuning for USB operation
//        #endif
        //Configure all I/O pins for digital mode (except RA0/AN0 which has POT on demo board)
        ANSELA = 0x00;
        ANSELB = 0xFF;
        ANSELC = 0x00;
        
        while(OSCCON2bits.PLLRDY != 1);   //Wait for PLL lock
//        while(ACTCONbits.ACTLOCK != 1);

        keypad_init();

        USBDeviceInit();
}

void usbbufservice(void)
{
	if(ubuf.cnt==0){//if the buffer is empty, get more data
		ubuf.cnt = getsUSBUSART(ubuf.inbuf,64);
		ubuf.rdptr=0;
	}
}

//puts a byte from the buffer in the byte, returns 1 if byte
unsigned char usbbufgetbyte(unsigned char* c)
{
	if(ubuf.cnt>0){
		*c=ubuf.inbuf[ubuf.rdptr];
		ubuf.cnt--;
		ubuf.rdptr++;
		return 1;
	}
	return 0;
}

void usbbufflush(void)
{
	ubuf.cnt = 0;
	ubuf.rdptr=0;
}

//
//
//the stack calls these, if they aren't here we get errors. 
//
//
void USBCBSuspend(void){}
void USBCBWakeFromSuspend(void){}
void USBCB_SOF_Handler(void){}
void USBCBErrorHandler(void){}
void USBCBCheckOtherReq(void){USBCheckCDCRequest();}//end
void USBCBStdSetDscHandler(void){}//end
void USBCBInitEP(void){CDCInitEP();}
BOOL USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, WORD size)
{
    switch(event){
        case EVENT_CONFIGURED: 
            USBCBInitEP();
            break;
        case EVENT_SET_DESCRIPTOR:
            USBCBStdSetDscHandler();
            break;
        case EVENT_EP0_REQUEST:
            USBCBCheckOtherReq();
            break;
        case EVENT_SOF:
            USBCB_SOF_Handler();
            break;
        case EVENT_SUSPEND:
            USBCBSuspend();
            break;
        case EVENT_RESUME:
            USBCBWakeFromSuspend();
            break;
        case EVENT_BUS_ERROR:
            USBCBErrorHandler();
            break;
        case EVENT_TRANSFER:
            Nop();
            break;
        default:
            break;
    }      
    return TRUE; 
}

/*      TODO remove all remaps        */

//#define REMAPPED_RESET_VECTOR_ADDRESS			0x800
//#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x808
//#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x818

#define REMAPPED_RESET_VECTOR_ADDRESS			0x00
#define REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS	0x08
#define REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS	0x18

//We didn't use the low priority interrupts, 
// but you could add your own code here
#pragma interruptlow InterruptHandlerLow
void InterruptHandlerLow(void){}

//We didn't use the low priority interrupts, 
// but you could add your own code here
#pragma interrupt InterruptHandlerHigh
void InterruptHandlerHigh(void){}

////these statements remap the vector to our function
////When the interrupt fires the PIC checks here for directions
//#pragma code REMAPPED_HIGH_INTERRUPT_VECTOR = REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS
//void Remapped_High_ISR (void){
//     _asm goto InterruptHandlerHigh _endasm
//}
//
//#pragma code REMAPPED_LOW_INTERRUPT_VECTOR = REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS
//void Remapped_Low_ISR (void){
//     _asm goto InterruptHandlerLow _endasm
//}
//
////relocate the reset vector
//extern void _startup (void);
//#pragma code REMAPPED_RESET_VECTOR = REMAPPED_RESET_VECTOR_ADDRESS
//void _reset (void){
//    _asm goto _startup _endasm
//}
////set the initial vectors so this works without the bootloader too.
//#pragma code HIGH_INTERRUPT_VECTOR = 0x08
//void High_ISR (void){
//     _asm goto REMAPPED_HIGH_INTERRUPT_VECTOR_ADDRESS _endasm
//}
//#pragma code LOW_INTERRUPT_VECTOR = 0x18
//void Low_ISR (void){
//     _asm goto REMAPPED_LOW_INTERRUPT_VECTOR_ADDRESS _endasm
//}
//
