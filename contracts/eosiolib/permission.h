
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
    *@brief检查事务是否由提供的一组密钥和权限授权
    *
    *@param trx_data-指向序列化事务开始的指针
    *@param trx_size-序列化事务的大小（以字节为单位）
    *@param pubkeys_data-指向所提供公钥的序列化向量开始的指针
    *@param pubkeys_size-所提供公钥的序列化向量的大小（字节）（如果不提供公钥，则可以为0）
    *@param perms_data-指向所提供权限的序列化向量开头的指针（空权限名用作通配符）
    *@param perms_size-所提供权限的序列化向量的大小（以字节为单位）
    *
    *@如果交易被授权返回1，否则返回0
    **/

   int32_t
   check_transaction_authorization( const char* trx_data,     uint32_t trx_size,
                                    const char* pubkeys_data, uint32_t pubkeys_size,
                                    const char* perms_data,   uint32_t perms_size
                                  );

   /*
    *@简要检查权限是否由提供的延迟和提供的密钥和权限集授权
    *
    *@param account-权限的帐户所有者
    *@param permission-检查授权的权限名称
    *@param pubkeys_data-指向所提供公钥的序列化向量开始的指针
    *@param pubkeys_size-所提供公钥的序列化向量的大小（字节）（如果不提供公钥，则可以为0）
    *@param perms_data-指向所提供权限的序列化向量开头的指针（空权限名用作通配符）
    *@param perms_size-所提供权限的序列化向量的大小（以字节为单位）
    *@param delay_us-以微秒为单位提供的延迟（不能超过Int64_max）
    *
    如果权限被授权，则返回1，否则返回0
    **/

   int32_t
   check_permission_authorization( account_name account,
                                   permission_name permission,
                                   const char* pubkeys_data, uint32_t pubkeys_size,
                                   const char* perms_data,   uint32_t perms_size,
                                   uint64_t delay_us
                                 );

   /*
    *@brief返回最后一次使用权限的时间
    *
    *@param account-权限的帐户所有者
    *@param permission-权限的名称
    *
    *@返回上次使用权限的时间（以自Unix Epoch以来的微秒为单位）
    **/

   int64_t get_permission_last_used( account_name account, permission_name permission );


   /*
    *@brief返回帐户的创建时间
    *
    *@param account-帐户
    *
    *@返回帐户的创建时间（从Unix Epoch开始以微秒为单位）
    **/

   int64_t get_account_creation_time( account_name account );

}
