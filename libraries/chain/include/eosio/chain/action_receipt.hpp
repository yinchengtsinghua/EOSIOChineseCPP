
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

namespace eosio { namespace chain {

   /*
    *对于发送的每个操作，都会生成此收据。
    **/

   struct action_receipt {
      account_name                    receiver;
      digest_type                     act_digest;
uint64_t                        global_sequence = 0; ///<Genesis以来调度的操作总数
uint64_t                        recv_sequence   = 0; ///<自Genesis以来此接收器的操作总数
      flat_map<account_name,uint64_t> auth_sequence;
fc::unsigned_int                code_sequence = 0; ///<设置代码总数
fc::unsigned_int                abi_sequence  = 0; ///<setabis总数

      digest_type digest()const { return digest_type::hash(*this); }
   };

} }  ///namespace eosio：：链

FC_REFLECT( eosio::chain::action_receipt, (receiver)(act_digest)(global_sequence)(recv_sequence)(auth_sequence)(code_sequence)(abi_sequence) )
