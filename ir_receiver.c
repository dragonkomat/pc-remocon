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
#include "ir_receiver.h"

#define SMTCLK  500E+3  // MFINTOSC(500kHz), 1:1
#define T       562E-6  // NECフォーマット
#define T_COUNT(t)  ((int)(T * t / (1.0 / SMTCLK)))

#define TIMEOUT_COUNT       T_COUNT(20)
#define PREAMBLE1_COUNT_H   T_COUNT(16 * 1.1)
#define PREAMBLE1_COUNT_L   T_COUNT(16 * 0.9)
#define PREAMBLE2_COUNT_H   T_COUNT(8 * 1.1)
#define PREAMBLE2_COUNT_L   T_COUNT(8 * 0.9)

typedef enum {
    IR_RECEIVER_STATE_IDLE,
    IR_RECEIVER_STATE_PREAMBLE,
    IR_RECEIVER_STATE_DATA
} ir_receiver_state_t;

ir_receiver_state_t ir_receiver_state = IR_RECEIVER_STATE_IDLE;

int ir_receiver_pwa_count = 0;
int ir_receiver_pra_count = 0;
int ir_receiver_cpwa[4];
int ir_receiver_cpra[4];

int ir_receiver_cpw = 0;
int ir_receiver_cpr = 0;

void ir_receiver_pwa_isr()
{
    ir_receiver_cpw = SMT1CPWH << 8 | SMT1CPWL;
    switch(ir_receiver_state)
    {
        case IR_RECEIVER_STATE_IDLE:
            if( ir_receiver_cpw >= PREAMBLE1_COUNT_L && ir_receiver_cpw <= PREAMBLE1_COUNT_H )
            {
                ir_receiver_state = IR_RECEIVER_STATE_PREAMBLE;
            }
            break;
        default:
            break;
    }
    if( ir_receiver_pwa_count < 4)
        ir_receiver_cpwa[ir_receiver_pwa_count++] = ir_receiver_cpw;
}

void ir_receiver_pra_isr()
{
    ir_receiver_cpr = SMT1CPRH << 8 | SMT1CPRL;
    switch(ir_receiver_state)
    {
        case IR_RECEIVER_STATE_PREAMBLE:
            if( ir_receiver_cpr >= PREAMBLE2_COUNT_L && ir_receiver_cpr <= PREAMBLE2_COUNT_H )
            {
                ir_receiver_state = IR_RECEIVER_STATE_DATA;
                LATCbits.LATC3 = 1;
            }
            break;
        default:
            break;
    }
    if( ir_receiver_pra_count < 4)
        ir_receiver_cpra[ir_receiver_pra_count++] = ir_receiver_cpr;
}

void ir_receiver_isr()
{
    ir_receiver_state = IR_RECEIVER_STATE_IDLE;
    //SMT1CON1bits.GO = 0;
    LATCbits.LATC3 = 0;
}

void ir_receiver_init()
{
    SMT1CON0 = 0x08;    // EN=0, (0), STP=0, WPOL=0, SPOL=1, CPOL=0, PS=00 ... 1:1
    SMT1CON1 = 0x43;    // GO=0, REPEAT=1, (0), (0), MODE=0011 (High and Low Measurement Mode)
    SMT1STAT = 0x00;    // CPRUP=0, CPWUP=0, (0), RST=0, (0), (TS=0), (WS=0), (AS=0)
    SMT1CLK = 0x04;     // (0), (0), (0), (0), (0), CSEL=100 ... MFINTOSC(500kHz)
    SMT1SIG = 0x00;     // (0), (0), (0), SSEL=00000 ... SMT1SIGPPS
    SMT1WIN = 0x00;     // (0), (0), (0), WSEL=00000 ... SMT1WINPPS(Unused)
    SMT1PRL = TIMEOUT_COUNT & 0xFF;
    SMT1PRH = (TIMEOUT_COUNT >> 8) & 0xFF;
    SMT1PRU = 0x00;
 
    PIR8bits.SMT1IF = 0;
    PIR8bits.SMT1PRAIF = 0;
    PIR8bits.SMT1PWAIF = 0;
    PIE8bits.SMT1IE = 1;
    PIE8bits.SMT1PRAIE = 1;
    PIE8bits.SMT1PWAIE = 1;

    SMT1CON0bits.EN = 1;

    SMT1STAT = 0xD0;    // CPRUP=CPWUP=RST=1
    while((SMT1STAT & 0xD0) != 0);
    SMT1CON1bits.GO = 1;
}