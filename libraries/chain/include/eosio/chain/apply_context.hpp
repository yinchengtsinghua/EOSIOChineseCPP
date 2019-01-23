
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
#include <eosio/chain/controller.hpp>
#include <eosio/chain/transaction.hpp>
#include <eosio/chain/contract_table_objects.hpp>
#include <fc/utility.hpp>
#include <sstream>
#include <algorithm>
#include <set>

namespace chainbase { class database; }

namespace eosio { namespace chain {

class controller;
class transaction_context;

class apply_context {
   private:
      template<typename T>
      class iterator_cache {
         public:
            iterator_cache(){
               _end_iterator_to_table.reserve(8);
               _iterator_to_object.reserve(32);
            }

///返回表的结束迭代器。
            int cache_table( const table_id_object& tobj ) {
               auto itr = _table_cache.find(tobj.id);
               if( itr != _table_cache.end() )
                  return itr->second.second;

               auto ei = index_to_end_iterator(_end_iterator_to_table.size());
               _end_iterator_to_table.push_back( &tobj );
               _table_cache.emplace( tobj.id, make_pair(&tobj, ei) );
               return ei;
            }

            const table_id_object& get_table( table_id_object::id_type i )const {
               auto itr = _table_cache.find(i);
               EOS_ASSERT( itr != _table_cache.end(), table_not_in_cache, "an invariant was broken, table should be in cache" );
               return *itr->second.first;
            }

            int get_end_iterator_by_table_id( table_id_object::id_type i )const {
               auto itr = _table_cache.find(i);
               EOS_ASSERT( itr != _table_cache.end(), table_not_in_cache, "an invariant was broken, table should be in cache" );
               return itr->second.second;
            }

            const table_id_object* find_table_by_end_iterator( int ei )const {
               EOS_ASSERT( ei < -1, invalid_table_iterator, "not an end iterator" );
               auto indx = end_iterator_to_index(ei);
               if( indx >= _end_iterator_to_table.size() ) return nullptr;
               return _end_iterator_to_table[indx];
            }

            const T& get( int iterator ) {
               EOS_ASSERT( iterator != -1, invalid_table_iterator, "invalid iterator" );
               EOS_ASSERT( iterator >= 0, table_operation_not_permitted, "dereference of end iterator" );
               EOS_ASSERT( iterator < _iterator_to_object.size(), invalid_table_iterator, "iterator out of range" );
               auto result = _iterator_to_object[iterator];
               EOS_ASSERT( result, table_operation_not_permitted, "dereference of deleted object" );
               return *result;
            }

            void remove( int iterator ) {
               EOS_ASSERT( iterator != -1, invalid_table_iterator, "invalid iterator" );
               EOS_ASSERT( iterator >= 0, table_operation_not_permitted, "cannot call remove on end iterators" );
               EOS_ASSERT( iterator < _iterator_to_object.size(), invalid_table_iterator, "iterator out of range" );
               auto obj_ptr = _iterator_to_object[iterator];
               if( !obj_ptr ) return;
               _iterator_to_object[iterator] = nullptr;
               _object_to_iterator.erase( obj_ptr );
            }

            int add( const T& obj ) {
               auto itr = _object_to_iterator.find( &obj );
               if( itr != _object_to_iterator.end() )
                    return itr->second;

               _iterator_to_object.push_back( &obj );
               _object_to_iterator[&obj] = _iterator_to_object.size() - 1;

               return _iterator_to_object.size() - 1;
            }

         private:
            map<table_id_object::id_type, pair<const table_id_object*, int>> _table_cache;
            vector<const table_id_object*>                  _end_iterator_to_table;
            vector<const T*>                                _iterator_to_object;
            map<const T*,int>                               _object_to_iterator;

///precondition:std:：numeric_limits<int>：：min（）<ei<-1
///iterator of-1是为无效的迭代器保留的（即尚未创建适当的表）。
            inline size_t end_iterator_to_index( int ei )const { return (-ei - 2); }
///precondition:indx<“end”迭代器“u to”table.size（）<=std：：numeric“limits<int>：：max（））
            inline int index_to_end_iterator( size_t indx )const { return -(indx + 2); }
}; ///class迭代器缓存

      template<typename>
      struct array_size;

      template<typename T, size_t N>
      struct array_size< std::array<T,N> > {
          static constexpr size_t size = N;
      };

      template <typename SecondaryKey, typename SecondaryKeyProxy, typename SecondaryKeyProxyConst, typename Enable = void>
      class secondary_key_helper;

      template<typename SecondaryKey, typename SecondaryKeyProxy, typename SecondaryKeyProxyConst>
      class secondary_key_helper<SecondaryKey, SecondaryKeyProxy, SecondaryKeyProxyConst,
         typename std::enable_if<std::is_same<SecondaryKey, typename std::decay<SecondaryKeyProxy>::type>::value>::type >
      {
         public:
            typedef SecondaryKey secondary_key_type;

            static void set(secondary_key_type& sk_in_table, const secondary_key_type& sk_from_wasm) {
               sk_in_table = sk_from_wasm;
            }

            static void get(secondary_key_type& sk_from_wasm, const secondary_key_type& sk_in_table ) {
               sk_from_wasm = sk_in_table;
            }

            static auto create_tuple(const table_id_object& tab, const secondary_key_type& secondary) {
               return boost::make_tuple( tab.id, secondary );
            }
      };

      template<typename SecondaryKey, typename SecondaryKeyProxy, typename SecondaryKeyProxyConst>
      class secondary_key_helper<SecondaryKey, SecondaryKeyProxy, SecondaryKeyProxyConst,
         typename std::enable_if<!std::is_same<SecondaryKey, typename std::decay<SecondaryKeyProxy>::type>::value &&
                                 std::is_pointer<typename std::decay<SecondaryKeyProxy>::type>::value>::type >
      {
         public:
            typedef SecondaryKey      secondary_key_type;
            typedef SecondaryKeyProxy secondary_key_proxy_type;
            typedef SecondaryKeyProxyConst secondary_key_proxy_const_type;

            static constexpr size_t N = array_size<SecondaryKey>::size;

            static void set(secondary_key_type& sk_in_table, secondary_key_proxy_const_type sk_from_wasm) {
               std::copy(sk_from_wasm, sk_from_wasm + N, sk_in_table.begin());
            }

            static void get(secondary_key_proxy_type sk_from_wasm, const secondary_key_type& sk_in_table) {
               std::copy(sk_in_table.begin(), sk_in_table.end(), sk_from_wasm);
            }

            static auto create_tuple(const table_id_object& tab, secondary_key_proxy_const_type sk_from_wasm) {
               secondary_key_type secondary;
               std::copy(sk_from_wasm, sk_from_wasm + N, secondary.begin());
               return boost::make_tuple( tab.id, secondary );
            }
      };

   public:
      template<typename ObjectType,
               typename SecondaryKeyProxy = typename std::add_lvalue_reference<typename ObjectType::secondary_key_type>::type,
               typename SecondaryKeyProxyConst = typename std::add_lvalue_reference<
                                                   typename std::add_const<typename ObjectType::secondary_key_type>::type>::type >
      class generic_index
      {
         public:
            typedef typename ObjectType::secondary_key_type secondary_key_type;
            typedef SecondaryKeyProxy      secondary_key_proxy_type;
            typedef SecondaryKeyProxyConst secondary_key_proxy_const_type;

            using secondary_key_helper_t = secondary_key_helper<secondary_key_type, secondary_key_proxy_type, secondary_key_proxy_const_type>;

            generic_index( apply_context& c ):context(c){}

            int store( uint64_t scope, uint64_t table, const account_name& payer,
                       uint64_t id, secondary_key_proxy_const_type value )
            {
               EOS_ASSERT( payer != account_name(), invalid_table_payer, "must specify a valid account to pay for new record" );

//context.需要写锁（scope）；

               const auto& tab = context.find_or_create_table( context.receiver, scope, table, payer );

               const auto& obj = context.db.create<ObjectType>( [&]( auto& o ){
                  o.t_id          = tab.id;
                  o.primary_key   = id;
                  secondary_key_helper_t::set(o.secondary_key, value);
                  o.payer         = payer;
               });

               context.db.modify( tab, [&]( auto& t ) {
                 ++t.count;
               });

               context.update_db_usage( payer, config::billable_size_v<ObjectType> );

               itr_cache.cache_table( tab );
               return itr_cache.add( obj );
            }

            void remove( int iterator ) {
               const auto& obj = itr_cache.get( iterator );
               context.update_db_usage( obj.payer, -( config::billable_size_v<ObjectType> ) );

               const auto& table_obj = itr_cache.get_table( obj.t_id );
               EOS_ASSERT( table_obj.code == context.receiver, table_access_violation, "db access violation" );

//context.require_write_lock（table_obj.scope）；

               context.db.modify( table_obj, [&]( auto& t ) {
                  --t.count;
               });
               context.db.remove( obj );

               if (table_obj.count == 0) {
                  context.remove_table(table_obj);
               }

               itr_cache.remove( iterator );
            }

            void update( int iterator, account_name payer, secondary_key_proxy_const_type secondary ) {
               const auto& obj = itr_cache.get( iterator );

               const auto& table_obj = itr_cache.get_table( obj.t_id );
               EOS_ASSERT( table_obj.code == context.receiver, table_access_violation, "db access violation" );

//context.require_write_lock（table_obj.scope）；

               if( payer == account_name() ) payer = obj.payer;

               int64_t billing_size =  config::billable_size_v<ObjectType>;

               if( obj.payer != payer ) {
                  context.update_db_usage( obj.payer, -(billing_size) );
                  context.update_db_usage( payer, +(billing_size) );
               }

               context.db.modify( obj, [&]( auto& o ) {
                 secondary_key_helper_t::set(o.secondary_key, secondary);
                 o.payer = payer;
               });
            }

            int find_secondary( uint64_t code, uint64_t scope, uint64_t table, secondary_key_proxy_const_type secondary, uint64_t& primary ) {
               auto tab = context.find_table( code, scope, table );
               if( !tab ) return -1;

               auto table_end_itr = itr_cache.cache_table( *tab );

               const auto* obj = context.db.find<ObjectType, by_secondary>( secondary_key_helper_t::create_tuple( *tab, secondary ) );
               if( !obj ) return table_end_itr;

               primary = obj->primary_key;

               return itr_cache.add( *obj );
            }

            int lowerbound_secondary( uint64_t code, uint64_t scope, uint64_t table, secondary_key_proxy_type secondary, uint64_t& primary ) {
               auto tab = context.find_table( code, scope, table );
               if( !tab ) return -1;

               auto table_end_itr = itr_cache.cache_table( *tab );

               const auto& idx = context.db.get_index< typename chainbase::get_index_type<ObjectType>::type, by_secondary >();
               auto itr = idx.lower_bound( secondary_key_helper_t::create_tuple( *tab, secondary ) );
               if( itr == idx.end() ) return table_end_itr;
               if( itr->t_id != tab->id ) return table_end_itr;

               primary = itr->primary_key;
               secondary_key_helper_t::get(secondary, itr->secondary_key);

               return itr_cache.add( *itr );
            }

            int upperbound_secondary( uint64_t code, uint64_t scope, uint64_t table, secondary_key_proxy_type secondary, uint64_t& primary ) {
               auto tab = context.find_table( code, scope, table );
               if( !tab ) return -1;

               auto table_end_itr = itr_cache.cache_table( *tab );

               const auto& idx = context.db.get_index< typename chainbase::get_index_type<ObjectType>::type, by_secondary >();
               auto itr = idx.upper_bound( secondary_key_helper_t::create_tuple( *tab, secondary ) );
               if( itr == idx.end() ) return table_end_itr;
               if( itr->t_id != tab->id ) return table_end_itr;

               primary = itr->primary_key;
               secondary_key_helper_t::get(secondary, itr->secondary_key);

               return itr_cache.add( *itr );
            }

            int end_secondary( uint64_t code, uint64_t scope, uint64_t table ) {
               auto tab = context.find_table( code, scope, table );
               if( !tab ) return -1;

               return itr_cache.cache_table( *tab );
            }

            int next_secondary( int iterator, uint64_t& primary ) {
if( iterator < -1 ) return -1; //无法增量超过索引的结束迭代器

const auto& obj = itr_cache.get(iterator); //检查迭代器！=-1在这个呼叫中发生
               const auto& idx = context.db.get_index<typename chainbase::get_index_type<ObjectType>::type, by_secondary>();

               auto itr = idx.iterator_to(obj);
               ++itr;

               if( itr == idx.end() || itr->t_id != obj.t_id ) return itr_cache.get_end_iterator_by_table_id(obj.t_id);

               primary = itr->primary_key;
               return itr_cache.add(*itr);
            }

            int previous_secondary( int iterator, uint64_t& primary ) {
               const auto& idx = context.db.get_index<typename chainbase::get_index_type<ObjectType>::type, by_secondary>();

if( iterator < -1 ) //结束迭代器
               {
                  auto tab = itr_cache.find_table_by_end_iterator(iterator);
                  EOS_ASSERT( tab, invalid_table_iterator, "not a valid end iterator" );

                  auto itr = idx.upper_bound(tab->id);
if( idx.begin() == idx.end() || itr == idx.begin() ) return -1; //空索引

                  --itr;

if( itr->t_id != tab->id ) return -1; //空索引

                  primary = itr->primary_key;
                  return itr_cache.add(*itr);
               }

const auto& obj = itr_cache.get(iterator); //检查迭代器！=-1在这个呼叫中发生

               auto itr = idx.iterator_to(obj);
if( itr == idx.begin() ) return -1; //不能在索引的起始迭代器之后递减

               --itr;

if( itr->t_id != obj.t_id ) return -1; //不能在索引的起始迭代器之后递减

               primary = itr->primary_key;
               return itr_cache.add(*itr);
            }

            int find_primary( uint64_t code, uint64_t scope, uint64_t table, secondary_key_proxy_type secondary, uint64_t primary ) {
               auto tab = context.find_table( code, scope, table );
               if( !tab ) return -1;

               auto table_end_itr = itr_cache.cache_table( *tab );

               const auto* obj = context.db.find<ObjectType, by_primary>( boost::make_tuple( tab->id, primary ) );
               if( !obj ) return table_end_itr;
               secondary_key_helper_t::get(secondary, obj->secondary_key);

               return itr_cache.add( *obj );
            }

            int lowerbound_primary( uint64_t code, uint64_t scope, uint64_t table, uint64_t primary ) {
               auto tab = context.find_table( code, scope, table );
               if (!tab) return -1;

               auto table_end_itr = itr_cache.cache_table( *tab );

               const auto& idx = context.db.get_index<typename chainbase::get_index_type<ObjectType>::type, by_primary>();
               auto itr = idx.lower_bound(boost::make_tuple(tab->id, primary));
               if (itr == idx.end()) return table_end_itr;
               if (itr->t_id != tab->id) return table_end_itr;

               return itr_cache.add(*itr);
            }

            int upperbound_primary( uint64_t code, uint64_t scope, uint64_t table, uint64_t primary ) {
               auto tab = context.find_table( code, scope, table );
               if ( !tab ) return -1;

               auto table_end_itr = itr_cache.cache_table( *tab );

               const auto& idx = context.db.get_index<typename chainbase::get_index_type<ObjectType>::type, by_primary>();
               auto itr = idx.upper_bound(boost::make_tuple(tab->id, primary));
               if (itr == idx.end()) return table_end_itr;
               if (itr->t_id != tab->id) return table_end_itr;

               itr_cache.cache_table(*tab);
               return itr_cache.add(*itr);
            }

            int next_primary( int iterator, uint64_t& primary ) {
if( iterator < -1 ) return -1; //无法增量超过表的结束迭代器

const auto& obj = itr_cache.get(iterator); //检查迭代器！=-1在这个呼叫中发生
               const auto& idx = context.db.get_index<typename chainbase::get_index_type<ObjectType>::type, by_primary>();

               auto itr = idx.iterator_to(obj);
               ++itr;

               if( itr == idx.end() || itr->t_id != obj.t_id ) return itr_cache.get_end_iterator_by_table_id(obj.t_id);

               primary = itr->primary_key;
               return itr_cache.add(*itr);
            }

            int previous_primary( int iterator, uint64_t& primary ) {
               const auto& idx = context.db.get_index<typename chainbase::get_index_type<ObjectType>::type, by_primary>();

if( iterator < -1 ) //结束迭代器
               {
                  auto tab = itr_cache.find_table_by_end_iterator(iterator);
                  EOS_ASSERT( tab, invalid_table_iterator, "not a valid end iterator" );

                  auto itr = idx.upper_bound(tab->id);
if( idx.begin() == idx.end() || itr == idx.begin() ) return -1; //空表

                  --itr;

if( itr->t_id != tab->id ) return -1; //空表

                  primary = itr->primary_key;
                  return itr_cache.add(*itr);
               }

const auto& obj = itr_cache.get(iterator); //检查迭代器！=-1在这个呼叫中发生

               auto itr = idx.iterator_to(obj);
if( itr == idx.begin() ) return -1; //不能在表的起始迭代器之后递减

               --itr;

if( itr->t_id != obj.t_id ) return -1; //不能在索引的起始迭代器之后递减

               primary = itr->primary_key;
               return itr_cache.add(*itr);
            }

            void get( int iterator, uint64_t& primary, secondary_key_proxy_type secondary ) {
               const auto& obj = itr_cache.get( iterator );
               primary   = obj.primary_key;
               secondary_key_helper_t::get(secondary, obj.secondary_key);
            }

         private:
            apply_context&              context;
            iterator_cache<ObjectType>  itr_cache;
}; ///class通用索引


///构造函数
   public:
      apply_context(controller& con, transaction_context& trx_ctx, const action& a, uint32_t depth=0)
      :control(con)
      ,db(con.mutable_db())
      ,trx_context(trx_ctx)
      ,act(a)
      ,receiver(act.account)
      ,used_authorizations(act.authorization.size(), false)
      ,recurse_depth(depth)
      ,idx64(*this)
      ,idx128(*this)
      ,idx256(*this)
      ,idx_double(*this)
      ,idx_long_double(*this)
      {
         reset_console();
      }


///执行方法：
   public:

      void exec_one( action_trace& trace );
      void exec( action_trace& trace );
      void execute_inline( action&& a );
      void execute_context_free_inline( action&& a );
      void schedule_deferred_transaction( const uint128_t& sender_id, account_name payer, transaction&& trx, bool replace_existing );
      bool cancel_deferred_transaction( const uint128_t& sender_id, account_name sender );
      bool cancel_deferred_transaction( const uint128_t& sender_id ) { return cancel_deferred_transaction(sender_id, receiver); }


///授权方式：
   public:

      /*
       *@brief要求@ref帐户已批准此消息
       *@param account需要审批的账户
       *
       *此方法将检查@ref account是否在消息的声明授权中列出，并标记
       *使用授权。请注意，必须使用对消息的所有授权，否则消息无效。
       *
       *如果找不到足够的权限，则引发缺少的\u auth \u异常
       **/

      void require_authorization(const account_name& account);
      bool has_authorization(const account_name& account) const;
      void require_authorization(const account_name& account, const permission_name& permission);

      /*
       *@如果帐户存在则返回true，如果不存在则返回false
       **/

      bool is_account(const account_name& account)const;

      /*
       *要求将当前操作传递到帐户
       **/

      void require_recipient(account_name account);

      /*
       *如果当前操作已计划为
       *发送到指定帐户。
       **/

      bool has_recipient(account_name account)const;

///console方法：
   public:

      void reset_console();
      std::ostringstream& get_console_stream()            { return _pending_console_output; }
      const std::ostringstream& get_console_stream()const { return _pending_console_output; }

      template<typename T>
      void console_append(T val) {
         _pending_console_output << val;
      }

      template<typename T, typename ...Ts>
      void console_append(T val, Ts ...rest) {
         console_append(val);
         console_append(rest...);
      };

      inline void console_append_formatted(const string& fmt, const variant_object& vo) {
         console_append(fc::format_string(fmt, vo));
      }

///数据库方法：
   public:

      void update_db_usage( const account_name& payer, int64_t delta );

      int  db_store_i64( uint64_t scope, uint64_t table, const account_name& payer, uint64_t id, const char* buffer, size_t buffer_size );
      void db_update_i64( int iterator, account_name payer, const char* buffer, size_t buffer_size );
      void db_remove_i64( int iterator );
      int  db_get_i64( int iterator, char* buffer, size_t buffer_size );
      int  db_next_i64( int iterator, uint64_t& primary );
      int  db_previous_i64( int iterator, uint64_t& primary );
      int  db_find_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
      int  db_lowerbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
      int  db_upperbound_i64( uint64_t code, uint64_t scope, uint64_t table, uint64_t id );
      int  db_end_i64( uint64_t code, uint64_t scope, uint64_t table );

   private:

      const table_id_object* find_table( name code, name scope, name table );
      const table_id_object& find_or_create_table( name code, name scope, name table, const account_name &payer );
      void                   remove_table( const table_id_object& tid );

      int  db_store_i64( uint64_t code, uint64_t scope, uint64_t table, const account_name& payer, uint64_t id, const char* buffer, size_t buffer_size );


///MISC方法：
   public:

      int get_action( uint32_t type, uint32_t index, char* buffer, size_t buffer_size )const;
      int get_context_free_data( uint32_t index, char* buffer, size_t buffer_size )const;
      vector<account_name> get_active_producers() const;
      bytes  get_packed_transaction();

      uint64_t next_global_sequence();
      uint64_t next_recv_sequence( account_name receiver );
      uint64_t next_auth_sequence( account_name actor );

      void add_ram_usage( account_name account, int64_t ram_delta );
      void finalize_trace( action_trace& trace, const fc::time_point& start );

///字段：
   public:

      controller&                   control;
chainbase::database&          db;  ///<存储状态的数据库
transaction_context&          trx_context; ///<正在运行操作的事务上下文
const action&                 act; ///<正在应用消息
account_name                  receiver; ///<当前正在运行的代码
vector<bool> used_authorizations; ///<parallel to act.authorization；跟踪处理消息时使用了哪些权限
uint32_t                      recurse_depth; ///<内联操作可以重复的深度
      bool                          privileged   = false;
      bool                          context_free = false;
      bool                          used_context_free_api = false;

      generic_index<index64_object>                                  idx64;
      generic_index<index128_object>                                 idx128;
      generic_index<index256_object, uint128_t*, const uint128_t*>   idx256;
      generic_index<index_double_object>                             idx_double;
      generic_index<index_long_double_object>                        idx_long_double;

   private:

      iterator_cache<key_value_object>    keyval_cache;
vector<account_name>                _notified; ///<跟踪不存在当前消息的新帐户
vector<action>                      _inline_actions; ///<排队的内联消息
vector<action>                      _cfa_inline_actions; ///<排队的内联消息
      std::ostringstream                  _pending_console_output;
flat_set<account_delta>             _account_ram_deltas; ///<flat_set of account_delta so json is an array of objects

//字节缓存\u trx；
};

using apply_handler = std::function<void(apply_context&)>;

} } //命名空间eosio:：chain

//fc_reflect（eosio:：chain:：apply_context:：apply_results，（applied_actions）（延迟的_transaction_请求）（延迟的_transactions_count））。
