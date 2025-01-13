# wallpad
## 구현 기능
1. 모드체인지버튼 (BTN_1) 
   - CLOCK (현재시간출력)
   - STATE (device의 습도 온도 물높이 data 출력)
   - Manual (모터 제어 및 상태 출력)
2. FAN ON/OFF (BTN_3)
   - 메뉴얼 모드에서 환풍기 모터를 제어한다.
3. Door Open/Close (BTN_4)
   - 메뉴얼 모드에서 문 동작을 제어한다.
4. 리셋버튼 (BTN_7)
   - GETTIME 명령어로 서버를 통해 현재 시각 정보를 받아온다.

## 🛠️ **주요 기술 스택**
| 기술                 | 설명                                             |
|----------------------|--------------------------------------------------|
| **Nucleo-F411RE**       | 하드웨어 제어 및 MCU 기반 시스템 개발             |
| **STM32CubeIDE** | Nucleo-F411RE 펌웨어 작성 및 빌드                    |
| **C언어**            | 코드 작성                             |
| **I2C 통신**         | LCD 정보 입력               |
| **UART 통신**        | 블루투스 및 펌웨어 업로드               |
| **Timer interrupt**        | HSE를 사용한 정밀 타이머 인터럽트      |

## 📂 **디렉토리 구조**

```plaintext
📁 wallpad
├── 📂 Core
│   ├── 📂 Inc         # 헤더파일 모음
│   ├── 📂 Src        # C파일 모음
├── 📂 Drivers              # IDE가 보드에 지원하는 드라이버 
├── 📂 Debug                # 빌드시 실행할 Make파일 및 부산물 파일
│   ├── Miniproject.ioc                # CubeIDE가 제공하는 보드 Peri 제어용 파일

```
## 📂 **사용 H/W**
1. Nucleo-F411RE
2. Zs-040
3. HW-061

## 실물 사진
