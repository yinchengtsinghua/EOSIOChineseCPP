
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
#include <eosio/chain/database_utils.hpp>

#include "multi_index_includes.hpp"

namespace eosio { namespace chain {

   class permission_usage_object : public chainbase::object<permission_usage_object_type, permission_usage_object> {
      OBJECT_CTOR(permission_usage_object)

      id_type           id;
time_point        last_used;   ///<上次使用此权限的时间
   };

   struct by_account_permission;
   using permission_usage_index = chainbase::shared_multi_index_container<
      permission_usage_object,
      indexed_by<
         ordered_unique<tag<by_id>, member<permission_usage_object, permission_usage_object::id_type, &permission_usage_object::id>>
      >
   >;


   class permission_object : public chainbase::object<permission_object_type, permission_object> {
      OBJECT_CTOR(permission_object, (auth) )

      id_type                           id;
      permission_usage_object::id_type  usage_id;
id_type                           parent; ///<父权限
account_name                      owner; ///<此权限所属的帐户
permission_name                   name; ///<权限的可读名称
time_point                        last_updated; ///<上次更新此授权的时间
shared_authority                  auth; ///<执行此权限所需的权限


      /*
       *@简要检查此权限是否等同于或大于其他权限
       *@tparam index权限索引
       *@如果此权限等于或大于其他权限，则返回true；否则返回false
       *
       *权限是按层次结构组织的，因此父权限的功能要比它的
       *子女/孙子女。此方法检查此权限是否具有更大或相等的权限（能够
       *令人满意）许可@参考其他。
       **/

      template <typename Index>
      bool satisfies(const permission_object& other, const Index& permission_index) const {
//如果所有者不同，则此权限不能满足其他
         if( owner != other.owner )
            return false;

//如果此权限与其他权限匹配，或者是其他权限的直接父级，则此权限满足其他权限
         if( id == other.id || id == other.parent )
            return true;

//走到别人的父树上，看看我们是否找到这个权限。如果是这样，此权限满足其他
         const permission_object* parent = &*permission_index.template get<by_id>().find(other.parent);
         while( parent ) {
            if( id == parent->parent )
               return true;
            if( parent->parent._id == 0 )
               return false;
            parent = &*permission_index.template get<by_id>().find(parent->parent);
         }
//此权限不是其他权限的父级，因此不满足其他权限
         return false;
      }
   };

   /*
    *特殊情况下，提取外键以供使用，并优化父级使用OID
    **/

   struct snapshot_permission_object {
permission_name   parent; ///<父权限
account_name      owner; ///<此权限所属的帐户
permission_name   name; ///<权限的可读名称
time_point        last_updated; ///<上次更新此授权的时间
time_point        last_used; ///<上次使用此权限的时间
authority         auth; ///<执行此权限所需的权限
   };


   struct by_parent;
   struct by_owner;
   struct by_name;
   using permission_index = chainbase::shared_multi_index_container<
      permission_object,
      indexed_by<
         ordered_unique<tag<by_id>, member<permission_object, permission_object::id_type, &permission_object::id>>,
         ordered_unique<tag<by_parent>,
            composite_key<permission_object,
               member<permission_object, permission_object::id_type, &permission_object::parent>,
               member<permission_object, permission_object::id_type, &permission_object::id>
            >
         >,
         ordered_unique<tag<by_owner>,
            composite_key<permission_object,
               member<permission_object, account_name, &permission_object::owner>,
               member<permission_object, permission_name, &permission_object::name>
            >
         >,
         ordered_unique<tag<by_name>,
            composite_key<permission_object,
               member<permission_object, permission_name, &permission_object::name>,
               member<permission_object, permission_object::id_type, &permission_object::id>
            >
         >
      >
   >;

   namespace config {
      template<>
struct billable_size<permission_object> { //还计算关联权限\使用情况\对象的内存使用情况
static const uint64_t  overhead = 5 * overhead_per_row_per_index_ram_bytes; ///<5个索引2x内部ID、父、所有者、名称
static const uint64_t  value = (config::billable_size_v<shared_authority> + 64) + overhead;  ///<固定字段大小+开销
      };
   }
} } //EOSIO：链

CHAINBASE_SET_INDEX_TYPE(eosio::chain::permission_object, eosio::chain::permission_index)
CHAINBASE_SET_INDEX_TYPE(eosio::chain::permission_usage_object, eosio::chain::permission_usage_index)

FC_REFLECT(eosio::chain::permission_object, (usage_id)(parent)(owner)(name)(last_updated)(auth))
FC_REFLECT(eosio::chain::snapshot_permission_object, (parent)(owner)(name)(last_updated)(last_used)(auth))

FC_REFLECT(eosio::chain::permission_usage_object, (last_used))
