/*
 * File:   avr-main.c
 * Author: hseiyama
 *
 * Created on 2023/07/16, 1:42
 */


#include <avr/io.h>
#include <avr/interrupt.h>

#define PORTD_MASK_IN    (0b00110000)   // portD : 4,5bit目を入力
#define PORTD_MASK_OUT   (0b00001100)   // portD : 2,3bit目のみ出力

volatile uint8_t portD_timer;           // タイマ割り込みからの指示用
volatile uint8_t usart_data;            // USART送受信データ

int main(void) {
    /* Replace with your application code */
    uint8_t portD_in;           // PortD入力用
    
    /* 初期化 */
    portD_timer = 0x00;
    usart_data = 0x00;
    /* PORT初期化 */
    DDRD = PORTD_MASK_OUT;      // 出力方向 (2,3bit)
    PORTD |= PORTD_MASK_IN;     // プルアップ抵抗を有効 (4,5bit)
    /* TIMER初期化 */
    TCCR1A = 0b00000000;        // 比較一致ﾀｲﾏ/ｶｳﾝﾀ解除(CTC)動作
    TCCR1B = (1 << WGM12) | (1 << CS11) | (1 << CS10);
                                // clkI/O/64 (64分周)
    OCR1A = 15625;              // 1000msごとに割り込み (OCR1AH,OCR1AL)
    TIMSK1 = (1 << OCIE1A);     // ﾀｲﾏ/ｶｳﾝﾀ1比較A割り込み許可
    /* USART初期化 */
    UBRR0 = 12;                 // ボーレート9600bps (1MHz,U2X=1,誤差=0.2%)
    UCSR0A = (1 << U2X0);       // 倍速許可
    UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
                                // 受信完了割り込み許可
                                // 受信許可, 送信許可
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
                                // 非同期動作,ﾊﾟﾘﾃｨ禁止,停止1ﾋﾞｯﾄ,ﾃﾞｰﾀ8ﾋﾞｯﾄ
    
    sei();                      // 割り込み許可
    while (1) {
        portD_in = (~PIND & PORTD_MASK_IN) ^ portD_timer;
        PORTD = (portD_in >> 2) | (PORTD & ~PORTD_MASK_OUT); // PIND4,5 -> PORTD2,3;
    }
}

/* タイマ割り込み */
ISR(TIMER1_COMPA_vect, ISR_BLOCK) {
    portD_timer = ~portD_timer;
}

/* USART受信完了割り込み */
ISR(USART_RX_vect, ISR_BLOCK) {
  usart_data = UDR0;    // 受信データを取得
  UDR0 = usart_data;    // 送信データを設定
}
