
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
#include <eosio/chain/exceptions.hpp>

namespace eosio { namespace chain {

   struct permission_level {
      account_name    actor;
      permission_name permission;
   };

   inline bool operator== (const permission_level& lhs, const permission_level& rhs) {
      return std::tie(lhs.actor, lhs.permission) == std::tie(rhs.actor, rhs.permission);
   }

   inline bool operator!= (const permission_level& lhs, const permission_level& rhs) {
      return std::tie(lhs.actor, lhs.permission) != std::tie(rhs.actor, rhs.permission);
   }

   inline bool operator< (const permission_level& lhs, const permission_level& rhs) {
      return std::tie(lhs.actor, lhs.permission) < std::tie(rhs.actor, rhs.permission);
   }

   inline bool operator<= (const permission_level& lhs, const permission_level& rhs) {
      return std::tie(lhs.actor, lhs.permission) <= std::tie(rhs.actor, rhs.permission);
   }

   inline bool operator> (const permission_level& lhs, const permission_level& rhs) {
      return std::tie(lhs.actor, lhs.permission) > std::tie(rhs.actor, rhs.permission);
   }

   inline bool operator>= (const permission_level& lhs, const permission_level& rhs) {
      return std::tie(lhs.actor, lhs.permission) >= std::tie(rhs.actor, rhs.permission);
   }

   /*
    *动作由参与者执行，即账户。它可能
    *由签名明确创建和授权，或者
    *通过执行应用程序代码隐式生成。
    *
    *这遵循反应通量的设计模式
    *命名，然后发送到一个或多个操作处理程序（aka stores）。
    *在eosio的上下文中，每个操作都被发送到定义的处理程序
    *按帐户“scope”和函数“name”，但默认处理程序也可以
    *将操作转发给任意数量的其他处理程序。任何应用
    *可以为“scope:：name”编写一个处理程序，该处理程序只有在
    *此操作将转发到该应用程序。
    *
    *每个操作可能需要特定参与者的许可。参与者可以定义
    *任何数量的权限级别。演员及其各自的许可
    *级别在操作上声明，并独立于执行情况进行验证。
    *应用程序代码。应用程序代码将检查是否需要授权
    *在执行时正确声明。
    **/

   struct action {
      account_name               account;
      action_name                name;
      vector<permission_level>   authorization;
      bytes                      data;

      action(){}

      template<typename T, std::enable_if_t<std::is_base_of<bytes, T>::value, int> = 1>
      action( vector<permission_level> auth, const T& value ) {
         account     = T::get_account();
         name        = T::get_name();
         authorization = move(auth);
         data.assign(value.data(), value.data() + value.size());
      }

      template<typename T, std::enable_if_t<!std::is_base_of<bytes, T>::value, int> = 1>
      action( vector<permission_level> auth, const T& value ) {
         account     = T::get_account();
         name        = T::get_name();
         authorization = move(auth);
         data        = fc::raw::pack(value);
      }

      action( vector<permission_level> auth, account_name account, action_name name, const bytes& data )
            : account(account), name(name), authorization(move(auth)), data(data) {
      }

      template<typename T>
      T data_as()const {
         EOS_ASSERT( account == T::get_account(), action_type_exception, "account is not consistent with action struct" );
         EOS_ASSERT( name == T::get_name(), action_type_exception, "action name is not consistent with action struct"  );
         return fc::raw::unpack<T>(data);
      }
   };

   struct action_notice : public action {
      account_name receiver;
   };

} } ///namespace eosio：：链

FC_REFLECT( eosio::chain::permission_level, (actor)(permission) )
FC_REFLECT( eosio::chain::action, (account)(name)(authorization)(data) )
