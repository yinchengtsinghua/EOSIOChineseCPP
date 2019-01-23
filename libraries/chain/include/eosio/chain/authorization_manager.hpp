
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
#include <eosio/chain/permission_object.hpp>
#include <eosio/chain/snapshot.hpp>

#include <utility>
#include <functional>

namespace eosio { namespace chain {

   class controller;
   struct updateauth;
   struct deleteauth;
   struct linkauth;
   struct unlinkauth;
   struct canceldelay;

   class authorization_manager {
      public:
         using permission_id_type = permission_object::id_type;

         explicit authorization_manager(controller& c, chainbase::database& d);

         void add_indices();
         void initialize_database();
         void add_to_snapshot( const snapshot_writer_ptr& snapshot ) const;
         void read_from_snapshot( const snapshot_reader_ptr& snapshot );

         const permission_object& create_permission( account_name account,
                                                     permission_name name,
                                                     permission_id_type parent,
                                                     const authority& auth,
                                                     time_point initial_creation_time = time_point()
                                                   );

         const permission_object& create_permission( account_name account,
                                                     permission_name name,
                                                     permission_id_type parent,
                                                     authority&& auth,
                                                     time_point initial_creation_time = time_point()
                                                   );

         void modify_permission( const permission_object& permission, const authority& auth );

         void remove_permission( const permission_object& permission );

         void update_permission_usage( const permission_object& permission );

         fc::time_point get_permission_last_used( const permission_object& permission )const;

         const permission_object*  find_permission( const permission_level& level )const;
         const permission_object&  get_permission( const permission_level& level )const;

         /*
          *@brief查找@ref authorizer_帐户授权消息所需的最低权限级别
          *指定类型
          *@param authorizer_帐户授权邮件的帐户
          *@param code_account发布处理消息的合同的帐户
          *@param键入消息类型
          **/

         optional<permission_name> lookup_minimum_permission( account_name authorizer_account,
                                                              scope_name code_account,
                                                              action_name type
                                                            )const;

         /*
          *@简要检查具有所提供密钥、权限级别和延迟的操作向量的授权
          *
          *@param actions-检查授权的操作
          *@param提供了\u个密钥-授权该事务的一组公钥
          *@param provided_permissions-授权事务的权限集（空权限名用作通配符）
          *@param provided_delay-事务满足的延迟
          *@param checktime-在检查授权的过程中，可以调用跟踪CPU使用情况和时间的函数
          *@param allow_unused_keys-true if方法不应在未使用的键上断言
          **/

         void
         check_authorization( const vector<action>&                actions,
                              const flat_set<public_key_type>&     provided_keys,
                              const flat_set<permission_level>&    provided_permissions = flat_set<permission_level>(),
                              fc::microseconds                     provided_delay = fc::microseconds(0),
                              const std::function<void()>&         checktime = std::function<void()>(),
                              bool                                 allow_unused_keys = false,
                              const flat_set<permission_level>&    satisfied_authorizations = flat_set<permission_level>()
                            )const;


         /*
          *@使用提供的密钥、权限级别和延迟对权限的授权进行简要检查
          *
          *@param account-权限的帐户所有者
          *@param permission-检查授权的权限名称
          *@param提供了一组公钥
          *@param provided_permissions-可以认为满足的权限集（空权限名用作通配符）
          *@param providedou delay-被认为满足授权检查的延迟
          *@param checktime-在检查授权的过程中，可以调用跟踪CPU使用情况和时间的函数
          *@param allow_unused_keys-如果方法不需要使用所有键，则为true
          **/

         void
         check_authorization( account_name                         account,
                              permission_name                      permission,
                              const flat_set<public_key_type>&     provided_keys,
                              const flat_set<permission_level>&    provided_permissions = flat_set<permission_level>(),
                              fc::microseconds                     provided_delay = fc::microseconds(0),
                              const std::function<void()>&         checktime = std::function<void()>(),
                              bool                                 allow_unused_keys = false
                            )const;

         flat_set<public_key_type> get_required_keys( const transaction& trx,
                                                      const flat_set<public_key_type>& candidate_keys,
                                                      fc::microseconds provided_delay = fc::microseconds(0)
                                                    )const;


         static std::function<void()> _noop_checktime;

      private:
         const controller&    _control;
         chainbase::database& _db;

         void             check_updateauth_authorization( const updateauth& update, const vector<permission_level>& auths )const;
         void             check_deleteauth_authorization( const deleteauth& del, const vector<permission_level>& auths )const;
         void             check_linkauth_authorization( const linkauth& link, const vector<permission_level>& auths )const;
         void             check_unlinkauth_authorization( const unlinkauth& unlink, const vector<permission_level>& auths )const;
         fc::microseconds check_canceldelay_authorization( const canceldelay& cancel, const vector<permission_level>& auths )const;

         optional<permission_name> lookup_linked_permission( account_name authorizer_account,
                                                             scope_name code_account,
                                                             action_name type
                                                           )const;
   };

} } ///namespace eosio：：链
