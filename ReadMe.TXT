----20180207
电表数据采集：部分电表不支持负荷数据采集命令，所以，对于负荷数据只能一个个采集上来，
完成一个电表所有数据采集后才存入数据库!一个表所有的数据采集上来需要的时间为：N * Tc，N-> 负荷数据项数  Tc-> 采集周期。






----20180326
sqlite数据库表情况
1、网关由2个数据库文件组成，分别是config.db、data.db
2、config.db有两个tables，分别是档案表(archive)和采集策略表(rule)
	   |_archive    数据字段：type | dev_id | net_id       即：类型 | 设备ID | 无线通讯网络ID
	   |_rule       数据字段：type | dev_id | net_id | collect_time | collect_cycle | no_response  类型 | 设备ID | 无线通讯网络ID | 下一次采集时间 | 采集周期 | 设备未响应次数
3、data.db存储历史记录数据，每一个设备创建一个单独的表，表的命名规则如下：设备名称_类型 _设备ID
     如：
        单相电表：elect_meter_40_000000060004	、 elect_meter_40_0000000324576、elect_meter_40_0000000554575
        三相电表：elect3_meter_40_000000060004、……
        水    表：water_meter_40_000000060004	、……
        气    表：gas_meter_40_000000060004	、……
        
     历史数据存储格式为：Timestamp,dev_id,value
     如：
        1522045238|000000040006|{\"type\":\"40\",\"id\":\"000000040006\",\"Timestamp\":\"20180326142038\",\"EnergyForward\":\"10.15 kWh\",\"EnergyBackward\":\"0.00 kvarh\",\"Voltage\":\"222.6 v\",\"Current\":\"0.039 A\",\"Frequency\":\"-110.00 hz\",\"Factor\":\"0.004\",\"ActPower\":\"-0.0065 kw\",\"ReactPower\":\"1.8322 kvar\"}
        
4、config.db数据库中的档案数据(archive表)通过远程或本地USB口方式下发和更新，网关程序在档案数据更新后重启运行
   a、档案数据在PC机上可通过记事本录入archive.txt文件中，录入格式如下(type,dev_id,net_id)：
        64,000000039803,1
				64,000000039804,2
				64,000000039805,3
				64,000000039806,4
				64,000000039807,5
				64,000000039808,6
				64,000000039809,7
				64,000000039810,8
				64,000000039811,9    
	b、archive.txt录入完成后，通过远程或本地USB口，将文件下发到网关程序同一级目录
	c、通过telnet登录网关设备，检查archive.txt内容和格式是否正确。确认后在网关程序下执行如下步骤
	      root@ubuntu:~/win7/gateway-collecter# sqlite3 config.db
				SQLite version 3.21.0 2017-10-24 18:55:49
				Enter ".help" for usage hints.				
				sqlite> .separator ','
				sqlite> .import archive.txt archive
				sqlite> select * from archive;
				64,000000039803,1
				64,000000039804,2
				64,000000039805,3
				64,000000039806,4
				64,000000039807,5
				64,000000039808,6
				64,000000039809,7
				64,000000039810,8
				64,000000039811,9
	d、	档案更新完成
5、重启网关程序，按照新的档案运行



----20180329
添加智能采集周期更新函数，采用一元线性回归方程 y = bx + a
1、从历史记录中，取最新的N(10)条记录，将时间 和 值 的变化值([time0 - time1 time1 - time2,……] [value0 - value1 value1 - value2,……] )作为回归方程输入，得到一元线性回归方程的 b 和 a 的值，
2、以最少变化值(DATA_RULE_BASE) 作为输出变量 x ，得到 y 即采集周期。 



------------------------------------TO DO-----------------------------------

1、添加全数据buff解码，当前protocol_decode解码程序只截取buff前面一条正确的DTL645帧\CJ188帧
   需要增加对剩余buff数据帧解码。