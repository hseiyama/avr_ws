void setup() {
  // put your setup code here, to run once:

/*
	//タイマ0,CTC,割り込み用、比較A一致で割り込み
	TCCR0A = 0b00000010;  // 比較一致ﾀｲﾏ/ｶｳﾝﾀ解除(CTC)動作
//	TCCR0B = 0b00000011; // N=64
	TCCR0B = 0b00000101; // clkI/O/1024 (1024分周)
//	OCR0A = 78;  // 5msごとに割り込み
	OCR0A = 255;  // 5msごとに割り込み
	TIMSK0 = 0b0000010;	//比較A一致割り込み有効
*/
	TCCR1A = 0b00000000;  // 比較一致ﾀｲﾏ/ｶｳﾝﾀ解除(CTC)動作
	TCCR1B = 0b00001101;  // clkI/O/1024 (1024分周)
  OCR1A = 15624;        // 1000msごとに割り込み (OCR1AH,OCR1AL)
  TIMSK1 = 0b0000010;	  // ﾀｲﾏ/ｶｳﾝﾀ1比較A割り込み許可

  //割り込み許可
//	sei();                // 全割り込み許可
//	cli();                // 全割り込み禁止

  Serial.begin(9600);	  // 9600bpsでポートを開く
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(5000);
  Serial.write("Z");
}

//ISR(TIMER0_COMPA_vect)  //タイマ割り込み
ISR(TIMER1_COMPA_vect)  //タイマ割り込み
{
//  Serial.println("TIMER0_COMPA_vect");
  Serial.write("*");
}
