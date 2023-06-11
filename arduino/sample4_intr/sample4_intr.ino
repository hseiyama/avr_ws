void setup() {
  // put your setup code here, to run once:
  DDRD = 0b00000000;    // portD:2,3bit目を入力
  PORTD |= 0b00001100;  // PD2,PD3のプルアップ抵抗を有効

  EICRA = 0b00001001;     // 外部割り込み制御ﾚｼﾞｽﾀA
                          // INT0ﾋﾟﾝの論理変化(両端)
                          // INT1ﾋﾟﾝの下降端で発生
  EIMSK = 0b00000011;     // 外部割り込み許可ﾚｼﾞｽﾀ
                          // 外部割り込み0許可
                          // 外部割り込み1許可

//	sei();                // 全割り込み許可

  Serial.begin(9600);	  // 9600bpsでポートを開く
}

void loop() {
  // put your main code here, to run repeatedly:
}

ISR(INT0_vect)  // 外部割り込み要求0
{
  Serial.write("A");
}

ISR(INT1_vect)  // 外部割り込み要求1
{
  Serial.write("B");
}
