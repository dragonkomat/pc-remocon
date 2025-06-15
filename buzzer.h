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

#ifndef _IR_REMOCON_ANALYZER_BUZZER_H_
#define _IR_REMOCON_ANALYZER_BUZZER_H_

void buzzer_init(void);
void buzzer_on(unsigned int cnt);
void buzzer_off(void);

#define BZR_NCOCLK      500E3   // 500KHz, PFM=0
#define BZR_FREQ2CNT(F) ((unsigned int)((((double)(F)) / BZR_NCOCLK) * (1L << 20) * 2)) // 引数は定数で指定

#endif // _IR_REMOCON_ANALYZER_BUZZER_H_
