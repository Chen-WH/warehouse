#include <ros/ros.h>
#include "std_msgs/Int8.h"
#include <iostream>
#include <warehouse/handlecmd.h>
#include <stdio.h>
#include <wiringPi.h>

using namespace std;

int command_type = 0; //用户输入的命令类型，-1表示出件，0表示空闲，1表示进件


/*
req.ID//找的物品的ID
req.turntable//在哪个转盘上
req.table_id//在转盘上的位置
req.time//入件时间
req.type//command_type
*/

struct position{
    int turntable = 0;
    int table_id = 0;
};

int Move_table;
int Move;

bool handle_command(warehouse::handlecmd::Request &req, warehouse::handlecmd::Response &res){
    command_type = req.type;
    Move = req.move;
    Move_table = req.move_table;
    if(command_type == 1){
        cout<<"import ";
	cout<<"move : "<<req.move;
	cout<<"table : "<<req.move_table<<endl;
    }
    else{
        cout<<"export ";
	cout<<"move : "<<req.move;
	cout<<" table : "<<req.move_table<<endl;
    }
    return true;
}

/*motor*/







int main(int argc, char **argv){
    std_msgs::Int8 status;
    status.data = 0;
    ros::init(argc, argv, "status");//用于发布status消息的节点
    ros::init(argc, argv, "server");//用于处理main发出的import、export请求的节
    ros::NodeHandle n; 
    ros::Publisher status_pub = n.advertise<std_msgs::Int8>("status", 100);//创建一个叫status的topic
    ros::ServiceServer service = n.advertiseService("command",handle_command);//创建叫command的service
    while(1){
        ros::spinOnce();//查看有没有main发出的请求
        if(command_type != 0){ //有发出请求
            //handlecommand();
            status.data = command_type;
            status_pub.publish(status);//发布现在机器的状态
            ros::Duration(0.5).sleep();
            if(command_type == 1){
		//move //move_table command_type
	//	import_action(Move,Move_table);
  		command_type = 0;
		cout<<"进件完毕"<<endl;
            } 
            else {
	//	export_action(Move,Move_table);
		cout<<"出件完毕"<<endl;
	    }
            command_type = 0;//处理完请求后置0
            status.data = 0;
        }
        status_pub.publish(status);
        ros::Duration(0.5).sleep();
    }
   return 0;
}
