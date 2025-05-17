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
#include "buzzer.h"

void buzzer_init(void)
{
    NCO1CON = 0x00; // EN=0, (0), OUT=0, POL=0, (0), (0), (0), PFM=0
    NCO1CLK = 0x03; // PWS=000, (0), CKS=0011
}

void buzzer_on(unsigned short freq)
{
    NCO1CONbits.EN = 0;
    NCO1ACCU = 0;
    NCO1ACCH = 0;
    NCO1ACCL = 0;
    NCO1INCH = freq >> 8;
    NCO1INCL = freq & 0xFF;
    NCO1CONbits.EN = 1;
}

void buzzer_off(void)
{
    NCO1CONbits.EN = 0;
}
