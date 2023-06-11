void setup() {
  // put your setup code here, to run once:
  UBRR0 = 103;            // USARTﾎﾞｰﾚｰﾄ ﾚｼﾞｽﾀ (UBRR0H, UBRR0L)
                          // ボーレート9600bps (16MHz, U2X=0)
  UCSR0A = 0b00000000;    // USART制御/状態ﾚｼﾞｽﾀA
                          // 受信すると10000000に変化 (USART受信完了ﾌﾗｸﾞ)
                          // 送信有効になると00100000に変化 (USART送信ﾃﾞｰﾀ ﾚｼﾞｽﾀ空きﾌﾗｸﾞ)
  UCSR0B = 0b00011000;    // USART制御/状態ﾚｼﾞｽﾀB
                          // 受信許可, 送信許可
  UCSR0C = 0b00000110;    // USART制御/状態ﾚｼﾞｽﾀC
                          // 非同期動作, ﾊﾟﾘﾃｨ禁止, 停止1ﾋﾞｯﾄ, ﾃﾞｰﾀ8ﾋﾞｯﾄ
}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t data;                     // 受信データ

  //受信するまで待つ
  while( !(UCSR0A & 0b10000000) );  // USART受信完了ﾌﾗｸﾞ=1 まで待機
  data = UDR0;                      // 受信データを格納

  //送信できるまで待つ
  while( !(UCSR0A & 0b00100000) );  // USART送信ﾃﾞｰﾀ ﾚｼﾞｽﾀ空きﾌﾗｸﾞ=1 まで待機
  UDR0 = data;                      //受信データを送信
}
