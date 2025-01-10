#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <mariadb/mysql.h>
#include <signal.h>

#define BUF_SIZE 100
#define NAME_SIZE 20
#define ARR_CNT 5

void* send_msg(void* arg);
void* recv_msg(void* arg);
void* db_insert(void* arg);
void error_handling(char* msg);

char name[NAME_SIZE] = "[Default]";
char msg[BUF_SIZE];

typedef struct {
    char sensorType[20];
    int value;         // 변경: float에서 int로
    int humidity;      // 변경: float에서 int로
    int waterLevel;
} SensorData;

int main(int argc, char* argv[]) {
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t snd_thread, rcv_thread, db_thread;
    void* thread_return;

    if (argc != 4) {
        printf("Usage: %s <IP> <port> <name>\n", argv[0]);
        exit(1);
    }

    sprintf(name, "%s", argv[3]);

    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock == -1)
        error_handling("socket() error");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("connect() error");

    sprintf(msg, "[%s:PASSWD]", name);
    write(sock, msg, strlen(msg));

    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
    pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);

    pthread_join(snd_thread, &thread_return);
    close(sock);
    return 0;
}

void* send_msg(void* arg) {
    int* sock = (int*)arg;
    int ret;
    fd_set initset, newset;
    struct timeval tv;
    char name_msg[NAME_SIZE + BUF_SIZE + 2];

    FD_ZERO(&initset);
    FD_SET(STDIN_FILENO, &initset);

    fputs("Input a message! [SensorType]value\n", stdout);
    while (1) {
        memset(msg, 0, sizeof(msg));
        name_msg[0] = '\0';
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        newset = initset;
        ret = select(STDIN_FILENO + 1, &newset, NULL, NULL, &tv);
        if (FD_ISSET(STDIN_FILENO, &newset)) {
            fgets(msg, BUF_SIZE, stdin);
            if (!strncmp(msg, "quit\n", 5)) {
                *sock = -1;
                return NULL;
            }
            strcpy(name_msg, msg);
            if (write(*sock, name_msg, strlen(name_msg)) <= 0) {
                *sock = -1;
                return NULL;
            }
        }
        if (ret == 0) {
            if (*sock == -1)
                return NULL;
        }
    }
}

void* recv_msg(void* arg) {
    int* sock = (int*)arg;
    int str_len;
    char name_msg[NAME_SIZE + BUF_SIZE + 1];
    while (1) {
        memset(name_msg, 0x0, sizeof(name_msg));
        str_len = read(*sock, name_msg, NAME_SIZE + BUF_SIZE);
        if (str_len <= 0) {
            *sock = -1;
            return NULL;
        }
        name_msg[str_len] = 0;
        fputs(name_msg, stdout);

        // 디버깅: 수신된 메시지 출력
        printf("Received message: %s\n", name_msg);

        // 데이터베이스에 삽입 스레드 생성
        SensorData* data = malloc(sizeof(SensorData));

        // 정확한 형식으로 데이터 파싱 (수정된 부분)
       int parsed = sscanf(name_msg, "[%[^]]]%*[^@]@%d@%d@%d", 
                    data->sensorType, &data->value, &data->humidity, &data->waterLevel);


        // 파싱된 항목이 4개가 아닌 경우 에러 메시지 출력
        if (parsed != 4) {
            printf("Error: Failed to parse sensor data. Parsed %d items.\n", parsed);
            free(data); // 할당된 메모리 해제
            continue; // 잘못된 데이터가 있으면 다시 수신
        }

        // 디버깅 출력
        printf("Parsed data: sensorType=%s, value=%d, humidity=%d, waterLevel=%d\n",
            data->sensorType, data->value, data->humidity, data->waterLevel);

        // 데이터베이스 삽입을 위한 스레드 생성
        pthread_t db_thread;
        pthread_create(&db_thread, NULL, db_insert, (void*)data);
        pthread_detach(db_thread); // 스레드 종료 후 자원 자동 회수
    }
}

void* db_insert(void* arg) {
    SensorData* data = (SensorData*)arg;

    MYSQL* con = mysql_init(NULL);
    if (con == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        free(data);
        return NULL;
    }

    if (mysql_real_connect(con, "127.0.0.1", "iot_", "1234", "wardrobe", 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed: %s\n", mysql_error(con));
        mysql_close(con);
        free(data);
        return NULL;
    }

    // 데이터 삽입 쿼리
    char sql_cmd[500];
    sprintf(sql_cmd,
        "INSERT INTO sensor_data(device_id, temperature, humidity, water_level, timestamp) "
        "VALUES('%s', %d, %d, %d, NOW())",
        data->sensorType, data->value, data->humidity, data->waterLevel);

    if (mysql_query(con, sql_cmd)) {
        fprintf(stderr, "INSERT error: %s\n", mysql_error(con));
    }
    else {
        printf("Data inserted: device_id=%s, temperature=%d, humidity=%d, water_level=%d\n",
            data->sensorType, data->value, data->humidity, data->waterLevel);
    }

    mysql_close(con);
    free(data);
    return NULL;
}

void error_handling(char* msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}

