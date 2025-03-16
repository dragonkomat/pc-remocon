/*
    MIT License

    Copyright (c) 2025 dragonkomat

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include <xc.h>
#include <pic16f18424.h>

#include <stdio.h>

#include "console.h"

//CONFIG1
#pragma config FCMEN = ON
#pragma config CSWEN = ON
#pragma config CLKOUTEN = OFF
#pragma config RSTOSC = HFINT32 // HFINTOSC with OSCFRQ=32MHz and CDIV=1:1
#pragma config FEXTOSC = OFF

//CONFIG2
#pragma config MCLRE = ON
#pragma config PWRTS = OFF
#pragma config LPBOREN = OFF
#pragma config BOREN = ON
#pragma config BORV = LO
#pragma config ZCD = OFF
#pragma config PPS1WAY = ON
#pragma config STVREN = ON

//CONFIG3
#pragma config WDTCPS = WDTCPS_31
#pragma config WDTE = OFF
#pragma config WDTCWS = WDTCWS_7
#pragma config WDTCCS = SC

//CONFIG4
#pragma config BBSIZE = BB512
#pragma config BBEN = OFF
#pragma config SAFEN = OFF
#pragma config WRTAPP = OFF
#pragma config WRTB = OFF
#pragma config WRTC = OFF
#pragma config WRTD = OFF
#pragma config WRTSAF = OFF
#pragma config LVP = ON

//CONFIG5
#pragma config CP = OFF

void __interrupt() isr()
{
    if(INTCONbits.PEIE == 1)
    {
        if(PIE3bits.RC1IE == 1 && PIR3bits.RC1IF == 1)
        {
            console_rx_isr();
        }
        else
        {
        }
    }      
    else
    {
    }
}

void init()
{
    // RC3: LED (OUT)
    ANSELCbits.ANSC3 = 0;
    TRISCbits.TRISC3 = 0;
    LATCbits.LATC3 = 0;

    // RC4: LED (OUT)
    ANSELCbits.ANSC4 = 0;
    TRISCbits.TRISC4 = 0;
    LATCbits.LATC4 = 0;

    // RA4: EUSART RX1 (IN)
    ANSELAbits.ANSA4 = 0;
    RX1PPS = 0x4;
    // RC2: EUSART TX1 (OUT)
    ANSELCbits.ANSC2 = 0;
    RC2PPS = 0x0F;

    console_init();

    PIR0bits.INTF = 0;
    INTCONbits.INTEDG = 1;
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
}

int main()
{
    init();

    printf("Hi!");

    //LATCbits.LATC3 = 1;
    LATCbits.LATC4 = 1;
    while(1);
    return 0;
};
