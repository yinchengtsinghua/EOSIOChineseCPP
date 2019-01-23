
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
#include <eosio/chain/authority.hpp>

#include "multi_index_includes.hpp"

namespace eosio { namespace chain {
   /*
    *@brief权限链接对象类为消息类型分配权限对象
    *
    *此类记录从合同和这些合同中的消息类型到权限对象的链接
    *由用户定义，用于记录在这些合同中执行这些消息所需的权限。为了
    *例如，假设我们有一个名为“currency”的合同，并且该合同定义了一个名为“transfer”的消息。
    *此外，假设用户“joe”具有一个称为“money”的权限级别，joe希望
    *权限级别，以便其帐户调用currency.transfer。要做到这一点，乔会创建一个
    *permission以“currency”作为代码帐户，以“transfer”作为消息类型为帐户链接对象，以及
    *“金钱”作为所需的许可。之后，为了验证发送到“货币”类型的任何消息
    *需要乔批准的“转让”需要足够的签名来满足乔的“金钱”权威。
    *
    *帐户可以设置到单个消息类型的链接，或者为所有消息设置默认权限要求。
    *特定合同。若要将所有消息的默认值设置为给定协定，请将@ref message\u type设置为空
    *字符串。在查找要使用的权限时，如果找到特定帐户、代码的链接，
    *消息类型三元组，然后使用该链接所需的权限。如果没有找到这样的链接，而是一个链接
    *为帐户找到，代码为“”，然后使用该链接所需的权限。如果找不到这样的链接，
    *使用账户的有效权限。
    **/

   class permission_link_object : public chainbase::object<permission_link_object_type, permission_link_object> {
      OBJECT_CTOR(permission_link_object)

      id_type        id;
///定义其权限要求的帐户
      account_name    account;
///the contract which account required@ref required_permission to invoke
account_name    code; ///TODO:重命名为作用域
///the message type which account required@ref required_permission to invoke
///可能为空；如果为空，则将所有消息的默认@ref必需权限设置为@ref code
      action_name       message_type;
///@ref account为指定的消息类型要求的权限级别
      permission_name required_permission;
   };

   struct by_action_name;
   struct by_permission_name;
   using permission_link_index = chainbase::shared_multi_index_container<
      permission_link_object,
      indexed_by<
         ordered_unique<tag<by_id>,
            BOOST_MULTI_INDEX_MEMBER(permission_link_object, permission_link_object::id_type, id)
         >,
         ordered_unique<tag<by_action_name>,
            composite_key<permission_link_object,
               BOOST_MULTI_INDEX_MEMBER(permission_link_object, account_name, account),
               BOOST_MULTI_INDEX_MEMBER(permission_link_object, account_name, code),
               BOOST_MULTI_INDEX_MEMBER(permission_link_object, action_name, message_type)
            >
         >,
         ordered_unique<tag<by_permission_name>,
            composite_key<permission_link_object,
               BOOST_MULTI_INDEX_MEMBER(permission_link_object, account_name, account),
               BOOST_MULTI_INDEX_MEMBER(permission_link_object, permission_name, required_permission),
               BOOST_MULTI_INDEX_MEMBER(permission_link_object, account_name, code),
               BOOST_MULTI_INDEX_MEMBER(permission_link_object, action_name, message_type)
            >
         >
      >
   >;

   namespace config {
      template<>
      struct billable_size<permission_link_object> {
static const uint64_t overhead = overhead_per_row_per_index_ram_bytes * 3; ///<3x索引ID、操作、权限
static const uint64_t value = 40 + overhead; ///<固定字段+开销
      };
   }
} } //EOSIO：链

CHAINBASE_SET_INDEX_TYPE(eosio::chain::permission_link_object, eosio::chain::permission_link_index)

FC_REFLECT(eosio::chain::permission_link_object, (account)(code)(message_type)(required_permission))
