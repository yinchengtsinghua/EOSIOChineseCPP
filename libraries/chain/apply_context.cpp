
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <algorithm>
#include <eosio/chain/apply_context.hpp>
#include <eosio/chain/controller.hpp>
#include <eosio/chain/transaction_context.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/wasm_interface.hpp>
#include <eosio/chain/generated_transaction_object.hpp>
#include <eosio/chain/authorization_manager.hpp>
#include <eosio/chain/resource_limits.hpp>
#include <eosio/chain/account_object.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <boost/container/flat_set.hpp>

using boost::container::flat_set;

namespace eosio { namespace chain {

static inline void print_debug(account_name receiver, const action_trace& ar) {
   if (!ar.console.empty()) {
      auto prefix = fc::format_string(
                                      "\n[(${a},${n})->${r}]",
                                      fc::mutable_variant_object()
                                      ("a", ar.act.account)
                                      ("n", ar.act.name)
                                      ("r", receiver));
      dlog(prefix + ": CONSOLE OUTPUT BEGIN =====================\n"
           + ar.console
           + prefix + ": CONSOLE OUTPUT END   =====================" );
   }
}

void apply_context::exec_one( action_trace& trace )
{
   auto start = fc::time_point::now();

   action_receipt r;
   r.receiver         = receiver;
   r.act_digest       = digest_type::hash(act);

   trace.trx_id = trx_context.id;
   trace.block_num = control.pending_block_state()->block_num;
   trace.block_time = control.pending_block_time();
   trace.producer_block_id = control.pending_producer_block_id();
   trace.act = act;
   trace.context_free = context_free;

   const auto& cfg = control.get_global_properties().configuration;
   try {
      try {
         const auto& a = control.get_account( receiver );
         privileged = a.privileged;
         auto native = control.find_apply_handler( receiver, act.account, act.name );
         if( native ) {
            if( trx_context.enforce_whiteblacklist && control.is_producing_block() ) {
               control.check_contract_list( receiver );
               control.check_action_list( act.account, act.name );
            }
            (*native)( *this );
         }

         if( a.code.size() > 0
             && !(act.account == config::system_account_name && act.name == N( setcode ) &&
                  receiver == config::system_account_name) ) {
            if( trx_context.enforce_whiteblacklist && control.is_producing_block() ) {
               control.check_contract_list( receiver );
               control.check_action_list( act.account, act.name );
            }
            try {
               control.get_wasm_interface().apply( a.code_version, a.code, *this );
            } catch( const wasm_exit& ) {}
         }
      } FC_RETHROW_EXCEPTIONS( warn, "pending console output: ${console}", ("console", _pending_console_output.str()) )
   } catch( fc::exception& e ) {
trace.receipt = r; //用已知数据填充
      trace.except = e;
      finalize_trace( trace, start );
      throw;
   }

   r.global_sequence  = next_global_sequence();
   r.recv_sequence    = next_recv_sequence( receiver );

   const auto& account_sequence = db.get<account_sequence_object, by_name>(act.account);
r.code_sequence    = account_sequence.code_sequence; //可以通过上面的操作执行进行修改
r.abi_sequence     = account_sequence.abi_sequence;  //可以通过上面的操作执行进行修改

   for( const auto& auth : act.authorization ) {
      r.auth_sequence[auth.actor] = next_auth_sequence( auth.actor );
   }

   trace.receipt = r;

   trx_context.executed.emplace_back( move(r) );

   finalize_trace( trace, start );

   if ( control.contracts_console() ) {
      print_debug(receiver, trace);
   }
}

void apply_context::finalize_trace( action_trace& trace, const fc::time_point& start )
{
   trace.account_ram_deltas = std::move( _account_ram_deltas );
   _account_ram_deltas.clear();

   trace.console = _pending_console_output.str();
   reset_console();

   trace.elapsed = fc::time_point::now() - start;
}

void apply_context::exec( action_trace& trace )
{
   _notified.push_back(receiver);
   exec_one( trace );
   for( uint32_t i = 1; i < _notified.size(); ++i ) {
      receiver = _notified[i];
      trace.inline_traces.emplace_back( );
      exec_one( trace.inline_traces.back() );
   }

   if( _cfa_inline_actions.size() > 0 || _inline_actions.size() > 0 ) {
      EOS_ASSERT( recurse_depth < control.get_global_properties().configuration.max_inline_action_depth,
                  transaction_exception, "max inline action depth per transaction reached" );
   }

   for( const auto& inline_action : _cfa_inline_actions ) {
      trace.inline_traces.emplace_back();
      trx_context.dispatch_action( trace.inline_traces.back(), inline_action, inline_action.account, true, recurse_depth + 1 );
   }

   for( const auto& inline_action : _inline_actions ) {
      trace.inline_traces.emplace_back();
      trx_context.dispatch_action( trace.inline_traces.back(), inline_action, inline_action.account, false, recurse_depth + 1 );
   }

} ///执行（）

bool apply_context::is_account( const account_name& account )const {
   return nullptr != db.find<account_object,by_name>( account );
}

void apply_context::require_authorization( const account_name& account ) {
   for( uint32_t i=0; i < act.authorization.size(); i++ ) {
     if( act.authorization[i].actor == account ) {
        used_authorizations[i] = true;
        return;
     }
   }
   EOS_ASSERT( false, missing_auth_exception, "missing authority of ${account}", ("account",account));
}

bool apply_context::has_authorization( const account_name& account )const {
   for( const auto& auth : act.authorization )
     if( auth.actor == account )
        return true;
  return false;
}

void apply_context::require_authorization(const account_name& account,
                                          const permission_name& permission) {
  for( uint32_t i=0; i < act.authorization.size(); i++ )
     if( act.authorization[i].actor == account ) {
        if( act.authorization[i].permission == permission ) {
           used_authorizations[i] = true;
           return;
        }
     }
  EOS_ASSERT( false, missing_auth_exception, "missing authority of ${account}/${permission}",
              ("account",account)("permission",permission) );
}

bool apply_context::has_recipient( account_name code )const {
   for( auto a : _notified )
      if( a == code )
         return true;
   return false;
}

void apply_context::require_recipient( account_name recipient ) {
   if( !has_recipient(recipient) ) {
      _notified.push_back(recipient);
   }
}


/*
 *这将在检查授权后执行一个操作。内联事务是
 *由当前接收器隐式授权（运行代码）。这种方法有重要意义
 *考虑了安全因素和几种选择：
 *
 * 1。特权账户（由区块生产商标记的账户）可以授权任何行动。
 * 2。所有其他行动仅由“接收者”授权，这意味着：
 *a.用户必须为其帐户设置权限，以允许“接收者”代表其行事。
 *
 *放弃实施：我们曾经允许任何授权当前交易的账户
 *隐式授权内联事务。这种方法将允许权限升级和
 *使用户与某些合同交互不安全。我们选择了申请
 *请求用户允许采取某些操作，而不是使其隐式操作。这种方式的用户
 *能够更好地了解安全风险。
 **/

void apply_context::execute_inline( action&& a ) {
   auto* code = control.db().find<account_object, by_name>(a.account);
   EOS_ASSERT( code != nullptr, action_validate_exception,
               "inline action's code account ${account} does not exist", ("account", a.account) );

   bool enforce_actor_whitelist_blacklist = trx_context.enforce_whiteblacklist && control.is_producing_block();
   flat_set<account_name> actors;

bool disallow_send_to_self_bypass = false; //最终设置为是否激活了适当的协议功能
   bool send_to_self = (a.account == receiver);
   bool inherit_parent_authorizations = (!disallow_send_to_self_bypass && send_to_self && (receiver == act.account) && control.is_producing_block());

   flat_set<permission_level> inherited_authorizations;
   if( inherit_parent_authorizations ) {
      inherited_authorizations.reserve( a.authorization.size() );
   }

   for( const auto& auth : a.authorization ) {
      auto* actor = control.db().find<account_object, by_name>(auth.actor);
      EOS_ASSERT( actor != nullptr, action_validate_exception,
                  "inline action's authorizing actor ${account} does not exist", ("account", auth.actor) );
      EOS_ASSERT( control.get_authorization_manager().find_permission(auth) != nullptr, action_validate_exception,
                  "inline action's authorizations include a non-existent permission: ${permission}",
                  ("permission", auth) );
      if( enforce_actor_whitelist_blacklist )
         actors.insert( auth.actor );

      if( inherit_parent_authorizations && std::find(act.authorization.begin(), act.authorization.end(), auth) != act.authorization.end() ) {
         inherited_authorizations.insert( auth );
      }
   }

   if( enforce_actor_whitelist_blacklist ) {
      control.check_actor_list( actors );
   }

//如果重放不可逆块或协定是特权的，则无需检查授权
   if( !control.skip_auth_check() && !privileged ) {
      try {
         control.get_authorization_manager()
                .check_authorization( {a},
                                      {},
                                      {{receiver, config::eosio_code_name}},
                                      control.pending_block_time() - trx_context.published,
                                      std::bind(&transaction_context::checktime, &this->trx_context),
                                      false,
                                      inherited_authorizations
                                    );

//问题：允许延迟了一段时间的事务离开是否明智？
//发送一个需要延迟的内联操作，即使决定发送该内联
//在执行延迟事务时采取了可能没有预警的措施？
      } catch( const fc::exception& e ) {
         if( disallow_send_to_self_bypass || !send_to_self ) {
            throw;
         } else if( control.is_producing_block() ) {
            subjective_block_production_exception new_exception(FC_LOG_MESSAGE( error, "Authorization failure with inline action sent to self"));
            for (const auto& log: e.get_log()) {
               new_exception.append_log(log);
            }
            throw new_exception;
         }
      } catch( ... ) {
         if( disallow_send_to_self_bypass || !send_to_self ) {
            throw;
         } else if( control.is_producing_block() ) {
            EOS_THROW(subjective_block_production_exception, "Unexpected exception occurred validating inline action sent to self");
         }
      }
   }

   _inline_actions.emplace_back( move(a) );
}

void apply_context::execute_context_free_inline( action&& a ) {
   auto* code = control.db().find<account_object, by_name>(a.account);
   EOS_ASSERT( code != nullptr, action_validate_exception,
               "inline action's code account ${account} does not exist", ("account", a.account) );

   EOS_ASSERT( a.authorization.size() == 0, action_validate_exception,
               "context-free actions cannot have authorizations" );

   _cfa_inline_actions.emplace_back( move(a) );
}


void apply_context::schedule_deferred_transaction( const uint128_t& sender_id, account_name payer, transaction&& trx, bool replace_existing ) {
   EOS_ASSERT( trx.context_free_actions.size() == 0, cfa_inside_generated_tx, "context free actions are not currently allowed in generated transactions" );
trx.expiration = control.pending_block_time() + fc::microseconds(999'999); //四舍五入到最接近的秒（使到期检查不必要）
trx.set_reference_block(control.head_block_id()); //无需Tapos检查

   bool enforce_actor_whitelist_blacklist = trx_context.enforce_whiteblacklist && control.is_producing_block()
                                             && !control.sender_avoids_whitelist_blacklist_enforcement( receiver );
   trx_context.validate_referenced_accounts( trx, enforce_actor_whitelist_blacklist );

//提前收取注销延期交易所需的额外净使用费
//无论是通过成功执行、软故障、硬故障还是过期。
   const auto& cfg = control.get_global_properties().configuration;
   trx_context.add_net_usage( static_cast<uint64_t>(cfg.base_per_transaction_net_usage)
+ static_cast<uint64_t>(config::transaction_id_net_usage) ); //如果无法支付净使用费，将提前退出。

   auto delay = fc::seconds(trx.delay_sec);

if( !control.skip_auth_check() && !privileged ) { //如果重放不可逆块或合同具有特权，则不需要检查授权。
      if( payer != receiver ) {
require_authorization(payer); ///使用付款人的存储
      }

//最初，如果契约只将操作延迟到自身，则此代码将跳过授权检查。
//其想法是，代码已经可以做任何延迟事务可以做的事情，所以没有必要检查授权。
//但这不是真的。原始实现没有验证允许权限提升的操作的授权。
//这将使向一些不相关的账户开单成为可能。
//此外，即使授权被强制为当前操作授权的一个子集，它仍然会违反预期。
//原始事务的签名者，因为延迟的事务将允许比最大限制计费更多的CPU和网络带宽
//在原始交易记录上指定。
//因此，如果延迟事务不是由特权契约发送的，那么它必须始终通过授权检查。
//但是，必须考虑旧的逻辑，因为在共识协议升级之前，它不能客观地改变。

bool disallow_send_to_self_bypass = false; //最终设置为是否激活了适当的协议功能

      auto is_sending_only_to_self = [&trx]( const account_name& self ) {
         bool send_to_self = true;
         for( const auto& act : trx.actions ) {
            if( act.account != self ) {
               send_to_self = false;
               break;
            }
         }
         return send_to_self;
      };

      try {
         control.get_authorization_manager()
                .check_authorization( trx.actions,
                                      {},
                                      {{receiver, config::eosio_code_name}},
                                      delay,
                                      std::bind(&transaction_context::checktime, &this->trx_context),
                                      false
                                    );
      } catch( const fc::exception& e ) {
         if( disallow_send_to_self_bypass || !is_sending_only_to_self(receiver) ) {
            throw;
         } else if( control.is_producing_block() ) {
            subjective_block_production_exception new_exception(FC_LOG_MESSAGE( error, "Authorization failure with sent deferred transaction consisting only of actions to self"));
            for (const auto& log: e.get_log()) {
               new_exception.append_log(log);
            }
            throw new_exception;
         }
      } catch( ... ) {
         if( disallow_send_to_self_bypass || !is_sending_only_to_self(receiver) ) {
            throw;
         } else if( control.is_producing_block() ) {
            EOS_THROW(subjective_block_production_exception, "Unexpected exception occurred validating sent deferred transaction consisting only of actions to self");
         }
      }
   }

   uint32_t trx_size = 0;
   if ( auto ptr = db.find<generated_transaction_object,by_sender_id>(boost::make_tuple(receiver, sender_id)) ) {
      EOS_ASSERT( replace_existing, deferred_tx_duplicate, "deferred transaction with the same sender_id and payer already exists" );

//TODO:在用硬分叉修复延迟的TRX替换RAM错误时，删除以下主观检查。
      EOS_ASSERT( !control.is_producing_block(), subjective_block_production_exception,
                  "Replacing a deferred transaction is temporarily disabled." );

//TODO:下一行的逻辑需要合并到下一个硬分叉中。
//添加_-ram-usage（ptr->payer-（config:：billable_-size_v<generated_-transaction_-object-+ptr->packed_-trx.size（））；

      db.modify<generated_transaction_object>( *ptr, [&]( auto& gtx ) {
            gtx.sender      = receiver;
            gtx.sender_id   = sender_id;
            gtx.payer       = payer;
            gtx.published   = control.pending_block_time();
            gtx.delay_until = gtx.published + delay;
            gtx.expiration  = gtx.delay_until + fc::seconds(control.get_global_properties().configuration.deferred_trx_expiration_window);

            trx_size = gtx.set( trx );
         });
   } else {
      db.create<generated_transaction_object>( [&]( auto& gtx ) {
            gtx.trx_id      = trx.id();
            gtx.sender      = receiver;
            gtx.sender_id   = sender_id;
            gtx.payer       = payer;
            gtx.published   = control.pending_block_time();
            gtx.delay_until = gtx.published + delay;
            gtx.expiration  = gtx.delay_until + fc::seconds(control.get_global_properties().configuration.deferred_trx_expiration_window);

            trx_size = gtx.set( trx );
         });
   }

   EOS_ASSERT( control.is_ram_billing_in_notify_allowed() || (receiver == act.account) || (receiver == payer) || privileged,
               subjective_block_production_exception, "Cannot charge RAM to other accounts during notify." );
   add_ram_usage( payer, (config::billable_size_v<generated_transaction_object> + trx_size) );
}

bool apply_context::cancel_deferred_transaction( const uint128_t& sender_id, account_name sender ) {
   auto& generated_transaction_idx = db.get_mutable_index<generated_transaction_multi_index>();
   const auto* gto = db.find<generated_transaction_object,by_sender_id>(boost::make_tuple(sender, sender_id));
   if ( gto ) {
      add_ram_usage( gto->payer, -(config::billable_size_v<generated_transaction_object> + gto->packed_trx.size()) );
      generated_transaction_idx.remove(*gto);
   }
   return gto;
}

const table_id_object* apply_context::find_table( name code, name scope, name table ) {
   return db.find<table_id_object, by_code_scope_table>(boost::make_tuple(code, scope, table));
}

const table_id_object& apply_context::find_or_create_table( name code, name scope, name table, const account_name &payer ) {
   const auto* existing_tid =  db.find<table_id_object, by_code_scope_table>(boost::make_tuple(code, scope, table));
   if (existing_tid != nullptr) {
      return *existing_tid;
   }

   update_db_usage(payer, config::billable_size_v<table_id_object>);

   return db.create<table_id_object>([&](table_id_object &t_id){
      t_id.code = code;
      t_id.scope = scope;
      t_id.table = table;
      t_id.payer = payer;
   });
}

void apply_context::remove_table( const table_id_object& tid ) {
   update_db_usage(tid.payer, - config::billable_size_v<table_id_object>);
   db.remove(tid);
}

vector<account_name> apply_context::get_active_producers() const {
   const auto& ap = control.active_producers();
   vector<account_name> accounts; accounts.reserve( ap.producers.size() );

   for(const auto& producer : ap.producers )
      accounts.push_back(producer.producer_name);

   return accounts;
}

void apply_context::reset_console() {
   _pending_console_output = std::ostringstream();
   _pending_console_output.setf( std::ios::scientific, std::ios::floatfield );
}

bytes apply_context::get_packed_transaction() {
   auto r = fc::raw::pack( static_cast<const transaction&>(trx_context.trx) );
   return r;
}

void apply_context::update_db_usage( const account_name& payer, int64_t delta ) {
   if( delta > 0 ) {
      if( !(privileged || payer == account_name(receiver)) ) {
         EOS_ASSERT( control.is_ram_billing_in_notify_allowed() || (receiver == act.account),
                     subjective_block_production_exception, "Cannot charge RAM to other accounts during notify." );
         require_authorization( payer );
      }
   }
   add_ram_usage(payer, delta);
}


int apply_context::get_action( uint32_t type, uint32_t index, char* buffer, size_t buffer_size )const
{
   const auto& trx = trx_context.trx;
   const action* act_ptr = nullptr;

   if( type == 0 ) {
      if( index >= trx.context_free_actions.size() )
         return -1;
      act_ptr = &trx.context_free_actions[index];
   }
   else if( type == 1 ) {
      if( index >= trx.actions.size() )
         return -1;
      act_ptr = &trx.actions[index];
   }

   EOS_ASSERT(act_ptr, action_not_found_exception, "action is not found" );

   auto ps = fc::raw::pack_size( *act_ptr );
   if( ps <= buffer_size ) {
      fc::datastream<char*> ds(buffer, buffer_size);
      fc::raw::pack( ds, *act_ptr );
   }
   return ps;
}

int apply_context::get_context_free_data( uint32_t index, char* buffer, size_t buffer_size )const
{
   const auto& trx = trx_context.trx;

   if( index >= trx.context_free_data.size() ) return -1;

   auto s = trx.context_free_data[index].size();
   if( buffer_size == 0 ) return s;

   auto copy_size = std::min( buffer_size, s );
   memcpy( buffer, trx.context_free_data[index].data(), copy_size );

   return copy_size;
}

int apply_context::db_store_i64( uint64_t scope, uint64_t table, const account_name& payer, uint64_t id, const char* buffer, size_t buffer_size ) {
   return db_store_i64( receiver, scope, table, payer, id, buffer, buffer_size);
}

int apply_context::db_store_i64( uint64_t code, uint64_t scope, uint64_t table, const account_name& payer, uint64_t id, const char* buffer, size_t buffer_size ) {
//需要写锁（作用域）；
   const auto& tab = find_or_create_table( code, scope, table, payer );
   auto tableid = tab.id;

   EOS_ASSERT( payer != account_name(), invalid_table_payer, "must specify a valid account to pay for new record" );

   const auto& obj = db.create<key_value_object>( [&]( auto& o ) {
      o.t_id        = tableid;
      o.primary_key = id;
      o.value.assign( buffer, buffer_size );
      o.payer       = payer;
   });

   db.modify( tab, [&]( auto& t ) {
     ++t.count;
   });

   int64_t billable_size = (int64_t)(buffer_size + config::billable_size_v<key_value_object>);
   update_db_usage( payer, billable_size);

   keyval_cache.cache_table( tab );
   return keyval_cache.add( obj );
}

void apply_context::db_update_i64( int iterator, account_name payer, const char* buffer, size_t buffer_size ) {
   const key_value_object& obj = keyval_cache.get( iterator );

   const auto& table_obj = keyval_cache.get_table( obj.t_id );
   EOS_ASSERT( table_obj.code == receiver, table_access_violation, "db access violation" );

//需要写锁（表\obj.scope）；

   const int64_t overhead = config::billable_size_v<key_value_object>;
   int64_t old_size = (int64_t)(obj.value.size() + overhead);
   int64_t new_size = (int64_t)(buffer_size + overhead);

   if( payer == account_name() ) payer = obj.payer;

   if( account_name(obj.payer) != payer ) {
//退还现有付款人
      update_db_usage( obj.payer,  -(old_size) );
//向新付款人收费
      update_db_usage( payer,  (new_size));
   } else if(old_size != new_size) {
//向现有付款人收取/退还差额
      update_db_usage( obj.payer, new_size - old_size);
   }

   db.modify( obj, [&]( auto& o ) {
     o.value.assign( buffer, buffer_size );
     o.payer = payer;
   });
}

void apply_context::db_remove_i64( int iterator ) {
   const key_value_object& obj = keyval_cache.get( iterator );

   const auto& table_obj = keyval_cache.get_table( obj.t_id );
   EOS_ASSERT( table_obj.code == receiver, table_access_violation, "db access violation" );

//需要写锁（表\obj.scope）；

   update_db_usage( obj.payer,  -(obj.value.size() + config::billable_size_v<key_value_object>) );

   db.modify( table_obj, [&]( auto& t ) {
      --t.count;
   });
   db.remove( obj );

   if (table_obj.count == 0) {
      remove_table(table_obj);
   }

   keyval_cache.remove( iterator );
}

int apply_context::db_get_i64( int iterator, char* buffer, size_t buffer_size ) {
   const key_value_object& obj = keyval_cache.get( iterator );

   auto s = obj.value.size();
   if( buffer_size == 0 ) return s;

   auto copy_size = std::min( buffer_size, s );
   memcpy( buffer, obj.value.data(), copy_size );

   return copy_size;
}

int apply_context::db_next_i64( int iterator, uint64_t& primary ) {
if( iterator < -1 ) return -1; //无法增量超过表的结束迭代器

const auto& obj = keyval_cache.get( iterator ); //检查迭代器！=-1在这个呼叫中发生
   const auto& idx = db.get_index<key_value_index, by_scope_primary>();

   auto itr = idx.iterator_to( obj );
   ++itr;

   if( itr == idx.end() || itr->t_id != obj.t_id ) return keyval_cache.get_end_iterator_by_table_id(obj.t_id);

   primary = itr->primary_key;
   return keyval_cache.add( *itr );
}

int apply_context::db_previous_i64( int iterator, uint64_t& primary ) {
   const auto& idx = db.get_index<key_value_index, by_scope_primary>();

if( iterator < -1 ) //结束迭代器
   {
      auto tab = keyval_cache.find_table_by_end_iterator(iterator);
      EOS_ASSERT( tab, invalid_table_iterator, "not a valid end iterator" );

      auto itr = idx.upper_bound(tab->id);
if( idx.begin() == idx.end() || itr == idx.begin() ) return -1; //空表

      --itr;

if( itr->t_id != tab->id ) return -1; //空表

      primary = itr->primary_key;
      return keyval_cache.add(*itr);
   }

const auto& obj = keyval_cache.get(iterator); //检查迭代器！=-1在这个呼叫中发生

   auto itr = idx.iterator_to(obj);
if( itr == idx.begin() ) return -1; //不能在表的起始迭代器之后递减

   --itr;

if( itr->t_id != obj.t_id ) return -1; //不能在表的起始迭代器之后递减

   primary = itr->primary_key;
   return keyval_cache.add(*itr);
}

int apply_context::db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
//需要读锁（code，scope）；//冗余？

   const auto* tab = find_table( code, scope, table );
   if( !tab ) return -1;

   auto table_end_itr = keyval_cache.cache_table( *tab );

   const key_value_object* obj = db.find<key_value_object, by_scope_primary>( boost::make_tuple( tab->id, id ) );
   if( !obj ) return table_end_itr;

   return keyval_cache.add( *obj );
}

int apply_context::db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
//需要读锁（code，scope）；//冗余？

   const auto* tab = find_table( code, scope, table );
   if( !tab ) return -1;

   auto table_end_itr = keyval_cache.cache_table( *tab );

   const auto& idx = db.get_index<key_value_index, by_scope_primary>();
   auto itr = idx.lower_bound( boost::make_tuple( tab->id, id ) );
   if( itr == idx.end() ) return table_end_itr;
   if( itr->t_id != tab->id ) return table_end_itr;

   return keyval_cache.add( *itr );
}

int apply_context::db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id ) {
//需要读锁（code，scope）；//冗余？

   const auto* tab = find_table( code, scope, table );
   if( !tab ) return -1;

   auto table_end_itr = keyval_cache.cache_table( *tab );

   const auto& idx = db.get_index<key_value_index, by_scope_primary>();
   auto itr = idx.upper_bound( boost::make_tuple( tab->id, id ) );
   if( itr == idx.end() ) return table_end_itr;
   if( itr->t_id != tab->id ) return table_end_itr;

   return keyval_cache.add( *itr );
}

int apply_context::db_end_i64( uint64_t code, uint64_t scope, uint64_t table ) {
//需要读锁（code，scope）；//冗余？

   const auto* tab = find_table( code, scope, table );
   if( !tab ) return -1;

   return keyval_cache.cache_table( *tab );
}

uint64_t apply_context::next_global_sequence() {
   const auto& p = control.get_dynamic_global_properties();
   db.modify( p, [&]( auto& dgp ) {
      ++dgp.global_action_sequence;
   });
   return p.global_action_sequence;
}

uint64_t apply_context::next_recv_sequence( account_name receiver ) {
   const auto& rs = db.get<account_sequence_object,by_name>( receiver );
   db.modify( rs, [&]( auto& mrs ) {
      ++mrs.recv_sequence;
   });
   return rs.recv_sequence;
}
uint64_t apply_context::next_auth_sequence( account_name actor ) {
   const auto& rs = db.get<account_sequence_object,by_name>( actor );
   db.modify( rs, [&](auto& mrs ){
      ++mrs.auth_sequence;
   });
   return rs.auth_sequence;
}

void apply_context::add_ram_usage( account_name account, int64_t ram_delta ) {
   trx_context.add_ram_usage( account, ram_delta );

   auto p = _account_ram_deltas.emplace( account, ram_delta );
   if( !p.second ) {
      p.first->delta += ram_delta;
   }
}


} } ///EOSIO：链
