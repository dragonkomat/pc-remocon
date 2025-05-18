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

//#define IRR_DEBUG   1

#define SMTCLK              500E+3  // MFINTOSC(500kHz), 1:1

#define T_NEC               562E-6  // NECフォーマット ... T=562us
#define T_AEHA              425E-6  // 家製協フォーマット ... T=425us

#define T_LEADER_COEFF_MIN  0.9
#define T_LEADER_COEFF_MAX  1.1

#define DATA_MAXLEN 16

#define T_COUNT(T, C)   ((int)(((double)(T)) * (C) / (1.0 / SMTCLK)))   // 引数は定数で指定

#define TIMEOUT_COUNT   T_COUNT(T_NEC, 20)

typedef enum {
    IRR_STATE_IDLE,
    IRR_STATE_LEADER,
    IRR_STATE_DATA,
} irr_state_t;

typedef enum {
    IRR_TYPE_NEC,
    IRR_TYPE_AEHA,
    IRR_TYPE_SONY,
} irr_type_t;

typedef enum {
    IRR_ERROR_NONE,
    IRR_ERROR_STATE_H,
    IRR_ERROR_STATE_L,
    IRR_ERROR_LEADER_H,
    IRR_ERROR_LEADER_L,
    IRR_ERROR_DATA_H,
    IRR_ERROR_DATA_L,
    IRR_ERROR_DATA_CHECK,
    IRR_ERROR_DATA_OVERRUN,
} irr_error_t;

typedef struct {
    int min;
    int typ;
    int max;
} irr_minmax_t;

typedef struct {
    irr_minmax_t leader_h;
    irr_minmax_t leader_l;
    irr_minmax_t data_th_h;
    irr_minmax_t data_th_l;
} irr_param_t;

const irr_param_t irr_params[] = {
    [IRR_TYPE_NEC] = {
       .leader_h = { 
            .min = T_COUNT(T_NEC, 16 * T_LEADER_COEFF_MIN),
            .typ = T_COUNT(T_NEC, 16                     ),
            .max = T_COUNT(T_NEC, 16 * T_LEADER_COEFF_MAX)
        },
       .leader_l = {
            .min = T_COUNT(T_NEC, 8 * T_LEADER_COEFF_MIN),
            .typ = T_COUNT(T_NEC, 8                     ),
            .max = T_COUNT(T_NEC, 8 * T_LEADER_COEFF_MAX)
        },
       .data_th_h = {
            .min = 0,
            .typ = T_COUNT(T_NEC, 2                     ),
            .max = T_COUNT(T_NEC, 4                     )
        },
       .data_th_l = {
            .min = 0,
            .typ = T_COUNT(T_NEC, 2                     ),
            .max = T_COUNT(T_NEC, 4                     )
        }
    },
    [IRR_TYPE_AEHA] = {
       .leader_h = { 
            .min = T_COUNT(T_AEHA, 8 * T_LEADER_COEFF_MIN),
            .typ = T_COUNT(T_AEHA, 8                     ),
            .max = T_COUNT(T_AEHA, 8 * T_LEADER_COEFF_MAX)
        },
       .leader_l = {
            .min = T_COUNT(T_AEHA, 4 * T_LEADER_COEFF_MIN),
            .typ = T_COUNT(T_AEHA, 4                     ),
            .max = T_COUNT(T_AEHA, 4 * T_LEADER_COEFF_MAX)
        },
       .data_th_h = {
            .min = 0,
            .typ = T_COUNT(T_AEHA, 2                     ),
            .max = T_COUNT(T_AEHA, 4                     )
        },
       .data_th_l = {
            .min = 0,
            .typ = T_COUNT(T_AEHA, 2                     ),
            .max = T_COUNT(T_AEHA, 4                     )
        }
    }
};
#define PARAMS irr_params

typedef struct {
    irr_state_t state;
    irr_type_t  type;
    irr_error_t error;
    int         width_h;
    int         width_l;
#ifndef IRR_DEBUG
    int         width_target0;
    int         width_target1;
    char        work_bitpos;
    char        work_byte;
    char        work_length;
    char        work[DATA_MAXLEN];
    char        data_length;
    char        data[DATA_MAXLEN];
#else   // IRR_DEBUG
    char debug_l_length;
    char debug_h_length;
    int debug_l[48];
    int debug_h[48];
#endif  // IRR_DEBUG
} irr_data_t;

irr_data_t irr_data = {
    .state = IRR_STATE_IDLE,
    .type = IRR_TYPE_NEC,
    .error = IRR_ERROR_NONE,
    .width_h = 0,
    .width_l = 0,
#ifndef IRR_DEBUG
    .width_target0 = 0,
    .width_target1 = 0,
    .work_bitpos = 0,
    .work_byte = 0,
    .work_length = 0,
    .work = {0},
    .data_length = 0,
    .data = {0}
#else   // IRR_DEBUG
    .debug_h ={0},
    .debug_l ={0},
    .debug_l_length = 0,
    .debug_h_length = 0
#endif  // IRR_DEBUG
};
#define DATA irr_data

void ir_receiver_pwa_isr(void)
{
#ifndef IRR_DEBUG
    if( DATA.error != IRR_ERROR_NONE )
        return;
#endif  // IRR_DEBUG

    DATA.width_h = SMT1CPWH << 8 | SMT1CPWL;

#ifndef IRR_DEBUG
    switch( DATA.state )
    {
        case IRR_STATE_IDLE:
            DATA.width_target0 = PARAMS[IRR_TYPE_NEC].leader_h.typ;
            DATA.width_target1 = PARAMS[IRR_TYPE_AEHA].leader_h.typ;
            if(    DATA.width_h >= PARAMS[IRR_TYPE_NEC].leader_h.min
                && DATA.width_h <= PARAMS[IRR_TYPE_NEC].leader_h.max )
            {
                DATA.type = IRR_TYPE_NEC;
                DATA.state = IRR_STATE_LEADER;
            }
            else if(    DATA.width_h >= PARAMS[IRR_TYPE_AEHA].leader_h.min
                     && DATA.width_h <= PARAMS[IRR_TYPE_AEHA].leader_h.max )
            {
                DATA.type = IRR_TYPE_AEHA;
                DATA.state = IRR_STATE_LEADER;
            }
            else
            {
                DATA.error = IRR_ERROR_LEADER_H;
            }
            break;

        case IRR_STATE_DATA:
            DATA.width_target0 = PARAMS[DATA.type].data_th_h.typ;
            DATA.width_target1 = PARAMS[DATA.type].data_th_h.max;
            if( DATA.width_h < PARAMS[DATA.type].data_th_h.typ )
            {
                // OK。何もしない
            }
            else
            {
                DATA.error = IRR_ERROR_DATA_H;
            }
            break;

        default:
            DATA.error = IRR_ERROR_STATE_H;
            break;
    }

    if( DATA.state == IRR_STATE_LEADER )
        LATCbits.LATC3 = 1;

#else   // IRR_DEBUG
    if( DATA.debug_h_length < 48 ) DATA.debug_h[DATA.debug_h_length++] = DATA.width_h;
#endif  // IRR_DEBUG
}

void ir_receiver_pra_isr(void)
{
#ifndef IRR_DEBUG
    if( DATA.error != IRR_ERROR_NONE )
        return;
#endif  // IRR_DEBUG
    DATA.width_l = SMT1CPRH << 8 | SMT1CPRL;
#ifndef IRR_DEBUG
    switch( DATA.state )
    {
        case IRR_STATE_LEADER:
            DATA.width_target0 = PARAMS[DATA.type].leader_l.typ;
            DATA.width_target1 = 0;
            if(    DATA.width_l >= PARAMS[DATA.type].leader_l.min
                && DATA.width_l <= PARAMS[DATA.type].leader_l.max )
            {
                DATA.state = IRR_STATE_DATA;
            }
            break;

        case IRR_STATE_DATA:
            DATA.width_target0 = PARAMS[DATA.type].data_th_l.typ;
            DATA.width_target1 = PARAMS[DATA.type].data_th_l.max;
            if( DATA.width_l < PARAMS[DATA.type].data_th_l.typ )
            {
                DATA.work_byte &= ~(1 << DATA.work_bitpos);
            }
            else if(    DATA.width_l >= PARAMS[DATA.type].data_th_l.typ
                     && DATA.width_l <= PARAMS[DATA.type].data_th_l.max )
            {
                DATA.work_byte |= (1 << DATA.work_bitpos);
            }
            else
            {
                DATA.error = IRR_ERROR_DATA_L;
                break;
            }
            DATA.work_bitpos++;
            if( DATA.work_bitpos >= 8 )
            {
                if( DATA.work_length < DATA_MAXLEN )
                {
                    DATA.work[DATA.work_length++] = DATA.work_byte;
                }
                else
                {
                    DATA.error = IRR_ERROR_DATA_OVERRUN;
                }
                DATA.work_byte = 0;
                DATA.work_bitpos = 0;
            }
            break;

        default:
            DATA.error = IRR_ERROR_STATE_L;
            break;
    }

#else   // IRR_DEBUG
    if( DATA.debug_l_length < 48 ) DATA.debug_l[DATA.debug_l_length++] = DATA.width_l;
#endif  // IRR_DEBUG
}

void ir_receiver_isr(void)
{
    NOP();
#ifndef IRR_DEBUG
    if( DATA.error == IRR_ERROR_NONE && DATA.state == IRR_STATE_DATA )
    {
        NOP();
    }

    DATA.work_byte = 0;
    DATA.work_length = 0;
    DATA.work_bitpos = 0;
    DATA.error = IRR_ERROR_NONE;
    DATA.state = IRR_STATE_IDLE;
#else   // IRR_DEBUG
    DATA.debug_l_length = 0;
    DATA.debug_h_length = 0;
#endif  // IRR_DEBUG
    LATCbits.LATC3 = 0;
    SMT1CON1bits.GO = 0;
    SMT1STAT = 0xD0;
    while((SMT1STAT & 0xD0) != 0);
    SMT1CON1bits.GO = 1;
}

void ir_receiver_init(void)
{
    SMT1CON0 = 0x08;    // EN=0, (0), STP=0, WPOL=0, SPOL=1, CPOL=0, PS=00 ... 1:1
    SMT1CON1 = 0x43;    // GO=0, REPEAT=1, (0), (0), MODE=0011 (High and Low Measurement Mode)
    SMT1STAT = 0xD0;    // CPRUP=1, CPWUP=1, (0), RST=1, (0), (TS=0), (WS=0), (AS=0)
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
    while((SMT1STAT & 0xD0) != 0);
    SMT1CON1bits.GO = 1;
}