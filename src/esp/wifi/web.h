#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "../wifi/wifi.h"

bool initWebServer();
void endServer();
void getListTime();
extern bool isWebServer;

#endif