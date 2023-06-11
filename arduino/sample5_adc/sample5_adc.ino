void setup() {
  // put your setup code here, to run once:
  ADMUX = 0b01000000;   // 基準電圧=ADC0, A/Dﾁｬﾈﾙ=ADC0
  ADCSRA = 0b10000111;  // A/D許可, A/D変換ｸﾛｯｸ=CK/128
  ADCSRA |= 0b01000000; // A/D変換開始
  Serial.begin(9600);	  // 9600bpsでポートを開く
}

void loop() {
  // put your main code here, to run repeatedly:
  uint16_t u16_ad_value;

  if (ADCSRA & 0b00010000) {  // A/D変換完了割り込み要求ﾌﾗｸﾞ
    ADCSRA |= 0b00010000;     // ADIFを解除
    u16_ad_value = ADC;       // A/D変換結果
    Serial.print("ADC: ");
    Serial.println(u16_ad_value, DEC);
    ADCSRA |= 0b01000000; // A/D変換開始
  }
  delay(1000);
}
