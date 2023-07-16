/*
 * File:   avr-main.c
 * Author: hseiyama
 *
 * Created on 2023/07/16, 1:42
 */


#include <avr/io.h>

#define PORTD_MASK_IN    (0b00110000)   // portD : 4,5bit目を入力
#define PORTD_MASK_OUT   (0b00001100)   // portD : 2,3bit目のみ出力

int main(void) {
    /* Replace with your application code */
    uint8_t portD_in;
    DDRD = PORTD_MASK_OUT;      // 出力方向 (2,3bit)
    PORTD |= PORTD_MASK_IN;     // プルアップ抵抗を有効 (4,5bit)
    
    while (1) {
        portD_in = (~PIND & PORTD_MASK_IN);
        PORTD = (portD_in >> 2) | (PORTD & ~PORTD_MASK_OUT); // PIND4,5 -> PORTD2,3;
    }
}
