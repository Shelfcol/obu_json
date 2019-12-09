#include <stdio.h>
#include <iostream>
#include <string>
#include <cstring>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>
#include "jsoncpp/json/json.h"

#define OBU_PORT 10253
#define OBU_IP "192.168.253.10"
#define BUFFER_SIZE 8192

using namespace std;

int str2int(const char *str, int exp);
uint8_t char2int(char chr);

int main()
{
    int sock_fd;
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    int on = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_BROADCAST, &on, sizeof(on));
    if (sock_fd < 0)
    {
        perror("socket_fd");
        exit(1);
    }

    /* 将套接字和IP、端口绑定 */
    struct sockaddr_in addr_serv;
    int len;
    memset(&addr_serv, 0, sizeof(struct sockaddr_in)); //每个字节都用0填充
    addr_serv.sin_family = AF_INET;                    //使用IPV4地址
    addr_serv.sin_port = htons(OBU_PORT);              //端口
    /* INADDR_ANY表示不管是哪个网卡接收到数据，只要目的端口是SERV_PORT，就会被该应用程序接收到 */
    addr_serv.sin_addr.s_addr = htonl(INADDR_ANY); //自动获取IP地址
    len = sizeof(addr_serv);

    /* 绑定socket */
    if (bind(sock_fd, (struct sockaddr *)&addr_serv, sizeof(addr_serv)) < 0)
    {
        perror("bind error:");
        exit(1);
    }

    int recv_num = 0;

    char recv_buf[BUFFER_SIZE];
    struct sockaddr_in addr_client;

    while (1)
    {
        printf("waiting for OBU:\n");
        recv_num = recvfrom(sock_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&addr_client, (socklen_t *)&len);

        if (recv_num < 0)
        {
            perror("recvfrom error:");
            exit(1);
        }

        recv_buf[recv_num] = '\0';
        //////////////////printf("receive %d bytes from OBU: \n", recv_num);
/*
        cout << "Frame Header:" << str2int(recv_buf, 2) << endl;
        cout << "Protocol Version:" << str2int(recv_buf + 2, 2) << endl;
        cout << "Device:" << str2int(recv_buf + 4, 2) << endl;
        cout << "Data:" << str2int(recv_buf + 6, 2) << endl;//5就是spat，4是rsm
        cout << "Message Length:" << str2int(recv_buf + 12, 4) << endl;
*/
		if(str2int(recv_buf + 6, 2) ==4)
       		 cout << "Data:" << str2int(recv_buf + 6, 2) << endl;
        int dataType = str2int(recv_buf + 6, 2);
        
        //这里可以对dataType进行筛选

        int MessageLength = str2int(recv_buf + 12, 4);
        char* JsonStr = new char[MessageLength + 1] ;
        
        memcpy(JsonStr, recv_buf + 16, MessageLength);
        
        Json::Reader reader;
        Json::Value json_object;

        if(!reader.parse(JsonStr, json_object))
        {
            cout << "error";
            continue;
        }
        
        
        if(str2int(recv_buf + 6, 2)==4)//表示是rsm
       		 cout<<json_object<<endl;
        //必须这么写 注意unsigned 和json逻辑层次
       
        const Json::Value intersections = json_object["intersections"];
        unsigned int i = 0;
        int intersectionId = intersections[i]["intersectionId"]["id"].asInt();    
                

        if (intersectionId == 3202)
        {

           // cout<<json_object<<endl;
            
        }
        
        //cout<<json_object<<endl;
        
        
        

        memset(recv_buf, 0, sizeof(recv_buf));
        delete[] JsonStr;
        
        
        
        
    }

    close(sock_fd);

    return 0;
}

int str2int(const char *str, int exp)
{

    int result = 0;
    for (int i = 0; i < exp; i++)
    {
        result += char2int(*(str + i))*pow(256, exp - 1 - i);
    }
    return result;
}

uint8_t char2int(char chr)
{
    return uint8_t(chr - 'A' + 65);
}
