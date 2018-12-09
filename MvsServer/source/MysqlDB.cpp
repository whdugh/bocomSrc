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

#include "Common.h"
#include "MysqlDB.h"


MysqlDB::MysqlDB()
{
	m_mysql=NULL;
	m_nPort = 3306;
	//初始化锁
	pthread_mutex_init(&m_mysql_mutex, NULL);
	return;
}

MysqlDB::~MysqlDB()
{
	//取消锁
	pthread_mutex_destroy(&m_mysql_mutex);
	return;
}

//初始化字符串
void MysqlDB::initCondition(string host,int port,string database,string user,string password)
{
	m_strHost=host;
	m_strDatabase=database;
	m_strUser=user;
    m_strPassword=password;
	m_nPort = port;

	return;
}


//打开数据库
bool MysqlDB::connectDB()
{
	//初始化mysql库的使用
	//mysql_library_init(-1,NULL,NULL);
	//分配连接
	m_mysql=mysql_init(NULL);

	if(m_mysql==NULL)
	{
		printf("初始化数据连接失败! \n");
		return false;
	}


	//设定连接字符集
	mysql_options(m_mysql,MYSQL_SET_CHARSET_NAME,"utf8");

	printf("Host:%s,User:%s,Pw:%s,DB:%s \n",m_strHost.c_str(),m_strUser.c_str(),m_strPassword.c_str(),m_strDatabase.c_str());
	//mydb为你所创建的数据库，3306为端口号，可自行设定，指定的多语句的执行
	if(!mysql_real_connect(m_mysql,m_strHost.c_str(),m_strUser.c_str(),m_strPassword.c_str(),m_strDatabase.c_str(),m_nPort,NULL,CLIENT_MULTI_STATEMENTS))
	{
		printf("打开数据库失败! \n");
		return false;
	}
	return true;
}

//执行sql语句，不需要结果集合
int MysqlDB::execSQL(string sql,unsigned int* id)
{

	if(m_mysql==NULL)
	{
		printf("No db connect! \n");
		return -1;
	}
#ifdef _DEBUG
//printf("%s \n",sql.c_str());
#endif

	pthread_mutex_lock(&m_mysql_mutex);
	int nRet=mysql_query(m_mysql,sql.c_str());

    //printf("========execSQL==========nRet =  %d \n",nRet);
	if(id)//获取当前插入的ID
	{
        *id = mysql_insert_id(m_mysql);
        //printf("=====id=%u",*id);
	}
	pthread_mutex_unlock(&m_mysql_mutex);

	if(nRet!=0)
	{
		printf("exec failed %d \n",nRet);
		printf("ERR! %s ##### \n", sql.c_str());
#ifdef _DEBUG
		printf("Exec:%d \n",nRet);
#endif
	}
	// 返回执行结果
	return nRet;
}

//获得查询结果集合
MysqlQuery MysqlDB::execQuery(string sql)
{

	if(m_mysql==NULL)
	{
		printf("No db connect!! \n");
		return MysqlQuery(true);
	}

	int nRet=0;

	MYSQL_RES *results=NULL;

	//printf("%s \n",sql.c_str());
	//printf("1 \n");
	/* Lock the query */
	pthread_mutex_lock(&m_mysql_mutex);
	nRet=mysql_query(m_mysql, sql.c_str());

	if(nRet!=0)
	{
		#ifdef _DEBUG
		printf("%s \n",sql.c_str());
		printf("查询结果失败 \n");
		#endif
		mysql_free_result(results);

		results=NULL;
		/* Get rid of the lock first */
		pthread_mutex_unlock(&m_mysql_mutex);
		return MysqlQuery(results);
	}
	//非空结果集合
	results = mysql_store_result(m_mysql);
	//printf("1 \n");

	if(results==NULL)
	{
		results=NULL;
		pthread_mutex_unlock(&m_mysql_mutex);
		return MysqlQuery(results);
	}

	int count = mysql_num_rows(results);
	//printf("count=%d \n",count);

	//判断是否为空
	if(count <=0)
	{
        mysql_free_result(results);
//		printf("Record is NULL \r\n");
		results=NULL;
		pthread_mutex_unlock(&m_mysql_mutex);
		return MysqlQuery(results);
	}
	//printf("1 \n");
	/* Release the lock		*/
	pthread_mutex_unlock(&m_mysql_mutex);

	return MysqlQuery(results);
}

//获得Int字段
int MysqlDB::getIntFiled(string sql)
{
	if(m_mysql==NULL)
	{
		printf("No db connect! \n");
		return -1;
	}

	int nRet=0;

	MYSQL_RES *results=NULL;
	MYSQL_ROW record=NULL;


	#ifdef _DEBUG
//	printf("%s \n",sql.c_str());
	#endif

	// Lock the mutex
	pthread_mutex_lock(&m_mysql_mutex);
	nRet = mysql_query(m_mysql, sql.c_str());

 	if(nRet!=0)
	{
		/* Get rid of the lock first */
		#ifdef _DEBUG
		printf("%s \n",sql.c_str());
		printf("查询结果失败 \n");
		#endif
		mysql_free_result(results);

		results=NULL;
		// Unlock the mutex
		pthread_mutex_unlock(&m_mysql_mutex);
		return -1;
	}
    string s="";
    if(results=mysql_store_result(m_mysql))
	{
        if((record = mysql_fetch_row(results)))
        {
            if(record[0])
                s=record[0];
        }
	}

	if(s!="")
		nRet = atoi(s.c_str());

	mysql_free_result(results);

	// Unlock the mutex
	pthread_mutex_unlock(&m_mysql_mutex);

	return nRet;
}

//获得String字段
string MysqlDB::getStringFiled(string sql)
{
	if(m_mysql==NULL)
	{
		printf("No db connect! \n");
		return NULL;
	}

	string strResult="";

	MYSQL_RES *results=NULL;
	MYSQL_ROW record=NULL;

	// Lock the mutex
	pthread_mutex_lock(&m_mysql_mutex);
	mysql_query(m_mysql, sql.c_str());

	if((results = mysql_store_result(m_mysql)))
	{
		if((record = mysql_fetch_row(results)))
		{
			if(record[0])
				strResult=record[0];
		}
	}
    mysql_free_result(results);
	// Unlock the mutex
	pthread_mutex_unlock(&m_mysql_mutex);

	return strResult;
}

//获得long字段
long MysqlDB::getLongFiled(string sql)
{
	if(m_mysql==NULL)
	{
		printf("No db connect! \n");
		return -1;
	}

	long nRet=0;

	MYSQL_RES *results=NULL;
	MYSQL_ROW record=NULL;

	// Lock the mutex
	pthread_mutex_lock(&m_mysql_mutex);
	mysql_query(m_mysql, sql.c_str());

	string s="";
	if((results = mysql_store_result(m_mysql)))
	{
	if((record = mysql_fetch_row(results))) {
		if(record[0])
			s=record[0];
	}
	}
	if(s!="")
		nRet = atol(s.c_str());

	mysql_free_result(results);
	// Unlock the mutex
	pthread_mutex_unlock(&m_mysql_mutex);

	return nRet;
}


//提交事务
bool MysqlDB::commitTrans()
{
	if(m_mysql==NULL)
	{
		printf("No db connect! \n");
		return false;
	}

	int nRet = mysql_commit(m_mysql);
	if(!nRet)
	{
		printf("提交事务失败");
		return false;
	}

	return true;
}

//会滚事务
void MysqlDB::rollbackTrans()
{
	if(m_mysql==NULL)
	{
		printf("No db connect! \n");
		return ;
	}
	mysql_rollback(m_mysql);
	return;
}

//加锁
bool MysqlDB::lockTable(string tableName,string pRiority)
{
	char buf[1024]={0};
	sprintf(buf,"LOCK TABLES %s %s",tableName.c_str(),pRiority.c_str());

	string sql(buf);

	if(mysql_query(m_mysql, sql.c_str()))
		return false;

	return true;
}

//解锁
bool MysqlDB::unlockTable()
{
	if(mysql_query(m_mysql,"UNLOCK TABLES"))
		return false;

	return true;
}

//关闭数据库
void MysqlDB::closeDB()
{
	//释放连接
	if(m_mysql!=NULL)
	{
		mysql_close(m_mysql);
		m_mysql=NULL;
	}
	//结束mysql库的使用
	//mysql_library_end();
	return;
}

//是否线程安全，返回值为1:是，0:否
unsigned int MysqlDB::IsThreadSafe()
{
	return mysql_thread_safe();
}


/*
 * 多行语句查询
 * 返回最后一行结果,中间结果被释放。可利用变量得到中间结果,调用者负责mysql_free_result()最后一行
 */
MysqlQuery MysqlDB::execMultiQuery(string sql)
{

	if(m_mysql==NULL)
	{
		printf("No db connect! \n");
		return MysqlQuery(true);
	}

	int nRet=0;

	MYSQL_RES *results=NULL;

	/* Lock the query */
	pthread_mutex_lock(&m_mysql_mutex);
	nRet=mysql_query(m_mysql, sql.c_str());


	if(nRet!=0)
	{
		/* Get rid of the lock first */
		#ifdef _DEBUG
		printf("%s \n",sql.c_str());
		printf("多语句查询结果失败 \n");
		#endif
		mysql_free_result(results);

		results=NULL;
		pthread_mutex_unlock(&m_mysql_mutex);
		return MysqlQuery(results);
	}

	int i = 0;
	do
	{
		if(i > 0)
		mysql_free_result(results);

		//非空结果集合
		results = mysql_store_result(m_mysql);

		if(results==NULL)
		{
			pthread_mutex_unlock(&m_mysql_mutex);
			printf(" 多语句查询结果为空 !\r\n");
			return MysqlQuery(results);
		}

		i++;

	}while (!mysql_next_result(m_mysql));


	/* Release the lock		*/
	pthread_mutex_unlock(&m_mysql_mutex);

	return MysqlQuery(results);
}
