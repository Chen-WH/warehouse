//#include <ros/ros.h>
//#include <geometry_msgs/PoseStamped.h>
#include <iostream>
#include <unordered_map>
#include <random>
#include <time.h>
#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <warehouse/handlecmd.h>
#include <warehouse/keyboard.h>
#include <ros/ros.h>
#include "std_msgs/Int8.h"
#include <cstdlib>
using namespace std;

MYSQL mysql;
MYSQL_ROW row;
MYSQL_FIELD* field = NULL;
MYSQL_RES* result;

const int MAX_ID = 10000;
int *IDmap =new int[MAX_ID]{};

struct object{
    int ID = 0;
    int turntable = 0;
    int table_id = 0;
    int time = 0;
};
struct command{
    int type = 0; //1进件 -1出件
    int move_table = 0;//放到哪个转盘上
    int move = 0;//正转为正,反转为负
};
/*数据库中添加物品*/
void mysql_add(int ID, int turntable, int table_id, int time){
    string str1 = to_string(ID);
    string str2 = to_string(turntable);
    string str3 = to_string(table_id);
    string str4 = to_string(time);
    string sql = "INSERT INTO goods(ID,turntable,table_id,time) values('"+str1+"','"+str2+"','"+str3+"','"+str4+"'"+");";
    mysql_query(&mysql,sql.c_str());
    //cout<<sql.c_str()<<endl;
}
/*数据库中删除物品*/
void mysql_delete(int ID){
    string ID_str = to_string(ID);
    string sql = "DELETE FROM goods WHERE ID='" + ID_str + "';";
    mysql_query(&mysql,sql.c_str());
}
/*数据库中查找物品*/
int mysql_find(int ID,object &ob){
    string ID_str = to_string(ID);
    string sql = "SELECT * FROM goods WHERE ID ='" + ID_str +"';";
    mysql_query(&mysql,sql.c_str());
    result = mysql_store_result(&mysql);
    if (result == NULL) {
        return 0;
    }
    int i = 0;
    for(i=0; i<mysql_num_fields(result); i++){
        field = mysql_fetch_field_direct(result,i);
        cout << field->name <<"\t\t";
    }
    cout<<endl;
    row = mysql_fetch_row(result);
    if(row == NULL){return 0;}
    while(row != NULL){
        int i = 0;
        for(i=0; i<mysql_num_fields(result); i++)
        {
            cout<<row[i]<<"\t\t";
        }
        cout<<endl;
    	ob.ID = stoi(row[0]);
    	ob.turntable = stoi(row[1]);
    	ob.table_id = stoi(row[2]);
    	ob.time = stoi(row[3]);
	row = mysql_fetch_row(result);
    }
    return 1;
}

void set_object(object &ob, int ID, int turntable, int table_id, int time = 0){
    ob.ID = ID;
    ob.turntable = turntable;
    ob.table_id = table_id;
    ob.time = time;
}

void set_cmd(command &cmd, int type, int move_table, int move){
    cmd.type = type;
    cmd.move_table = move_table;
    cmd.move = move;
}



struct position{
    int table_id = 0;
    int turntable = 0;
};


int get_new_ID(){
    int ID = rand() % (MAX_ID);
    if(IDmap[ID] == 0){//The ID havn't been used
        IDmap[ID] = 1; // set busy
        return ID;
    }
    else{//The ID is being used
       if (IDmap[(ID+1)%MAX_ID] == 0){
            IDmap[(ID+1)%MAX_ID] = 1;
            return (ID+1)%MAX_ID;
       }
       int i = 2;
       int di = 2;
       while(true){
            if(IDmap[(ID+di)%MAX_ID] == 0){
                IDmap[(ID+di)%MAX_ID] = 1;
                return (ID+di)%MAX_ID;
            }
            di = i^2;
            i++;
       }
    }
}

void init_mysql(){
    mysql_init(&mysql);
    /*连接数据库*/
    if(!mysql_real_connect(&mysql,"localhost","root","raspberry","warehouse",0,NULL,0))
    {
        cout << "connect faild\n";
    }
}

int status = 0; //0表示空闲, -1出件, 1进件.

void sendcommand(command cmd, ros::ServiceClient &client){
    /*给server发送command*/
    warehouse::handlecmd srv;
    srv.request.type = cmd.type;
    srv.request.move_table = cmd.move_table;
    srv.request.move = cmd.move;
    if(client.call(srv)){
        ROS_INFO("call\n");
    }else{
        ROS_ERROR("Faild to call server\n");
    }

}
void get_status(const std_msgs::Int8::ConstPtr &msg){
    status = msg->data;
}

int command_type = 0;
int out_ID = 0;
bool handle_keyboard(warehouse::keyboard::Request &req, warehouse::keyboard::Response &res){
    command_type = req.command_type;
    out_ID = req.ID;
    return true;
}

const int max_num = 15;
int table1[max_num]{};
int table2[max_num]{};
int ptr1=0;
int ptr2=0;

int Add(int ptr,int i){
	return (ptr+i)%max_num;
}
int Minus(int ptr,int i){
	if(ptr >= i) return (ptr-i);
	else return (ptr+max_num-i);
}
position get_new_pos(int &move,int &move_table){
	int i=0;
	for(i = 0; i < (max_num-1)/2; i++){
		if(table1[Add(ptr1,i)] == 0){ //1正转i个后有空闲位置
			position new_pos;
			new_pos.turntable = 1;
			new_pos.table_id = Add(ptr1,i);
			table1[Add(ptr1,i)] = 1;
			move = i;
			move_table = 1;
			ptr1 = Add(ptr1,i);
			return new_pos;
		}
		else if(table1[Minus(ptr1,i)] == 0){//1反转i个后有空闲位置
			position new_pos;
			new_pos.turntable = 1;
			new_pos.table_id = Minus(ptr1,i);
			table1[Minus(ptr1,i)] = 1;
			move = -i;
			move_table = 1;
			ptr1 = Minus(ptr1,i);
			return new_pos;
		}
	}
	for(i = 0; i< (max_num-1)/2; i++){
		if(table2[Add(ptr2,i)] == 0){ //2正转i个后有空闲位置
			position new_pos;
			new_pos.turntable = 2;
			new_pos.table_id = Add(ptr2,i);
			table2[Add(ptr2,i)] = 1;
			move = i;
			move_table = 2;
			ptr2 = Add(ptr2,i);
			return new_pos;
		}
		else if(table2[Minus(ptr2,i)] == 0){//2反转
			position new_pos;
			new_pos.turntable = 2;
			new_pos.table_id = Minus(ptr2,i);
			table2[Minus(ptr2,i)] = 1;
			move = -i;
			move_table = 2;
			ptr2 = Minus(ptr2,i);
			return new_pos;
		}
	}

}

void get_action(object ob, int &move_table, int &move){
	move_table = ob.turntable;
	if(ob.turntable == 1){//取下的物品在一号转盘
		//ptr1 当前指针位置
		int i = 0;
		while(Add(ptr1,i) != ob.table_id){
			i++;
		}
		if(i <= 7){
			cout<<"momomomo"<<i<<endl;
			move = i;
		}
		if(i > 7){
			move = -(max_num - i);
			cout<<"momomom:"<<move<<endl;
		}
		table1[ob.table_id] = 0;
		ptr1 = ob.table_id;
		return ;
	}	
	else{
		//ptr2 当前指针位置
		int i = 0;
		while(Add(ptr2,i) != ob.table_id){
			i++;
		}
		if(i <= 7){
			move = i;
		}
		if(i > 7){
			move = -(max_num - i);
		}
		table2[ob.table_id] = 0;
		ptr2 = ob.table_id;
		return ;
	}

}

int main(int argc,char** argv){
    //unordered_map<int, object> map; //记录所有物件的hash map
   
    ros::init(argc,argv,"client"); 
    ros::NodeHandle n;
    ros::ServiceClient client = n.serviceClient<warehouse::handlecmd>("command");
    ros::Subscriber status_sub = n.subscribe<std_msgs::Int8>("status",100,get_status);
    /*读取keyboard*/
    ros::ServiceServer service = n.advertiseService("keyboard",handle_keyboard);
    /**/
    queue<command> command_que;
    clock_t start_time,end_time;
    init_mysql();
    int move = 0;
    int move_table = 0;
    //
    while(true){
        ros::Duration(0.5).sleep();
        command_type = 0;
        
        /*get status and command*/
        ros::spinOnce();
	cout<<"machine status : "<<status<<endl;
	cout<<"ptr1:"<<ptr1<<"  ptr2:"<<ptr2<<endl;
        if(command_type == -1){//export
            object ob;
            command cmd;
            /*寻找ID对应的物品信息* */  
            if(!mysql_find(out_ID,ob)){
                cout<<"No such ID."<<endl;
		command_type = 0;
                continue;
            }
            /*从数据库中删除* */
            mysql_delete(out_ID);
            /*释放空闲ID* */
            IDmap[out_ID] = 0;
            /*set cmd* */
	    cout<<"出件 ID:"<<ob.ID<<"turn_table:"<<ob.turntable<<"table_id"<<ob.table_id<<endl;
	    get_action(ob,move_table,move);
            set_cmd(cmd,-1,move_table,move);
            /*cmd 进队*/
            command_que.push(cmd);
            //pop_object(ob);
        }
        if(command_type == 1){//import
            if(status == 1){//正在进件不接受进件命令
                cout<<"Wrong. I am busy!\n";
                continue;
            }
            else{

                position pos;
                object ob;
                command cmd;
                int ID;
                start_time = clock()/1000;
                ID = get_new_ID();//随机生成ID *
		cout<<"进件:"<<ID<<endl;
                pos = get_new_pos(move,move_table);//生成最优位置，并生成移动的策略 *
                set_object(ob,ID,pos.turntable,pos.table_id,start_time); //set 进入的物品 *
                mysql_add(ID,pos.turntable,pos.table_id,start_time);//写入数据库 *
                set_cmd(cmd,1,move_table,move); //set cmd *
                command_que.push(cmd); //cmd 进队*
                //push_object(ob);//控制机械结构进件
            }
        }
        /*处理command*/
        if(!command_que.empty()){
            command cmd;
            /*cmd出队*/
            cmd = command_que.front();
            command_que.pop();
            sendcommand(cmd,client);
        }
        
    }

}


