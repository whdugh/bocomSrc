// 明迈智能交通视频检测识别软件 V1.0
// Mimax Intelligent Transport Video Detection & Recognition Software V1.0
// 版权所有 2008 南京明迈视讯科技有限公司
// Copyright 2008 Nanjing Mimax Vision Technologies Ltd
// 明迈视讯公司秘密  Mimax Vision Confidential Proprietary
//
// 博康智能视频检测识别软件 V2.0
// Bocom Intelligent Video Detection & Recognition Software V2.0
// 版权所有 2008-2009 上海博康智能信息技术有限公司
// Copyright 2008-2009 Shanghai Bocom Intelligent Information Technologies Ltd
// 博康智能公司秘密   Bocom Intelligent Information Technologies Ltd Confidential Proprietary
//

#ifndef MYSQLDB_H
#define MYSQLDB_H

#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include "/usr/include/mysql/mysql.h"
#include "MysqlQuery.h"
//*********************************************************************************
//mysql 数据库操作类
/**********************************************************************************/
using namespace std;

class MysqlDB
{
	public:

		MysqlDB();
	    ~MysqlDB();

	public:

		void initCondition(string host,int port,string database,string user,string password);
		//初始化字符串

		bool connectDB();//打开数据库

		void closeDB();//关闭数据库

		int execSQL(string sql,unsigned int* id=NULL);//执行sql语句
		int getIntFiled(string sql);//获得Int字段
		string getStringFiled(string sql);//获得String字段
		long getLongFiled(string sql);//获得long字段
		MysqlQuery execQuery(string sql);//获得查询结果集合
		MysqlQuery execMultiQuery(string sql); // 多行语句查询


		bool commitTrans();//提交事务
		void rollbackTrans();//回滚事务

		unsigned int IsThreadSafe();//是否线程安全，返回值为1:是，0:否

		//加锁
		bool lockTable(string tableName,string pRiority);
		//解锁
		bool unlockTable();

	private:
		MYSQL  *m_mysql;//mysql 连接


	    string  m_strHost;//数据库主机名称
		int		m_nPort;	//数据库端口
		string  m_strDatabase;//数据库名称
		string  m_strUser;//用户名
        string  m_strPassword;//密码

		pthread_mutex_t		m_mysql_mutex;	//mysql互斥锁

};
#endif
