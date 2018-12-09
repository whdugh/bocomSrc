#include "MysqlTransitionSqliteCyToUSB.h"



/* 版权所有 2009 上海博康智能信息技术有限公司
* Copyright (c) 2009，Shanghai Bocom Intelligent Information Technologies Ltd
* 博康智能公司秘密
* Bocom Intelligent Confidential Proprietary
* 文件名称：CopyToUSB.cpp
* 摘要: USB自动拷贝,需要将U盘格式化为Fat32格式
* 版本: V1.0
* 作者: qianwenming
* 完成日期: 2012年8月29日
*/

#include "CopyToUSB.h"
#include "ximage.h"
#include "CSeekpaiDB.h"
#include <sys/vfs.h>
#include <iostream>

using namespace std;

#ifdef SQLITE_OK
//拷贝服务
//CopyToUSB g_copyToUSB;
//usb拷贝标识位
volatile bool g_bEndToUsb_sqlite = false;

//char sqliteDB_path[100] = {"/home/road/sqlite/sqlite-autoconf-3070603/epolice.db"};
char sqliteDB_path[100] = {"/data/data/epolice.db"};


MysqlTransitionSqlteCyToUSB::MysqlTransitionSqlteCyToUSB()
{
	//初始化锁
	pthread_mutex_init(&m_sqlite3_mutex, NULL);
}

MysqlTransitionSqlteCyToUSB::~MysqlTransitionSqlteCyToUSB()
{

	//取消锁
	pthread_mutex_destroy(&m_sqlite3_mutex);
}



// 打开sqlite3 数据库
int MysqlTransitionSqlteCyToUSB::openSqlite()
{
	 int sOpen = sqlite3_open(sqliteDB_path, &m_db);

	if(sOpen != SQLITE_OK)
	{
			printf("opeen sqlite3 error \n");
			return -1;
	}
	return 1;
}



//根据传进来的参数， 来在制定路径创建数据库， 插入数据 
int MysqlTransitionSqlteCyToUSB::insetIntosqliteMessage(string sqlPaht, string strSql)
{
	printf("insetIntosqliteMessage(string sqlPaht, string strSql):%s\n",strSql.c_str());
	// 创建数据库
	if (sqlPaht.size() <= 0 || strSql.size() <= 0)
	{
		//LogError("strSql.size() <= 0 \n");
		return -1;
	}
	string newSqlPath = "mkdir -p ";
	newSqlPath += sqlPaht;
	system(newSqlPath.c_str());
	newSqlPath = sqlPaht + "/epolice.db";

	sqlite3 * sql_db;
	int sOpen = sqlite3_open(newSqlPath.c_str(), &sql_db);          //打开数据库

	if(sOpen != SQLITE_OK)
	{
		//LogError("opeen sqlite3 error \n");
		return -1;
	}

	char createTableSql[1024] = {0};                           // 创建数据库
	strcpy(createTableSql , "CREATE TABLE PASSVEHICLE (XH INTEGER PRIMARY KEY autoincrement,HPHM VARCHAR(16),HPZL INTEGER ,TGSJ DateTime,CLSD INTEGER,XZSD INTEGER , CLCD INTEGER, CSYS VARCHAR(3), WJKZM INTEGER , WFLX INTEGER ,WFXW VARCHAR(5) ,HDSJ INTEGER, LKHM Varchar(7), SBHM Varchar(32),  CSDM Varchar(2),CLBZ INTEGER, SPLX INTEGER, SPLJ Varchar(1000), TGSJ1 DateTime ); ");
	int result = sqlite3_exec(sql_db, createTableSql, NULL, 0, 0);
	if (result != SQLITE_OK)
	{
		
		//return -1;
	}

	// 插入数据库
	result = sqlite3_exec(sql_db, strSql.c_str(), NULL, 0, 0);
	if( result != SQLITE_OK )
	{
		//LogError("sqlite3_exec error \n");
		sqlite3_close(sql_db);
		return -1;
	}
	//关闭数据库
	sqlite3_close(sql_db);
	return 1;

}

// 在sqlite3 中执行sql语句
int MysqlTransitionSqlteCyToUSB::execsqlInsqlite(string strSql)
{
	const char *  sqlBuff = strSql.c_str();
	printf("- execsqlInsqlite----%s \n", sqlBuff);
	pthread_mutex_lock(&m_sqlite3_mutex);
	int result = sqlite3_exec(m_db, sqlBuff, NULL, 0, 0);
	pthread_mutex_unlock(&m_sqlite3_mutex);
	if( result != SQLITE_OK )
	{
		printf("exec sql error %d\n", result);
		cerr<<"exec sql error "<<result<<endl;
		closeSqlite();
		return -1;
	}
	return  1;
}

// 把数据插入到sqlite3 里面
int MysqlTransitionSqlteCyToUSB::insetIntosqliteMessage(string strSql)
{
	const char *  sqlBuff = strSql.c_str();

	printf("- insetrSql----%s \n", sqlBuff);
	pthread_mutex_lock(&m_sqlite3_mutex);
	int result = sqlite3_exec(m_db, sqlBuff, NULL, 0, 0);
	pthread_mutex_unlock(&m_sqlite3_mutex);
	if( result != SQLITE_OK )
	{
		printf("insert into content error %d\n", result);
		cerr<<"insert into content error "<<result<<endl;
		closeSqlite();
		return -1;
	}
	return  1;
}

int  MysqlTransitionSqlteCyToUSB::AddField()
{
	char createTableSql[1024] = {0};

	strcpy(createTableSql , "alter table PASSVEHICLE add TGSJ1 datetime; ");

	pthread_mutex_lock(&m_sqlite3_mutex);
	int result = sqlite3_exec(m_db, createTableSql, NULL, 0, 0);
	pthread_mutex_unlock(&m_sqlite3_mutex);
	if (result != SQLITE_OK)
	{
		printf("add passVehicle field \n");
		return -1;
	}
	return 1;
}

// 创建 passVehicle 表
int  MysqlTransitionSqlteCyToUSB::createTablePassVehicle()
{
	char createTableSql[1024] = {0};

	strcpy(createTableSql , "CREATE TABLE PASSVEHICLE (XH INTEGER PRIMARY KEY autoincrement,HPHM VARCHAR(16),HPZL INTEGER ,TGSJ DateTime,CLSD INTEGER,XZSD INTEGER , CLCD INTEGER, CSYS VARCHAR(3), WJKZM INTEGER , WFLX INTEGER ,WFXW VARCHAR(5) ,HDSJ INTEGER, LKHM Varchar(7), SBHM Varchar(32),  CSDM Varchar(2),CLBZ INTEGER, SPLX INTEGER, SPLJ Varchar(1000), TGSJ1 DateTime); ");

	pthread_mutex_lock(&m_sqlite3_mutex);
	int result = sqlite3_exec(m_db, createTableSql, NULL, 0, 0);
	pthread_mutex_unlock(&m_sqlite3_mutex);
	if (result != SQLITE_OK)
	{
	//	printf("create passVehicle error \n");
	}
	return 1;
}

// 删除 passVehicle 表
int  MysqlTransitionSqlteCyToUSB::deleteTablePassVehicle()
{
	char deletePassVehicleSql[1024] = {0};
	strcpy(deletePassVehicleSql, "drop table PASSVEHICLE; ");
	pthread_mutex_lock(&m_sqlite3_mutex);
	if (sqlite3_exec(m_db, deletePassVehicleSql, NULL, 0, 0) != SQLITE_OK)
	{
		printf("delete Table Passvehicle error \n");
		return -1;
	}
	pthread_mutex_unlock(&m_sqlite3_mutex);
	return 1;

}

// 关闭sqlite3 数据库
void MysqlTransitionSqlteCyToUSB::closeSqlite()
{
	sqlite3_close(m_db);
}


//车标
int MysqlTransitionSqlteCyToUSB::carLabel(int number)
{

	if (number == 5)
	{
		return 1;
	}
	else if (number == 0)
	{
		return 2;
	}
	else if (number == 3)
	{
		return 3;
	}
	else if (number == 4)
	{
		return 4;
	}
	else if (number == 7)
	{
		return 5;
	}
	else if (number == 2)
	{
		return 6;
	}
	else if (number == 8)
	{
		return 7;
	}
	else if (number == 28)
	{
		return 8;
	}
	else if (number == 30)
	{
		return 9;
	}
	else if (number == 78)
	{
		return 10;
	}
	else if (number == 74)
	{
		return 11;
	}
	/*else if (number ==  )
	{
		return 12;              // BLAC
	}*/
	else if (number == 10)
	{
		return 13;
	}
	else if (number == 6)
	{
		return 14;
	}
	else if (number == 80)
	{
		return 15;
	}
	/*else if (str == "")
	{
		return 16;           //松花江
	}*/

	return 0;
}


//车身颜色
int MysqlTransitionSqlteCyToUSB::carColor(int number)
{
	if (number == 0)
	{
		return 0;
	}
	else if (number == 10)
	{
		return 1;
	}
	else if (number == 6)
	{
		return 2;
	}
	else if (number == 9)
	{
		return 3;
	}
	else if (number == 3)
	{
		return 4;
	}
	else if (number == 4)
	{
		return 5;
	}
	else if (number == 7)
	{
		return 6;
	}
	else if (number == 5)
	{
		return 7;
	}
	else if (number == 8)
	{
		return 8;
	}
	else if (number == 2)
	{
		return 9;
	}
	return 10;
}

int MysqlTransitionSqlteCyToUSB::wflx(int number)
{
	if (number == 13030 || number == 43050 || number == 43060 || number == 16030 || number == 13031 || number == 13032 || number ==  43051 ||  number == 43061)
	{
		return 1;
	}
	else if (number == 13020 || number == 13022 || number == 13021)
	{
		return 4;
	}
	else if (number == 10900 || number == 10910 )
	{
		return 10;
	}
	else if (number == 10430 || number == 10440 || number == 10450 || number == 13440 || number== 13450)
	{
		return 11;
	}
	else if (number == 10420 || number == 10180 || number == 10190 || number == 12430 || number == 12431 || number == 10230 || number == 10240 || number == 10250 || number == 12080 || number == 12120 || number == 12280 || number == 10251 || number == 10252 || number == 10253 )
	{
		return 23;
	}
	else if (number == 12110)
	{
		return 30;
	}
	else if (number == 13010)
	{
		return 42;
	}
	else if (number == 12070)
	{
		return 45;
	}
	else if (number == 16140 || number == 16150)
	{
		return 46;
	}
	else if (number == 13440 )
	{
		return 47;
	}
	else if (number == 12100)
	{
		return 50;
	}
	else if (number == 13040)
	{
		return 51;
	}
	else if (number == 10391)
	{
		return 99;
	}
	else if (number == 13441)
	{
		return 12;
	}
	else if (number == 13443)
	{
		return 13;
	}
	else if (number == 60230)
	{
		return 23;
	}

	return 11;
}

//检测结果类型
int MysqlTransitionSqlteCyToUSB::wfxw(int number)
{

	if (number == 1 ||number == 17 || number == 57)//禁止停车
	{
		return 10391;
	}

	if (number == 2 || number == 26)//逆行
	{
		return 13010;
	}

	if (number == 5) //慢行
	{
		return 43061;
	}

	if (number == 6 || number == 54)//超速
	{
		return 13030;
	}

	if (number == 8 || number == 29)//变道
	{
		return 10430;
	}

	if (number == 16)//闯红灯
	{
		return 13022;
	}
	if (number ==18 || number == 22 || number == 23 || number == 24)//禁左，禁右
	{
		return 10420;
	}

	if ( number == 32)//占用公交道
	{
		return 10190;
	}

	if (number == 55)//机占非
	{
		return 10180;
	}

	if(number == 56)//禁止驶入
	{
		return 13440;
	}

	if(number == 58)//路口滞留
	{
		return 12110;
	}

	if (number == 59)//禁止调头
	{
		return 10440;
	}

	if(number == 50)//黄标车
	{
		return 13441;
	}

	if(number == 51)//国1标准汽油车禁行
	{
		return 13443;
	}

	if(number ==19 || number == 20)//不按规定车道行驶
	{
		return 60230;
	}

	return 0;
}

//车的类型
int MysqlTransitionSqlteCyToUSB::carType(int number, string str)
{

	printf("carType--------%s--------", str.c_str());
	char *in_gb2312hu = "沪";
	char *in_gb2312jing = "警";
	char out[100] = {0};
	char out2[100] = {0};

	//gb2312--> utf-8
	//CodeConverterOpen("gb2312","utf-8");

	//converting(in_gb2312hu,strlen(in_gb2312hu),out,100);
	//converting(in_gb2312jing,strlen(in_gb2312jing),out2,100);

	//string strhu(out);
	//string strjing(out2);

	//CodeConverterClose();

	string strhu("沪");
	string strjing("警");


	if (str.length() < 3)
	{
		return 0;
	}
	string str1 = str.substr(str.length() - 3, 3);
	string str2 = str.substr(0, 3);

	if (str1 == strjing)
	{
		return 4;
	}
	if (str2 != strhu)
	{
		return 3;
	}
	if (number == 1)
	{
		return 2;
	}
	else if (number == 3)
	{
		return 1;
	}
	return 0;
}






/*if (!q.eof())
	{
		printf(" ------------1 enter into selectMySql function  -- if \n");
		result = q.getSelectResult();
		while(row = mysql_fetch_row(result))
		{
			printf(" ------------1 enter into selectMySql function  --while \n");
			//char * strInsertSql = transitionData(row);
			//insetIntosqliteMessage(strInsertSql);
			for (int a;a<100; a++)
			{
				for(int index = 0; index < 44; index++ )
				{
					printf("%s\t", row[index]);
				}
				printf("\n");
			}

		}

	}*/


// 这个函数的功能能是： 把mysql里面的数据重新排序组合成插入sqlite3的sql 语句
char * MysqlTransitionSqlteCyToUSB::transitionData(MYSQL_ROW row)
{
	char * strsql = NULL;

	strsql = (char *)malloc(2048);
	memset(strsql, '\0', 2048);

	strcat(strsql, "INSERT INTO PASSVEHICLE values( " );

	char  XH[20]   = {0};
	char  HPHM[20] = {0};
	char  HPZL[10] = {0};
	char  TGSJ[40] = {0};
	char  CLSD[10] = {0};
	char  XZSD[10] = {0};
	char  CLCD[10] = {0};
	char  CSYS[5]  = {0};
	char  WJKZM[10]= {0};
	char  WFLX[10] = {0};
	char  WFXW[15] = {0};
	char  HDSJ[20] = {0};
	char  LKHM[10] = {0};
	char  SBHM[40] = {0};
	char  CSDM[5]  = {0};
	char  CLBZ[10] = {0};
	char  SPLX[10] = {0};
	char  SPLJ[1024]={0};
	char  TGSJ1[40] = {0};
	// MySql  ---------> sqlite3
	strcat(strsql, XH);              //                   XH
	strcat(strsql, ", ' ");
	strcat(strsql, row[1]);          // NUMBER ---------> HPHM   char
	strcat(strsql, " ', ");
	strcat(strsql, row[22]);         // TYPE   ---------> HPZL
	strcat(strsql, ", ' ");
	strcat(strsql, row[6]);          // TIME   ---------> TGSJ    time 需要 ''
	strcat(strsql, " ' , ");
	strcat(strsql, row[24]);		 // SPEED  ---------> CLSD
	strcat(strsql, ", ");
	strcat(strsql, XZSD);			 //					  XZSD
	strcat(strsql, ", ");
	strcat(strsql, CLCD);   		 //					  CLCD
	strcat(strsql, ", ' ");
	strcat(strsql, CSYS);   		 //			--------> CSYS     char
	strcat(strsql, " ' , ");
	strcat(strsql, WJKZM);           //	                  WJKZM
	strcat(strsql, ", ");
	strcat(strsql, WFLX);		   	 //					  WFLX
	strcat(strsql, ", ' ");
	strcat(strsql, row[31]);         // PECCANCY_KIND-->  WFXW      char
	strcat(strsql, " ' , ");
	//strcat(strsql, HDSJ);			 //					  HDSJ
	strcat(strsql, row[43]);
	strcat(strsql, ", ");
	strcat(strsql,LKHM);			 //					  LKHM
	strcat(strsql, ", ' ");
	strcat(strsql,SBHM);			 //					  SBHM      char
	strcat(strsql, " ' , ' ");
	strcat(strsql, CSDM);			 //					  CSDM      char
	strcat(strsql, " ' , ");
	strcat(strsql, row[23]);  		 // FACTORY ------->  CLBZ
	strcat(strsql, ", ");
	strcat(strsql,SPLX);			 //					  SPLX
	strcat(strsql, ", ' ");
	strcat(strsql, SPLJ);			 //					  SPLJ       char
	strcat(strsql, row[6]);          // TIME   ---------> TGSJ    time 需要 ''//TODO GCS
	strcat(strsql, ", ' ");
	strcat(strsql, " '); ");


	return strsql;

}
#endif
