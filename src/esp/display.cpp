#include "display.h"

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// enum PIN_I2C {
//     SDA = 4, SCA = 5
// };

void inittt() {
  pinMode(P_SDA, OUTPUT);
  pinMode(P_SCA, OUTPUT);

  digitalWrite(P_SDA, LOW);
  digitalWrite(P_SCA, LOW);
}

void draw() {
  static uint32 timer = 0;
  uint32 current = millis();
  if(current - timer > 20) {
    timer = current;
    // todo draw
  }
}