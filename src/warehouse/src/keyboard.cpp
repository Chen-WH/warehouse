#include <iostream>
#include <string>
#include <warehouse/keyboard.h>
#include <ros/ros.h>

using namespace std;
int main(int argc,char** argv){
    ros::init(argc,argv,"keyboard_client");
    ros::NodeHandle n;
    ros::ServiceClient client = n.serviceClient<warehouse::keyboard>("keyboard");
    string command_type;
    string ID;
    warehouse::keyboard srv;
    while(1){
        cout<<"输入命令"<<endl;
        cin>>command_type;
        if(strcmp(command_type.c_str(),"-1") == 0){
            cout<<"输入ID"<<endl;
            cin>>ID;
            srv.request.command_type = atoi(command_type.c_str());
            srv.request.ID = atoi(ID.c_str());
        }
        if(strcmp(command_type.c_str(),"1") == 0){
            srv.request.command_type = atoi(command_type.c_str());
        }
        if(client.call(srv)){ 
            ROS_INFO("call\n");
        }else{
            ROS_ERROR("Faild to call server\n");
        }
    }
}
