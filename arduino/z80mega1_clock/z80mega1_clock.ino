// --------------------------------------------------------------------------------
// Z80 on Arduino Mega board code
// Address Lines:
//  Arduino Pin 22..29   <--->   Z80 A0..A7
// Data Lines:
//  Arduino Pin 49..42   <--->   Z80 D0..D7
// Control lines:
//  Arduino Pin 10  <--->  Z80 Pin 6 (CLK)
//  Arduino Pin 19  <--->  Z80 Pin 21 (RD)
//  Arduino Pin 18  <--->  Z80 Pin 22 (WR)
//  Arduino Pin A9  <--->  Z80 Pin 19 (MREQ)
//  Arduino Pin A10 <--->  Z80 Pin 20 (IORQ)
//  Arduino Pin A13 <--->  Z80 Pin 27 (M1)
//  Arduino Pin 39  <--->  Z80 PIN 24 (WAIT)
//  Arduino Pin 40  <--->  Z80 PIN 26 (RESET)
//  Arduino Pin 41  <--->  Z80 PIN 16 (INT)
// User input lines: 
//  Arduino Pin 2  <--->  Clock Button
//  Arduino Pin 3  <--->  Reset Button
// Memory map:
//  0000-00FF  Ram   255
// --------------------------------------------------------------------------------

// Ram memory shared with Z80 mapped 0000H-00FFH
uint8_t RAM[0x0100];  // Emulated RAM memory
uint8_t IO[0x0100];   // Emulated IO memory

// Pin difines
#define ADDR          PINA
#define DATA_R        PINL
#define DATA_W        PORTL
#define CLK_PIN       PB4
#define RD_PIN        PD2
#define WR_PIN        PD3
#define MREQ_PIN      PK1
#define IORQ_PIN      PK2
#define M1_PIN        PK5
#define WAIT_PIN      PG0
#define RESET_PIN     PG1
#define INT_PIN       PG2
#define U_CLK_PIN     PE4
#define U_RST_PIN     PE5

// Test pins
#define RD            ((PIND & (1 << RD_PIN)) == 0)
#define WR            ((PIND & (1 << WR_PIN)) == 0)
#define MREQ          ((PINK & (1 << MREQ_PIN)) == 0)
#define IORQ          ((PINK & (1 << IORQ_PIN)) == 0)
#define M1            ((PINK & (1 << M1_PIN)) == 0)
#define U_CLK         ((PINE & (1 << U_CLK_PIN)) == 0)
#define U_RST         ((PINE & (1 << U_RST_PIN)) == 0)

// Drive pins
#define SET_CLK(a)    (a == 0 ? PORTB |= (1 << CLK_PIN) : PORTB &= ~(1 << CLK_PIN))
#define SET_WAIT(a)   (a == 0 ? PORTG |= (1 << WAIT_PIN) : PORTG &= ~(1 << WAIT_PIN))
#define SET_RESET(a)  (a == 0 ? PORTG |= (1 << RESET_PIN) : PORTG &= ~(1 << RESET_PIN))
#define SET_INT(a)    (a == 0 ? PORTG |= (1 << INT_PIN) : PORTG &= ~(1 << INT_PIN))

// Mask pins
#define PORT_MASK_B   (1 << CLK_PIN)
#define PORT_MASK_D   ((1 << RD_PIN) + (1 << WR_PIN))
#define PORT_MASK_E   ((1 << U_CLK_PIN) + (1 << U_RST_PIN))
#define PORT_MASK_G   ((1 << WAIT_PIN) + (1 << RESET_PIN) + (1 << INT_PIN))

// Program hex code
uint8_t Program[] = {
  0x3E, 0x03,         // LD A,3
  0xC6, 0x04,         // ADD A,4
  0x32, 0x08, 0x00,   // LD (BUFF),A
  0x76,               // HALT
  // BUFF:
  0xFF                // DEFB 0FFh
/*
  // START:
  0xDB, 0x03,         // IN A,(03h)
  0xC6, 0x04,         // ADD A,4
  0xD3, 0x03,         // OUT (03h),A
  0x18, 0xF8          // JR START
*/
};

void setup() {
  // put your setup code here, to run once:
  DDRA = 0x00;            // 入力方向 (ADDR)
  DDRB = PORT_MASK_B;     // 出力方向 (CLK)
  PORTB = PORT_MASK_B;    // CLK=ON
  DDRD = 0x00;            // 入力方向 (RD, WR)
  DDRE = 0x00;            // 入力方向 (U_CLK, U_RST)
  PORTE = PORT_MASK_E;    // プルアップ設定 (U_CLK, U_RST)
  DDRG = PORT_MASK_G;     // 出力方向 (WAIT, RESET, INT)
  PORTG = PORT_MASK_G;    // WAIT=ON, RESET=ON, INT=ON
  DDRK = 0x00;            // 入力方向 (MREQ, IORQ, M1)
  DDRL = 0x00;            // 入力方向 (DATA)
  // RAM初期化
  memset(RAM, 0, sizeof(RAM));
  memcpy(RAM, Program, sizeof(Program));
  // IO初期化
  memset(IO, 0, sizeof(IO));
  Serial.begin(9600);	    // 9600bpsでポートを開く
}

static void echo_hex(uint8_t hex_data) {
  static const uint8_t hex_table[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
  };
  Serial.write(hex_table[(hex_data >> 4) & 0x0F]);
  Serial.write(hex_table[hex_data & 0x0F]);
}

static void echo_dump(uint8_t *range) {
  Serial.println(">>> 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
  for (uint8_t index_i=0; index_i<16; index_i++) {
    echo_hex(index_i * 16);
    Serial.print(">");
    for (uint8_t index_j=0; index_j<16; index_j++) {
      Serial.print(" ");
      echo_hex(range[index_i * 16 + index_j]);
    }
    Serial.println("");
  }
}

static void dump_req(void) {
  int8_t rcv_data = Serial.read();
  if (rcv_data == 'm') {
    Serial.println("RAM[] =");
    echo_dump(&RAM[0]);
  }
  else if (rcv_data == 'i') {
    Serial.println("IO[] =");
    echo_dump(&IO[0]);
  }
}

static uint8_t event_flag = 0;

static void echo_event(uint8_t *msg, uint8_t flag) {
  if (flag == 0) {
    event_flag = 0;
  }
  else if (event_flag == 0) {
    event_flag = 1;
    Serial.print((const char[])msg);
    Serial.print(": ADDR=0x");
    echo_hex(ADDR);
    Serial.print(", DATA=0x");
    echo_hex(DATA_R);
    Serial.println("");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  SET_CLK(U_CLK);
  SET_RESET(U_RST);

  if (MREQ && RD) {
    DDRL = 0xFF;          // 出力方向 (DATA)
    DATA_W = RAM[ADDR];   // DATA <- RAM
    echo_event("MREQ_RD", 1);
  }
  else if (MREQ && WR) {
    DDRL = 0x00;          // 入力方向 (DATA)
    RAM[ADDR] = DATA_R;   // RAM <- DATA
    echo_event("MREQ_WR", 1);
  }
  else if (IORQ && RD) {
    DDRL = 0xFF;          // 出力方向 (DATA)
    DATA_W = IO[ADDR];    // DATA <- IO
    echo_event("IORQ_RD", 1);
  }
  else if (IORQ && WR) {
    DDRL = 0x00;          // 入力方向 (DATA)
    IO[ADDR] = DATA_R;    // IO <-  DATA
    echo_event("IORQ_WR", 1);
  }
  else {
    DDRL = 0x00;          // 入力方向 (DATA)
    echo_event("", 0);
  }

  dump_req();
  delay(5);
}
