/*
 * File:   avr-main.c
 * Author: hseiyama
 *
 * Created on 2023/07/16, 1:42
 */


#include <avr/io.h>
#include <avr/interrupt.h>

#define PORTD_MASK_IN    (0b00001100)   // PortD2,3: 入力
#define PORTD_MASK_OUT   (0b10010000)   // PortD4,7: 出力
#define PORTD_MASK_PWM   (0b01100000)   // PortD5,6: PWM

volatile uint8_t portD_timer;           // タイマ割り込みからの指示用
volatile uint8_t usart_data;            // USART送受信データ
volatile uint16_t adc0_data;            // ADC0入力データ

int main(void) {
    /* Replace with your application code */
    uint8_t pinD2_in;           // PIND2入力用
    uint8_t pinD3_in;           // PIND3入力用
    
    /* 初期化 */
    portD_timer = 0x00;
    usart_data = 0x00;
    /* PORT初期化 */
    DDRD = PORTD_MASK_OUT | PORTD_MASK_PWM;
                                // 出力方向 (4,5,6,7bit)
    PORTD |= PORTD_MASK_IN;     // プルアップ抵抗を有効 (2,3bit)
    /* TIMER初期化 */
    TCCR1A = 0b00000000;        // 比較一致ﾀｲﾏ/ｶｳﾝﾀ解除(CTC)動作
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);
                                // clkI/O/1024 (1024分周)
    OCR1A = 15624;              // 1000msごとに割り込み (OCR1AH,OCR1AL)
    TIMSK1 = (1 << OCIE1A);     // ﾀｲﾏ/ｶｳﾝﾀ1比較A割り込み許可
    /* USART初期化 */
    UBRR0 = 103;                // ボーレート9600bps (16MHz,U2X=0,誤差=0.2%)
    UCSR0A = 0b00000000;        // USART制御/状態ﾚｼﾞｽﾀA
    UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
                                // 受信完了割り込み許可
                                // 受信許可, 送信許可
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
                                // 非同期動作,ﾊﾟﾘﾃｨ禁止,停止1ﾋﾞｯﾄ,ﾃﾞｰﾀ8ﾋﾞｯﾄ
    /* 外部割り込み初期化 */
    EICRA = (1 << ISC11) | (1 << ISC00);
                                // INT1ﾋﾟﾝの下降端で発生
                                // INT0ﾋﾟﾝの論理変化(両端)
    EIMSK = (1 << INT1) | (1 << INT0);
                                // 外部割り込み0許可
                                // 外部割り込み1許可
    /* ADC初期化 */
    ADMUX = (1 << REFS0);       // 基準電圧=AVCC, A/Dﾁｬﾈﾙ=ADC0
    ADCSRA = (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
                                // A/D許可, A/D変換完了割り込み許可, A/D変換ｸﾛｯｸ=CK/128
    /* PWM初期化 */
	TCCR0A = (1 << COM0A1) | (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
                                // 比較一致でLow、BOTTOMでHighをOC1xﾋﾟﾝへ出力 (非反転動作)
	TCCR0B = (1 << CS02) | (1 << CS00);
                                // 8ﾋﾞｯﾄ高速PWM動作, clkI/O/1024 (1024分周)
    OCR0A = 127;                // Duty比50% (初期値)
    OCR0B = 127;                // Duty比50% (初期値)
                                // 【補足1】WGM02-0=111で上限値をOCR0Aに設定可能
                                //          その場合、OC0Aﾋﾟﾝの出力は常に100%
                                // 【補足2】Duty比0%でも完全消灯にはならない
    
    sei();                      // 割り込み許可
    while (1) {
        /* PORT出力 */
        pinD2_in = ((~PIND >> PIND2) ^ portD_timer) & 0x01;
        pinD3_in = ((~PIND >> PIND3) ^ portD_timer) & 0x01;
        PORTD = (pinD3_in << PIND7) | (pinD2_in << PIND4) | (PORTD & ~PORTD_MASK_OUT);
                                // PIND2,3 -> PORTD4,7
    }
}

/* USART送信 (1文字) */
static void send_char(uint8_t data) {
    // USART送信ﾃﾞｰﾀ ﾚｼﾞｽﾀ空きﾌﾗｸﾞ=1 まで待機
    while(!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;                // 送信データを設定
}

/* USART送信 (文字列) */
static void send_strg(uint8_t *data) {
    // 文字がNULLになるまで継続
    while(*data != 0x00) {
        send_char(*data);       // USART送信 (1文字)
        data++;
    }
}

/* HEX数字送信 */
static void echo_hex(uint8_t hex_data) {
    uint8_t hex_table[] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };
  send_char(hex_table[(hex_data >> 4) & 0x0F]);
  send_char(hex_table[hex_data & 0x0F]);
}

/* 16進数データ送信 (2byte) */
static void echo_2byte(uint16_t data) {
    uint8_t msg[] = "0x";
    send_strg(msg);
    echo_hex((data >> 8) & 0xFF);   // HEX数字送信
    echo_hex(data & 0xFF);          // HEX数字送信
}

/* タイマ割り込み */
ISR(TIMER1_COMPA_vect, ISR_BLOCK) {
    portD_timer = ~portD_timer;
    ADCSRA |= (1 << ADSC);      // A/D変換開始
}

/* USART受信完了割り込み */
ISR(USART_RX_vect, ISR_BLOCK) {
    usart_data = UDR0;          // 受信データを取得
    send_char(usart_data);      // USART送信 (1文字)
}

/* 外部割り込み0 */
ISR(INT0_vect, ISR_BLOCK) {
    uint8_t msg[] = ">ExtInt0\r\n";
    send_strg(msg);             // USART送信 (文字列)
}

/* 外部割り込み1 */
ISR(INT1_vect, ISR_BLOCK) {
    uint8_t msg[] = ">ExtInt1\r\n";
    send_strg(msg);             // USART送信 (文字列)
}

/* A/D変換完了割り込み */
ISR(ADC_vect, ISR_BLOCK) {
    uint8_t msg1[] = ">ADC0=";
    uint8_t msg2[] = "\r\n";
    adc0_data = ADC;            // A/D変換結果
    send_strg(msg1);
    echo_2byte(adc0_data);      // 16進数データ送信 (2byte)
    send_strg(msg2);
    OCR0A = (adc0_data >> 2) & 0xFF;
                                // ﾀｲﾏ/ｶｳﾝﾀ0 比較Aﾚｼﾞｽﾀ
    OCR0B = ~(adc0_data >> 2) & 0xFF;
                                // ﾀｲﾏ/ｶｳﾝﾀ0 比較Bﾚｼﾞｽﾀ
}
