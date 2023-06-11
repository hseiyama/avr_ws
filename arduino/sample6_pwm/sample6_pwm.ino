void setup() {
  // put your setup code here, to run once:
	TCCR1A = 0b10100010;  // 比較一致でLow、BOTTOMでHighをOC1xﾋﾟﾝへ出力 (非反転動作)
	TCCR1B = 0b00011101;  // 高速PWM動作, clkI/O/1024 (1024分周)
//  ICR1 = 31248;         // 全体時間 2000ms (ICR1H,ICR1L)
//  OCR1A = 15624;        // 1000msで反転 (OCR1AH,OCR1AL)
  ICR1 = 312;           // 全体時間 20ms (ICR1H,ICR1L)
  OCR1A = 156;          // 10msで反転 (OCR1AH,OCR1AL)
  OCR1B = 156;          // 10msで反転 (OCR1AH,OCR1AL)
  DDRB = 0b00000110;    // PB1,2のみ出力
  Serial.begin(9600);	  // 9600bpsでポートを開く
  delay(5000);
}

void loop() {
  // put your main code here, to run repeatedly:
  uint8_t u8_index;
  for(u8_index=0; u8_index<10; u8_index++){
    OCR1A = 31 * u8_index;   // PWMのDuty比を変更
    delay(1000);
  }
}
