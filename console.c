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
#include "console.h"

#include <stdio.h>

#define TXBUF_MAXLEN 32
#define RXBUF_MAXLEN 32

typedef struct {
    char tx_pos_push;
    char tx_pos_pop;
    char tx_len;
    char tx_buf[TXBUF_MAXLEN];
    char rx_pos_push;
    char rx_pos_pop;
    char rx_len;
    char rx_buf[RXBUF_MAXLEN];
} console_data_t;

volatile console_data_t console_data = {
    .tx_pos_push = 0,
    .tx_pos_pop = 0,
    .tx_len = 0,
    .rx_pos_push = 0,
    .rx_pos_pop = 0,
    .rx_len = 0,
};
#define DATA console_data

void console_rx_isr(void)
{
    if( RC1STAbits.OERR )
    {
        // 何もしない
    }  
    if( RC1STAbits.FERR )
    {
        // 何もしない
    } 
    
    DATA.rx_buf[DATA.rx_pos_push++] = RC1REG;
    if( DATA.rx_pos_push >= RXBUF_MAXLEN )
    {
        DATA.rx_pos_push = 0;
    }
    if( DATA.rx_len < RXBUF_MAXLEN )
    {
        DATA.rx_len++;
    }
    else
    {
        // バッファが満杯の場合は一番古い1文字を捨てる
        DATA.rx_pos_pop++;
        if( DATA.rx_pos_pop >= RXBUF_MAXLEN )
        {
            DATA.rx_pos_pop = 0;
        }        
    }
}

void console_tx_isr(void)
{
    if( DATA.tx_len > 0 )
    {
        TX1REG = DATA.tx_buf[DATA.tx_pos_pop++];
        if( DATA.tx_pos_pop >= TXBUF_MAXLEN )
        {
            DATA.tx_pos_pop = 0;
        }
        DATA.tx_len--;
    }
    else
    {
        PIE3bits.TX1IE = 0;
    }
}

void console_init(void)
{
    PIE3bits.RC1IE = 0;
    BAUD1CON = 0x08;    // (ABDOVF=0), (RCIDL=0), (0), SCKP=0, BRG16=1, (0), WUE=0, ABDEN=0
    RC1STA   = 0x90;    // SPEN=1, RX9=0, SREN=0, CREN=1, ADDEN=0, (FERR=0), OERR=0, RX9D=0
    TX1STA   = 0x24;    // CSRC=0, TX9=0, TXEN=1, SYNC=0, SENDB=0, BRGH=1, (TRMT=0), TX9D=0
    SP1BRGL  = 0x44;    // Fosc=32MHz, (SYNC=x, BRGH=1, BRG16=1 ... x4), SP1BRG=0x0044 ... 115.2kbps
    SP1BRGH  = 0x0;
    PIE3bits.RC1IE = 1;
    PIE3bits.TX1IE = 0;
}

//int getch(void)
int console_getch(void)
{
    int ch;
    di();
    if( DATA.rx_len > 0 )
    {
        ch = DATA.rx_buf[DATA.rx_pos_pop++];
        if( DATA.rx_pos_pop >= RXBUF_MAXLEN )
        {
            DATA.rx_pos_pop = 0;
        }
        DATA.rx_len--;
    }
    else
    {
        ch = 0;
    }
    ei();
    return ch;
}

void putch(char txData)
{
    while(DATA.tx_len >= TXBUF_MAXLEN);
    di();
    if(DATA.tx_len < TXBUF_MAXLEN)
    {
        DATA.tx_buf[DATA.tx_pos_push++] = txData;
        if( DATA.tx_pos_push >= TXBUF_MAXLEN )
        {
            DATA.tx_pos_push = 0;
        }
        DATA.tx_len++;
        PIE3bits.TX1IE = 1;
    }
    ei();
}
