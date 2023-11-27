#include"usermodel.hpp"
#include"db.h"
#include<iostream>
using namespace std;

bool UserModel::insert(User &user){
    char sql[1024] = {0};//构建查询语句
    sprintf(sql,"insert into user(name,password,state) values('%s','%s','%s')",user.getName().c_str(),user.getPwd().c_str(),user.getState().c_str());
    MySQL mysql;//创建连接，也可以说是数据库连接池的
    if(mysql.connect()){
        if(mysql.update(sql)){//更新
            user.setId(mysql_insert_id(mysql.getConnect()));//获取插入用户数据主键生成的ID
            return true;
        }
    }
    return false;
}
User UserModel::query(int id)
{
    char sql[1024] = {0};
    sprintf(sql,"select * from user where id = %d",id);//构建sql语句
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES* res = mysql.query(sql);
        if(res != nullptr){
            MYSQL_ROW row = mysql_fetch_row(res);//获取查找的一行
            if(row != nullptr){
                User user;
                user.setId(atoi(row[0]));//获得查找到的那一行的id
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User();
}
bool UserModel::updateState(User user){
    char sql[1024] = {0};
    sprintf(sql,"update user set state = '%s' where id = %d",user.getState().c_str(),user.getId());//构建sql语句
    MySQL mysql;
    if(mysql.connect()){
        if(mysql.update(sql)){//更新数据
            return true;
        }
    }
    return false;
}
void UserModel::resetState(){
    char sql[1024] = "update user set state = 'offline' where state = 'online'";
    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);//更新数据
    }
}