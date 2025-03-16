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

void console_rx_isr()
{
    if(RC1STAbits.OERR)
    {
    }  
    if(RC1STAbits.FERR)
    {
    } 
    
    char rxdata = RC1REG;
    LATCbits.LATC3 = rxdata & 0x01;
}

void console_init()
{
    PIE3bits.RC1IE = 0;
    BAUD1CON = 0x08;    // (ABDOVF=0), (RCIDL=0), (0), SCKP=0, BRG16=1, (0), WUE=0, ABDEN=0
    RC1STA   = 0x90;    // SPEN=1, RX9=0, SREN=0, CREN=1, ADDEN=0, (FERR=0), OERR=0, RX9D=0
    TX1STA   = 0x24;    // CSRC=0, TX9=0, TXEN=1, SYNC=0, SENDB=0, BRGH=1, (TRMT=0), TX9D=0
    SP1BRGL  = 0x44;    // Fosc=32MHz, (SYNC=x, BRGH=1, BRG16=1 ... x4), SP1BRG=0x0044 ... 115.2kbps
    SP1BRGH  = 0x0;
    PIR3bits.RC1IF = 0;
    PIE3bits.RC1IE = 1;
}

int getch(void)
{
    return 0;
}

void putch(char txData)
{
    while(!(PIR3bits.TX1IF && TX1STAbits.TXEN));
    TX1REG = txData;
}
