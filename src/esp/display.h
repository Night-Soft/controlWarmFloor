#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <U8g2lib.h>

#include "pins.h"
#include "PostponedTask.h"
#include "rtc/RealRtc.h"

enum FONT_SIZE : uint8 { F10, F11, F12, F15, F17, F22 };

struct Modal {
  char title[30], str1[34], str2[34];
  uint16 timeShow = 0; // in seconds, 0 always show
};

class Display { 
  bool &canUpdate;
  uint8 modalId = 0;

 public:
  Display();

  U8G2_SSD1306_128X64_NONAME_F_HW_I2C& gDisplay;

  void init();
  void startUpdate();
  void stopUpdate();
  // draw everything that's already in the buffer
  void forceUpdade();
  // call draw() -> draw the time, modal window, other
  void drawNow(); 
  void powerSave();

  void print(const char* str, uint8 x, uint8 y);
  void printToCenter(const char* str);
  void printLeftPumpTime(const char* str);
  void setCursorToCenter(const char* str);
  void setFontSize(FONT_SIZE size);
  FONT_SIZE fontSize = FONT_SIZE::F12;

  Modal modal;
  bool isModal = false;
  uint8 getModalId();
  uint8 showModal(Modal &modal);
  uint8 showModal();
  void hideModal();
  void hideModal(uint8 id);

  void toggleWiFi(bool isEnable);
  void toggleHeating(bool isEnable);
  void togglePump(bool isEnable);  
  void togglePumpBtn(bool isEnable);  
};

extern Display display;

#endif