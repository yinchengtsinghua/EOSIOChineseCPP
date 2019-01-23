
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once
#include <eosio/chain/block_timestamp.hpp>
#include <eosio/chain/producer_schedule.hpp>

namespace eosio { namespace chain {

   struct block_header
   {
      block_timestamp_type             timestamp;
      account_name                     producer;

      /*
       *通过对此块进行签名，此生成器将确认块[block_num（）-confirmed，block num（））
       *作为该范围内最好的区块，他没有签署任何其他区块。
       *与之矛盾的陈述。
       *
       *任何生产商都不应在范围重叠的块上签字，或是拜占庭式的证明。
       *行为。当生产一个区块时，生产商总是至少确认该区块。
       *正在建设。生产商不能确认“this”块，只能确认先前的块。
       **/

      uint16_t                         confirmed = 1;  

      block_id_type                    previous;

checksum256_type                 transaction_mroot; ///mroot of cycles_摘要
checksum256_type                 action_mroot; ///mroot所有已传递的操作收据


      /*应验证此块的生产商计划版本，用于
       *表示包含新的_producers->version的先前块已被标记
       *不可逆，并且新的生产商计划在此块生效。
       **/

      uint32_t                          schedule_version = 0;
      optional<producer_schedule_type>  new_producers;
      extensions_type                   header_extensions;


      digest_type       digest()const;
      block_id_type     id() const;
      uint32_t          block_num() const { return num_from_id(previous) + 1; }
      static uint32_t   num_from_id(const block_id_type& id);
   };


   struct signed_block_header : public block_header
   {
      signature_type    producer_signature;
   };

   struct header_confirmation {
      block_id_type   block_id;
      account_name    producer;
      signature_type  producer_signature;
   };

} } ///namespace eosio：：链

FC_REFLECT(eosio::chain::block_header, 
           (timestamp)(producer)(confirmed)(previous)
           (transaction_mroot)(action_mroot)
           (schedule_version)(new_producers)(header_extensions))

FC_REFLECT_DERIVED(eosio::chain::signed_block_header, (eosio::chain::block_header), (producer_signature))
FC_REFLECT(eosio::chain::header_confirmation,  (block_id)(producer)(producer_signature) )
