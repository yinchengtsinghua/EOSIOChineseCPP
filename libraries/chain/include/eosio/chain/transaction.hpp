
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

#include <eosio/chain/action.hpp>
#include <numeric>

namespace eosio { namespace chain {

   /*
    *事务头包含固定大小的数据
    *与每个交易相关。它与
    *便于部分解析的事务体
    *不需要动态内存分配的事务。
    *
    *所有交易都有一个到期时间
    *可能不再包含在区块链中。曾经一个街区
    *使用块头：：时间戳大于到期时间
    *被认为是不可逆的，那么用户就可以安全地信任该事务。
    *不包括在内。
    *

    *每个区域都是一个独立的区块链，作为路由包含在内。
    *区块链间通信信息。一份合同
    *地区可能会产生或授权一项针对外国的交易。
    *区域。
    **/

   struct transaction_header {
time_point_sec         expiration;   ///<事务过期的时间
uint16_t               ref_block_num       = 0U; ///<指定最后2^16个块中的块编号。
uint32_t               ref_block_prefix    = 0UL; ///<指定get_ref_blocknum处blockid的低位32位
fc::unsigned_int       max_net_usage_words = 0UL; ///此事务的总网络带宽（8字节字）的上限
uint8_t                max_cpu_usage_ms    = 0; ///此事务的总CPU时间的上限
fc::unsigned_int       delay_sec           = 0UL; ///延迟此事务的秒数，在此期间可以取消该事务。

      /*
       *@返回给定相对参考块编号的绝对块编号
       **/

      block_num_type get_ref_blocknum( block_num_type head_blocknum )const {
         return ((head_blocknum/0xffff)*0xffff) + head_blocknum%0xffff;
      }
      void set_reference_block( const block_id_type& reference_block );
      bool verify_reference_block( const block_id_type& reference_block )const;
      void validate()const;
   };

   /*
    *事务由一组必须全部应用的消息组成，或者
    *全部被拒绝。这些消息可以访问给定的
    *读写作用域。
    **/

   struct transaction : public transaction_header {
      vector<action>         context_free_actions;
      vector<action>         actions;
      extensions_type        transaction_extensions;

      transaction_id_type        id()const;
      digest_type                sig_digest( const chain_id_type& chain_id, const vector<bytes>& cfd = vector<bytes>() )const;
      fc::microseconds           get_signature_keys( const vector<signature_type>& signatures,
                                                     const chain_id_type& chain_id,
                                                     fc::time_point deadline,
                                                     const vector<bytes>& cfd,
                                                     flat_set<public_key_type>& recovered_pub_keys,
                                                     bool allow_duplicate_keys = false) const;

      uint32_t total_actions()const { return context_free_actions.size() + actions.size(); }
      account_name first_authorizor()const {
         for( const auto& a : actions ) {
            for( const auto& u : a.authorization )
               return u.actor;
         }
         return account_name();
      }

   };

   struct signed_transaction : public transaction
   {
      signed_transaction() = default;
//signed_transaction（const signed_transaction&）=默认值；
//signed_transaction（signed_transaction&&）=默认值；
      signed_transaction( transaction&& trx, const vector<signature_type>& signatures, const vector<bytes>& context_free_data)
      : transaction(std::move(trx))
      , signatures(signatures)
      , context_free_data(context_free_data)
      {}
      signed_transaction( transaction&& trx, const vector<signature_type>& signatures, vector<bytes>&& context_free_data)
      : transaction(std::move(trx))
      , signatures(signatures)
      , context_free_data(std::move(context_free_data))
      {}

      vector<signature_type>    signatures;
vector<bytes>             context_free_data; ///<对于每个上下文无关的操作，这里有一个条目

      const signature_type&     sign(const private_key_type& key, const chain_id_type& chain_id);
      signature_type            sign(const private_key_type& key, const chain_id_type& chain_id)const;
      fc::microseconds          get_signature_keys( const chain_id_type& chain_id, fc::time_point deadline,
                                                    flat_set<public_key_type>& recovered_pub_keys,
                                                    bool allow_duplicate_keys = false )const;
   };

   struct packed_transaction {
      enum compression_type {
         none = 0,
         zlib = 1,
      };

      packed_transaction() = default;
      packed_transaction(packed_transaction&&) = default;
      explicit packed_transaction(const packed_transaction&) = default;
      packed_transaction& operator=(const packed_transaction&) = delete;
      packed_transaction& operator=(packed_transaction&&) = default;

      explicit packed_transaction(const signed_transaction& t, compression_type _compression = none)
      :signatures(t.signatures), compression(_compression), unpacked_trx(t)
      {
         local_pack_transaction();
         local_pack_context_free_data();
      }

      explicit packed_transaction(signed_transaction&& t, compression_type _compression = none)
      :signatures(t.signatures), compression(_compression), unpacked_trx(std::move(t))
      {
         local_pack_transaction();
         local_pack_context_free_data();
      }

//由abi_序列化程序使用
      packed_transaction( bytes&& packed_txn, vector<signature_type>&& sigs, bytes&& packed_cfd, compression_type _compression );
      packed_transaction( bytes&& packed_txn, vector<signature_type>&& sigs, vector<bytes>&& cfd, compression_type _compression );
      packed_transaction( transaction&& t, vector<signature_type>&& sigs, bytes&& packed_cfd, compression_type _compression );

      uint32_t get_unprunable_size()const;
      uint32_t get_prunable_size()const;

      digest_type packed_digest()const;

      transaction_id_type id()const { return unpacked_trx.id(); }
      bytes               get_raw_transaction()const;

      time_point_sec                expiration()const { return unpacked_trx.expiration; }
      const vector<bytes>&          get_context_free_data()const { return unpacked_trx.context_free_data; }
      const transaction&            get_transaction()const { return unpacked_trx; }
      const signed_transaction&     get_signed_transaction()const { return unpacked_trx; }
      const vector<signature_type>& get_signatures()const { return signatures; }
      const fc::enum_type<uint8_t,compression_type>& get_compression()const { return compression; }
      const bytes&                  get_packed_context_free_data()const { return packed_context_free_data; }
      const bytes&                  get_packed_transaction()const { return packed_trx; }

   private:
      void local_unpack_transaction(vector<bytes>&& context_free_data);
      void local_unpack_context_free_data();
      void local_pack_transaction();
      void local_pack_context_free_data();

      friend struct fc::reflector<packed_transaction>;
      friend struct fc::reflector_init_visitor<packed_transaction>;
      void reflector_init();
   private:
      vector<signature_type>                  signatures;
      fc::enum_type<uint8_t,compression_type> compression;
      bytes                                   packed_context_free_data;
      bytes                                   packed_trx;

   private:
//缓存解包的trx，为了线程安全，在构造后不要修改
      signed_transaction                      unpacked_trx;
   };

   using packed_transaction_ptr = std::shared_ptr<packed_transaction>;

   /*
    *当生成事务时，可以计划发生
    *未来。它也可能由于某些原因无法执行
    *哪种情况需要通知发件人。当发送者
    *发送一个事务，他们将为其分配一个ID，该ID将
    *如果某些事务失败，则返回给发送方
    *理由。
    **/

   struct deferred_transaction : public signed_transaction
   {
uint128_t      sender_id; ///id由生成的发送方分配，执行正常或错误时可通过WASM API访问。
account_name   sender; ///接收错误处理程序回调
      account_name   payer;
time_point_sec execute_after; ///延迟执行

      deferred_transaction() = default;

      deferred_transaction(uint128_t sender_id, account_name sender, account_name payer,time_point_sec execute_after,
                           const signed_transaction& txn)
      : signed_transaction(txn),
        sender_id(sender_id),
        sender(sender),
        payer(payer),
        execute_after(execute_after)
      {}
   };

   struct deferred_reference {
      deferred_reference(){}
      deferred_reference( const account_name& sender, const uint128_t& sender_id)
      :sender(sender),sender_id(sender_id)
      {}

      account_name   sender;
      uint128_t      sender_id;
   };

   uint128_t transaction_id_to_sender_id( const transaction_id_type& tid );

} } ///namespace eosio：：链

FC_REFLECT( eosio::chain::transaction_header, (expiration)(ref_block_num)(ref_block_prefix)
                                              (max_net_usage_words)(max_cpu_usage_ms)(delay_sec) )
FC_REFLECT_DERIVED( eosio::chain::transaction, (eosio::chain::transaction_header), (context_free_actions)(actions)(transaction_extensions) )
FC_REFLECT_DERIVED( eosio::chain::signed_transaction, (eosio::chain::transaction), (signatures)(context_free_data) )
FC_REFLECT_ENUM( eosio::chain::packed_transaction::compression_type, (none)(zlib))
FC_REFLECT( eosio::chain::packed_transaction, (signatures)(compression)(packed_context_free_data)(packed_trx) )
FC_REFLECT_DERIVED( eosio::chain::deferred_transaction, (eosio::chain::signed_transaction), (sender_id)(sender)(payer)(execute_after) )
FC_REFLECT( eosio::chain::deferred_reference, (sender)(sender_id) )
