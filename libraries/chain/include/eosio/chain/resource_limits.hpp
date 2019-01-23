
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/types.hpp>
#include <eosio/chain/snapshot.hpp>
#include <chainbase/chainbase.hpp>
#include <set>

namespace eosio { namespace chain { namespace resource_limits {
   namespace impl {
      template<typename T>
      struct ratio {
         static_assert(std::is_integral<T>::value, "ratios must have integral types");
         T numerator;
         T denominator;
      };
   }

   using ratio = impl::ratio<uint64_t>;

   struct elastic_limit_parameters {
uint64_t target;           //所需用途
uint64_t max;              //最大使用量
uint32_t periods;          //导致平均使用率的聚合期间数

uint32_t max_multiplier;   //虚拟空间在未被占用时可以过度使用的乘法器。
ratio    contract_rate;    //拥挤的资源收缩其限制的速率
ratio    expand_rate;       //未聚集资源扩展其限制的速率。

void validate()const; //如果参数不满足基本健全性检查，则引发
   };

   struct account_resource_limit {
int64_t used = 0; ///<当前窗口中使用的数量
int64_t available = 0; ///<当前窗口中的可用数量（基于分数保留）
int64_t max = 0; ///<当前拥塞下每个窗口的最大值
   };

   class resource_limits_manager {
      public:
         explicit resource_limits_manager(chainbase::database& db)
         :_db(db)
         {
         }

         void add_indices();
         void initialize_database();
         void add_to_snapshot( const snapshot_writer_ptr& snapshot ) const;
         void read_from_snapshot( const snapshot_reader_ptr& snapshot );

         void initialize_account( const account_name& account );
         void set_block_parameters( const elastic_limit_parameters& cpu_limit_parameters, const elastic_limit_parameters& net_limit_parameters );

         void update_account_usage( const flat_set<account_name>& accounts, uint32_t ordinal );
         void add_transaction_usage( const flat_set<account_name>& accounts, uint64_t cpu_usage, uint64_t net_usage, uint32_t ordinal );

         void add_pending_ram_usage( const account_name account, int64_t ram_delta );
         void verify_account_ram_usage( const account_name accunt )const;

///set_account_limits如果新的RAM_字节限制比以前设置的限制更严格，则返回true。
         bool set_account_limits( const account_name& account, int64_t ram_bytes, int64_t net_weight, int64_t cpu_weight);
         void get_account_limits( const account_name& account, int64_t& ram_bytes, int64_t& net_weight, int64_t& cpu_weight) const;

         void process_account_limit_updates();
         void process_block_usage( uint32_t block_num );

//访问器
         uint64_t get_virtual_block_cpu_limit() const;
         uint64_t get_virtual_block_net_limit() const;

         uint64_t get_block_cpu_limit() const;
         uint64_t get_block_net_limit() const;

         int64_t get_account_cpu_limit( const account_name& name, bool elastic = true) const;
         int64_t get_account_net_limit( const account_name& name, bool elastic = true) const;

         account_resource_limit get_account_cpu_limit_ex( const account_name& name, bool elastic = true) const;
         account_resource_limit get_account_net_limit_ex( const account_name& name, bool elastic = true) const;

         int64_t get_account_ram_usage( const account_name& name ) const;

      private:
         chainbase::database& _db;
   };
} } } ///EOSIO：链

FC_REFLECT( eosio::chain::resource_limits::account_resource_limit, (used)(available)(max) )
FC_REFLECT( eosio::chain::resource_limits::ratio, (numerator)(denominator))
FC_REFLECT( eosio::chain::resource_limits::elastic_limit_parameters, (target)(max)(periods)(max_multiplier)(contract_rate)(expand_rate))
