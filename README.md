# Arduino-Web-MP3-Player

[![Visual Studio Code](https://img.shields.io/badge/-Visual%20Studio%20Code-007ACC?style=flat&logo=visual-studio-code&logoColor=white)](https://code.visualstudio.com/)
[![Platform](https://img.shields.io/badge/Platform-ESP32-E7352C?style=flat&logo=espressif&logoColor=white)](https://www.espressif.com/en/products/socs/esp32)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-orange?style=flat&logo=platformio&logoColor=white)](https://platformio.org/)
[![Framework](https://img.shields.io/badge/Framework-Arduino-00979D?style=flat&logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![License](https://img.shields.io/badge/License-GPL3.0-blue.svg)](LICENSE)

Arduino-Web-MP3-Player는 Arduino Nano ESP32를 통해 웹에 MP3 파일를 버튼을 눌러 랜덤하게 재생하는 아두이노 프로젝트입니다.

# 배선도
> 아래 배선도는 참고용 일뿐, 실제와 다를 수 있습니다.
<img width="610" height="448" alt="image" src="https://github.com/user-attachments/assets/a4607fd3-fc11-4681-a3fd-4ae3f44d04d7" />

# 빌드 및 업로드

1. Visual Studio Code에서 PlatformIO IDE 확장을 설치합니다.
2. 프로젝트를 열고 Arduino Nano ESP32를 USB로 연결합니다.
3. VS Code 하단의 **Upload(→)** 버튼을 눌러 빌드 및 업로드합니다.

# 참고  

Dropbox, Google Drive와 같은 클라우드 스토리지 서비스는  
ESP32에서 직접 접근하기 어렵기 때문에 사용할 수 없습니다.  

대신 **로컬에서 직접 웹 서버를 열고**,  
**Python HTTP 서버와 Cloudflare Tunnel**을 이용하면  
**임시로 외부에서 접근 가능한 MP3 스트리밍 서버를 구성할 수 있습니다.**
