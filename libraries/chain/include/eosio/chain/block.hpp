
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once
#include <eosio/chain/block_header.hpp>
#include <eosio/chain/transaction.hpp>

namespace eosio { namespace chain {

   /*
    *当一个事务被一个块引用时，它可能意味着以下几种结果之一：
    *描述块生产商进行的状态转换。
    **/


   struct transaction_receipt_header {
      enum status_enum {
executed  = 0, ///<成功，未执行错误处理程序
soft_fail = 1, ///<客观上失败（未执行），执行错误处理程序
hard_fail = 2, ///<客观失败，错误处理程序客观失败，因此没有状态更改
delayed   = 3, ///<事务延迟/延迟/计划未来执行
expired   = 4  ///<事务已过期，存储空间被用户拒绝
      };

      transaction_receipt_header():status(hard_fail){}
      explicit transaction_receipt_header( status_enum s ):status(s){}

      friend inline bool operator ==( const transaction_receipt_header& lhs, const transaction_receipt_header& rhs ) {
         return std::tie(lhs.status, lhs.cpu_usage_us, lhs.net_usage_words) == std::tie(rhs.status, rhs.cpu_usage_us, rhs.net_usage_words);
      }

      fc::enum_type<uint8_t,status_enum>   status;
uint32_t                             cpu_usage_us = 0; ///<总计费CPU使用率（微秒）
fc::unsigned_int                     net_usage_words; ///<total billed net usage，因此我们可以在跳过上下文无关数据时重建资源状态…硬故障…
   };

   struct transaction_receipt : public transaction_receipt_header {

      transaction_receipt():transaction_receipt_header(){}
      explicit transaction_receipt( const transaction_id_type& tid ):transaction_receipt_header(executed),trx(tid){}
      explicit transaction_receipt( const packed_transaction& ptrx ):transaction_receipt_header(executed),trx(ptrx){}

      fc::static_variant<transaction_id_type, packed_transaction> trx;

      digest_type digest()const {
         digest_type::encoder enc;
         fc::raw::pack( enc, status );
         fc::raw::pack( enc, cpu_usage_us );
         fc::raw::pack( enc, net_usage_words );
         if( trx.contains<transaction_id_type>() )
            fc::raw::pack( enc, trx.get<transaction_id_type>() );
         else
            fc::raw::pack( enc, trx.get<packed_transaction>().packed_digest() );
         return enc.result();
      }
   };


   /*
    **/

   struct signed_block : public signed_block_header {
    private:
      signed_block( const signed_block& ) = default;
    public:
      signed_block() = default;
      explicit signed_block( const signed_block_header& h ):signed_block_header(h){}
      signed_block( signed_block&& ) = default;
      signed_block& operator=(const signed_block&) = delete;
      signed_block clone() const { return *this; }

vector<transaction_receipt>   transactions; ///新的或生成的事务
      extensions_type               block_extensions;
   };
   using signed_block_ptr = std::shared_ptr<signed_block>;

   struct producer_confirmation {
      block_id_type   block_id;
      digest_type     block_digest;
      account_name    producer;
      signature_type  sig;
   };

} } ///EOSIO：链

FC_REFLECT_ENUM( eosio::chain::transaction_receipt::status_enum,
                 (executed)(soft_fail)(hard_fail)(delayed)(expired) )

FC_REFLECT(eosio::chain::transaction_receipt_header, (status)(cpu_usage_us)(net_usage_words) )
FC_REFLECT_DERIVED(eosio::chain::transaction_receipt, (eosio::chain::transaction_receipt_header), (trx) )
FC_REFLECT_DERIVED(eosio::chain::signed_block, (eosio::chain::signed_block_header), (transactions)(block_extensions) )
