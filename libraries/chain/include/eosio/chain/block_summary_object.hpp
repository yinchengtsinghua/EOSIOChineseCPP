
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

#include "multi_index_includes.hpp"

namespace eosio { namespace chain {
   /*
    *@brief跟踪有关实现tapos的过去块的最少信息
    *@ingroup对象
    *
    *当试图计算交易的有效性时，我们需要
    *查找一个过去的块并检查其块散列和发生的时间
    *因此，我们可以计算当前交易是否有效，并且
    *到期时间。
    **/

   class block_summary_object : public chainbase::object<block_summary_object_type, block_summary_object>
   {
         OBJECT_CTOR(block_summary_object)

         id_type        id;
         block_id_type  block_id;
   };

   struct by_block_id;
   using block_summary_multi_index = chainbase::shared_multi_index_container<
      block_summary_object,
      indexed_by<
         ordered_unique<tag<by_id>, BOOST_MULTI_INDEX_MEMBER(block_summary_object, block_summary_object::id_type, id)>
//按“block”id>排序的“unique<tag”，boost“multi-index”成员（block“summary”对象，block“id”类型，block“id”）>
      >
   >;

} }

CHAINBASE_SET_INDEX_TYPE(eosio::chain::block_summary_object, eosio::chain::block_summary_multi_index)

FC_REFLECT( eosio::chain::block_summary_object, (block_id) )
