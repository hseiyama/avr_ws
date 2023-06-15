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
//  0000-00FF  Ram   256
// --------------------------------------------------------------------------------

// Ram memory shared with Z80 mapped 0000H-00FFH
static uint8_t RAM[0x0100];  // Emulated RAM memory
static uint8_t IO[0x0100];   // Emulated IO memory

// Common defines
#define OFF           (0)
#define ON            (1)
#define ON2           (2)     // Special flag

// Pin defines
#define ADDR          PINA
#define DATA_R        PINL
#define DATA_W        PORTL
#define CLK_PIN       PB4
#define RD_PIN        PD2
#define WR_PIN        PD3
#define MREQ_PIN      PK1
#define IORQ_PIN      PK2
#define M1_PIN        PK5
#define RFSH_PIN      PK6
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
#define RFSH          ((PINK & (1 << RFSH_PIN)) == 0)
#define U_CLK         ((PINE & (1 << U_CLK_PIN)) == 0)
#define U_RST         ((PINE & (1 << U_RST_PIN)) == 0)

// Drive pins
#define SET_CLK(a)    (a == 0 ? PORTB |= (1 << CLK_PIN) : PORTB &= ~(1 << CLK_PIN))
#define REV_CLK()     (PINB = (1 << CLK_PIN))
#define SET_WAIT(a)   (a == 0 ? PORTG |= (1 << WAIT_PIN) : PORTG &= ~(1 << WAIT_PIN))
#define SET_RESET(a)  (a == 0 ? PORTG |= (1 << RESET_PIN) : PORTG &= ~(1 << RESET_PIN))
#define SET_INT(a)    (a == 0 ? PORTG |= (1 << INT_PIN) : PORTG &= ~(1 << INT_PIN))

// Mask pins
#define PORT_MASK_B   (1 << CLK_PIN)
#define PORT_MASK_D   ((1 << RD_PIN) + (1 << WR_PIN))
#define PORT_MASK_E   ((1 << U_CLK_PIN) + (1 << U_RST_PIN))
#define PORT_MASK_G   ((1 << WAIT_PIN) + (1 << RESET_PIN) + (1 << INT_PIN))

// Program hex code
static const uint8_t Program[] = {
/*
  0x3E, 0x03,         // LD A,3
  0xC6, 0x04,         // ADD A,4
  0x32, 0x08, 0x00,   // LD (BUFF),A
  0x76,               // HALT
  // BUFF:
  0xFF                // DEFB 0FFh
*/
  // START:
  0xDB, 0x03,         // IN A,(03h)
  0xC6, 0x04,         // ADD A,4
  0xD3, 0x03,         // OUT (03h),A
  0x18, 0xF8          // JR START
};

// Variable for debug
static uint8_t debug_clock;
static uint16_t debug_cycle;

static void echo_dump(uint8_t*);
static void debug_req(void);

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
  DDRK = 0x00;            // 入力方向 (MREQ, IORQ, M1, RFSH)
  DDRL = 0x00;            // 入力方向 (DATA)

  memset(RAM, 0, sizeof(RAM));            // RAM初期化
  memcpy(RAM, Program, sizeof(Program));  // Programをロード
  memset(IO, 0, sizeof(IO));              // IO初期化

  debug_clock = ON;       // デバッグ用のクロック設定 (手動)
  debug_cycle = 5;        // デバッグ用の処理間隔 (5ms)

  Serial.begin(9600);	    // 9600bpsでポートを開く
}

void loop() {
  // put your main code here, to run repeatedly:
  if (debug_clock == ON) {
    SET_CLK(U_CLK);
  }
  else {
    REV_CLK();            // CLOCK反転
  }
  SET_RESET(U_RST);

  if (MREQ && RD) {
    DDRL = 0xFF;          // 出力方向 (DATA)
    DATA_W = RAM[ADDR];   // DATA <- RAM
    debug_log("MREQ_RD", ON);
  }
  else if (MREQ && WR) {
    DDRL = 0x00;          // 入力方向 (DATA)
    RAM[ADDR] = DATA_R;   // RAM <- DATA
    debug_log("MREQ_WR", ON);
  }
  else if (IORQ && RD) {
    DDRL = 0xFF;          // 出力方向 (DATA)
    DATA_W = IO[ADDR];    // DATA <- IO
    debug_log("IORQ_RD", ON);
  }
  else if (IORQ && WR) {
    DDRL = 0x00;          // 入力方向 (DATA)
    IO[ADDR] = DATA_R;    // IO <-  DATA
    debug_log("IORQ_WR", ON);
  }
  else if (RFSH && MREQ) {
    DDRL = 0x00;          // 入力方向 (DATA)
    debug_log("RFSH_MREQ", ON2);    // Set Special flag
  }
  else {
    DDRL = 0x00;          // 入力方向 (DATA)
    debug_log("", OFF);
  }

  debug_req();
  delay(debug_cycle);
}

static void echo_hex(uint8_t hex_data) {
  static const uint8_t hex_table[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
  };
  Serial.write(hex_table[(hex_data >> 4) & 0x0F]);
  Serial.write(hex_table[hex_data & 0x0F]);
}

static void echo_dump(uint8_t* addr_hi, uint8_t* range) {
  Serial.println("     | 00 01 02 03 04 05 06 07  08 09 0A 0B 0C 0D 0E 0F");
  Serial.println("-----+-------------------------------------------------");
  for (uint8_t index_i=0; index_i<16; index_i++) {
    Serial.print((const char[])addr_hi);
    echo_hex(index_i * 16);
    Serial.print(" |");
    for (uint8_t index_j=0; index_j<16; index_j++) {
      Serial.print(" ");
      if (index_j == 8) {
        Serial.print(" ");
      }
      echo_hex(range[index_i * 16 + index_j]);
    }
    Serial.println("");
  }
}

static void debug_req(void) {
  int8_t rcv_data = Serial.read();
  switch (rcv_data) {
    case 'm':   // RAM領域をダンプ
      Serial.println("RAM[ 0x0000-0x00FF ] =");
      echo_dump("00", &RAM[0]);
      break;
    case 'i':   // IO領域をダンプ
      Serial.println("IO[ 0x00-0xFF ] =");
      echo_dump("  ", &IO[0]);
      break;
    case 'c':   // クロック設定を変更
      debug_clock = !debug_clock;
      debug_cycle = (debug_clock == ON) ? 5 : 500;
      break;
    case '0':   // 処理間隔を変更 (5ms)
      debug_cycle = 5;
      break;
    case '1':   // 処理間隔を変更 (50ms)
      debug_cycle = 50;
      break;
    case '2':   // 処理間隔を変更 (200ms)
      debug_cycle = 200;
      break;
    case '3':   // 処理間隔を変更 (500ms)
      debug_cycle = 500;
      break;
    case '4':   // 処理間隔を変更 (1000ms)
      debug_cycle = 1000;
      break;
  }
}

static void debug_log(uint8_t* msg, uint8_t flag) {
  static uint8_t event_flag = OFF;
  if (flag == OFF) {
    event_flag = OFF;
  }
  else if (event_flag == OFF) {
    event_flag = ON;
    Serial.print((const char[])msg);
    Serial.print(": ADDR=0x");
    echo_hex(ADDR);
    if (flag == ON) {               // Except for Special flag ON2
      Serial.print(", DATA=0x");
      echo_hex(DATA_R);
    }
    if (M1) {
      Serial.print(" <M1>");
    }
    Serial.println("");
  }
}
