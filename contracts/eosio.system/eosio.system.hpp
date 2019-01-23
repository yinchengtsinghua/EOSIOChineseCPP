
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

#include <eosio.system/native.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/privileged.hpp>
#include <eosiolib/singleton.hpp>
#include <eosio.system/exchange_state.hpp>

#include <string>

namespace eosiosystem {

   using eosio::asset;
   using eosio::indexed_by;
   using eosio::const_mem_fun;
   using eosio::block_timestamp;

   struct name_bid {
     account_name            newname;
     account_name            high_bidder;
int64_t                 high_bid = 0; ///<负高出价==等待索赔的已结束拍卖
     uint64_t                last_bid_time = 0;

     auto     primary_key()const { return newname;                          }
     uint64_t by_high_bid()const { return static_cast<uint64_t>(-high_bid); }
   };

   typedef eosio::multi_index< N(namebids), name_bid,
                               indexed_by<N(highbid), const_mem_fun<name_bid, uint64_t, &name_bid::by_high_bid>  >
                               >  name_bid_table;


   struct eosio_global_state : eosio::blockchain_parameters {
      uint64_t free_ram()const { return max_ram_size - total_ram_bytes_reserved; }

      uint64_t             max_ram_size = 64ll*1024 * 1024 * 1024;
      uint64_t             total_ram_bytes_reserved = 0;
      int64_t              total_ram_stake = 0;

      block_timestamp      last_producer_schedule_update;
      uint64_t             last_pervote_bucket_fill = 0;
      int64_t              pervote_bucket = 0;
      int64_t              perblock_bucket = 0;
uint32_t             total_unpaid_blocks = 0; ///已生产但未付款的所有块
      int64_t              total_activated_stake = 0;
      uint64_t             thresh_activated_stake_time = 0;
      uint16_t             last_producer_schedule_size = 0;
double               total_producer_vote_weight = 0; ///所有制作人投票的总和
      block_timestamp      last_name_close;

//不需要显式序列化宏，此处仅用于提高编译时间
      EOSLIB_SERIALIZE_DERIVED( eosio_global_state, eosio::blockchain_parameters,
                                (max_ram_size)(total_ram_bytes_reserved)(total_ram_stake)
                                (last_producer_schedule_update)(last_pervote_bucket_fill)
                                (pervote_bucket)(perblock_bucket)(total_unpaid_blocks)(total_activated_stake)(thresh_activated_stake_time)
                                (last_producer_schedule_size)(total_producer_vote_weight)(last_name_close) )
   };

   struct producer_info {
      account_name          owner;
      double                total_votes = 0;
eosio::public_key     producer_key; ///a打包的公钥对象
      bool                  is_active = true;
      std::string           url;
      uint32_t              unpaid_blocks = 0;
      uint64_t              last_claim_time = 0;
      uint16_t              location = 0;

      uint64_t primary_key()const { return owner;                                   }
      double   by_votes()const    { return is_active ? -total_votes : total_votes;  }
      bool     active()const      { return is_active;                               }
      void     deactivate()       { producer_key = public_key(); is_active = false; }

//不需要显式序列化宏，此处仅用于提高编译时间
      EOSLIB_SERIALIZE( producer_info, (owner)(total_votes)(producer_key)(is_active)(url)
                        (unpaid_blocks)(last_claim_time)(location) )
   };

   struct voter_info {
account_name                owner = 0; //投票人
account_name                proxy = 0; ///投票人设置的代理（如果有）
std::vector<account_name>   producers; ///如果没有设置代理，则由该投票人批准的制作人。
      int64_t                     staked = 0;

      /*
       *每次投票时，我们必须先“撤销”最后一个投票权，然后再决定
       *新投票权。投票权计算如下：
       *
       *说明金额*2^（自发布后的周数/每年的周数）
       **/

double                      last_vote_weight = 0; ///上次更新投票时的投票权重

      /*
       *分配给该投票人的总投票权。
       **/

double                      proxied_vote_weight= 0; ///作为代理人委托给此投票人的总投票权
bool                        is_proxy = 0; ///投票人是否是其他人的代理人


      uint32_t                    reserved1 = 0;
      time                        reserved2 = 0;
      eosio::asset                reserved3;

      uint64_t primary_key()const { return owner; }

//不需要显式序列化宏，此处仅用于提高编译时间
      EOSLIB_SERIALIZE( voter_info, (owner)(proxy)(producers)(staked)(last_vote_weight)(proxied_vote_weight)(is_proxy)(reserved1)(reserved2)(reserved3) )
   };

   typedef eosio::multi_index< N(voters), voter_info>  voters_table;


   typedef eosio::multi_index< N(producers), producer_info,
                               indexed_by<N(prototalvote), const_mem_fun<producer_info, double, &producer_info::by_votes>  >
                               >  producers_table;

   typedef eosio::singleton<N(global), eosio_global_state> global_state_singleton;

//静态常量uint32_t max_inflation_rate=5；//5%年通货膨胀率
   static constexpr uint32_t     seconds_per_day = 24 * 3600;
   static constexpr uint64_t     system_token_symbol = CORE_SYMBOL;

   class system_contract : public native {
      private:
         voters_table           _voters;
         producers_table        _producers;
         global_state_singleton _global;

         eosio_global_state     _gstate;
         rammarket              _rammarket;

      public:
         system_contract( account_name s );
         ~system_contract();

//行动：
         void onblock( block_timestamp timestamp, account_name producer );
//const block_header&header）；///只分析block header的前3个字段

//delegate_bandwidth.cpp中定义的函数

         /*
          *从'From'余额中为'Receiver'的Benfit建立股份系统。
          *如果transfer==true，则“receiver”可以解除对其帐户的冻结。
          *否则“从”随时可以松开。
          **/

         void delegatebw( account_name from, account_name receiver,
                          asset stake_net_quantity, asset stake_cpu_quantity, bool transfer );


         /*
          *减少从委派给接收者和/或的令牌总数
          *如果没有任何内容，则释放与委派关联的内存。
          *委托。
          *
          *这将导致
          *接收器。
          *
          *计划事务在之后将令牌发送回“发件人”
          *标桩期已过。如果已调度现有事务，则
          *将被取消，并发布一个新的交易
          *未分配金额。
          *
          *由于此呼叫，“发件人”帐户失去投票权，并且
          *更新所有生产商计数。
          **/

         void undelegatebw( account_name from, account_name receiver,
                            asset unstake_net_quantity, asset unstake_cpu_quantity );


         /*
          *根据当前价格和数量增加接收器的RAM配额
          *提供代币。从接收器到系统合同的内联传输
          *将执行令牌。
          **/

         void buyram( account_name buyer, account_name receiver, asset tokens );
         void buyrambytes( account_name buyer, account_name receiver, uint32_t bytes );

         /*
          *减少我的字节配额，然后执行令牌的内联传输
          *根据原始配额的平均购买价格发送给接收者。
          **/

         void sellram( account_name receiver, int64_t bytes );

         /*
          *此操作在授权期后调用，以声明所有挂起的
          *属于所有者的未激活令牌
          **/

         void refund( account_name owner );

//voting.cpp中定义的函数

         void regproducer( const account_name producer, const public_key& producer_key, const std::string& url, uint16_t location );

         void unregprod( const account_name producer );

         void setram( uint64_t max_ram_size );

         void voteproducer( const account_name voter, const account_name proxy, const std::vector<account_name>& producers );

         void regproxy( const account_name proxy, bool isproxy );

         void setparams( const eosio::blockchain_parameters& params );

//producer_pay.cpp中定义的函数
         void claimrewards( const account_name& owner );

         void setpriv( account_name account, uint8_t ispriv );

         void rmvproducer( account_name producer );

         void bidname( account_name bidder, account_name newname, asset bid );
      private:
         void update_elected_producers( block_timestamp timestamp );

//实施细节：

//在delegate_bandwidth.cpp中定义
         void changebw( account_name from, account_name receiver,
                        asset stake_net_quantity, asset stake_cpu_quantity, bool transfer );

//在voting.hpp中定义
         static eosio_global_state get_default_parameters();

         void update_votes( const account_name voter, const account_name proxy, const std::vector<account_name>& producers, bool voting );

//在voting.cpp中定义
         void propagate_weight_change( const voter_info& voter );
   };

} //地球系统
