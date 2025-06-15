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
#include "interrupts.h"
#include "console.h"
#include "ir_receiver.h"

void __interrupt() isr(void)
{
    if(INTCONbits.PEIE == 1)
    {
        if(PIE4bits.TMR4IE == 1 && PIR4bits.TMR4IF == 1)
        {
            PIR4bits.TMR4IF = 0;
            ir_receiver_tmr_isr();
        }

        if(PIE8bits.SMT1PWAIE == 1 && PIR8bits.SMT1PWAIF == 1)
        {
            PIR8bits.SMT1PWAIF = 0;
            ir_receiver_pwa_isr();
        }

        if(PIE8bits.SMT1PRAIE == 1 && PIR8bits.SMT1PRAIF == 1)
        {
            PIR8bits.SMT1PRAIF = 0;
            ir_receiver_pra_isr();
        }

        if(PIE8bits.SMT1IE == 1 && PIR8bits.SMT1IF == 1)
        {
            PIR8bits.SMT1IF = 0;
            ir_receiver_isr();
        }

        if(PIE3bits.RC1IE == 1 && PIR3bits.RC1IF == 1)
        {
            console_rx_isr();
        }

        if(PIE3bits.TX1IE == 1 && PIR3bits.TX1IF == 1)
        {
            console_tx_isr();
        }
    }      
}

void interrupts_init(void)
{
    PIR0bits.INTF = 0;
    INTCONbits.INTEDG = 1;
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
}