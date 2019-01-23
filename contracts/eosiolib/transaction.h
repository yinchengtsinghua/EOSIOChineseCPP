
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
/*
 *@文件
 *@eos/license中定义的版权
 **/

#pragma once
#include <eosiolib/types.h>

extern "C" {
   /*
    *@defgroup transaction api事务API
    *@ingroup contractdev公司
    *@brief定义用于发送事务和内联操作的API
    *
    *
    *延迟的交易将在将来的块处理之前不处理。他们
    *因此对父母的成败没有影响。
    *只要交易形式良好。如果有其他情况
    *导致父事务标记为失败，然后延迟
    *永远不会处理事务。
    *
    *延迟交易必须遵守
    *父交易或将来委托给合同账户
    *供将来使用。
    *
    *内联消息允许一个契约向另一个契约发送消息
    *当前消息处理后立即处理。
    *结束时，父事务的成功或失败是
    *取决于消息的成功。如果内联消息在
    *然后处理基于
    *块将被标记为失败，对数据库的任何影响都不会
    *坚持。
    *
    *内联操作和延迟事务必须符合权限
    *可用于父交易，或将来委托给
    *供将来使用的合同账户。
    **/


   /*
    *@defgroup transaction c api transaction C api
    *@ingroup事务处理api
    *@brief定义用于发送事务的API
    *
    *@
    **/


    /*
     *发送延期交易。
     *
     *@brief发送延迟事务。
     *@param sender_id-发件人的ID
     *@param payer-支付RAM的帐户
     *@param serialized_transaction-要延迟的序列化事务的指针
     *@参数大小-要保留的大小
     *@param replace_existing-f这是“0”，如果提供的发送方_id已被来自此合同的一个正在运行的事务使用，这将是一个失败的断言。如果“1”，则事务将自动取消/替换机上事务
     **/

     void send_deferred(const uint128_t& sender_id, account_name payer, const char *serialized_transaction, size_t size, uint32_t replace_existing = 0);

    /*
     *取消延期交易。
     *
     *@brief取消延迟的事务。
     *@param sender_id-发件人的ID
     *
     *@pre存在延迟事务ID。
     *@pre延迟事务ID尚未发布。
     *@post deferred transaction已取消。
     *
     *@如果事务被取消，返回1；如果未找到事务，返回0
     *
     *实例：
     *
     *@代码
     *id=0xffffffffffffffffffff
     *取消延迟（ID）；
     *@终结码
     **/

   int cancel_deferred(const uint128_t& sender_id);

   /*
    *访问当前正在执行的事务的副本。
    *
    *@brief访问当前正在执行的事务的副本。
    *@param buffer-将当前事务写入的缓冲区
    *@param size-缓冲区的大小，0返回所需的大小
    *@返回写入缓冲区的事务的大小，或者如果大小=0，则可以复制的字节数
    **/

   size_t read_transaction(char *buffer, size_t size);

   /*
    *获取当前执行事务的大小。
    *
    *@brief获取当前执行事务的大小。
    *@返回当前执行事务的大小
    **/

   size_t transaction_size();

   /*
    *获取当前执行事务上用于TAPO的块号。
    *
    *@brief获取当前执行事务的tapos使用的块号。
    *@return块号，用于当前执行事务的tapos
    *实例：
    *@代码
    *int tbn=tapos_block_num（）；
    *@终结码
    **/

   int tapos_block_num();

   /*
    *获取当前执行事务上用于TAPO的块前缀。
    *
    *@brief获取当前正在执行的事务上用于tapos的块前缀。
    *@返回块前缀，用于当前正在执行的事务上的tapos
    *实例：
    *@代码
    *int tbp=tapos_block_prefix（）；
    *@终结码
    **/

   int tapos_block_prefix();

   /*
    *获取当前正在执行的事务的过期时间。
    *
    *@brief获取当前执行事务的到期时间。
    *@返回当前执行事务的过期时间
    *实例：
    *@代码
    *time tm=expiration（）；
    *eosio_打印（tm）；
    *@终结码
    **/

   time expiration();

   /*
    *从活动事务中检索指示的操作。
    *
    *@brief从活动事务中检索指定的操作。
    *@param type-0表示上下文无关操作，1表示操作
    *@param index-请求操作的索引
    *@param buff-输出动作的压缩buff
    *@param size-读取的buff数量，通过0返回大小
    *@返回操作的大小，失败时返回-1
    **/

   int get_action( uint32_t type, uint32_t index, char* buff, size_t size );

   /*
    *检索已签名的_transaction.context_free_data[索引]。
    *
    *@brief检索已签名的\u transaction.context \u free \u data[索引]。
    *@param index-要检索的上下文自由数据项的索引
    *@param buff-上下文自由数据输入的输出buff
    *@param size-要检索到buff中的上下文空闲数据量[索引]，0以报告所需大小
    *@return size copied，或者context_free_data[index].size（）如果传递0表示大小，或者-1表示索引无效
    **/

   int get_context_free_data( uint32_t index, char* buff, size_t size );

///@交易资本
}
