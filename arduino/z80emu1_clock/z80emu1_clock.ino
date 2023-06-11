void setup() {
  // put your setup code here, to run once:
  DDRD = 0b00110000;    // PD4,5のみ出力
  PORTD |= 0b00001100;  // PD2,3のプルアップ抵抗を有効
  Serial.begin(9600);	  // 9600bpsでポートを開く
}

void loop() {
  // put your main code here, to run repeatedly:
  PORTD = ((PIND & 0b00001100) << 2) | (PORTD & 0b11001111); // PIND2,3 -> PORTD4,5
  delay(5);
/*
  PIND = 0b00110000;    // PD4,5の出力を反転
  Serial.print("PORTD: 0b");
  Serial.println(PORTD, BIN);
  Serial.print("PIND: 0b");
  Serial.println(PIND, BIN);
  delay(1000);
*/
}
