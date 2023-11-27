#include"db.h"
#include<string>
#include<muduo/base/Logging.h>
#include<iostream>
static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string dbname = "chat";
MySQL::MySQL()
{
     _conn = mysql_init(nullptr);//初始化连接
}
MySQL::~MySQL()
    {
        if(_conn != nullptr)
        {
            mysql_close(_conn);//释放连接
        }
    }
    //连接数据库
    bool MySQL::connect()
    {
        MYSQL *p = mysql_real_connect(_conn,server.c_str(),user.c_str(),password.c_str(),dbname.c_str(),3306,nullptr,0);
        if(p != nullptr)
        {
            mysql_query(_conn,"set name gbk");//C和C++默认编码为ASSCII,否则会有乱码识别中文
            LOG_INFO<<"connect mysql success!";
        }
        else
        {
            LOG_INFO<<"connect mysql fail!";
        }
        return p;
    }
    bool MySQL::update(string sql)
    {
        if(mysql_query(_conn,sql.c_str()))
        {
            int error_num = mysql_errno(_conn);
            const char* error_msg = mysql_error(_conn);
            cout<<error_num<<endl;
            cout<<error_msg<<endl;
            LOG_INFO<<__FILE__<<":"<<__LINE__<<":"<<sql<<"更新失败";
            return false;
        }
        return true;
    }
    MYSQL_RES* MySQL::query(string sql)//返回结果
    {
            if(mysql_query(_conn,sql.c_str()))
            {
                LOG_INFO<<__FILE__<<":"<<__LINE__<<":"<<sql<<"查询失败";
                return nullptr;
            }
        return mysql_use_result(_conn);
    }
    MYSQL* MySQL::getConnect()
    {
        return _conn;
    }
