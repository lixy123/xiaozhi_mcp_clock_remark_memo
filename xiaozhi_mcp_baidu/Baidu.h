#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <UrlEncode.h>
//#include <base64.hpp>
#include "I2S.h"
//#include "wen_ap.h"

//供调用
void get_access_token() ;
void baiduTTS_Send(String text);

//内部用
String getAccessToken(const char* api_key, const char* secret_key);
void playAudio(uint8_t* audioData, size_t audioDataSize) ;
