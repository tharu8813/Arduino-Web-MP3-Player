#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "AudioFileSourceHTTPStream.h"
#include "AudioFileSourceICYStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

#include "setting/setting.h"

AudioGeneratorMP3 *mp3 = nullptr;
AudioFileSourceICYStream* fileHttp = nullptr;
AudioFileSourceBuffer *fileBuf = nullptr;
AudioOutputI2S *out = nullptr;

int lastButtonState = HIGH;

bool isPlaying = false;

// 랜덤 MP3 선택
String getRandomMP3Url()
{
  int n = random(1, 11);
  return String(mp3BaseUrl) + String(n) + String(".mp3");
}

// 오디오 정리
void stopAndCleanup()
{
  if (mp3)
  {
    if (mp3->isRunning())
      mp3->stop();
    delete mp3;
    mp3 = nullptr;
  }
  if (fileBuf)
  {
    delete fileBuf;
    fileBuf = nullptr;
  }
  if (fileHttp)
  {
    fileHttp->close();
    delete fileHttp;
    fileHttp = nullptr;
  }
  isPlaying = false;
}

// MP3 재생
void playMP3(const String &url)
{
  stopAndCleanup();

  Serial.print("재생 시작: ");
  Serial.println(url);

  fileHttp = new AudioFileSourceICYStream(url.c_str());
  fileBuf = new AudioFileSourceBuffer(fileHttp, 8192);
  mp3 = new AudioGeneratorMP3();

  if (!mp3->begin(fileBuf, out))
  {
    Serial.println("MP3 재생 실패!");
    stopAndCleanup();
  }
  else
  {
    isPlaying = true;
    Serial.println("재생 중...");
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n=== ESP32 MP3 플레이어 시작 ===");

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  lastButtonState = digitalRead(BUTTON_PIN);

  // Wi-Fi 연결
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Wi-Fi 연결 중");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWi-Fi 연결 성공!");
  Serial.print("IP 주소: ");
  Serial.println(WiFi.localIP());

  out = new AudioOutputI2S();
  out->SetPinout(I2S_SPK_BCLK, I2S_SPK_LRCL, I2S_SPK_DIN);
  out->SetGain(1.0);

  randomSeed(millis());

  Serial.println("\n=== 준비 완료 ===");
  Serial.println("버튼을 누르면 랜덤 MP3 재생\n");
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Wi-Fi 연결 끊김! 재연결 시도...");
    WiFi.reconnect();
    delay(3000);
    return;
  }

  int buttonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && buttonState == LOW)
  {
    Serial.println(">>> 버튼 눌림!");
    playMP3(getRandomMP3Url());
  }

  lastButtonState = buttonState;

  if (mp3 && mp3->isRunning())
  {
    if (!mp3->loop())
    {
      Serial.println(">>> MP3 재생 종료");
      stopAndCleanup();
    }
  }

  delay(1);
}
