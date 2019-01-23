
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
#include <eosio/chain/wasm_interface.hpp>
#include <fc/time.hpp>

#pragma GCC diagnostic ignored "-Wunused-variable"

namespace eosio { namespace chain { namespace config {

typedef __uint128_t uint128_t;

const static auto default_blocks_dir_name    = "blocks";
const static auto reversible_blocks_dir_name = "reversible";
const static auto default_reversible_cache_size = 340*1024*1024ll;///1MB*340块，基于21个生产商bft延迟
const static auto default_reversible_guard_size = 2*1024*1024ll;///1MB*340块，基于21个生产商bft延迟

const static auto default_state_dir_name     = "state";
const static auto forkdb_filename            = "forkdb.dat";
const static auto default_state_size            = 1*1024*1024*1024ll;
const static auto default_state_guard_size      =    128*1024*1024ll;


const static uint64_t system_account_name    = N(eosio);
const static uint64_t null_account_name      = N(eosio.null);
const static uint64_t producers_account_name = N(eosio.prods);

//生产者帐户的有效许可要求超过2/3的生产者授权
const static uint64_t majority_producers_permission_name = N(prod.major); //超过1/2的生产商需要授权
const static uint64_t minority_producers_permission_name = N(prod.minor); //超过1/3的生产商需要授权0

const static uint64_t eosio_auth_scope       = N(eosio.auth);
const static uint64_t eosio_all_scope        = N(eosio.all);

const static uint64_t active_name = N(active);
const static uint64_t owner_name  = N(owner);
const static uint64_t eosio_any_name = N(eosio.any);
const static uint64_t eosio_code_name = N(eosio.code);

const static int      block_interval_ms = 500;
const static int      block_interval_us = block_interval_ms*1000;
const static uint64_t block_timestamp_epoch = 946684800000ll; //纪元是2000年。

/*百分比是固定点，分母为10000。*/
const static int percent_100 = 10000;
const static int percent_1   = 100;

static const uint32_t account_cpu_usage_average_window_ms  = 24*60*60*1000l;
static const uint32_t account_net_usage_average_window_ms  = 24*60*60*1000l;
static const uint32_t block_cpu_usage_average_window_ms    = 60*1000l;
static const uint32_t block_size_average_window_ms         = 60*1000l;

//const static uint64_t默认_max_storage_size=10*1024；
//const static uint32_t默认_max_trx_runtime=10*1000；
//const static uint32_t默认_max_gen_trx_size=64*1024；

const static uint32_t   rate_limiting_precision        = 1000*1000;


const static uint32_t   default_max_block_net_usage                 = 1024 * 1024; ///在500ms块和200byte trx时，这将启用约10000tps突发
const static uint32_t   default_target_block_net_usage_pct           = 10 * percent_1; ///我们的目标是1000 tps
const static uint32_t   default_max_transaction_net_usage            = default_max_block_net_usage / 2;
const static uint32_t   default_base_per_transaction_net_usage       = 12;  //12字节（最坏情况下为11字节的事务_receipt_header+1字节的静态_variant标记）
const static uint32_t   default_net_usage_leeway                     = 500; //托多：这合理吗？
const static uint32_t   default_context_free_discount_net_usage_num  = 20; //托多：这合理吗？
const static uint32_t   default_context_free_discount_net_usage_den  = 100;
const static uint32_t   transaction_id_net_usage                     = 32; //事务ID大小为32字节

const static uint32_t   default_max_block_cpu_usage                 = 200'000; ///max以微秒为单位阻止CPU使用
const static uint32_t   default_target_block_cpu_usage_pct          = 10 * percent_1;
const static uint32_t   default_max_transaction_cpu_usage           = 3*default_max_block_cpu_usage/4; ///max trx CPU使用率（以微秒计）
const static uint32_t   default_min_transaction_cpu_usage           = 100; ///min以微秒为单位的trx CPU使用率（10000 tps当量）

const static uint32_t   default_max_trx_lifetime               = 60*60; //1小时
const static uint32_t   default_deferred_trx_expiration_window = 10*60; //10分钟
const static uint32_t   default_max_trx_delay                  = 45*24*3600; //45天
const static uint32_t   default_max_inline_action_size         = 4 * 1024;   //4千字节
const static uint16_t   default_max_inline_action_depth        = 4;
const static uint16_t   default_max_auth_depth                 = 6;
const static uint32_t   default_sig_cpu_bill_pct               = 50 * percent_1; //签名恢复的计费百分比
const static uint16_t   default_controller_thread_pool_size    = 2;

const static uint32_t   min_net_usage_delta_between_base_and_max_for_trx  = 10*1024;
//应该足够大，以便在没有硬分叉的情况下从设置错误的区块链参数中恢复。
//（除非net-usage-leeway设置为0，所有账户的净限额也设置为0，这有助于重置区块链参数）。

const static uint32_t   fixed_net_overhead_of_packed_trx = 16; //托多：这合理吗？

const static uint32_t   fixed_overhead_shared_vector_ram_bytes = 16; ///<共享向量场大小的固定部分的开销
const static uint32_t   overhead_per_row_per_index_ram_bytes = 32;    ///<每个索引一行中基本跟踪结构的间接费用科目
const static uint32_t   overhead_per_account_ram_bytes     = 2*1024; ///<基本帐户存储和预付款功能（如帐户恢复）的间接费用帐户
const static uint32_t   setcode_ram_bytes_multiplier       = 10;     ///<考虑多个副本和缓存编译的合同大小乘数

const static uint32_t   hashing_checktime_block_size       = 10*1024;  ///CALL CHECKTIME FROM HASHING INTERNAL每个字节数一次

const static eosio::chain::wasm_interface::vm_type default_wasm_runtime = eosio::chain::wasm_interface::vm_type::wabt;
const static uint32_t   default_abi_serializer_max_time_ms = 15*1000; ///<ABI序列化方法的默认截止时间

/*
 *单个生产商生产的顺序块的数量
 **/

const static int producer_repetitions = 12;
const static int max_producers = 125;

const static size_t maximum_tracked_dpos_confirmations = 1024;     //<
static_assert(maximum_tracked_dpos_confirmations >= ((max_producers * 2 / 3) + 1) * producer_repetitions, "Settings never allow for DPOS irreversibility" );


/*
 *每轮生产的块数取决于所有有机会生产的生产商。
 *生成所有连续块。
 **/

//const static int blocks_per_round=生产商计数*生产商重复次数；

const static int irreversible_threshold_percent= 70 * percent_1;

const static uint64_t billable_alignment = 16;

template<typename T>
struct billable_size;

template<typename T>
constexpr uint64_t billable_size_v = ((billable_size<T>::value + billable_alignment - 1) / billable_alignment) * billable_alignment;


} } } //命名空间eosio:：chain:：config

constexpr uint64_t EOS_PERCENT(uint64_t value, uint32_t percentage) {
   return (value * percentage) / eosio::chain::config::percent_100;
}

template<typename Number>
Number EOS_PERCENT_CEIL(Number value, uint32_t percentage) {
   return ((value * percentage) + eosio::chain::config::percent_100 - eosio::chain::config::percent_1)  / eosio::chain::config::percent_100;
}
