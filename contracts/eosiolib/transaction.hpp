
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
#include <eosiolib/transaction.h>
#include <eosiolib/action.hpp>
#include <eosiolib/types.hpp>
#include <eosiolib/time.hpp>
#include <eosiolib/serialize.hpp>
#include <vector>

namespace eosio {

   /*
    *df群组事务处理CPAPI事务C++
    *@ingroup事务处理api
    *@事务C API的简短类型安全C++包装
    *
    *>注意，可以从C++直接使用的@ REF事务处理CAPI中有一些方法
    *
    *@
    **/


   class transaction_header {
   public:
      transaction_header( time_point_sec exp = time_point_sec(now() + 60) )
         :expiration(exp)
      {}

      time_point_sec  expiration;
      uint16_t        ref_block_num;
      uint32_t        ref_block_prefix;
unsigned_int    net_usage_words = 0UL; ///压缩后此事务可以序列化为的8字节字数
uint8_t         max_cpu_usage_ms = 0UL; ///要为其记帐的CPU使用单位数
unsigned_int    delay_sec = 0UL; ///要为其记帐的CPU使用单位数

      EOSLIB_SERIALIZE( transaction_header, (expiration)(ref_block_num)(ref_block_prefix)(net_usage_words)(max_cpu_usage_ms)(delay_sec) )
   };

   class transaction : public transaction_header {
   public:
      transaction(time_point_sec exp = time_point_sec(now() + 60)) : transaction_header( exp ) {}

      void send(const uint128_t& sender_id, account_name payer, bool replace_existing = false) const {
         auto serialize = pack(*this);
         send_deferred(sender_id, payer, serialize.data(), serialize.size(), replace_existing);
      }

      vector<action>  context_free_actions;
      vector<action>  actions;
      extensions_type transaction_extensions;

      EOSLIB_SERIALIZE_DERIVED( transaction, transaction_header, (context_free_actions)(actions)(transaction_extensions) )
   };

   /*
    *
    *
    *
    *
    **/

   struct onerror {
      uint128_t sender_id;
      bytes     sent_trx;

      static onerror from_current_action() {
         return unpack_action_data<onerror>();
      }

      transaction unpack_sent_trx() const {
         return unpack<transaction>(sent_trx);
      }

      EOSLIB_SERIALIZE( onerror, (sender_id)(sent_trx) )
   };

   /*
    *从活动事务中检索指示的操作。
    *@param type-0表示上下文无关操作，1表示操作
    *@param index-请求操作的索引
    *@返回指示动作
    **/

   inline action get_action( uint32_t type, uint32_t index ) {
      constexpr size_t max_stack_buffer_size = 512;
      int s = ::get_action( type, index, nullptr, 0 );
      eosio_assert( s > 0, "get_action size failed" );
      size_t size = static_cast<size_t>(s);
      char* buffer = (char*)( max_stack_buffer_size < size ? malloc(size) : alloca(size) );
      auto size2 = ::get_action( type, index, buffer, size );
      eosio_assert( size == static_cast<size_t>(size2), "get_action failed" );
      return eosio::unpack<eosio::action>( buffer, size );
   }

///@事务CPP API

} //命名空间EOS
