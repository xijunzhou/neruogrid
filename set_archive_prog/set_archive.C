int db_init(void)
{
	sqlite3 *db; 
	char *zErrMsg = NULL;
	int rc,ret; 
	
  char *sql = NULL;
  sql = (char *)malloc(sizeof(char) * 2000);
  if(sql == NULL)
  	return -1;
  	
/*======================档案表，存放在archiver.db文件中==============*/  	
  rc = sqlite3_open("config_new.db",&db);   //打开数据库  
  if(rc != SQLITE_OK)  
  {  
        printf("zErrMsg = %s\n",zErrMsg);  
        sqlite3_free(zErrMsg);  
			  free(sql);
        return -1;  
  }

/**************************************************************
* 设备类型   设备ID    无线通讯地址    
*  type      dev_id     net_id 
***************************************************************/
  memset(sql,'\0',sizeof(sql));
  sprintf(sql,"CREATE TABLE IF NOT EXISTS archive(type integer,dev_id text,net_id text);");//创建表  
	ret = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);   //执行sqlite命令语句
	if(ret != SQLITE_OK)
		{
			printf("zErrMsg = %s\n",zErrMsg);
			sqlite3_free(zErrMsg);
			  free(sql);
			return -1;
		} 
  sqlite3_close(db);      //关闭数据库

/*======================采集策略表，存放在rule.db文件中================*/  
  rc = sqlite3_open("config.db",&db);   //打开数据库  
  if(rc != SQLITE_OK)
  {  
        printf("zErrMsg = %s\n",zErrMsg);
        sqlite3_free(zErrMsg);
			  free(sql);
        return -1;
  }
/**************************************************************
*  设备类型 设备ID    无线通讯地址   最新一次采集时间  采集周期   
*   type    dev_id     net_id       collect_time     collect_cycle
***************************************************************/
  memset(sql,'\0',sizeof(sql));  
  sprintf(sql,"CREATE TABLE IF NOT EXISTS rule(type integer,dev_id text,net_id text,collect_time integer,collect_cycle integer,no_response integer);");//创建表  
	ret = sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);   //执行sqlite命令语句  
	if(ret != SQLITE_OK)
		{
			printf("zErrMsg = %s\n",zErrMsg);
		} 	
  free(sql); 
  sql = NULL;

	sqlite3_free(zErrMsg);  
  sqlite3_close(db);      //关闭数据库  	
  return 0;
}