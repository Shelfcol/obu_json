#include <ros/ros.h>
#include <obu/spat.h>
#include <iostream>

using namespace std;

void spatCallBack(const obu::spat &msg)
{
    if(msg.IntersectionID == 3202)
        ROS_INFO("intersectionID: %u '\n' phaseID:%u '\n' RedTime:%u '\n' GreenTime:%u '\n' Yellow Time:%u '\n'", msg.IntersectionID, msg.PhaseID, msg.RedTime, msg.GreenTime, msg.YellowTime);
}
int main(int argc, char ** argv)
{
    ros::init(argc, argv, "obu_subscriber"); //解析传入的ROS参数，定义节点名称
    ros::NodeHandle nh;           //定义句柄，一个句柄可以理解为一个进入一个类的门把手
    ros::Subscriber sub = nh.subscribe("spat",10,spatCallBack);
    ros::spin(); // 触发回调函数，并循环处理，若处理一次则使用ros：：spinOnce
    return 0;
}