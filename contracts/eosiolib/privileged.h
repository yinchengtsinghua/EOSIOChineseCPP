
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

   /*
    *@defgroup privileged api特权api
    *@ingroup系统API
    *@brief定义了一个API，用于访问只有特权帐户才能完成的链配置。
    **/


   /*
    *@defgroup privileged c api特权C API
    *@InGroup特权API
    *@brief定义了%c特权API
    *
    *@
    **/


  /*
    *@brief获取帐户的资源限制
    *获取帐户的资源限制
    *@param account-要获取其资源限制的帐户的名称
    *@param ram_bytes-指向“int64_t”的指针，以绝对字节为单位保留检索到的RAM限制
    *@param net_weight-指向“int64”的指针以保持net限制
    *@param cpu_weight-指向“int64”的指针以保持cpu限制
    **/

   void get_resource_limits( account_name account, int64_t* ram_bytes, int64_t* net_weight, int64_t* cpu_weight );

   /*
    *@brief设置帐户的资源限制
    *设置帐户的资源限制
    *@param account-要设置其资源限制的帐户的名称
    *@param ram_bytes-以绝对字节表示的RAM限制
    *@param net_weight-根据（所有_帐户的权重/总_weight_）按比例分配可用资源的净限制
    *@param cpu_weight-按比例分配可用资源的cpu限制（所有_帐户的权重/总_weight）
    **/

   void set_resource_limits( account_name account, int64_t ram_bytes, int64_t net_weight, int64_t cpu_weight );

   /*
    *建议更改计划，一旦包含建议的块变为不可逆，计划将自动升级为“挂起”。一旦升级计划的块不可逆，计划将变为“活动”
    *@param producer_data-按适当的producer schedule顺序打包producer_密钥的数据
    *@param producer_data_size-数据缓冲区的大小
    *
    *@return-1如果提议新的生产商计划失败，则返回新提议计划的版本。
    **/

   int64_t set_proposed_producers( char *producer_data, uint32_t producer_data_size );

   /*
    *@brief设置新的活动生产者
    *设置新的活动生产者。只有当启动下一轮的块是IRR可逆的时，生产者才会被激活。
    *@param producer_data-指向生产者计划的指针，压缩为字节
    *@param producer_data_size-打包生产商计划的大小
    *@pre'producer_data'是指向内存范围的有效指针，该内存范围至少为'producer_data_size'字节长，其中包含序列化的生成的计划数据
    **/

   void set_active_producers( char *producer_data, uint32_t producer_data_size );
   /*
    *@brief检查帐户是否有特权
    *检查帐户是否具有特权
    *@param account-要检查的帐户的名称
    *@如果帐户有特权，返回true
    *@如果帐户没有特权，返回false
    **/

   bool is_privileged( account_name account );

   /*
    *@brief设置帐户的特权状态
    *设置帐户的特权状态
    *@param account-要设置其特权帐户的帐户的名称
    *@param是特权状态
    **/

   void set_privileged( account_name account, bool is_priv );

   /*
    *@brief设置区块链参数
    *设置区块链参数
    *@param data-指向区块链参数的指针，打包为字节
    *@param datalen-打包区块链参数的大小
    *@pre'data'是指向包含压缩区块链参数数据的至少'datalen'字节长的内存范围的有效指针。
    **/

   void set_blockchain_parameters_packed(char* data, uint32_t datalen);

   /*
    *@brief检索blolckchain参数
    *检索blolckchain参数
    *@param data-区块链参数的输出缓冲区，只有在足够大的空间容纳压缩数据时才检索。
    *@param datalen-数据缓冲区的大小，0以报告所需的大小。
    *@返回区块链参数的大小
    *@pre`data`是指向内存范围的有效指针，长度至少为'datalen'字节
    *@post`data`中填充了打包的区块链参数
    **/

   uint32_t get_blockchain_parameters_packed(char* data, uint32_t datalen);

   /*
    *@brief激活新功能
    *激活新功能
    *@param f-要激活的功能的名称（标识符）
    **/

   void activate_feature( int64_t f );

///@私人资本
#ifdef __cplusplus
}
#endif
