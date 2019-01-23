
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

#include <eosio/chain/types.hpp>
#include <eosio/chain/config.hpp>

namespace eosio { namespace chain {

/*
 *@brief生产者投票的区块链配置参数
 *
 *此对象存储区块链配置，由区块生产者设置。阻止生产者投票赞成
 *它们对该对象中的每个参数的偏好，并且区块链根据
 *生产商指定的值。
 **/

struct chain_config {
uint64_t   max_block_net_usage;                 ///<块指令中的最大净使用量
uint32_t   target_block_net_usage_pct;          ///<最大净使用率的目标百分比（1%=100，100%=10000）；超过此百分比将触发拥塞处理
uint32_t   max_transaction_net_usage;           ///<链允许的最大客观测量净使用量，无论帐户限制如何。
uint32_t   base_per_transaction_net_usage;      ///<为交易计费的用于支付杂项费用的净使用量的基本金额
   uint32_t   net_usage_leeway;
uint32_t   context_free_discount_net_usage_num; ///<上下文无关数据净使用折扣的分子
uint32_t   context_free_discount_net_usage_den; ///<上下文无关数据净使用折扣的分母

uint32_t   max_block_cpu_usage;                 ///<块的最大计费CPU使用率（以微秒为单位）
uint32_t   target_block_cpu_usage_pct;          ///<最大CPU使用率的目标百分比（1%=100，100%=10000）；超过此百分比将触发拥塞处理
uint32_t   max_transaction_cpu_usage;           ///<链允许的最大可计费CPU使用率（以微秒为单位），无论帐户限制如何。
uint32_t   min_transaction_cpu_usage;           ///<链所需的最小可计费CPU使用率（以微秒为单位）

uint32_t   max_transaction_lifetime;            ///<输入事务的到期时间可以早于其首次包含的块的时间的最大秒数
uint32_t   deferred_trx_expiration_window;      ///<延迟事务第一次执行到到期后的秒数
uint32_t   max_transaction_delay;               ///<授权检查可作为延迟要求施加的最大秒数
uint32_t   max_inline_action_size;              ///<内联操作的最大允许大小（字节）
uint16_t   max_inline_action_depth;             ///<发送内联操作时的递归深度限制
uint16_t   max_authority_depth;                 ///<用于检查是否满足权限的递归深度限制

   void validate()const;

   template<typename Stream>
   friend Stream& operator << ( Stream& out, const chain_config& c ) {
      return out << "Max Block Net Usage: " << c.max_block_net_usage << ", "
                 << "Target Block Net Usage Percent: " << ((double)c.target_block_net_usage_pct / (double)config::percent_1) << "%, "
                 << "Max Transaction Net Usage: " << c.max_transaction_net_usage << ", "
                 << "Base Per-Transaction Net Usage: " << c.base_per_transaction_net_usage << ", "
                 << "Net Usage Leeway: " << c.net_usage_leeway << ", "
                 << "Context-Free Data Net Usage Discount: " << (double)c.context_free_discount_net_usage_num * 100.0 / (double)c.context_free_discount_net_usage_den << "% , "

                 << "Max Block CPU Usage: " << c.max_block_cpu_usage << ", "
                 << "Target Block CPU Usage Percent: " << ((double)c.target_block_cpu_usage_pct / (double)config::percent_1) << "%, "
                 << "Max Transaction CPU Usage: " << c.max_transaction_cpu_usage << ", "
                 << "Min Transaction CPU Usage: " << c.min_transaction_cpu_usage << ", "

                 << "Max Transaction Lifetime: " << c.max_transaction_lifetime << ", "
                 << "Deferred Transaction Expiration Window: " << c.deferred_trx_expiration_window << ", "
                 << "Max Transaction Delay: " << c.max_transaction_delay << ", "
                 << "Max Inline Action Size: " << c.max_inline_action_size << ", "
                 << "Max Inline Action Depth: " << c.max_inline_action_depth << ", "
                 << "Max Authority Depth: " << c.max_authority_depth << "\n";
   }

   friend inline bool operator ==( const chain_config& lhs, const chain_config& rhs ) {
      return   std::tie(   lhs.max_block_net_usage,
                           lhs.target_block_net_usage_pct,
                           lhs.max_transaction_net_usage,
                           lhs.base_per_transaction_net_usage,
                           lhs.net_usage_leeway,
                           lhs.context_free_discount_net_usage_num,
                           lhs.context_free_discount_net_usage_den,
                           lhs.max_block_cpu_usage,
                           lhs.target_block_cpu_usage_pct,
                           lhs.max_transaction_cpu_usage,
                           lhs.max_transaction_cpu_usage,
                           lhs.max_transaction_lifetime,
                           lhs.deferred_trx_expiration_window,
                           lhs.max_transaction_delay,
                           lhs.max_inline_action_size,
                           lhs.max_inline_action_depth,
                           lhs.max_authority_depth
                        )
               ==
               std::tie(   rhs.max_block_net_usage,
                           rhs.target_block_net_usage_pct,
                           rhs.max_transaction_net_usage,
                           rhs.base_per_transaction_net_usage,
                           rhs.net_usage_leeway,
                           rhs.context_free_discount_net_usage_num,
                           rhs.context_free_discount_net_usage_den,
                           rhs.max_block_cpu_usage,
                           rhs.target_block_cpu_usage_pct,
                           rhs.max_transaction_cpu_usage,
                           rhs.max_transaction_cpu_usage,
                           rhs.max_transaction_lifetime,
                           rhs.deferred_trx_expiration_window,
                           rhs.max_transaction_delay,
                           rhs.max_inline_action_size,
                           rhs.max_inline_action_depth,
                           rhs.max_authority_depth
                        );
   };

   friend inline bool operator !=( const chain_config& lhs, const chain_config& rhs ) { return !(lhs == rhs); }

};

} } //命名空间eosio:：chain

FC_REFLECT(eosio::chain::chain_config,
           (max_block_net_usage)(target_block_net_usage_pct)
           (max_transaction_net_usage)(base_per_transaction_net_usage)(net_usage_leeway)
           (context_free_discount_net_usage_num)(context_free_discount_net_usage_den)

           (max_block_cpu_usage)(target_block_cpu_usage_pct)
           (max_transaction_cpu_usage)(min_transaction_cpu_usage)

           (max_transaction_lifetime)(deferred_trx_expiration_window)(max_transaction_delay)
           (max_inline_action_size)(max_inline_action_depth)(max_authority_depth)

)
