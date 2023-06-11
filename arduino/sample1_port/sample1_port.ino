void setup() {
  // put your setup code here, to run once:
  DDRD = 0b00001100;    // portD : 2,3bit目のみ出力
  PORTD |= 0b00110000;  // PD4,5のプルアップ抵抗を有効に.
  Serial.begin(9600);	  // 9600bpsでポートを開く
}

void loop() {
  // put your main code here, to run repeatedly:
  PORTD = ((PIND & 0b00110000) >> 2) | (PORTD & 0b11110011); // PIND4,5 -> PORTD2,3;
//  PIND = 0b00001100; // PORTD2,3 反転(使える);
/*  Serial.print("PORTD: ");
  Serial.println(PORTD, BIN);
  Serial.print("PIND: ");
  Serial.println(PIND, BIN);
  delay(5000);*/
  delay(5);
}
