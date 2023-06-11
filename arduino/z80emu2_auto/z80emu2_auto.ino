#define HALF_CLOCK (500)

void setup() {
  // put your setup code here, to run once:
  uint8_t u8_index;
  DDRD = 0b00110000;    // PD4,5のみ出力
  PORTD = 0b00010000;  // RESET端子=OFF,CLOCK端子=ON
  Serial.begin(9600);	  // 9600bpsでポートを開く

  // リセット処理
  Serial.println("Reset.");
  for(u8_index=0; u8_index<10; u8_index++) {
    delay(HALF_CLOCK);
    PIND = 0b000010000; // CLOCK端子を反転
  }
  PORTD |= 0b00100000;  // RESET端子=ON
  Serial.println("Start.");
  delay(HALF_CLOCK);
}

void loop() {
  // put your main code here, to run repeatedly:
  PIND = 0b00010000;    // CLOCK端子を反転
  delay(HALF_CLOCK);
}
