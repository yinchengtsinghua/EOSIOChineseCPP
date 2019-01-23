
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
#include <eosio/chain/authority.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/parallel_markers.hpp>

#include <fc/scoped_exit.hpp>

#include <boost/range/algorithm/find.hpp>
#include <boost/algorithm/cxx11/all_of.hpp>

#include <functional>

namespace eosio { namespace chain {

namespace detail {

//静态变量中模板类型的顺序关系到meta-permission-comparator。
   using meta_permission = static_variant<permission_level_weight, key_weight, wait_weight>;

   struct get_weight_visitor {
      using result_type = uint32_t;

      template<typename Permission>
      uint32_t operator()( const Permission& permission ) { return permission.weight; }
   };

//按权重降序排列权限，并断开与等待权限小于的关系
//依次小于帐户权限的密钥权限
   struct meta_permission_comparator {
      bool operator()( const meta_permission& lhs, const meta_permission& rhs ) const {
         get_weight_visitor scale;
         auto lhs_weight = lhs.visit(scale);
         auto lhs_type   = lhs.which();
         auto rhs_weight = rhs.visit(scale);
         auto rhs_type   = rhs.which();
         return std::tie( lhs_weight, lhs_type ) > std::tie( rhs_weight, rhs_type );
      }
   };

   using meta_permission_set = boost::container::flat_multiset<meta_permission, meta_permission_comparator>;

} ///命名空间详细信息

   /*
    *@brief此类确定一组签名密钥是否足以满足权限要求
    *
    *为了确定是否满足授权，我们首先确定哪些密钥已批准消息，以及
    *然后确定该密钥列表是否足以满足权限要求。这个类获取一个键列表和
    *提供@ref满足方法来确定密钥列表是否满足所提供的权限。
    *
    *@tparam f一个接受@ref accountpermission类型的单个参数并返回相应的
    *当局
    **/

   template<typename PermissionToAuthorityFunc>
   class authority_checker {
      private:
         PermissionToAuthorityFunc            permission_to_authority;
         const std::function<void()>&         checktime;
vector<public_key_type>              provided_keys; //将其设为“平面集”<public_key_type>会由于某种原因导致utilities:：filter_data_by_marker出现运行时问题。托多：找出原因。
         flat_set<permission_level>           provided_permissions;
         vector<bool>                         _used_keys;
         fc::microseconds                     provided_delay;
         uint16_t                             recursion_depth_limit;

      public:
         authority_checker( PermissionToAuthorityFunc            permission_to_authority,
                            uint16_t                             recursion_depth_limit,
                            const flat_set<public_key_type>&     provided_keys,
                            const flat_set<permission_level>&    provided_permissions,
                            fc::microseconds                     provided_delay,
                            const std::function<void()>&         checktime
                         )
         :permission_to_authority(permission_to_authority)
         ,checktime( checktime )
         ,provided_keys(provided_keys.begin(), provided_keys.end())
         ,provided_permissions(provided_permissions)
         ,_used_keys(provided_keys.size(), false)
         ,provided_delay(provided_delay)
         ,recursion_depth_limit(recursion_depth_limit)
         {
            EOS_ASSERT( static_cast<bool>(checktime), authorization_exception, "checktime cannot be empty" );
         }

         enum permission_cache_status {
            being_evaluated,
            permission_unsatisfied,
            permission_satisfied
         };

         typedef map<permission_level, permission_cache_status> permission_cache_type;

         bool satisfied( const permission_level& permission,
                         fc::microseconds override_provided_delay,
                         permission_cache_type* cached_perms = nullptr
                       )
         {
            auto delay_reverter = fc::make_scoped_exit( [this, delay = provided_delay] () mutable {
               provided_delay = delay;
            });

            provided_delay = override_provided_delay;

            return satisfied( permission, cached_perms );
         }

         bool satisfied( const permission_level& permission, permission_cache_type* cached_perms = nullptr ) {
            permission_cache_type cached_permissions;

            if( cached_perms == nullptr )
               cached_perms = initialize_permission_cache( cached_permissions );

            weight_tally_visitor visitor(*this, *cached_perms, 0);
            return ( visitor(permission_level_weight{permission, 1}) > 0 );
         }

         template<typename AuthorityType>
         bool satisfied( const AuthorityType& authority,
                         fc::microseconds override_provided_delay,
                         permission_cache_type* cached_perms = nullptr
                       )
         {
            auto delay_reverter = fc::make_scoped_exit( [this, delay = provided_delay] () mutable {
               provided_delay = delay;
            });

            provided_delay = override_provided_delay;

            return satisfied( authority, cached_perms );
         }

         template<typename AuthorityType>
         bool satisfied( const AuthorityType& authority, permission_cache_type* cached_perms = nullptr ) {
            permission_cache_type cached_permissions;

            if( cached_perms == nullptr )
               cached_perms = initialize_permission_cache( cached_permissions );

            return satisfied( authority, *cached_perms, 0 );
         }

         bool all_keys_used() const { return boost::algorithm::all_of_equal(_used_keys, true); }

         flat_set<public_key_type> used_keys() const {
            auto range = filter_data_by_marker(provided_keys, _used_keys, true);
            return {range.begin(), range.end()};
         }
         flat_set<public_key_type> unused_keys() const {
            auto range = filter_data_by_marker(provided_keys, _used_keys, false);
            return {range.begin(), range.end()};
         }

         static optional<permission_cache_status>
         permission_status_in_cache( const permission_cache_type& permissions,
                                     const permission_level& level )
         {
            auto itr = permissions.find( level );
            if( itr != permissions.end() )
               return itr->second;

            itr = permissions.find( {level.actor, permission_name()} );
            if( itr != permissions.end() )
               return itr->second;

            return optional<permission_cache_status>();
         }

      private:
         permission_cache_type* initialize_permission_cache( permission_cache_type& cached_permissions ) {
            for( const auto& p : provided_permissions ) {
               cached_permissions.emplace_hint( cached_permissions.end(), p, permission_satisfied );
            }
            return &cached_permissions;
         }

         template<typename AuthorityType>
         bool satisfied( const AuthorityType& authority, permission_cache_type& cached_permissions, uint16_t depth ) {
//保存当前使用的密钥；如果我们不满足此权限，则不会实际使用新使用的密钥。
            auto KeyReverter = fc::make_scoped_exit([this, keys = _used_keys] () mutable {
               _used_keys = keys;
            });

//将密钥权限和帐户权限一起排序为一组元权限
            detail::meta_permission_set permissions;

            permissions.insert(authority.waits.begin(), authority.waits.end());
            permissions.insert(authority.keys.begin(), authority.keys.end());
            permissions.insert(authority.accounts.begin(), authority.accounts.end());

//检查所有权限，从最高权重到最低权限，查看提供的授权因素是否满足这些权限
            weight_tally_visitor visitor(*this, cached_permissions, depth);
            for( const auto& permission : permissions )
//如果我们有足够的体重来满足权威，那就回来吧！
               if( permission.visit(visitor) >= authority.threshold ) {
                  KeyReverter.cancel();
                  return true;
               }
            return false;
         }

         struct weight_tally_visitor {
            using result_type = uint32_t;

            authority_checker&     checker;
            permission_cache_type& cached_permissions;
            uint16_t               recursion_depth;
            uint32_t               total_weight = 0;

            weight_tally_visitor(authority_checker& checker, permission_cache_type& cached_permissions, uint16_t recursion_depth)
            :checker(checker)
            ,cached_permissions(cached_permissions)
            ,recursion_depth(recursion_depth)
            {}

            uint32_t operator()(const wait_weight& permission) {
               if( checker.provided_delay >= fc::seconds(permission.wait_sec) ) {
                  total_weight += permission.weight;
               }
               return total_weight;
            }

            uint32_t operator()(const key_weight& permission) {
               auto itr = boost::find( checker.provided_keys, permission.key );
               if( itr != checker.provided_keys.end() ) {
                  checker._used_keys[itr - checker.provided_keys.begin()] = true;
                  total_weight += permission.weight;
               }
               return total_weight;
            }

            uint32_t operator()(const permission_level_weight& permission) {
               auto status = authority_checker::permission_status_in_cache( cached_permissions, permission.permission );
               if( !status ) {
                  if( recursion_depth < checker.recursion_depth_limit ) {
                     bool r = false;
                     typename permission_cache_type::iterator itr = cached_permissions.end();

                     bool propagate_error = false;
                     try {
                        auto&& auth = checker.permission_to_authority( permission.permission );
                        propagate_error = true;
                        auto res = cached_permissions.emplace( permission.permission, being_evaluated );
                        itr = res.first;
                        r = checker.satisfied( std::forward<decltype(auth)>(auth), cached_permissions, recursion_depth + 1 );
                     } catch( const permission_query_exception& ) {
                        if( propagate_error )
                           throw;
                        else
return total_weight; //如果权限不存在，请在没有权限的情况下继续
                     }

                     if( r ) {
                        total_weight += permission.weight;
                        itr->second = permission_satisfied;
                     } else {
                        itr->second = permission_unsatisfied;
                     }
                  }
               } else if( *status == permission_satisfied ) {
                  total_weight += permission.weight;
               }
               return total_weight;
            }
         };

}; ///权限检查

   template<typename PermissionToAuthorityFunc>
   auto make_auth_checker( PermissionToAuthorityFunc&&          pta,
                           uint16_t                             recursion_depth_limit,
                           const flat_set<public_key_type>&     provided_keys,
                           const flat_set<permission_level>&    provided_permissions = flat_set<permission_level>(),
                           fc::microseconds                     provided_delay = fc::microseconds(0),
                           const std::function<void()>&         _checktime = std::function<void()>()
                         )
   {
      auto noop_checktime = []() {};
      const auto& checktime = ( static_cast<bool>(_checktime) ? _checktime : noop_checktime );
      return authority_checker< PermissionToAuthorityFunc>( std::forward<PermissionToAuthorityFunc>(pta),
                                                            recursion_depth_limit,
                                                            provided_keys,
                                                            provided_permissions,
                                                            provided_delay,
                                                            checktime );
   }

} } //命名空间eosio:：chain
