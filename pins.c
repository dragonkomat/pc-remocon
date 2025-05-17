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
#include "pins.h"

void pins_init(void)
{
    // RC3: LED (OUT)
    ANSELCbits.ANSC3 = 0;
    LATCbits.LATC3 = 0;
    TRISCbits.TRISC3 = 0;

    // RC4: LED (OUT)
    ANSELCbits.ANSC4 = 0;
    LATCbits.LATC4 = 0;
    TRISCbits.TRISC4 = 0;

    // RA4: EUSART RX1 (IN)
    ANSELAbits.ANSA4 = 0;
    RX1PPS = 0x04;  // RA4
    // RC2: EUSART TX1 (OUT)
    ANSELCbits.ANSC2 = 0;
    RC2PPS = 0x0F;  // CK1/TX1

    // RA5: BUZZER (OUT)
    ANSELAbits.ANSA5 = 0;
    LATAbits.LATA5 = 0;
    TRISAbits.TRISA5 = 0;
    RA5PPS = 0x18;  // NCO1OUT

    // RC5: IR receiver (IN)
    ANSELCbits.ANSC5 = 0;
    TRISCbits.TRISC5 = 1;
    SMT1SIGPPS = 0x15;  // RC5
    // 負論理の場合はSMT1CON0のSPOLビットを1にする
}
