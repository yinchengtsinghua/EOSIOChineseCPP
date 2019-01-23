
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

#include <stdint.h>
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *@defgroup类型内置类型
 *@ingroup contractdev公司
 *@brief指定内置类型、typedef和别名
 *
 *@
 **/


/*
 *@帐户的简要名称
 *@details账户名称
 **/

typedef uint64_t account_name;

/*
 *@权限的简短名称
 *@详细说明权限的名称
 **/

typedef uint64_t permission_name;

/*
 *@表的简短名称
 *@详细信息表名
 **/

typedef uint64_t table_name;

/*
 *@简短时间
 *详细时间
 **/

typedef uint32_t time;

/*
 *@作用域的简短名称
 *@详细说明作用域的名称
 **/

typedef uint64_t scope_name;

/*
 *@动作的简短名称
 *@详细说明操作的名称
 **/

typedef uint64_t action_name;

/*
 *@brief用于对齐/过度对齐类型的宏，以确保使用指针/引用对intrinsic的调用正确对齐
 *@details宏，用于对齐/过度对齐类型，以确保对具有指针/引用的内部函数的调用正确对齐
 **/


typedef uint16_t weight_type;

/*用于对齐/过度对齐类型的宏，以确保使用指针/引用对Intrinsic的调用正确对齐*/
#define ALIGNED(X) __attribute__ ((aligned (16))) X

/*
 *@brief eosio公钥
 *@details eosio公钥。它是34字节。
 **/

struct public_key {
   char data[34];
};

/*
 *@brief eosio签名
 *details eosio签名。它是66字节。
 **/

struct signature {
   uint8_t data[66];
};

/*
 *@brief 256位哈希
 *@详细信息256位哈希
 **/

struct ALIGNED(checksum256) {
   uint8_t hash[32];
};

/*
 *@brief 160位散列
 *@详细信息160位哈希
 **/

struct ALIGNED(checksum160) {
   uint8_t hash[20];
};

/*
 *@brief 512位哈希
 *@详细信息512位哈希
 **/

struct ALIGNED(checksum512) {
   uint8_t hash[64];
};

/*
 *@EOSIO事务ID的简短类型
 *@details eosio事务ID类型，256位散列
 **/

typedef struct checksum256 transaction_id_type;
typedef struct checksum256 block_id_type;

struct account_permission {
   account_name account;
   permission_name permission;
};

#ifdef __cplusplus
} //[外部]“C”
#endif
///@
