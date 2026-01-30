#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>
#include "AudioFileSourceHTTPStream.h"
#include "AudioFileSourceICYStream.h"
#include "AudioFileSourceBuffer.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

#include "setting.h"

// ===== Preferences 객체 =====
Preferences prefs;

// ===== 설정 변수 =====
String wifiSSID;
String wifiPASS;
String serverURL;

AudioGeneratorMP3 *mp3 = nullptr;
AudioFileSourceICYStream* fileHttp = nullptr;
AudioFileSourceBuffer *fileBuf = nullptr;
AudioOutputI2S *out = nullptr;

int lastButtonState = HIGH;

bool isPlaying = false;
bool wifiConnected = false;

// ===== URL 정규화 함수 =====
String normalizeServerURL(String url)
{
  url.trim();
  
  // http:// 또는 https:// 제거
  if (url.startsWith("http://"))
  {
    url = url.substring(7);
  }
  else if (url.startsWith("https://"))
  {
    url = url.substring(8);
  }
  
  // 마지막 슬래시 제거
  while (url.endsWith("/"))
  {
    url = url.substring(0, url.length() - 1);
  }
  
  return url;
}

String buildHTTPURL(String path)
{
  String normalized = normalizeServerURL(serverURL);
  
  // path가 슬래시로 시작하지 않으면 추가
  if (!path.startsWith("/"))
  {
    path = "/" + path;
  }
  
  return "http://" + normalized + path;
}

// ===== 설정 관리 =====
void loadSettings()
{
  prefs.begin("config", true);
  wifiSSID = prefs.getString("ssid", "");
  wifiPASS = prefs.getString("pass", "");
  serverURL = prefs.getString("server", "");
  prefs.end();

  if (wifiSSID.isEmpty() || wifiPASS.isEmpty() || serverURL.isEmpty())
  {
    Serial.println("⚠️ 설정 없음. 시리얼로 설정하세요.");
  }
  else
  {
    Serial.println("✅ 설정 로드 완료");
    Serial.println("SSID   : " + wifiSSID);
    Serial.println("SERVER : " + serverURL);
    Serial.println("정규화 : " + normalizeServerURL(serverURL));
  }
}

void saveSettings()
{
  // 저장 전 URL 정규화
  serverURL = normalizeServerURL(serverURL);
  
  prefs.begin("config", false);
  prefs.putString("ssid", wifiSSID);
  prefs.putString("pass", wifiPASS);
  prefs.putString("server", serverURL);
  prefs.end();
  Serial.println("💾 설정 저장 완료");
  Serial.println("정규화된 서버: " + serverURL);
}

// ===== Wi-Fi 연결 =====
bool connectWiFi()
{
  if (wifiSSID.isEmpty())
  {
    Serial.println("❌ WiFi 설정 없음");
    return false;
  }

  Serial.print("📡 WiFi 연결 중: ");
  Serial.println(wifiSSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID.c_str(), wifiPASS.c_str());

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");

    if (millis() - startTime > 10000)  // 10초 타임아웃
    {
      Serial.println("\n⏱️ WiFi 연결 시간 초과!");
      Serial.println("❌ WiFi 연결 실패");
      Serial.println("💡 시리얼 명령어로 설정을 확인하거나 변경하세요.");
      WiFi.disconnect();
      return false;
    }
  }

  Serial.println("\n✅ WiFi 연결 완료");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  return true;
}

// ===== 랜덤 MP3 선택 =====
String getRandomMP3Url()
{
  int n = random(1, 11);
  // URL 정규화를 사용하여 올바른 URL 생성
  return buildHTTPURL("/" + String(n) + ".mp3");
}

// ===== 오디오 정리 =====
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

// ===== MP3 재생 =====
void playMP3(const String &url)
{
  if (!wifiConnected)
  {
    Serial.println("⚠️ WiFi 연결 안됨. 재생 불가");
    return;
  }

  stopAndCleanup();

  Serial.print("▶ 재생 시작: ");
  Serial.println(url);

  fileHttp = new AudioFileSourceICYStream(url.c_str());
  fileBuf = new AudioFileSourceBuffer(fileHttp, 8192);
  mp3 = new AudioGeneratorMP3();

  if (!mp3->begin(fileBuf, out))
  {
    Serial.println("❌ MP3 재생 실패!");
    stopAndCleanup();
  }
  else
  {
    isPlaying = true;
    Serial.println("✅ 재생 중...");
  }
}

// ===== 시리얼 명령어 처리 =====
void printHelp()
{
  Serial.println("❓ 알 수 없는 명령");
  Serial.println("사용 가능한 명령어:");
  Serial.println("  WIFI SSID <이름>");
  Serial.println("  WIFI PASS <비밀번호>");
  Serial.println("  SERVER <주소>");
  Serial.println("  SAVE");
  Serial.println("  STATUS");
  Serial.println("  CONNECT");
  Serial.println("  REBOOT");
}

void printStatus()
{
  Serial.println("===== 상태 =====");
  Serial.println("SSID   : " + wifiSSID);
  Serial.println("SERVER : " + serverURL);
  Serial.println("정규화 : " + normalizeServerURL(serverURL));
  Serial.println("WiFi   : " + String(wifiConnected ? "연결됨" : "연결 안됨"));
  Serial.println("재생   : " + String(isPlaying ? "재생 중" : "정지"));
  Serial.println("================");
}

void handleSerialCommand()
{
  if (!Serial.available()) return;

  String cmd = Serial.readStringUntil('\n');
  cmd.trim();

  if (cmd.startsWith("WIFI SSID "))
  {
    wifiSSID = cmd.substring(10);
    Serial.println("✅ SSID 설정: " + wifiSSID);
  }
  else if (cmd.startsWith("WIFI PASS "))
  {
    wifiPASS = cmd.substring(10);
    Serial.println("✅ 비밀번호 설정됨");
  }
  else if (cmd.startsWith("SERVER "))
  {
    String rawURL = cmd.substring(7);
    serverURL = normalizeServerURL(rawURL);
    Serial.println("✅ 서버 설정: " + rawURL);
    Serial.println("   정규화됨: " + serverURL);
  }
  else if (cmd == "SAVE")
  {
    saveSettings();
  }
  else if (cmd == "REBOOT")
  {
    Serial.println("🔄 재부팅 중...");
    delay(1000);
    ESP.restart();
  }
  else if (cmd == "STATUS")
  {
    printStatus();
  }
  else if (cmd == "CONNECT")
  {
    Serial.println("🔄 WiFi 재연결 시도 중...");
    wifiConnected = connectWiFi();
  }
  else
  {
    printHelp();
  }
}

void setup()
{
  Serial.begin(115200);
  delay(3000);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  lastButtonState = digitalRead(BUTTON_PIN);

  Serial.println("╔════════════════════════════════════╗");
  Serial.println("║  🎵 ESP32 MP3 플레이어             ║");
  Serial.println("╚════════════════════════════════════╝");

  loadSettings();

  wifiConnected = connectWiFi();

  out = new AudioOutputI2S();
  out->SetPinout(I2S_SPK_BCLK, I2S_SPK_LRCL, I2S_SPK_DIN);
  out->SetGain(1.0);

  randomSeed(millis());

  Serial.println("\n===== 준비 완료 =====");
  if (wifiConnected)
  {
    Serial.println("✅ 버튼을 누르면 랜덤 MP3 재생");
  }
  else
  {
    Serial.println("⚠️ WiFi 연결 실패");
    Serial.println("💡 시리얼 명령어를 입력하여 설정하세요");
  }
  Serial.println();
}

void loop()
{
  // 시리얼 명령어 처리
  handleSerialCommand();

  // WiFi 연결 상태 체크
  if (wifiConnected && WiFi.status() != WL_CONNECTED)
  {
    Serial.println("⚠️ WiFi 연결 끊김!");
    wifiConnected = false;
    if (isPlaying)
    {
      stopAndCleanup();
    }
  }

  // WiFi 자동 재연결
  if (!wifiConnected && WiFi.status() == WL_CONNECTED)
  {
    Serial.println("✅ WiFi 자동 재연결됨");
    wifiConnected = true;
  }

  // 버튼 처리
  int buttonState = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && buttonState == LOW)
  {
    Serial.println(">>> 버튼 눌림!");
    playMP3(getRandomMP3Url());
  }

  lastButtonState = buttonState;

  // MP3 재생 처리
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
