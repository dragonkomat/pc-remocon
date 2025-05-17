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

#define T_NEC       562E-6  // NECフォーマット ... T=562us
#define T_AEHA      425E-6  // 家製協フォーマット ... T=425us

#define T_COUNT(T, C)   ((int)(((double)(T)) * (C) / (1.0 / SMTCLK))) // T, Cは定数で指定

#define TIMEOUT_COUNT   T_COUNT(T_NEC, 20)

#define DATA_MAXLEN 16

typedef enum {
    IRR_STATE_IDLE,
    IRR_STATE_PREAMBLE,
    IRR_STATE_DATA
} irr_state_t;

typedef enum {
    IRR_TYPE_NEC,
    IRR_TYPE_AEHA,
    IRR_TYPE_SONY
} irr_type_t;

typedef struct {
    int min;
    int max;
} irr_minmax_t;

typedef struct {
    irr_minmax_t start_h;
    irr_minmax_t start_l;
    irr_minmax_t datax_h;
    irr_minmax_t data0_l;
    irr_minmax_t data1_l;
} irr_param_t;

const irr_param_t irr_params[] = {
    [IRR_TYPE_NEC] = {
       .start_h = { 
            T_COUNT(T_NEC, 16 * 0.9),
            T_COUNT(T_NEC, 16 * 1.1)
        },
       .start_l = {
            T_COUNT(T_NEC, 8 * 0.9),
            T_COUNT(T_NEC, 8 * 1.1)
        },
       .datax_h = {
            T_COUNT(T_NEC, 1 * 0.9),
            T_COUNT(T_NEC, 1 * 1.1)
        },
       .data0_l = {
            T_COUNT(T_NEC, 1 * 0.9),
            T_COUNT(T_NEC, 1 * 1.1)
        },
       .data1_l = {
            T_COUNT(T_NEC, 3 * 0.9),
            T_COUNT(T_NEC, 3 * 1.1)
        }
    },
    [IRR_TYPE_AEHA] = {
       .start_h = { 
            T_COUNT(T_AEHA, 8 * 0.9),
            T_COUNT(T_AEHA, 8 * 1.1)
        },
       .start_l = {
            T_COUNT(T_AEHA, 4 * 0.9),
            T_COUNT(T_AEHA, 4 * 1.1)
        },
       .datax_h = {
            T_COUNT(T_AEHA, 1 * 0.9),
            T_COUNT(T_AEHA, 1 * 1.1)
        },
       .data0_l = {
            T_COUNT(T_AEHA, 1 * 0.9),
            T_COUNT(T_AEHA, 1 * 1.1)
        },
       .data1_l = {
            T_COUNT(T_AEHA, 3 * 0.9),
            T_COUNT(T_AEHA, 3 * 1.1)
        }
    }
};
#define PARAMS irr_params

typedef struct {
    irr_state_t state;
    irr_type_t  type;
    int         width_h;
    int         width_l;
    char        work_bitpos;
    char        work_byte;
    char        work_length;
    char        work[DATA_MAXLEN];
    char        data_length;
    char        data[DATA_MAXLEN];
} irr_data_t;

irr_data_t irr_data = {
    .state = IRR_STATE_IDLE,
    .type = IRR_TYPE_NEC,
    .width_h = 0,
    .width_l = 0,
    .work_bitpos = 0,
    .work_byte = 0,
    .work_length = 0,
    .work = {0},
    .data_length = 0,
    .data = {0}
};
#define DATA irr_data

void ir_receiver_pwa_isr(void)
{
    DATA.width_h = SMT1CPWH << 8 | SMT1CPWL;
    switch( DATA.state )
    {
        case IRR_STATE_IDLE:
            if(    DATA.width_h >= PARAMS[IRR_TYPE_NEC].start_h.min
                && DATA.width_h <= PARAMS[IRR_TYPE_NEC].start_h.max )
            {
                DATA.type = IRR_TYPE_NEC;
                DATA.state = IRR_STATE_PREAMBLE;
                LATCbits.LATC3 = 1;
            }
            else if(    DATA.width_h >= PARAMS[IRR_TYPE_AEHA].start_h.min
                     && DATA.width_h <= PARAMS[IRR_TYPE_AEHA].start_h.max )
            {
                DATA.type = IRR_TYPE_AEHA;
                DATA.state = IRR_STATE_PREAMBLE;
                LATCbits.LATC3 = 1;
            }
            break;
        case IRR_STATE_DATA:
            break;
        default:
            break;
    }
}

void ir_receiver_pra_isr(void)
{
    DATA.width_l = SMT1CPRH << 8 | SMT1CPRL;
    switch( DATA.state )
    {
        case IRR_STATE_PREAMBLE:
            if(    DATA.width_l >= PARAMS[DATA.type].start_l.min
                && DATA.width_l <= PARAMS[DATA.type].start_l.max )
            {
                DATA.state = IRR_STATE_DATA;
            }
            break;
        case IRR_STATE_DATA:
            break;
        default:
            break;
    }
}

void ir_receiver_isr(void)
{
    DATA.state = IRR_STATE_IDLE;
    LATCbits.LATC3 = 0;
}

void ir_receiver_init(void)
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