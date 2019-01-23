
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
#include <eosio/chain/database_utils.hpp>

#include <eosio/chain/transaction.hpp>
#include <fc/uint128.hpp>

#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>

#include "multi_index_includes.hpp"

namespace eosio { namespace chain {
   using boost::multi_index_container;
   using namespace boost::multi_index;
   /*
    *此对象的目的是存储通过处理
    *链中包含的交易。这些交易应被视为
    *为安排交易而签署的真实/有效的交易
    *插入到新块
    **/

   class generated_transaction_object : public chainbase::object<generated_transaction_object_type, generated_transaction_object>
   {
         OBJECT_CTOR(generated_transaction_object, (packed_trx) )

         id_type                       id;
         transaction_id_type           trx_id;
         account_name                  sender;
uint128_t                     sender_id = 0; ///id由发件人提供此事务
         account_name                  payer;
time_point                    delay_until; ///在指定的时间之前，不会应用此生成的事务
time_point                    expiration; ///this generated transaction will not applied after this time
         time_point                    published;
         shared_blob                   packed_trx;

         uint32_t set( const transaction& trx ) {
            auto trxsize = fc::raw::pack_size( trx );
            packed_trx.resize( trxsize );
            fc::datastream<char*> ds( packed_trx.data(), trxsize );
            fc::raw::pack( ds, trx );
            return trxsize;
         }
   };

   struct by_trx_id;
   struct by_expiration;
   struct by_delay;
   struct by_status;
   struct by_sender_id;

   using generated_transaction_multi_index = chainbase::shared_multi_index_container<
      generated_transaction_object,
      indexed_by<
         ordered_unique< tag<by_id>, BOOST_MULTI_INDEX_MEMBER(generated_transaction_object, generated_transaction_object::id_type, id)>,
         ordered_unique< tag<by_trx_id>, BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, transaction_id_type, trx_id)>,
         ordered_unique< tag<by_expiration>,
            composite_key< generated_transaction_object,
               BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, time_point, expiration),
               BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, generated_transaction_object::id_type, id)
            >
         >,
         ordered_unique< tag<by_delay>,
            composite_key< generated_transaction_object,
               BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, time_point, delay_until),
               BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, generated_transaction_object::id_type, id)
            >
         >,
         ordered_unique< tag<by_sender_id>,
            composite_key< generated_transaction_object,
               BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, account_name, sender),
               BOOST_MULTI_INDEX_MEMBER( generated_transaction_object, uint128_t, sender_id)
            >
         >
      >
   >;

   class generated_transaction
   {
      public:
         generated_transaction(const generated_transaction_object& gto)
         :trx_id(gto.trx_id)
         ,sender(gto.sender)
         ,sender_id(gto.sender_id)
         ,payer(gto.payer)
         ,delay_until(gto.delay_until)
         ,expiration(gto.expiration)
         ,published(gto.published)
         ,packed_trx(gto.packed_trx.begin(), gto.packed_trx.end())
         {}

         generated_transaction(const generated_transaction& gt) = default;
         generated_transaction(generated_transaction&& gt) = default;

         transaction_id_type           trx_id;
         account_name                  sender;
         uint128_t                     sender_id;
         account_name                  payer;
time_point                    delay_until; ///在指定的时间之前，不会应用此生成的事务
time_point                    expiration; ///this generated transaction will not applied after this time
         time_point                    published;
         vector<char>                  packed_trx;

   };

   namespace config {
      template<>
      struct billable_size<generated_transaction_object> {
static const uint64_t overhead = overhead_per_row_per_index_ram_bytes * 5;  ///<5x索引内部密钥、txid、过期、延迟、发送方\u id的开销
static const uint64_t value = 96 + 4 + overhead; ///<96字节（对于我们的常量大小字段），4字节（对于打包后的变量）和96字节的实现开销
      };
   }
} } //EOSIO：链

CHAINBASE_SET_INDEX_TYPE(eosio::chain::generated_transaction_object, eosio::chain::generated_transaction_multi_index)

FC_REFLECT(eosio::chain::generated_transaction_object, (trx_id)(sender)(sender_id)(payer)(delay_until)(expiration)(published)(packed_trx))