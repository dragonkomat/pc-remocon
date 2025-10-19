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

#ifndef _IR_REMOCON_ANALYZER_COMMON_H_
#define _IR_REMOCON_ANALYZER_COMMON_H_
// IWYU pragma: always_keep

#include <xc.h>
#include <pic16f18424.h>

#define _XTAL_FREQ 32000000

#define LED1            LATCbits.LATC3
#define PC_POWER_SW     LATAbits.LATA2
#define PC_POWER_LED_N  PORTCbits.RC0

void c_memzero(void *data, int length);
void c_memcopy(void *dest, const void *src, int length);
int c_memcmp(void *data1, void *data2, int length);

#endif // _IR_REMOCON_ANALYZER_COMMON_H_
