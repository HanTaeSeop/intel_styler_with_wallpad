# Server
## 구현 기능
1. TCP/IP통신으로 하나의 서버와 다수의 클라이언트간 통신구현
2. 각자의 클라이언트들은 각각 BT와 WIFI로 

## 🛠️ **주요 기술 스택**
| 기술                 | 설명                                             |
|----------------------|--------------------------------------------------|
| **RasberryPi4**       | Ubuntu             |
| **Raspbian GNU/Linux** | OS                    |
| **C언어**            | 코드 작성                             |
| **소켓 통신 (TCP/IP)**         | 같은 pi 내 서버와 클라이언트 통신에 사용               |
| **소켓 통신 (BT,WIFI)**        | 다른 보드와의 통신에 사용               |
| **Putty**        | ssh 통신으로 DeskTop 접속 지원      |

## 📂 **디렉토리 구조**

```plaintext
📁 iot_client_Bluetooth
├──  build_bt.sh
├──  iot_client_blutooth.c      # server와 stm32 보드간의 통신
📁 iot_client_SQL
├──  iot_client_mini.c          # mysql db와 통신
📁 iot_server/iot_socket
├──  Makefile
├──  iot_server.c               # client들의 메시지 수신 및 송신


```

