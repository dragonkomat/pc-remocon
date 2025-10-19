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
#include "buzzer.h"
#include "interrupts.h"
#include "ir_receiver.h"
#include "pins.h"

#include <pic.h>

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

typedef enum {
    CMD_NONE = 0,
    CMD_OFF,
    CMD_ON,
    CMD_LONGPUSH,
} pcremocon_cmd_t;

volatile irr_common_data_t irr_common_data = {
    .received = 0,
    .keycode = KEYCODE_NONE
};

void init()
{
    pins_init();

    buzzer_init();
    ir_receiver_init();

    interrupts_init();
}

int main()
{
    pcremocon_cmd_t cmd;
    int i;

    init();

    while(1)
    {
        // PWM使用時はSLEEPは使用不可(発振が止まる)
        //SLEEP();

        if( COMMON.received != 0 )
        {
            LED1 = 1;

            // 動作判定
            cmd = CMD_NONE;
            if( COMMON.keycode == KEYCODE_OFF )
            {
                //buzzer_on(BZR_FREQ2CNT(523));
                if( PC_POWER_LED_N == 0 )
                {
                    cmd = CMD_OFF;
                }
            }
            else if ( COMMON.keycode == KEYCODE_FAVORITE )
            {
                //buzzer_on(BZR_FREQ2CNT(587));
            }
            else if ( COMMON.keycode == KEYCODE_NIGHTLIGHT )
            {
                //buzzer_on(BZR_FREQ2CNT(659));
                cmd = CMD_LONGPUSH;
            }
            else if ( COMMON.keycode == KEYCODE_MINUS )
            {
                //buzzer_on(BZR_FREQ2CNT(698));
            }
            else if ( COMMON.keycode == KEYCODE_PLUS )
            {
                //buzzer_on(BZR_FREQ2CNT(783));
            }
            else if ( COMMON.keycode == KEYCODE_ALL )
            {
                //buzzer_on(BZR_FREQ2CNT(880));
                if( PC_POWER_LED_N != 0 )
                {
                    cmd = CMD_ON;
                }
            }

            // 動作実行
            if( cmd == CMD_OFF )
            {
                PC_POWER_SW = 1;
                buzzer_on(BZR_FREQ2CNT(2000));
                __delay_ms(400);
                buzzer_off();
                __delay_ms(100);
                PC_POWER_SW = 0;
            }
            else if( cmd == CMD_ON )
            {
                PC_POWER_SW = 1;
                buzzer_on(BZR_FREQ2CNT(2000));
                __delay_ms(200);
                buzzer_on(BZR_FREQ2CNT(1000));
                __delay_ms(200);
                buzzer_off();
                __delay_ms(100);
                PC_POWER_SW = 0;
            }
            else if( cmd == CMD_LONGPUSH )
            {
                PC_POWER_SW = 1;
                for( i=0; i<120/2; i++ )
                {
                    LED1 = 1;
                    buzzer_on(BZR_FREQ2CNT(2000));
                    __delay_ms(100);
                    LED1 = 0;
                    buzzer_off();
                    __delay_ms(100);
                }
                buzzer_off();
                PC_POWER_SW = 0;
            }
            else
            {
                buzzer_on(BZR_FREQ2CNT(1000));
                __delay_ms(100);
                buzzer_off();        
            }
            LED1 = 0;
            COMMON.received = 0;
        }
    }
    return 0;
};
