#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8081);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }
    char buf[256];
    char format1[] = "GET_TIME [dd/mm/yyyy]";
    char format2[] = "GET_TIME [dd/mm/yy]";
    char format3[] = "GET_TIME [mm/dd/yyyy]";
    char format4[] = "GET_TIME [mm/dd/yy]";
    time_t currentTime;
    struct tm *localTime;
    for (int i = 0; i < 8; i++)
    {
        if (fork() == 0)
        {
            while (1)
            {
                int client = accept(listener, NULL, NULL);
                printf("New client accepted: %d\n", client);

                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                    continue;
                buf[ret] = 0;
                printf("Received from %d: %s\n", client, buf);

                currentTime = time(NULL);
                localTime = localtime(&currentTime);
                int day = localTime->tm_mday;
                int month = localTime->tm_mon + 1;
                int year = localTime->tm_year + 1900;
                char res_format1[11];
                char res_format2[9];
                char res_format3[11];
                char res_format4[9];
                snprintf(res_format1, sizeof(res_format1), "%02d/%02d/%04d", day, month, year);
                snprintf(res_format2, sizeof(res_format2), "%02d/%02d/%02d", day, month, year % 100);
                snprintf(res_format3, sizeof(res_format3), "%02d/%02d/%04d", month, day, year);
                snprintf(res_format4, sizeof(res_format4), "%02d/%02d/%02d", month, day, year % 100);

                buf[strcspn(buf, "\n")] = '\0';
                char *msg = malloc(20 * sizeof(char));
                if (strcmp(buf, format1) == 0)
                {
                    strcpy(msg, res_format1);
                }
                else if (strcmp(buf, format2) == 0)
                {
                    strcpy(msg, res_format2);
                }
                else if (strcmp(buf, format3) == 0)
                {
                    strcpy(msg, res_format3);
                }
                else if (strcmp(buf, format4) == 0)
                {
                    strcpy(msg, res_format4);
                }
                else
                {
                    strcpy(msg, "Sai dinh dang");
                }
                send(client, msg, strlen(msg), 0);
                // Dong ket noi
                close(client);
            }
            exit(0);
        }
    }
    getchar();
    killpg(0, SIGKILL);
    return 0;
}