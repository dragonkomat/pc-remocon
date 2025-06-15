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

#include "common.h"
#include <pic16f18424.h>
#include "ir_transmitter.h"

#define TMRCLK              32E+6       // FOSC(32MHz), CS=0010
#define TMRCLK_PS           4           // 1:4, CKPS=010, OUTPS=0000

#define TMR_COUNT(F)    ((unsigned char)((1.0 / ((double)(F))) * (TMRCLK / TMRCLK_PS))) // 引数は定数で指定

#define PERIOD_COUNT    TMR_COUNT(38E3)
#define DUTY_COUNT      ((unsigned char)(((double)PERIOD_COUNT) / 3))


void ir_transmitter_init(void)
{
    PWM6CON = 0x00;     // EN=0, (0), (OUT=0), POL=0, (0), (0), (0), (0) 
    PWM6DCL = 0x00;
    PWM6DCH = DUTY_COUNT;
    CCPTMRS1bits.P6TSEL = 3;    // TMR6

    T6CON = 0x20;       // ON=0, CKPS=010, OUTPS=0000 ... 1:4
    T6HLT = 0x00;       // PSYNC=0, CPOL=0, CSYNC=0, MODE=00000 (Free-Running Period Software Gate Mode)
    T6CLKCON = 0x02;    // (0), (0), (0), (0), CS=0010 ... FOSC(32MHz)
    T6PR = PERIOD_COUNT;
    PIR4bits.TMR6IF = 0;

    CLC4CON = 0x00;     // EN=0, (0), (OUT=0), INTP=0, INTN=0, MODE=000 (AND-OR)
    CLC4POL = 0x00;     // POL=0, (0), (0), (0), G4POL=0, G3POL=0, G2POL=0, G1POL=0
    CLC4GLS0 = 0x00;    // lcxg1 = logic 0
    CLC4GLS1 = 0x00;    // lcxg2 = logic 0
    CLC4GLS2 = 0x00;    // lcxg3 = logic 0
    CLC4GLS3 = 0x00;    // lcxg4 = logic 0

    MD1CON0 = 0x00;     // EN=0, (0), (OUT=0), OPOL=0, (0), (0), (0), BIT=0
    MD1CON1 = 0x11;     // (0), (0), CHPOL=0, CHSYNC=1, (0), (0), CLPOL=0, CLSYNC=1
    MD1SRC = 0x01;      // (0), (0), (0), SRCS=00001 (MDBIT)
    MD1CARL = 0x0E;     // (0), (0), (0), (0), CLS=1110 (CLC4 OUT = 0)
    MD1CARH = 0x08;     // (0), (0), (0), (0), CLH=1000 (PWM6 OUT)

    T6CONbits.ON = 1;
    PWM6CONbits.EN = 1;
    CLC4CONbits.EN = 1;
    MD1CON0bits.EN = 1;
}