
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once
#include "privileged.h"
#include "serialize.hpp"
#include "types.h"

namespace eosio {

   /*
    *@特权组特权CPCPI特权C++接口
    *@InGroup特权API
    *@简要定义C++特权API
    *
    *@
    **/


   /*
    *可调区块链配置，可通过协商一致进行更改
    *
    *@brief可调区块链配置，可通过共识更改
    **/

   struct blockchain_parameters {

      uint64_t max_block_net_usage;

      uint32_t target_block_net_usage_pct;

      uint32_t max_transaction_net_usage;

      /*
       *为交易收取的净使用费的基本金额，用于支付杂费。
       *@brief交易的净使用账单的基本金额，以涵盖杂费。
       **/

      uint32_t base_per_transaction_net_usage;

      uint32_t net_usage_leeway;

      uint32_t context_free_discount_net_usage_num;

      uint32_t context_free_discount_net_usage_den;

      uint32_t max_block_cpu_usage;

      uint32_t target_block_cpu_usage_pct;

      uint32_t max_transaction_cpu_usage;

      uint32_t min_transaction_cpu_usage;


      /*
       *CFA CPU使用折扣的分子
       *
       *@简述CFA CPU使用折扣的分子
       **/

      uint64_t context_free_discount_cpu_usage_num;

      /*
       *CFA CPU使用折扣的分母
       *
       *@简述CFA CPU使用折扣的分母

       **/

      uint64_t context_free_discount_cpu_usage_den;

      /*
       *事务的最长生存期
       *
       *@事务的最短生存期
       **/

      uint32_t max_transaction_lifetime;

      uint32_t deferred_trx_expiration_window;

      uint32_t max_transaction_delay;

      /*
       *内联操作的最大大小
       *
       *@brief内联操作的最大大小
       **/

      uint32_t max_inline_action_size;

      /*
       *内联操作的最大深度
       *
       *@brief内联操作的最大深度
       **/

      uint16_t max_inline_action_depth;

      /*
       *最大权限深度
       *
       *Brief最大权限深度
       **/

      uint16_t max_authority_depth;


      EOSLIB_SERIALIZE( blockchain_parameters,
                        (max_block_net_usage)(target_block_net_usage_pct)
                        (max_transaction_net_usage)(base_per_transaction_net_usage)(net_usage_leeway)
                        (context_free_discount_net_usage_num)(context_free_discount_net_usage_den)

                        (max_block_cpu_usage)(target_block_cpu_usage_pct)
                        (max_transaction_cpu_usage)(min_transaction_cpu_usage)

                        (max_transaction_lifetime)(deferred_trx_expiration_window)(max_transaction_delay)
                        (max_inline_action_size)(max_inline_action_depth)(max_authority_depth)
      )
   };

   /*
    *@brief设置区块链参数
    *设置区块链参数
    *@param params-要设置的新区块链参数
    **/

   void set_blockchain_parameters(const eosio::blockchain_parameters& params);

   /*
    *@brief检索blolckchain参数
    *检索blolckchain参数
    *@param params-它将替换为检索到的区块链参数
    **/

   void get_blockchain_parameters(eosio::blockchain_parameters& params);

///@priviledgedcppapi

   /*
   *@defgroup producer type生产商类型
   *@ingroup类型
   *@brief定义生产者类型
   *
   *@
   **/


   /*
    *使用其签名密钥映射生产者，用于生产者计划
    *
    *@brief maps producer及其签名密钥
    **/

   struct producer_key {

      /*
       *生产商名称
       *
       生产商的*@brief名称
       **/

      account_name     producer_name;

      /*
       *阻止此生产商使用的签名密钥
       *
       *@brief块签名密钥由该生产者使用
       **/

      public_key       block_signing_key;

      friend bool operator < ( const producer_key& a, const producer_key& b ) {
         return a.producer_name < b.producer_name;
      }

      EOSLIB_SERIALIZE( producer_key, (producer_name)(block_signing_key) )
   };
}
