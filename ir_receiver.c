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
#include "main.h"
#include "ir_receiver.h"

#define IRR_DUMP            // 受信結果のダンプ
//#define IRR_REPEAT_CHECK   // 2回連続で同じデータかのチェック

#ifdef IRR_DUMP
#include <stdio.h>
#endif

#define SMTCLK              500E+3      // MFINTOSC(500kHz), CSEL=100
#define SMTCLK_PS           1           // 1:1, PS=00
#define TMRCLK              31.25E+3    // MFINTOSC(31.25kHz), CS=0110
#define TMRCLK_PS           128         // 1:128, CKPS=111, OUTPS=0000

#define T_NEC               562E-6      // NECフォーマットの単位時間, T=562us
#define T_AEHA              425E-6      // 家製協フォーマットの単位時間, T=425us

#define T_LEADER_COEFF_MIN  0.9
#define T_LEADER_COEFF_MAX  1.1

#define DATA_MAXLEN         48          // 最大データ長
#define DATA_EXTEND_MAX     4           // 連続してLeaderが来る場合の最大カウント
#define DATA_MAXLEN_DEBUG   48          // デバッグ用の最大データ長

#define SMT_COUNT(T)    ((int)(((double)(T)) * (SMTCLK / SMTCLK_PS)))           // 引数は定数で指定
#define TMR_COUNT(T)    ((unsigned char)(((double)(T)) * (TMRCLK / TMRCLK_PS))) // 引数は定数で指定

#define SMT_TIMEOUT         SMT_COUNT(T_NEC * 20)   // データの終了を判断する時間
#define REPEAT_TIMEOUT      TMR_COUNT(300E-3)       // リピートの終了を判断する時間

typedef enum {
    IRR_STATE_IDLE = 0,
    IRR_STATE_LEADER,
    IRR_STATE_DATA,
} irr_state_t;

typedef enum {
    IRR_TYPE_NEC = 0,
    IRR_TYPE_AEHA,
    IRR_TYPE_SONY,  // 未実装
} irr_type_t;

typedef enum {
    IRR_ERROR_NONE = 0,
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
    int max;
} irr_minmax_t;

typedef struct {
    irr_minmax_t    leader_h;
    irr_minmax_t    leader_l;
    int             data_th;
    int             data_max;
} irr_param_t;

const irr_param_t irr_params[] = {
    [IRR_TYPE_NEC] = {
       .leader_h = { 
            .min =  SMT_COUNT(T_NEC * 16 * T_LEADER_COEFF_MIN),
            .max =  SMT_COUNT(T_NEC * 16 * T_LEADER_COEFF_MAX)
        },
       .leader_l = {
            .min =  SMT_COUNT(T_NEC * 8 * T_LEADER_COEFF_MIN),
            .max =  SMT_COUNT(T_NEC * 8 * T_LEADER_COEFF_MAX)
        },
       .data_th =   SMT_COUNT(T_NEC * 2                     ),
       .data_max =  SMT_COUNT(T_NEC * 4                     ),
    },
    [IRR_TYPE_AEHA] = {
       .leader_h = { 
            .min =  SMT_COUNT(T_AEHA * 8 * T_LEADER_COEFF_MIN),
            .max =  SMT_COUNT(T_AEHA * 8 * T_LEADER_COEFF_MAX)
        },
       .leader_l = {
            .min =  SMT_COUNT(T_AEHA * 4 * T_LEADER_COEFF_MIN),
            .max =  SMT_COUNT(T_AEHA * 4 * T_LEADER_COEFF_MAX)
        },
       .data_th =   SMT_COUNT(T_AEHA * 2                     ),
       .data_max =  SMT_COUNT(T_AEHA * 4                     ),
    }
};
#define PARAMS irr_params

typedef struct {
    irr_type_t  type;
    char        length;
    char        data[DATA_MAXLEN];
    char        extended_count;
    char        extended[DATA_EXTEND_MAX];
} irr_data_analyze_result_t;

typedef struct {
    irr_state_t                 state;
    irr_error_t                 error;
    char                        received;
    char                        work_bitpos;
    char                        work_byte;
    irr_data_analyze_result_t   work;
    irr_data_analyze_result_t   last;
} irr_data_analyze_t;

typedef struct {
    char                        l_length;
    int                         l_time[DATA_MAXLEN_DEBUG];
    char                        h_length;
    int                         h_time[DATA_MAXLEN_DEBUG];
} irr_data_measurement_t;

typedef struct {
    volatile char               processing;
    irr_mode_t                  mode;
    int                         width_h;
    int                         width_l;
    union {
        irr_data_analyze_t      analyze;
        irr_data_measurement_t  measurement;
    } u;
} irr_data_t;

irr_data_t irr_data;
#define DATA    irr_data
#define DATA_A  DATA.u.analyze
#define DATA_M  DATA.u.measurement

void ir_receiver_pwa_isr(void)
{
    DATA.width_h = SMT1CPWH << 8 | SMT1CPWL;
    DATA.processing = 1;

    if( COMMON.received == 0 )
    {
        if( DATA.mode == IRR_MODE_ANALIZE )
        {
            if( DATA_A.error != IRR_ERROR_NONE || DATA_A.received != 0 )
                return;

            switch( DATA_A.state )
            {
                case IRR_STATE_IDLE:
                    if(    DATA.width_h >= PARAMS[IRR_TYPE_NEC].leader_h.min
                        && DATA.width_h <= PARAMS[IRR_TYPE_NEC].leader_h.max )
                    {
                        DATA_A.work.type = IRR_TYPE_NEC;
                        DATA_A.state = IRR_STATE_LEADER;
                    }
                    else if(    DATA.width_h >= PARAMS[IRR_TYPE_AEHA].leader_h.min
                            && DATA.width_h <= PARAMS[IRR_TYPE_AEHA].leader_h.max )
                    {
                        DATA_A.work.type = IRR_TYPE_AEHA;
                        DATA_A.state = IRR_STATE_LEADER;
                    }
                    else
                    {
                        DATA_A.error = IRR_ERROR_LEADER_H;
                    }
                    break;

                case IRR_STATE_DATA:
                    if( DATA.width_h < PARAMS[DATA_A.work.type].data_th )
                    {
                        // OK。何もしない
                    }
                    else
                    {
                        DATA_A.error = IRR_ERROR_DATA_H;
                    }
                    break;

                default:
                    DATA_A.error = IRR_ERROR_STATE_H;
                    break;
            }
        }
        else if (DATA.mode == IRR_MODE_MEASUREMENT )
        {
            if( DATA_M.h_length < DATA_MAXLEN_DEBUG )
                DATA_M.h_time[DATA_M.h_length++] = DATA.width_h;
        }
        else
        {
        }
    }
}

void ir_receiver_pra_isr(void)
{
    DATA.width_l = SMT1CPRH << 8 | SMT1CPRL;
    DATA.processing = 1;

    if( COMMON.received == 0 )
    {
        if( DATA.mode == IRR_MODE_ANALIZE )
        {
            if( DATA_A.error != IRR_ERROR_NONE || DATA_A.received != 0 )
                return;

            switch( DATA_A.state )
            {
                case IRR_STATE_LEADER:
                    if(    DATA.width_l >= PARAMS[DATA_A.work.type].leader_l.min
                        && DATA.width_l <= PARAMS[DATA_A.work.type].leader_l.max )
                    {
                        DATA_A.state = IRR_STATE_DATA;
                        LED2 = 1;
                    }
                    break;

                case IRR_STATE_DATA:
                    if( DATA.width_l < PARAMS[DATA_A.work.type].data_th )
                    {
                        DATA_A.work_byte &= ~(1 << DATA_A.work_bitpos);
                    }
                    else if(    DATA.width_l >= PARAMS[DATA_A.work.type].data_th
                             && DATA.width_l <= PARAMS[DATA_A.work.type].data_max )
                    {
                        DATA_A.work_byte |= (1 << DATA_A.work_bitpos);
                    }
                    else
                    {
                        if( DATA_A.work.type == IRR_TYPE_AEHA )
                        {
                            // 8ms以上のL期間(Trailer)の後に再度Leaderが来る場合は
                            // IDLEステートに戻して続きのデータを受信する
                            if( DATA_A.work.extended_count < DATA_EXTEND_MAX )
                            {
                                DATA_A.work.extended[DATA_A.work.extended_count++] = DATA_A.work.length;
                                DATA_A.state = IRR_STATE_IDLE;
                            }
                            else
                            {
                                DATA_A.error = IRR_ERROR_DATA_L;
                            }
                        }
                        else
                        {
                            DATA_A.error = IRR_ERROR_DATA_L;
                        }
                        break;
                    }
                    DATA_A.work_bitpos++;
                    if( DATA_A.work_bitpos >= 8 )
                    {
                        if( DATA_A.work.length < DATA_MAXLEN )
                        {
                            DATA_A.work.data[DATA_A.work.length++] = DATA_A.work_byte;
                        }
                        else
                        {
                            DATA_A.error = IRR_ERROR_DATA_OVERRUN;
                        }
                        DATA_A.work_byte = 0;
                        DATA_A.work_bitpos = 0;
                    }
                    break;

                default:
                    DATA_A.error = IRR_ERROR_STATE_L;
                    break;
            }
        }
        else if( DATA.mode == IRR_MODE_MEASUREMENT )
        {
            if( DATA_M.l_length < DATA_MAXLEN_DEBUG )
                DATA_M.l_time[DATA_M.l_length++] = DATA.width_l;
        }
        else
        {
        }
    }

}

void ir_receiver_isr(void)
{
    DATA.processing = 1;

    if( COMMON.received == 0 )
    {
        if( DATA.mode == IRR_MODE_ANALIZE )
        {
            if( DATA_A.received == 0 )
            {
                if( DATA_A.error == IRR_ERROR_NONE && DATA_A.state == IRR_STATE_DATA )
                {
                    // 受信成功
                    if( DATA_A.work.type == IRR_TYPE_NEC && DATA_A.work.length == 4 )
                    {
                        if( DATA_A.work.data[2] != (DATA_A.work.data[3] ^ 0xFF) )
                        {
                            DATA_A.error = IRR_ERROR_DATA_CHECK;
                        }
                    }
                    else if( DATA_A.work.type == IRR_TYPE_AEHA && DATA_A.work.length >= 4 )
                    {
                        if( ((DATA_A.work.data[0] & 0x0F)
                            ^ ((DATA_A.work.data[0] >> 4) & 0x0F)
                            ^ (DATA_A.work.data[1] & 0x0F)
                            ^ ((DATA_A.work.data[1] >> 4) & 0x0F)) != (DATA_A.work.data[2] & 0x0F) )
                        {
                            DATA_A.error = IRR_ERROR_DATA_CHECK;
                        }
                    }
                    else
                    {
                        DATA_A.error = IRR_ERROR_DATA_CHECK;
                    }

                    if( DATA_A.error == IRR_ERROR_NONE )
                    {
                        if( DATA_A.last.length != 0 )
                        {
#ifdef IRR_REPEAT_CHECK
                            // 2回連続で同じデータを受信した場合は受信完了
                            if( c_memcmp(&DATA_A.last, &DATA_A.work, sizeof(DATA_A.work)) == 0 )
                            {
                                DATA_A.received = 1;
                                COMMON.received = 1;
                            }
#else
                            DATA_A.received = 1;
                            COMMON.received = 1;
#endif  // IRR_REPEAT_CHECK
                        }
                        c_memcopy(&DATA_A.last, &DATA_A.work, sizeof(DATA_A.last));
                    }
                    else
                    {
                        // データチェック失敗
                    }
                }
                else
                {
                    // 受信失敗
                }

                c_memzero(&DATA_A.work, sizeof(DATA_A.work));
                DATA_A.work_byte = 0;
                DATA_A.work_bitpos = 0;
                DATA_A.error = IRR_ERROR_NONE;
                DATA_A.state = IRR_STATE_IDLE;
            }
        }
        else if( DATA.mode == IRR_MODE_MEASUREMENT )
        {
            COMMON.received = 1;
        }
        else
        {
        }
    }

    SMT1CON1bits.GO = 0;
    SMT1STAT = 0xD0;
    while((SMT1STAT & 0xD0) != 0);
    SMT1CON1bits.GO = 1;

    T4CONbits.ON = 0;
    T4TMR = 0x00;
    T4CONbits.ON = 1;
}

void ir_receiver_tmr_isr(void)
{
    if( DATA.mode == IRR_MODE_ANALIZE )
    {
        c_memzero(&DATA_A.work, sizeof(DATA_A.work));
        DATA_A.work_byte = 0;
        DATA_A.work_bitpos = 0;
        DATA_A.error = IRR_ERROR_NONE;
        DATA_A.state = IRR_STATE_IDLE;
        DATA_A.received = 0;
    }
    else if ( DATA.mode == IRR_MODE_MEASUREMENT )
    {
        DATA_M.l_length = 0;
        DATA_M.h_length = 0;
    }
    else
    {
    }

    LED2 = 0;
    DATA.processing = 0;
}

void ir_receiver_dump(void)
{
    char i;
    char *t;

    if( COMMON.received != 0 )
    {
#ifdef IRR_DUMP
        printf("=== Data received ===\r\n");
        if( DATA.mode == IRR_MODE_ANALIZE )
        {
            switch( DATA_A.last.type )
            {
            case IRR_TYPE_NEC:
                t = "NEC";
                break;
            case IRR_TYPE_AEHA:
                t = "AEHA";
                break;
            case IRR_TYPE_SONY:
                t = "SONY";
                break;
            default:
                t = "UNKNOWN";
                break;
            }

            printf("Type       : %s\r\n", t);
            printf("Length     : %02X\r\n", DATA_A.last.length);
            printf("Ext. count : %02X\r\n", DATA_A.last.extended_count);
            printf("Data       : ");
            for( i=0; i < DATA_A.last.length; i++)
            {
                printf("%02X ",DATA_A.last.data[i]);
            }
            printf("\r\n");
        }
        else if (DATA.mode == IRR_MODE_MEASUREMENT )
        {
            for( i=0; i<DATA_MAXLEN_DEBUG; i++ )
            {
                if( i < DATA_M.h_length || i < DATA_M.l_length )
                {
                    printf("%02X: ", i);
                    if( i < DATA_M.h_length )
                    {
                        printf("%04X ", DATA_M.h_time[i]);
                    }
                    else
                    {
                        printf("---- ");
                    }
                    if( i < DATA_M.l_length )
                    {
                        printf("%04X ", DATA_M.l_time[i]);
                    }
                    else
                    {
                        printf("---- ");
                    }
                    printf("\r\n");
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
        }
#endif  // IRR_DUMP
        c_memzero(&DATA_A.last, sizeof(DATA_A.last));
        COMMON.received = 0;
    }
}

void ir_receiver_set_mode(irr_mode_t mode)
{
    while( DATA.processing != 0 );
    DATA.mode = mode;
}

void ir_receiver_init(void)
{
    SMT1CON0 = 0x08;    // EN=0, (0), STP=0, WPOL=0, SPOL=1, CPOL=0, PS=00 ... 1:1
    SMT1CON1 = 0x43;    // GO=0, REPEAT=1, (0), (0), MODE=0011 (High and Low Measurement Mode)
    SMT1STAT = 0xD0;    // CPRUP=1, CPWUP=1, (0), RST=1, (0), (TS=0), (WS=0), (AS=0)
    SMT1CLK = 0x04;     // (0), (0), (0), (0), (0), CSEL=100 ... MFINTOSC(500kHz)
    SMT1SIG = 0x00;     // (0), (0), (0), SSEL=00000 ... SMT1SIGPPS
    SMT1WIN = 0x00;     // (0), (0), (0), WSEL=00000 ... SMT1WINPPS(Unused)
    SMT1PRL = SMT_TIMEOUT & 0xFF;
    SMT1PRH = (SMT_TIMEOUT >> 8) & 0xFF;
    SMT1PRU = 0x00;
 
    PIR8bits.SMT1IF = 0;
    PIR8bits.SMT1PRAIF = 0;
    PIR8bits.SMT1PWAIF = 0;
    PIE8bits.SMT1IE = 1;
    PIE8bits.SMT1PRAIE = 1;
    PIE8bits.SMT1PWAIE = 1;

    T4CON = 0x70;       // ON=0, CKPS=111, OUTPS=0000 ... 1:128
    T4HLT = 0x08;       // PSYNC=0, CPOL=0, CSYNC=0, MODE=01000 (Software Start One Shot Mode)
    T4CLKCON = 0x06;    // (0), (0), (0), (0), CS=0110 ... MFINTOSC(31.25kHz)
    T4PR = REPEAT_TIMEOUT;

    c_memzero(&DATA, sizeof(DATA));
    DATA.mode = IRR_MODE_ANALIZE;

    PIR4bits.TMR4IF = 0;
    PIE4bits.TMR4IE = 1;

    SMT1CON0bits.EN = 1;
    while((SMT1STAT & 0xD0) != 0);
    SMT1CON1bits.GO = 1;
}

