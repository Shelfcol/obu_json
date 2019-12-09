#include <ros/ros.h>
#include <obu/spat.h>
//备用#include <obu/rsm.h>
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

int main(int argc, char **argv)
{
    ros::init(argc, argv, "obu_publisher");
    ros::NodeHandle nh;
    ros::Publisher spat_pub = nh.advertise<obu::spat>("spat", 100);
    //备用ros::Publisher rsm_pub = nh.advertise<obu::rsm>("rsm",100);
    ros::Rate loop_rate(20); //不能再低了
    /*---------UDP BEGIN---------*/
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
    /* INADDR_ANY表示不管是哪个网卡接收到数据，只要目的端口是SERV_PORT，就会被该应用程序接收到 */
    addr_serv.sin_family = AF_INET;                //使用IPV4地址
    addr_serv.sin_port = htons(OBU_PORT);          //端口
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
    /*---------UDP END---------*/
    while (ros::ok())
    {
        printf("waiting for OBU:\n");
        recv_num = recvfrom(sock_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&addr_client, (socklen_t *)&len);

        if (recv_num < 0)
        {
            perror("recvfrom error:");
            exit(1);
        }

        recv_buf[recv_num] = '\0';
        printf("receive %d bytes from OBU: \n", recv_num);

        /*      调试用
        cout << "Frame Header:" << str2int(recv_buf, 2) << endl;
        cout << "Protocol Version:" << str2int(recv_buf + 2, 2) << endl;
        cout << "Device:" << str2int(recv_buf + 4, 2) << endl;
        cout << "Data:" << str2int(recv_buf + 6, 2) << endl;//5就是spat，4是rsm
        cout << "Message Length:" << str2int(recv_buf + 12, 4) << endl;
*/
        /*整体解析*/
        int DataType = str2int(recv_buf + 6, 2);
        int MessageLength = str2int(recv_buf + 12, 4);
        char *JsonStr = new char[MessageLength + 1];
        memcpy(JsonStr, recv_buf + 16, MessageLength);

        Json::Reader reader;
        Json::Value json_object;

        if (!reader.parse(JsonStr, json_object))
        {
            cout << "error";
            continue;
        }
        /*整体解析结束*/

        // 注意unsigned int
        /*spat*/
        if (DataType == 5)
        {
            const Json::Value intersections = json_object["intersections"];
            unsigned int i = 0;
            const Json::Value phases = intersections[i]["phases"];
            for (unsigned int j = 0; j < phases.size(); j++)
            {
                int intersectionId = intersections[i]["intersectionId"]["id"].asInt();
                int phaseId = phases[j]["id"].asInt();
                unsigned int k = 0;
                int redTime = phases[j]["phaseStates"][k++]["timing"]["likelyEndTime"].asInt();
                int greenTime = phases[j]["phaseStates"][k++]["timing"]["likelyEndTime"].asInt();
                int yellowTime = phases[j]["phaseStates"][k++]["timing"]["likelyEndTime"].asInt();

                obu::spat spat_msg;
                spat_msg.IntersectionID = intersectionId;
                spat_msg.PhaseID = phaseId;
                spat_msg.RedTime = redTime / 10;
                spat_msg.GreenTime = greenTime / 10;
                spat_msg.YellowTime = yellowTime / 10;

                spat_pub.publish(spat_msg);
                ROS_INFO("Publish successfully!'\n' Intersection: %u  Phase: %u ", intersectionId, phaseId);
                loop_rate.sleep();
            }
        }

        /*rsm*/
        if (DataType == 4)
        {
        }

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
        result += char2int(*(str + i)) * pow(256, exp - 1 - i);
    }
    return result;
}

uint8_t char2int(char chr)
{
    return uint8_t(chr - 'A' + 65);
}