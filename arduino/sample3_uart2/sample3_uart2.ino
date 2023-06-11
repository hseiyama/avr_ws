uint8_t data = 0;         // 受信データ

void setup() {
  // put your setup code here, to run once:
  UBRR0 = 103;            // USARTﾎﾞｰﾚｰﾄ ﾚｼﾞｽﾀ (UBRR0H, UBRR0L)
                          // ボーレート9600bps (16MHz, U2X=0)
  UCSR0A = 0b00000000;    // USART制御/状態ﾚｼﾞｽﾀA
//                          // 送信有効になると00100000に変化 (USART送信ﾃﾞｰﾀ ﾚｼﾞｽﾀ空きﾌﾗｸﾞ)
  UCSR0B = 0b10011000;    // USART制御/状態ﾚｼﾞｽﾀB
                          // 受信完了割り込み許可
                          // 【注意】送信完了割り込み許可を設定すると割込みが連続で発生
                          // 【注意】送信ﾃﾞｰﾀ ﾚｼﾞｽﾀ空き割り込み許可を設定すると割込みが連続で発生
                          // 受信許可, 送信許可
  UCSR0C = 0b00000110;    // USART制御/状態ﾚｼﾞｽﾀC
                          // 非同期動作, ﾊﾟﾘﾃｨ禁止, 停止1ﾋﾞｯﾄ, ﾃﾞｰﾀ8ﾋﾞｯﾄ
}

void loop() {
  // put your main code here, to run repeatedly:
}

ISR(USART_RX_vect)  // USART 受信完了
{
  data = UDR0;  // 受信データを格納
  UDR0 = data;  //受信データを送信
}

/*
ISR(USART_TX_vect)  // USART 送信完了
{
  UDR0 = '1';
}
*/

/*
ISR(USART_UDRE_vect)  // USART 送信緩衝部空き
{
  UDR0 = '2';
}
*/
