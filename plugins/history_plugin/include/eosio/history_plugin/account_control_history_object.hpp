
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

#include <chainbase/chainbase.hpp>
#include <eosio/chain/types.hpp>

namespace eosio {
using chain::account_name;
using chain::permission_name;
using chain::shared_vector;
using chain::transaction_id_type;
using namespace boost::multi_index;

class account_control_history_object : public chainbase::object<chain::account_control_history_object_type, account_control_history_object> {
   OBJECT_CTOR(account_control_history_object)

   id_type                            id;
   account_name                       controlled_account;
   permission_name                    controlled_permission;
   account_name                       controlling_account;
};

struct by_id;
struct by_controlling;
struct by_controlled_authority;
using account_control_history_multi_index = chainbase::shared_multi_index_container<
   account_control_history_object,
   indexed_by<
      ordered_unique<tag<by_id>, BOOST_MULTI_INDEX_MEMBER(account_control_history_object, account_control_history_object::id_type, id)>,
      ordered_unique<tag<by_controlling>,
         composite_key< account_control_history_object,
            member<account_control_history_object, account_name,                            &account_control_history_object::controlling_account>,
            member<account_control_history_object, account_control_history_object::id_type, &account_control_history_object::id>
         >
      >,
      ordered_unique<tag<by_controlled_authority>,
         composite_key< account_control_history_object,
            member<account_control_history_object, account_name, &account_control_history_object::controlled_account>,
            member<account_control_history_object, permission_name, &account_control_history_object::controlled_permission>,
            member<account_control_history_object, account_name, &account_control_history_object::controlling_account>
         >
      >
   >
>;

typedef chainbase::generic_index<account_control_history_multi_index> account_control_history_index;

}

CHAINBASE_SET_INDEX_TYPE( eosio::account_control_history_object, eosio::account_control_history_multi_index )

FC_REFLECT( eosio::account_control_history_object, (controlled_account)(controlled_permission)(controlling_account) )

