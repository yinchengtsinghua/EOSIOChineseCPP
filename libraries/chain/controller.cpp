
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <eosio/chain/controller.hpp>
#include <eosio/chain/transaction_context.hpp>

#include <eosio/chain/block_log.hpp>
#include <eosio/chain/fork_database.hpp>
#include <eosio/chain/exceptions.hpp>

#include <eosio/chain/account_object.hpp>
#include <eosio/chain/block_summary_object.hpp>
#include <eosio/chain/eosio_contract.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <eosio/chain/contract_table_objects.hpp>
#include <eosio/chain/generated_transaction_object.hpp>
#include <eosio/chain/transaction_object.hpp>
#include <eosio/chain/reversible_block_object.hpp>

#include <eosio/chain/authorization_manager.hpp>
#include <eosio/chain/resource_limits.hpp>
#include <eosio/chain/chain_snapshot.hpp>
#include <eosio/chain/thread_utils.hpp>

#include <chainbase/chainbase.hpp>
#include <fc/io/json.hpp>
#include <fc/scoped_exit.hpp>
#include <fc/variant_object.hpp>


namespace eosio { namespace chain {

using resource_limits::resource_limits_manager;

using controller_index_set = index_set<
   account_index,
   account_sequence_index,
   global_property_multi_index,
   dynamic_global_property_multi_index,
   block_summary_multi_index,
   transaction_multi_index,
   generated_transaction_multi_index,
   table_id_multi_index
>;

using contract_database_index_set = index_set<
   key_value_index,
   index64_index,
   index128_index,
   index256_index,
   index_double_index,
   index_long_double_index
>;

class maybe_session {
   public:
      maybe_session() = default;

      maybe_session( maybe_session&& other)
      :_session(move(other._session))
      {
      }

      explicit maybe_session(database& db) {
         _session = db.start_undo_session(true);
      }

      maybe_session(const maybe_session&) = delete;

      void squash() {
         if (_session)
            _session->squash();
      }

      void undo() {
         if (_session)
            _session->undo();
      }

      void push() {
         if (_session)
            _session->push();
      }

      maybe_session& operator = ( maybe_session&& mv ) {
         if (mv._session) {
            _session = move(*mv._session);
            mv._session.reset();
         } else {
            _session.reset();
         }

         return *this;
      };

   private:
      optional<database::session>     _session;
};

struct pending_state {
   pending_state( maybe_session&& s )
   :_db_session( move(s) ){}

   maybe_session                      _db_session;

   block_state_ptr                    _pending_block_state;

   vector<action_receipt>             _actions;

   controller::block_status           _block_status = controller::block_status::incomplete;

   optional<block_id_type>            _producer_block_id;

   void push() {
      _db_session.push();
   }
};

struct controller_impl {
   controller&                    self;
   chainbase::database            db;
chainbase::database            reversible_blocks; ///<a special database to persiste blocks that have successfully been applied but are still reversible.一个特殊的数据库，用于保存已成功应用但仍可逆的块。
   block_log                      blog;
   optional<pending_state>        pending;
   block_state_ptr                head;
   fork_database                  fork_db;
   wasm_interface                 wasmif;
   resource_limits_manager        resource_limits;
   authorization_manager          authorization;
   controller::config             conf;
   chain_id_type                  chain_id;
   bool                           replaying= false;
   optional<fc::time_point>       replay_head_time;
   db_read_mode                   read_mode = db_read_mode::SPECULATIVE;
bool                           in_trx_requiring_checks = false; ///<如果为真，则无法跳过通常在重播时跳过的检查（例如身份验证检查）
   optional<fc::microseconds>     subjective_cpu_leeway;
   bool                           trusted_producer_light_validation = false;
   uint32_t                       snapshot_head_block = 0;
   boost::asio::thread_pool       thread_pool;

   typedef pair<scope_name,action_name>                   handler_key;
   map< account_name, map<handler_key, apply_handler> >   apply_handlers;

   /*
    *被弹出块或中止块撤消的事务，事务
    *如果在其他块中重新应用，则从此列表中删除。生产者
    *可以在将新交易计划为块时查询此列表。
    **/

   map<digest_type, transaction_metadata_ptr>     unapplied_transactions;

   void pop_block() {
      auto prev = fork_db.get_block( head->header.previous );
      EOS_ASSERT( prev, block_validate_exception, "attempt to pop beyond last irreversible block" );

      if( const auto* b = reversible_blocks.find<reversible_block_object,by_num>(head->block_num) )
      {
         reversible_blocks.remove( *b );
      }

      if ( read_mode == db_read_mode::SPECULATIVE ) {
         EOS_ASSERT( head->block, block_validate_exception, "attempting to pop a block that was sparsely loaded from a snapshot");
         for( const auto& t : head->trxs )
            unapplied_transactions[t->signed_id] = t;
      }
      head = prev;
      db.undo();

   }


   void set_apply_handler( account_name receiver, account_name contract, action_name action, apply_handler v ) {
      apply_handlers[receiver][make_pair(contract,action)] = v;
   }

   controller_impl( const controller::config& cfg, controller& s  )
   :self(s),
    db( cfg.state_dir,
        cfg.read_only ? database::read_only : database::read_write,
        cfg.state_size ),
    reversible_blocks( cfg.blocks_dir/config::reversible_blocks_dir_name,
        cfg.read_only ? database::read_only : database::read_write,
        cfg.reversible_cache_size ),
    blog( cfg.blocks_dir ),
    fork_db( cfg.state_dir ),
    wasmif( cfg.wasm_runtime ),
    resource_limits( db ),
    authorization( s, db ),
    conf( cfg ),
    chain_id( cfg.genesis.compute_chain_id() ),
    read_mode( cfg.read_mode ),
    thread_pool( cfg.thread_pool_size )
   {

#define SET_APP_HANDLER( receiver, contract, action) \
   set_apply_handler( #receiver, #contract, #action, &BOOST_PP_CAT(apply_, BOOST_PP_CAT(contract, BOOST_PP_CAT(_,action) ) ) )

   SET_APP_HANDLER( eosio, eosio, newaccount );
   SET_APP_HANDLER( eosio, eosio, setcode );
   SET_APP_HANDLER( eosio, eosio, setabi );
   SET_APP_HANDLER( eosio, eosio, updateauth );
   SET_APP_HANDLER( eosio, eosio, deleteauth );
   SET_APP_HANDLER( eosio, eosio, linkauth );
   SET_APP_HANDLER( eosio, eosio, unlinkauth );
/*
   设置“应用程序处理程序”（eosio，eosio，post recovery）；
   设置“应用程序处理程序”（eosio、eosio、passrecovery）；
   设置“应用程序处理程序”（eosio、eosio、veotrecovery）；
**/


   SET_APP_HANDLER( eosio, eosio, canceldelay );

   fork_db.irreversible.connect( [&]( auto b ) {
                                 on_irreversible(b);
                                 });

   }

   /*
    *监听发出信号的插件/观察器（如接受的事务）可能会触发
    *错误和引发异常。除非发现这些例外情况，否则可能影响共识和/或
    *导致节点分叉。
    *
    *如果希望让信号处理程序从该方法中冒泡出异常
    *需要对其用途进行全面审计。
    *
    **/

   template<typename Signal, typename Arg>
   void emit( const Signal& s, Arg&& a ) {
      try {
        s(std::forward<Arg>(a));
      } catch (boost::interprocess::bad_alloc& e) {
         wlog( "bad alloc" );
         throw e;
      } catch ( controller_emit_signal_exception& e ) {
         wlog( "${details}", ("details", e.to_detail_string()) );
         throw e;
      } catch ( fc::exception& e ) {
         wlog( "${details}", ("details", e.to_detail_string()) );
      } catch ( ... ) {
         wlog( "signal handler threw exception" );
      }
   }

   void on_irreversible( const block_state_ptr& s ) {
      if( !blog.head() )
         blog.read_head();

      const auto& log_head = blog.head();
      bool append_to_blog = false;
      if (!log_head) {
         if (s->block) {
            EOS_ASSERT(s->block_num == blog.first_block_num(), block_log_exception, "block log has no blocks and is appending the wrong first block.  Expected ${expected}, but received: ${actual}",
                      ("expected", blog.first_block_num())("actual", s->block_num));
            append_to_blog = true;
         } else {
            EOS_ASSERT(s->block_num == blog.first_block_num() - 1, block_log_exception, "block log has no blocks and is not properly set up to start after the snapshot");
         }
      } else {
         auto lh_block_num = log_head->block_num();
         if (s->block_num > lh_block_num) {
            EOS_ASSERT(s->block_num - 1 == lh_block_num, unlinkable_block_exception, "unlinkable block", ("s->block_num", s->block_num)("lh_block_num", lh_block_num));
            EOS_ASSERT(s->block->previous == log_head->id(), unlinkable_block_exception, "irreversible doesn't link to block log head");
            append_to_blog = true;
         }
      }


      db.commit( s->block_num );

      if( append_to_blog ) {
         blog.append(s->block);
      }

      const auto& ubi = reversible_blocks.get_index<reversible_block_index,by_num>();
      auto objitr = ubi.begin();
      while( objitr != ubi.end() && objitr->blocknum <= s->block_num ) {
         reversible_blocks.remove( *objitr );
         objitr = ubi.begin();
      }

//加载快照时的“head”块是虚拟的，没有块数据，其所有效果
//应该已经从快照加载，因此无法应用
      if (s->block) {
         if (read_mode == db_read_mode::IRREVERSIBLE) {
//应用快照时，头部可能不存在
//不应用快照时，请确保这是下一个块
            if (!head || s->block_num == head->block_num + 1) {
               apply_block(s->block, controller::block_status::complete);
               head = s;
            } else {
//否则，断言初始化链的一个奇数情况
//从Genesis自动创建并应用第一个块。
//从另一个链同步时，会再次将其推入
               EOS_ASSERT(!head || head->block_num == 1, block_validate_exception, "Attempting to re-apply an irreversible block that was not the implied genesis block");
            }

            fork_db.mark_in_current_chain(head, true);
            fork_db.set_validity(head, true);
         }
         emit(self.irreversible_block, s);
      }
   }

   void replay(std::function<bool()> shutdown) {
      auto blog_head = blog.read_head();
      auto blog_head_time = blog_head->timestamp.to_time_point();
      replaying = true;
      replay_head_time = blog_head_time;
      auto start_block_num = head->block_num + 1;
      ilog( "existing block log, attempting to replay from ${s} to ${n} blocks",
            ("s", start_block_num)("n", blog_head->block_num()) );

      auto start = fc::time_point::now();
      while( auto next = blog.read_block_by_num( head->block_num + 1 ) ) {
         replay_push_block( next, controller::block_status::irreversible );
         if( next->block_num() % 100 == 0 ) {
            std::cerr << std::setw(10) << next->block_num() << " of " << blog_head->block_num() <<"\r";
            if( shutdown() ) break;
         }
      }
      std::cerr<< "\n";
      ilog( "${n} blocks replayed", ("n", head->block_num - start_block_num) );

//如果在未启用撤消会话的情况下播放不可恢复日志，则需要同步
//此处按适当的预期值修订序号。
      if( self.skip_db_sessions( controller::block_status::irreversible ) )
         db.set_revision(head->block_num);

      int rev = 0;
      while( auto obj = reversible_blocks.find<reversible_block_object,by_num>(head->block_num+1) ) {
         ++rev;
         replay_push_block( obj->get_block(), controller::block_status::validated );
      }

      ilog( "${n} reversible blocks replayed", ("n",rev) );
      auto end = fc::time_point::now();
      ilog( "replayed ${n} blocks in ${duration} seconds, ${mspb} ms/block",
            ("n", head->block_num - start_block_num)("duration", (end-start).count()/1000000)
            ("mspb", ((end-start).count()/1000.0)/(head->block_num-start_block_num)) );
      replaying = false;
      replay_head_time.reset();
   }

   void init(std::function<bool()> shutdown, const snapshot_reader_ptr& snapshot) {

      bool report_integrity_hash = !!snapshot;
      if (snapshot) {
         EOS_ASSERT( !head, fork_database_exception, "" );
         snapshot->validate();

         read_from_snapshot( snapshot );

         auto end = blog.read_head();
         if( !end ) {
            blog.reset( conf.genesis, signed_block_ptr(), head->block_num + 1 );
         } else if( end->block_num() > head->block_num ) {
            replay( shutdown );
         } else {
            EOS_ASSERT( end->block_num() == head->block_num, fork_database_exception,
                        "Block log is provided with snapshot but does not contain the head block from the snapshot" );
         }
      } else {
         if( !head ) {
initialize_fork_db(); //设为创世状态
         }

         auto end = blog.read_head();
         if( !end ) {
            blog.reset( conf.genesis, head->block );
         } else if( end->block_num() > head->block_num ) {
            replay( shutdown );
            report_integrity_hash = true;
         }
      }

      if( shutdown() ) return;

      const auto& ubi = reversible_blocks.get_index<reversible_block_index,by_num>();
      auto objitr = ubi.rbegin();
      if( objitr != ubi.rend() ) {
         EOS_ASSERT( objitr->blocknum == head->block_num, fork_database_exception,
                    "reversible block database is inconsistent with fork database, replay blockchain",
                    ("head",head->block_num)("unconfimed", objitr->blocknum)         );
      } else {
         auto end = blog.read_head();
         EOS_ASSERT( !end || end->block_num() == head->block_num, fork_database_exception,
                    "fork database exists but reversible block database does not, replay blockchain",
                    ("blog_head",end->block_num())("head",head->block_num)  );
      }

      EOS_ASSERT( db.revision() >= head->block_num, fork_database_exception, "fork database is inconsistent with shared memory",
                 ("db",db.revision())("head",head->block_num) );

      if( db.revision() > head->block_num ) {
         wlog( "warning: database revision (${db}) is greater than head block number (${head}), "
               "attempting to undo pending changes",
               ("db",db.revision())("head",head->block_num) );
      }
      while( db.revision() > head->block_num ) {
         db.undo();
      }

      if( report_integrity_hash ) {
         const auto hash = calculate_integrity_hash();
         ilog( "database initialized with hash: ${hash}", ("hash", hash) );
      }

   }

   ~controller_impl() {
      pending.reset();

      db.flush();
      reversible_blocks.flush();
   }

   void add_indices() {
      reversible_blocks.add_index<reversible_block_index>();

      controller_index_set::add_indices(db);
      contract_database_index_set::add_indices(db);

      authorization.add_indices();
      resource_limits.add_indices();
   }

   void clear_all_undo() {
//将数据库倒带到最后一个不可逆块
      db.with_write_lock([&] {
         db.undo_all();
         /*
         fc_assert（db.revision（）==self.head_block_num（），
                   “chainbase版本与head block num不匹配”，
                   （rev，db.revision（））（head_block，self.head_block_num（））；
                   **/

      });
   }

   void add_contract_tables_to_snapshot( const snapshot_writer_ptr& snapshot ) const {
      snapshot->write_section("contract_tables", [this]( auto& section ) {
         index_utils<table_id_multi_index>::walk(db, [this, &section]( const table_id_object& table_row ){
//为表添加行
            section.add_row(table_row, db);

//对于每种类型的表，后跟一个大小行，然后是n个数据行
            contract_database_index_set::walk_indices([this, &section, &table_row]( auto utils ) {
               using utils_t = decltype(utils);
               using value_t = typename decltype(utils)::index_t::value_type;
               using by_table_id = object_to_table_id_tag_t<value_t>;

               auto tid_key = boost::make_tuple(table_row.id);
               auto next_tid_key = boost::make_tuple(table_id_object::id_type(table_row.id._id + 1));

               unsigned_int size = utils_t::template size_range<by_table_id>(db, tid_key, next_tid_key);
               section.add_row(size, db);

               utils_t::template walk_range<by_table_id>(db, tid_key, next_tid_key, [this, &section]( const auto &row ) {
                  section.add_row(row, db);
               });
            });
         });
      });
   }

   void read_contract_tables_from_snapshot( const snapshot_reader_ptr& snapshot ) {
      snapshot->read_section("contract_tables", [this]( auto& section ) {
         bool more = !section.empty();
         while (more) {
//读取表的行
            table_id_object::id_type t_id;
            index_utils<table_id_multi_index>::create(db, [this, &section, &t_id](auto& row) {
               section.read_row(row, db);
               t_id = row.id;
            });

//读取每种类型表的大小和数据行
            contract_database_index_set::walk_indices([this, &section, &t_id, &more](auto utils) {
               using utils_t = decltype(utils);

               unsigned_int size;
               more = section.read_row(size, db);

               for (size_t idx = 0; idx < size.value; idx++) {
                  utils_t::create(db, [this, &section, &more, &t_id](auto& row) {
                     row.t_id = t_id;
                     more = section.read_row(row, db);
                  });
               }
            });
         }
      });
   }

   void add_to_snapshot( const snapshot_writer_ptr& snapshot ) const {
      snapshot->write_section<chain_snapshot_header>([this]( auto &section ){
         section.add_row(chain_snapshot_header(), db);
      });

      snapshot->write_section<genesis_state>([this]( auto &section ){
         section.add_row(conf.genesis, db);
      });

      snapshot->write_section<block_state>([this]( auto &section ){
         section.template add_row<block_header_state>(*fork_db.head(), db);
      });

      controller_index_set::walk_indices([this, &snapshot]( auto utils ){
         using value_t = typename decltype(utils)::index_t::value_type;

//跳过Table_id_对象作为其与契约表的内联部分
         if (std::is_same<value_t, table_id_object>::value) {
            return;
         }

         snapshot->write_section<value_t>([this]( auto& section ){
            decltype(utils)::walk(db, [this, &section]( const auto &row ) {
               section.add_row(row, db);
            });
         });
      });

      add_contract_tables_to_snapshot(snapshot);

      authorization.add_to_snapshot(snapshot);
      resource_limits.add_to_snapshot(snapshot);
   }

   void read_from_snapshot( const snapshot_reader_ptr& snapshot ) {
      snapshot->read_section<chain_snapshot_header>([this]( auto &section ){
         chain_snapshot_header header;
         section.read_row(header, db);
         header.validate();
      });


      snapshot->read_section<block_state>([this]( auto &section ){
         block_header_state head_header_state;
         section.read_row(head_header_state, db);

         auto head_state = std::make_shared<block_state>(head_header_state);
         fork_db.set(head_state);
         fork_db.set_validity(head_state, true);
         fork_db.mark_in_current_chain(head_state, true);
         head = head_state;
         snapshot_head_block = head->block_num;
      });

      controller_index_set::walk_indices([this, &snapshot]( auto utils ){
         using value_t = typename decltype(utils)::index_t::value_type;

//跳过Table_id_对象作为其与契约表的内联部分
         if (std::is_same<value_t, table_id_object>::value) {
            return;
         }

         snapshot->read_section<value_t>([this]( auto& section ) {
            bool more = !section.empty();
            while(more) {
               decltype(utils)::create(db, [this, &section, &more]( auto &row ) {
                  more = section.read_row(row, db);
               });
            }
         });
      });

      read_contract_tables_from_snapshot(snapshot);

      authorization.read_from_snapshot(snapshot);
      resource_limits.read_from_snapshot(snapshot);

      db.set_revision( head->block_num );
   }

   sha256 calculate_integrity_hash() const {
      sha256::encoder enc;
      auto hash_writer = std::make_shared<integrity_hash_snapshot_writer>(enc);
      add_to_snapshot(hash_writer);
      hash_writer->finalize();

      return enc.result();
   }


   /*
    *将fork数据库头设置为genesis状态。
    **/

   void initialize_fork_db() {
      wlog( " Initializing new blockchain with genesis state                  " );
      producer_schedule_type initial_schedule{ 0, {{config::system_account_name, conf.genesis.initial_key}} };

      block_header_state genheader;
      genheader.active_schedule       = initial_schedule;
      genheader.pending_schedule      = initial_schedule;
      genheader.pending_schedule_hash = fc::sha256::hash(initial_schedule);
      genheader.header.timestamp      = conf.genesis.initial_timestamp;
      genheader.header.action_mroot   = conf.genesis.compute_chain_id();
      genheader.id                    = genheader.header.id();
      genheader.block_num             = genheader.header.block_num();

      head = std::make_shared<block_state>( genheader );
      head->block = std::make_shared<signed_block>(genheader.header);
      fork_db.set( head );
      db.set_revision( head->block_num );

      initialize_database();
   }

   void create_native_account( account_name name, const authority& owner, const authority& active, bool is_privileged = false ) {
      db.create<account_object>([&](auto& a) {
         a.name = name;
         a.creation_date = conf.genesis.initial_timestamp;
         a.privileged = is_privileged;

         if( name == config::system_account_name ) {
            a.set_abi(eosio_contract_abi(abi_def()));
         }
      });
      db.create<account_sequence_object>([&](auto & a) {
        a.name = name;
      });

      const auto& owner_permission  = authorization.create_permission(name, config::owner_name, 0,
                                                                      owner, conf.genesis.initial_timestamp );
      const auto& active_permission = authorization.create_permission(name, config::active_name, owner_permission.id,
                                                                      active, conf.genesis.initial_timestamp );

      resource_limits.initialize_account(name);

      int64_t ram_delta = config::overhead_per_account_ram_bytes;
      ram_delta += 2*config::billable_size_v<permission_object>;
      ram_delta += owner_permission.auth.get_billable_size();
      ram_delta += active_permission.auth.get_billable_size();

      resource_limits.add_pending_ram_usage(name, ram_delta);
      resource_limits.verify_account_ram_usage(name);
   }

   void initialize_database() {
//初始化块摘要索引
      for (int i = 0; i < 0x10000; i++)
         db.create<block_summary_object>([&](block_summary_object&) {});

      const auto& tapos_block_summary = db.get<block_summary_object>(1);
      db.modify( tapos_block_summary, [&]( auto& bs ) {
        bs.block_id = head->id;
      });

      conf.genesis.initial_configuration.validate();
      db.create<global_property_object>([&](auto& gpo ){
        gpo.configuration = conf.genesis.initial_configuration;
      });
      db.create<dynamic_global_property_object>([](auto&){});

      authorization.initialize_database();
      resource_limits.initialize_database();

      authority system_auth(conf.genesis.initial_key);
      create_native_account( config::system_account_name, system_auth, system_auth, true );

      auto empty_authority = authority(1, {}, {});
      auto active_producers_authority = authority(1, {}, {});
      active_producers_authority.accounts.push_back({{config::system_account_name, config::active_name}, 1});

      create_native_account( config::null_account_name, empty_authority, empty_authority );
      create_native_account( config::producers_account_name, empty_authority, active_producers_authority );
      const auto& active_permission       = authorization.get_permission({config::producers_account_name, config::active_name});
      const auto& majority_permission     = authorization.create_permission( config::producers_account_name,
                                                                             config::majority_producers_permission_name,
                                                                             active_permission.id,
                                                                             active_producers_authority,
                                                                             conf.genesis.initial_timestamp );
      const auto& minority_permission     = authorization.create_permission( config::producers_account_name,
                                                                             config::minority_producers_permission_name,
                                                                             majority_permission.id,
                                                                             active_producers_authority,
                                                                             conf.genesis.initial_timestamp );
   }



   /*
    *@post无论提交块是否成功，都没有活动的挂起块
    **/

   void commit_block( bool add_to_fork_db ) {
      auto reset_pending_on_exit = fc::make_scoped_exit([this]{
         pending.reset();
      });

      try {
         if (add_to_fork_db) {
            pending->_pending_block_state->validated = true;
            auto new_bsp = fork_db.add(pending->_pending_block_state, true);
            emit(self.accepted_block_header, pending->_pending_block_state);
            head = fork_db.head();
            EOS_ASSERT(new_bsp == head, fork_database_exception, "committed block did not become the new head in fork database");
         }

         if( !replaying ) {
            reversible_blocks.create<reversible_block_object>( [&]( auto& ubo ) {
               ubo.blocknum = pending->_pending_block_state->block_num;
               ubo.set_block( pending->_pending_block_state->block );
            });
         }

         emit( self.accepted_block, pending->_pending_block_state );
      } catch (...) {
//不要麻烦重置挂起，而是中止块
         reset_pending_on_exit.cancel();
         abort_block();
         throw;
      }

//将状态推到挂起状态。
      pending->push();
   }

//返回的作用域退出不应超过调用make块还原点时存在的挂起的生存期。
   fc::scoped_exit<std::function<void()>> make_block_restore_point() {
      auto orig_block_transactions_size = pending->_pending_block_state->block->transactions.size();
      auto orig_state_transactions_size = pending->_pending_block_state->trxs.size();
      auto orig_state_actions_size      = pending->_actions.size();

      std::function<void()> callback = [this,
                                        orig_block_transactions_size,
                                        orig_state_transactions_size,
                                        orig_state_actions_size]()
      {
         pending->_pending_block_state->block->transactions.resize(orig_block_transactions_size);
         pending->_pending_block_state->trxs.resize(orig_state_transactions_size);
         pending->_actions.resize(orig_state_actions_size);
      };

      return fc::make_scoped_exit( std::move(callback) );
   }

   transaction_trace_ptr apply_onerror( const generated_transaction& gtrx,
                                        fc::time_point deadline,
                                        fc::time_point start,
uint32_t& cpu_time_to_bill_us, //仅在失败时设置
                                        uint32_t billed_cpu_time_us,
                                        bool explicit_billed_cpu_time = false,
                                        bool enforce_whiteblacklist = true
                                      )
   {
      signed_transaction etrx;
//将包含失败延迟事务的OnError操作直接传递回发件人。
      etrx.actions.emplace_back( vector<permission_level>{{gtrx.sender, config::active_name}},
                                 onerror( gtrx.sender_id, gtrx.packed_trx.data(), gtrx.packed_trx.size() ) );
etrx.expiration = self.pending_block_time() + fc::microseconds(999'999); //四舍五入以避免出现过期
      etrx.set_reference_block( self.head_block_id() );

      transaction_context trx_context( self, etrx, etrx.id(), start );
      trx_context.deadline = deadline;
      trx_context.explicit_billed_cpu_time = explicit_billed_cpu_time;
      trx_context.billed_cpu_time_us = billed_cpu_time_us;
      trx_context.enforce_whiteblacklist = enforce_whiteblacklist;
      transaction_trace_ptr trace = trx_context.trace;
      try {
         trx_context.init_for_implicit_trx();
         trx_context.published = gtrx.published;
         trx_context.trace->action_traces.emplace_back();
         trx_context.dispatch_action( trx_context.trace->action_traces.back(), etrx.actions.back(), gtrx.sender );
trx_context.finalize(); //如果成功，自动在跟踪和帐单支付程序中查找网络和CPU使用情况

         auto restore = make_block_restore_point();
         trace->receipt = push_receipt( gtrx.trx_id, transaction_receipt::soft_fail,
                                        trx_context.billed_cpu_time_us, trace->net_usage );
         fc::move_append( pending->_actions, move(trx_context.executed) );

         trx_context.squash();
         restore.cancel();
         return trace;
      } catch( const fc::exception& e ) {
         cpu_time_to_bill_us = trx_context.update_billed_cpu_time( fc::time_point::now() );
         trace->except = e;
         trace->except_ptr = std::current_exception();
      }
      return trace;
   }

   void remove_scheduled_transaction( const generated_transaction_object& gto ) {
      resource_limits.add_pending_ram_usage(
         gto.payer,
         -(config::billable_size_v<generated_transaction_object> + gto.packed_trx.size())
      );
//无需验证帐户RAM的使用情况，因为我们只是在减少内存

      db.remove( gto );
   }

   bool failure_is_subjective( const fc::exception& e ) const {
      auto code = e.code();
      return    (code == subjective_block_production_exception::code_value)
             || (code == block_net_usage_exceeded::code_value)
             || (code == greylist_net_usage_exceeded::code_value)
             || (code == block_cpu_usage_exceeded::code_value)
             || (code == greylist_cpu_usage_exceeded::code_value)
             || (code == deadline_exception::code_value)
             || (code == leeway_deadline_exception::code_value)
             || (code == actor_whitelist_exception::code_value)
             || (code == actor_blacklist_exception::code_value)
             || (code == contract_whitelist_exception::code_value)
             || (code == contract_blacklist_exception::code_value)
             || (code == action_blacklist_exception::code_value)
             || (code == key_blacklist_exception::code_value);
   }

   bool scheduled_failure_is_subjective( const fc::exception& e ) const {
      auto code = e.code();
      return    (code == tx_cpu_usage_exceeded::code_value)
             || failure_is_subjective(e);
   }

   transaction_trace_ptr push_scheduled_transaction( const transaction_id_type& trxid, fc::time_point deadline, uint32_t billed_cpu_time_us, bool explicit_billed_cpu_time = false ) {
      const auto& idx = db.get_index<generated_transaction_multi_index,by_trx_id>();
      auto itr = idx.find( trxid );
      EOS_ASSERT( itr != idx.end(), unknown_transaction_exception, "unknown transaction" );
      return push_scheduled_transaction( *itr, deadline, billed_cpu_time_us, explicit_billed_cpu_time );
   }

   transaction_trace_ptr push_scheduled_transaction( const generated_transaction_object& gto, fc::time_point deadline, uint32_t billed_cpu_time_us, bool explicit_billed_cpu_time = false )
   { try {
      maybe_session undo_session;
      if ( !self.skip_db_sessions() )
         undo_session = maybe_session(db);

      auto gtrx = generated_transaction(gto);

//复制后删除生成的事务对象
//这将确保影响GTO多索引容器的任何内容都不会失效。
//我们需要成功注销此事务的数据。
//
//如果事务以主观方式失败，“撤消会话”应在不被挤压的情况下过期
//从而恢复GTO并可供将来的块使用。
      remove_scheduled_transaction(gto);

      fc::datastream<const char*> ds( gtrx.packed_trx.data(), gtrx.packed_trx.size() );

      EOS_ASSERT( gtrx.delay_until <= self.pending_block_time(), transaction_exception, "this transaction isn't ready",
                 ("gtrx.delay_until",gtrx.delay_until)("pbt",self.pending_block_time())          );

      signed_transaction dtrx;
      fc::raw::unpack(ds,static_cast<transaction&>(dtrx) );
      transaction_metadata_ptr trx = std::make_shared<transaction_metadata>( dtrx );
      trx->accepted = true;
      trx->scheduled = true;

      transaction_trace_ptr trace;
      if( gtrx.expiration < self.pending_block_time() ) {
         trace = std::make_shared<transaction_trace>();
         trace->id = gtrx.trx_id;
         trace->block_num = self.pending_block_state()->block_num;
         trace->block_time = self.pending_block_time();
         trace->producer_block_id = self.pending_producer_block_id();
         trace->scheduled = true;
trace->receipt = push_receipt( gtrx.trx_id, transaction_receipt::expired, billed_cpu_time_us, 0 ); //交易到期
         emit( self.accepted_transaction, trx );
         emit( self.applied_transaction, trace );
         undo_session.squash();
         return trace;
      }

      auto reset_in_trx_requiring_checks = fc::make_scoped_exit([old_value=in_trx_requiring_checks,this](){
         in_trx_requiring_checks = old_value;
      });
      in_trx_requiring_checks = true;

      uint32_t cpu_time_to_bill_us = billed_cpu_time_us;

      transaction_context trx_context( self, dtrx, gtrx.trx_id );
trx_context.leeway =  fc::microseconds(0); //避免窃取CPU资源
      trx_context.deadline = deadline;
      trx_context.explicit_billed_cpu_time = explicit_billed_cpu_time;
      trx_context.billed_cpu_time_us = billed_cpu_time_us;
      trx_context.enforce_whiteblacklist = gtrx.sender.empty() ? true : !sender_avoids_whitelist_blacklist_enforcement( gtrx.sender );
      trace = trx_context.trace;
      try {
         trx_context.init_for_deferred_trx( gtrx.published );

         if( trx_context.enforce_whiteblacklist && pending->_block_status == controller::block_status::incomplete ) {
check_actor_list( trx_context.bill_to_accounts ); //假设billou toou帐户是授权交易的一组参与者
         }

         trx_context.exec();
trx_context.finalize(); //如果成功，自动在跟踪和帐单支付程序中查找网络和CPU使用情况

         auto restore = make_block_restore_point();

         trace->receipt = push_receipt( gtrx.trx_id,
                                        transaction_receipt::executed,
                                        trx_context.billed_cpu_time_us,
                                        trace->net_usage );

         fc::move_append( pending->_actions, move(trx_context.executed) );

         emit( self.accepted_transaction, trx );
         emit( self.applied_transaction, trace );

         trx_context.squash();
         undo_session.squash();

         restore.cancel();

         return trace;
      } catch( const fc::exception& e ) {
         cpu_time_to_bill_us = trx_context.update_billed_cpu_time( fc::time_point::now() );
         trace->except = e;
         trace->except_ptr = std::current_exception();
         trace->elapsed = fc::time_point::now() - trx_context.start;
      }
      trx_context.undo();

//只有主观或软或硬故障逻辑如下：

      if( gtrx.sender != account_name() && !failure_is_subjective(*trace->except)) {
//尝试对生成的事务进行错误处理。

         auto error_trace = apply_onerror( gtrx, deadline, trx_context.pseudo_start,
                                           cpu_time_to_bill_us, billed_cpu_time_us, explicit_billed_cpu_time,
                                           trx_context.enforce_whiteblacklist );
         error_trace->failed_dtrx_trace = trace;
         trace = error_trace;
         if( !trace->except_ptr ) {
            emit( self.accepted_transaction, trx );
            emit( self.applied_transaction, trace );
            undo_session.squash();
            return trace;
         }
         trace->elapsed = fc::time_point::now() - trx_context.start;
      }

//以下仅为主观或硬故障逻辑：

//基于产生与验证的主观性变化
      bool subjective  = false;
      if (explicit_billed_cpu_time) {
         subjective = failure_is_subjective(*trace->except);
      } else {
         subjective = scheduled_failure_is_subjective(*trace->except);
      }

      if ( !subjective ) {
//硬故障逻辑

         if( !explicit_billed_cpu_time ) {
            auto& rl = self.get_mutable_resource_limits_manager();
            rl.update_account_usage( trx_context.bill_to_accounts, block_timestamp_type(self.pending_block_time()).slot );
            int64_t account_cpu_limit = 0;
            std::tie( std::ignore, account_cpu_limit, std::ignore, std::ignore ) = trx_context.max_bandwidth_billed_accounts_can_pay( true );

            cpu_time_to_bill_us = static_cast<uint32_t>( std::min( std::min( static_cast<int64_t>(cpu_time_to_bill_us),
                                                                             account_cpu_limit                          ),
                                                                   trx_context.initial_objective_duration_limit.count()    ) );
         }

         resource_limits.add_transaction_usage( trx_context.bill_to_accounts, cpu_time_to_bill_us, 0,
block_timestamp_type(self.pending_block_time()).slot ); //永远不会失败

         trace->receipt = push_receipt(gtrx.trx_id, transaction_receipt::hard_fail, cpu_time_to_bill_us, 0);

         emit( self.accepted_transaction, trx );
         emit( self.applied_transaction, trace );

         undo_session.squash();
      } else {
         emit( self.accepted_transaction, trx );
         emit( self.applied_transaction, trace );
      }

      return trace;
} FC_CAPTURE_AND_RETHROW() } ///push_scheduled_事务


   /*
    *将交易凭证添加到挂起块并返回。
    **/

   template<typename T>
   const transaction_receipt& push_receipt( const T& trx, transaction_receipt_header::status_enum status,
                                            uint64_t cpu_usage_us, uint64_t net_usage ) {
      uint64_t net_usage_words = net_usage / 8;
      EOS_ASSERT( net_usage_words*8 == net_usage, transaction_exception, "net_usage is not divisible by 8" );
      pending->_pending_block_state->block->transactions.emplace_back( trx );
      transaction_receipt& r = pending->_pending_block_state->block->transactions.back();
      r.cpu_usage_us         = cpu_usage_us;
      r.net_usage_words      = net_usage_words;
      r.status               = status;
      return r;
   }

   /*
    *这是块状态新事务的入口点。它将检查授权和
    *决定现在执行还是延迟执行。最后，它将交易凭证插入
    *挂起的块。
    **/

   transaction_trace_ptr push_transaction( const transaction_metadata_ptr& trx,
                                           fc::time_point deadline,
                                           uint32_t billed_cpu_time_us,
                                           bool explicit_billed_cpu_time = false )
   {
      EOS_ASSERT(deadline != fc::time_point(), transaction_exception, "deadline cannot be uninitialized");

      transaction_trace_ptr trace;
      try {
         auto start = fc::time_point::now();
         if( !explicit_billed_cpu_time ) {
            fc::microseconds already_consumed_time( EOS_PERCENT(trx->sig_cpu_usage.count(), conf.sig_cpu_bill_pct) );

            if( start.time_since_epoch() <  already_consumed_time ) {
               start = fc::time_point();
            } else {
               start -= already_consumed_time;
            }
         }

         const signed_transaction& trn = trx->packed_trx->get_signed_transaction();
         transaction_context trx_context(self, trn, trx->id, start);
         if ((bool)subjective_cpu_leeway && pending->_block_status == controller::block_status::incomplete) {
            trx_context.leeway = *subjective_cpu_leeway;
         }
         trx_context.deadline = deadline;
         trx_context.explicit_billed_cpu_time = explicit_billed_cpu_time;
         trx_context.billed_cpu_time_us = billed_cpu_time_us;
         trace = trx_context.trace;
         try {
            if( trx->implicit ) {
               trx_context.init_for_implicit_trx();
               trx_context.enforce_whiteblacklist = false;
            } else {
               bool skip_recording = replay_head_time && (time_point(trn.expiration) <= *replay_head_time);
               trx_context.init_for_input_trx( trx->packed_trx->get_unprunable_size(),
                                               trx->packed_trx->get_prunable_size(),
                                               skip_recording);
            }

            trx_context.delay = fc::seconds(trn.delay_sec);

            if( !self.skip_auth_check() && !trx->implicit ) {
               authorization.check_authorization(
                       trn.actions,
                       trx->recover_keys( chain_id ),
                       {},
                       trx_context.delay,
                       [](){}
                       /*td:：bind（&transaction_context:：add_cpu_usage_and_check_time，&trx_context，，
                                 标准：：占位符：：1*/,

                       false
               );
            }
            trx_context.exec();
trx_context.finalize(); //如果成功，自动在跟踪和帐单支付程序中查找网络和CPU使用情况

            auto restore = make_block_restore_point();

            if (!trx->implicit) {
               transaction_receipt::status_enum s = (trx_context.delay == fc::seconds(0))
                                                    ? transaction_receipt::executed
                                                    : transaction_receipt::delayed;
               trace->receipt = push_receipt(*trx->packed_trx, s, trx_context.billed_cpu_time_us, trace->net_usage);
               pending->_pending_block_state->trxs.emplace_back(trx);
            } else {
               transaction_receipt_header r;
               r.status = transaction_receipt::executed;
               r.cpu_usage_us = trx_context.billed_cpu_time_us;
               r.net_usage_words = trace->net_usage / 8;
               trace->receipt = r;
            }

            fc::move_append(pending->_actions, move(trx_context.executed));

//只为此事务调用一次接受信号
            if (!trx->accepted) {
               trx->accepted = true;
               emit( self.accepted_transaction, trx);
            }

            emit(self.applied_transaction, trace);


            if ( read_mode != db_read_mode::SPECULATIVE && pending->_block_status == controller::block_status::incomplete ) {
//这可能会在析构函数中自动发生，但我更倾向于使其更明确。
               trx_context.undo();
            } else {
               restore.cancel();
               trx_context.squash();
            }

            if (!trx->implicit) {
               unapplied_transactions.erase( trx->signed_id );
            }
            return trace;
         } catch (const fc::exception& e) {
            trace->except = e;
            trace->except_ptr = std::current_exception();
         }

         if (!failure_is_subjective(*trace->except)) {
            unapplied_transactions.erase( trx->signed_id );
         }

         emit( self.accepted_transaction, trx );
         emit( self.applied_transaction, trace );

         return trace;
      } FC_CAPTURE_AND_RETHROW((trace))
} ///推送事务


   void start_block( block_timestamp_type when, uint16_t confirm_block_count, controller::block_status s,
                     const optional<block_id_type>& producer_block_id )
   {
      EOS_ASSERT( !pending, block_validate_exception, "pending block already exists" );

      auto guard_pending = fc::make_scoped_exit([this](){
         pending.reset();
      });

      if (!self.skip_db_sessions(s)) {
         EOS_ASSERT( db.revision() == head->block_num, database_exception, "db revision is not on par with head block",
                     ("db.revision()", db.revision())("controller_head_block", head->block_num)("fork_db_head_block", fork_db.head()->block_num) );

         pending.emplace(maybe_session(db));
      } else {
         pending.emplace(maybe_session());
      }

      pending->_block_status = s;
      pending->_producer_block_id = producer_block_id;
pending->_pending_block_state = std::make_shared<block_state>( *head, when ); //将挂起的计划（如果有）提升为活动
      pending->_pending_block_state->in_current_chain = true;

      pending->_pending_block_state->set_confirmed(confirm_block_count);

      auto was_pending_promoted = pending->_pending_block_state->maybe_promote_pending();

//仅当我们处于推测性读取模式时，才修改推测性块中的状态（否则，我们需要清除头或不可逆读取的状态）
      if ( read_mode == db_read_mode::SPECULATIVE || pending->_block_status != controller::block_status::incomplete ) {

         const auto& gpo = db.get<global_property_object>();
if( gpo.proposed_schedule_block_num.valid() && //如果在一个街区内有一个提议的时间表…
( *gpo.proposed_schedule_block_num <= pending->_pending_block_state->dpos_irreversible_blocknum ) && //…现在这已经变得不可逆转…
pending->_pending_block_state->pending_schedule.producers.size() == 0 && //…还有一个新的等待时间表…
!was_pending_promoted //…不仅仅是因为它在这个块的开头被提升为活动的，然后：
         )
            {
//将建议的计划升级为待定计划。
               if( !replaying ) {
                  ilog( "promoting proposed schedule (set in block ${proposed_num}) to pending; current block: ${n} lib: ${lib} schedule: ${schedule} ",
                        ("proposed_num", *gpo.proposed_schedule_block_num)("n", pending->_pending_block_state->block_num)
                        ("lib", pending->_pending_block_state->dpos_irreversible_blocknum)
                        ("schedule", static_cast<producer_schedule_type>(gpo.proposed_schedule) ) );
               }
               pending->_pending_block_state->set_new_producers( gpo.proposed_schedule );
               db.modify( gpo, [&]( auto& gp ) {
                     gp.proposed_schedule_block_num = optional<block_num_type>();
                     gp.proposed_schedule.clear();
                  });
            }

         try {
            auto onbtrx = std::make_shared<transaction_metadata>( get_on_block_transaction() );
            onbtrx->implicit = true;
            auto reset_in_trx_requiring_checks = fc::make_scoped_exit([old_value=in_trx_requiring_checks,this](){
                  in_trx_requiring_checks = old_value;
               });
            in_trx_requiring_checks = true;
            push_transaction( onbtrx, fc::time_point::maximum(), self.get_global_properties().configuration.min_transaction_cpu_usage, true );
         } catch( const boost::interprocess::bad_alloc& e  ) {
            elog( "on block transaction failed due to a bad allocation" );
            throw;
         } catch( const fc::exception& e ) {
            wlog( "on block transaction failed, but shouldn't impact block generation, system contract needs update" );
            edump((e.to_detail_string()));
         } catch( ... ) {
         }

         clear_expired_input_transactions();
         update_producers_authority();
      }

      guard_pending.cancel();
} //启动块



   void sign_block( const std::function<signature_type( const digest_type& )>& signer_callback  ) {
      auto p = pending->_pending_block_state;

      p->sign( signer_callback );

      static_cast<signed_block_header&>(*p->block) = p->header;
} ///信号块

   void apply_block( const signed_block_ptr& b, controller::block_status s ) { try {
      try {
         EOS_ASSERT( b->block_extensions.size() == 0, block_validate_exception, "no supported extensions" );
         auto producer_block_id = b->id();
         start_block( b->timestamp, b->confirmed, s , producer_block_id);

         std::vector<transaction_metadata_ptr> packed_transactions;
         packed_transactions.reserve( b->transactions.size() );
         for( const auto& receipt : b->transactions ) {
            if( receipt.trx.contains<packed_transaction>()) {
               auto& pt = receipt.trx.get<packed_transaction>();
               auto mtrx = std::make_shared<transaction_metadata>( std::make_shared<packed_transaction>( pt ) );
               if( !self.skip_auth_check() ) {
                  transaction_metadata::create_signing_keys_future( mtrx, thread_pool, chain_id, microseconds::maximum() );
               }
               packed_transactions.emplace_back( std::move( mtrx ) );
            }
         }

         transaction_trace_ptr trace;

         size_t packed_idx = 0;
         for( const auto& receipt : b->transactions ) {
            auto num_pending_receipts = pending->_pending_block_state->block->transactions.size();
            if( receipt.trx.contains<packed_transaction>() ) {
               trace = push_transaction( packed_transactions.at(packed_idx++), fc::time_point::maximum(), receipt.cpu_usage_us, true );
            } else if( receipt.trx.contains<transaction_id_type>() ) {
               trace = push_scheduled_transaction( receipt.trx.get<transaction_id_type>(), fc::time_point::maximum(), receipt.cpu_usage_us, true );
            } else {
               EOS_ASSERT( false, block_validate_exception, "encountered unexpected receipt type" );
            }

            bool transaction_failed =  trace && trace->except;
            bool transaction_can_fail = receipt.status == transaction_receipt_header::hard_fail && receipt.trx.contains<transaction_id_type>();
            if( transaction_failed && !transaction_can_fail) {
               edump((*trace));
               throw *trace->except;
            }

            EOS_ASSERT( pending->_pending_block_state->block->transactions.size() > 0,
                        block_validate_exception, "expected a receipt",
                        ("block", *b)("expected_receipt", receipt)
                      );
            EOS_ASSERT( pending->_pending_block_state->block->transactions.size() == num_pending_receipts + 1,
                        block_validate_exception, "expected receipt was not added",
                        ("block", *b)("expected_receipt", receipt)
                      );
            const transaction_receipt_header& r = pending->_pending_block_state->block->transactions.back();
            EOS_ASSERT( r == static_cast<const transaction_receipt_header&>(receipt),
                        block_validate_exception, "receipt does not match",
                        ("producer_receipt", receipt)("validator_receipt", pending->_pending_block_state->block->transactions.back()) );
         }

         finalize_block();

//这隐式地断言所有头字段（减去签名）都是相同的
         EOS_ASSERT(producer_block_id == pending->_pending_block_state->header.id(),
                   block_validate_exception, "Block ID does not match",
                   ("producer_block_id",producer_block_id)("validator_block_id",pending->_pending_block_state->header.id()));

//我们需要填写挂起块状态的块，因为它在可逆块日志中被序列化。
//在将来，我们可以通过将原件而不是副本序列化来优化这一点。

//我们可以一直信任这个签名，因为，
//-在应用\u块之前，我们调用fork \u db.add，它在该块不受信任时执行签名检查
//-否则块是可信的，因此我们相信签名是有效的
//另外，由于：：sign_块不会懒惰地计算块的摘要，因此我们可以短路以节省周期。
         pending->_pending_block_state->header.producer_signature = b->producer_signature;
         static_cast<signed_block_header&>(*pending->_pending_block_state->block) =  pending->_pending_block_state->header;

         commit_block(false);
         return;
      } catch ( const fc::exception& e ) {
         edump((e.to_detail_string()));
         abort_block();
         throw;
      }
} FC_CAPTURE_AND_RETHROW() } //应用程序块

   std::future<block_state_ptr> create_block_state_future( const signed_block_ptr& b ) {
      EOS_ASSERT( b, block_validate_exception, "null block" );

      auto id = b->id();

//如果fork-db已经知道block，那么就没有理由使用block-state。
      auto existing = fork_db.get_block( id );
      EOS_ASSERT( !existing, fork_database_exception, "we already know about this block: ${id}", ("id", id) );

      auto prev = fork_db.get_block( b->previous );
      EOS_ASSERT( prev, unlinkable_block_exception, "unlinkable block ${id}", ("id", id)("previous", b->previous) );

      return async_thread_pool( thread_pool, [b, prev]() {
         const bool skip_validate_signee = false;
         return std::make_shared<block_state>( *prev, move( b ), skip_validate_signee );
      } );
   }

   void push_block( std::future<block_state_ptr>& block_state_future ) {
      controller::block_status s = controller::block_status::complete;
      EOS_ASSERT(!pending, block_validate_exception, "it is not valid to push a block when there is a pending block");

      auto reset_prod_light_validation = fc::make_scoped_exit([old_value=trusted_producer_light_validation, this]() {
         trusted_producer_light_validation = old_value;
      });
      try {
         block_state_ptr new_header_state = block_state_future.get();
         auto& b = new_header_state->block;
         emit( self.pre_accepted_block, b );

         fork_db.add( new_header_state, false );

         if (conf.trusted_producers.count(b->producer)) {
            trusted_producer_light_validation = true;
         };
         emit( self.accepted_block_header, new_header_state );

         if ( read_mode != db_read_mode::IRREVERSIBLE ) {
            maybe_switch_forks( s );
         }

      } FC_LOG_AND_RETHROW( )
   }

   void replay_push_block( const signed_block_ptr& b, controller::block_status s ) {
      self.validate_db_available_size();
      self.validate_reversible_available_size();

      EOS_ASSERT(!pending, block_validate_exception, "it is not valid to push a block when there is a pending block");

      try {
         EOS_ASSERT( b, block_validate_exception, "trying to push empty block" );
         EOS_ASSERT( (s == controller::block_status::irreversible || s == controller::block_status::validated),
                     block_validate_exception, "invalid block status for replay" );
         emit( self.pre_accepted_block, b );
         const bool skip_validate_signee = !conf.force_all_checks;
         auto new_header_state = fork_db.add( b, skip_validate_signee );

         emit( self.accepted_block_header, new_header_state );

         if ( read_mode != db_read_mode::IRREVERSIBLE ) {
            maybe_switch_forks( s );
         }

//在重播时，fork数据库不会发出不可逆转的消息，因此在此处显式发出
         if( s == controller::block_status::irreversible )
            emit( self.irreversible_block, new_header_state );

      } FC_LOG_AND_RETHROW( )
   }

   void maybe_switch_forks( controller::block_status s ) {
      auto new_head = fork_db.head();

      if( new_head->header.previous == head->id ) {
         try {
            apply_block( new_head->block, s );
            fork_db.mark_in_current_chain( new_head, true );
            fork_db.set_validity( new_head, true );
            head = new_head;
         } catch ( const fc::exception& e ) {
fork_db.set_validity( new_head, false ); //从fork-db索引中删除新的头，因此无需将其标记为不在当前链中。
            throw;
         }
      } else if( new_head->id != head->id ) {
         ilog("switching forks from ${current_head_id} (block number ${current_head_num}) to ${new_head_id} (block number ${new_head_num})",
              ("current_head_id", head->id)("current_head_num", head->block_num)("new_head_id", new_head->id)("new_head_num", new_head->block_num) );
         auto branches = fork_db.fetch_branch_from( new_head->id, head->id );

         for( auto itr = branches.second.begin(); itr != branches.second.end(); ++itr ) {
            fork_db.mark_in_current_chain( *itr, false );
            pop_block();
         }
         EOS_ASSERT( self.head_block_id() == branches.second.back()->header.previous, fork_database_exception,
"loss of sync between fork_db and chainbase during fork switch" ); //你应该永远不会失败

         for( auto ritr = branches.first.rbegin(); ritr != branches.first.rend(); ++ritr ) {
            optional<fc::exception> except;
            try {
               apply_block( (*ritr)->block, (*ritr)->validated ? controller::block_status::validated : controller::block_status::complete );
               head = *ritr;
               fork_db.mark_in_current_chain( *ritr, true );
               (*ritr)->validated = true;
            }
            catch (const fc::exception& e) { except = e; }
            if (except) {
               elog("exception thrown while switching forks ${e}", ("e", except->to_detail_string()));

//Ritr当前指向抛出的块
//如果我们将其标记为无效，它将自动移除所有从其上构建的分叉。
               fork_db.set_validity( *ritr, false );

//把坏叉子的所有木块都打出来
//RITR BASE是一个转发ITR到成功应用的最后一个块
               auto applied_itr = ritr.base();
               for( auto itr = applied_itr; itr != branches.first.end(); ++itr ) {
                  fork_db.mark_in_current_chain( *itr, false );
                  pop_block();
               }
               EOS_ASSERT( self.head_block_id() == branches.second.back()->header.previous, fork_database_exception,
"loss of sync between fork_db and chainbase during fork switch reversal" ); //你应该永远不会失败

//重新应用好的块
               for( auto ritr = branches.second.rbegin(); ritr != branches.second.rend(); ++ritr ) {
                  /*ly_block（（*ritr）->block，controller:：block_status:：validated/*我们以前验证过这些块*/）；
                  头= *RITR；
                  fork_db.mark_in_current_chain（*ritr，true）；
               }
               抛出*除；
            //异常结束
         //分支中每个块的结束
         ilog（“成功地将fork切换到新的head$新的head”，“新的head_id”，“新的head->id”）；
      }
   //推块

   void abort_block（）
      如果（待定）{
         if（read_模式==db_read_模式：：推测）
            for（const auto&t:pending->_pending_block_state->trxs）
               未应用的_事务[t->signed_id]=t；
         }
         挂起.reset（）；
      }
   }


   bool应该执行运行时限制（）常量
      返回错误；
   }

   void set_action_merkle（）
      vector<digest_type>action_digests；
      action_digests.reserve（pending->u actions.size（））；
      for（const auto&a:挂起->操作）
         action_digests.emplace_back（a.digest（））；

      挂起->_Pending_Block_State->header.action_mroot=merkle（move（action_digests））；
   }

   void set_trx_merkle（）
      vector<digest_type>trx_digests；
      const auto&trxs=pending->_pending_block_state->block->transactions；
      trx_digests.reserve（trxs.size（））；
      用于（const auto&a:trxs）
         trx_digests.emplace_back（a.digest（））；

      挂起->_Pending_Block_State->header.transaction_mroot=merkle（move（trx_digests））；
   }


   void finalize_块（）
   {
      eos_assert（pending，block_validate_exception，“当没有pending block时，完成是无效的”）；
      尝试{


      /*
      ilog（“在$t处完成块$n（$id），由$p（$签名密钥）；计划版本：$v lib：$lib dtrxs：$n dtrxs n p”，
            （“N”，挂起->挂起块状态->块编号）
            （“id”，pending->_pending_block_state->header.id（））
            （“T”，挂起->挂起\块\状态->header.timestamp）
            （“P”，挂起->挂起\块\状态->header.producer）
            （“签名\密钥”，挂起->挂起\块\状态->块\签名\密钥）
            （“V”，挂起->挂起\块\状态->标题。计划\版本）
            （“lib”，挂起->挂起\块\状态->dpos \不可逆\块编号）
            （“ndtrx”，db.get_index<generated_transaction_multi_index，by_trx_id>（）.size（））
            （“NP”，挂起->挂起\块\状态->标题。新的\生产商）
            ；
      **/


//更新资源限制：
      resource_limits.process_account_limit_updates();
      const auto& chain_config = self.get_global_properties().configuration;
      uint32_t max_virtual_mult = 1000;
      uint64_t CPU_TARGET = EOS_PERCENT(chain_config.max_block_cpu_usage, chain_config.target_block_cpu_usage_pct);
      resource_limits.set_block_parameters(
         { CPU_TARGET, chain_config.max_block_cpu_usage, config::block_cpu_usage_average_window_ms / config::block_interval_ms, max_virtual_mult, {99, 100}, {1000, 999}},
         {EOS_PERCENT(chain_config.max_block_net_usage, chain_config.target_block_net_usage_pct), chain_config.max_block_net_usage, config::block_size_average_window_ms / config::block_interval_ms, max_virtual_mult, {99, 100}, {1000, 999}}
      );
      resource_limits.process_block_usage(pending->_pending_block_state->block_num);

      set_action_merkle();
      set_trx_merkle();

      auto p = pending->_pending_block_state;
      p->id = p->header.id();

      create_block_summary(p->id);

   } FC_CAPTURE_AND_RETHROW() }

   void update_producers_authority() {
      const auto& producers = pending->_pending_block_state->active_schedule.producers;

      auto update_permission = [&]( auto& permission, auto threshold ) {
         auto auth = authority( threshold, {}, {});
         for( auto& p : producers ) {
            auth.accounts.push_back({{p.producer_name, config::active_name}, 1});
         }

if( static_cast<authority>(permission.auth) != auth ) { //TODO:使用更有效的方法检查权限是否未更改
            db.modify(permission, [&]( auto& po ) {
               po.auth = auth;
            });
         }
      };

      uint32_t num_producers = producers.size();
      auto calculate_threshold = [=]( uint32_t numerator, uint32_t denominator ) {
         return ( (num_producers * numerator) / denominator ) + 1;
      };

      update_permission( authorization.get_permission({config::producers_account_name,
                                                       config::active_name}),
                         /*计算阈值（2，3）/*超过三分之二*/）；

      更新_权限（authorization.get_权限（config:：producers_account_name，
                                                       config：：多数_生产者_权限名称），
                         计算阈值（1，2）/*超过一半*/                        );


      update_permission( authorization.get_permission({config::producers_account_name,
                                                       config::minority_producers_permission_name}),
                         /*计算阈值（1，3）/*超过三分之一*/）；

      //TODO:添加测试
   }

   void create_block_summary（const block_id_type&id）
      auto block_num=block_header:：num_from_id（id）；
      auto sid=块编号&0xffff；
      db.modify（db.get<block_summary_object，by_id>（sid），[&]（block_summary_object&bso）
          bso.block_id=id；
      （}）；
   }


   void clear_expired_input_transactions（）
      //在重复数据消除列表中查找过期的事务，然后删除它们。
      auto&transaction_idx=db.get_mutable_index<transaction_multi_index>（）；
      const auto&dedupe_index=transaction_idx.indexs（）。get<by_expiration>（）；
      auto now=self.pending_block_time（）；
      （）dedupe_index.empty（））和&（现在>fc:：time_point（dedupe_index.begin（）->expiration）））；
         transaction_idx.remove（*dedupe_index.begin（））；
      }
   }

   bool sender_避免白名单_黑名单_强制执行（account_name sender）const_
      if（conf.sender_bypass_whiteblacklist.size（）>0&&
          （conf.sender_bypass_whiteblacklist.find（sender）！=conf.sender_bypass_whiteblacklist.end（））
      {
         回归真实；
      }

      返回错误；
   }

   void check_actor_list（const flat_set<account_name>&actors）const_
      if（actors.size（）==0）返回；

      if（conf.actor_whitelist.size（）>0）
         //如果参与者不是白名单的子集，则引发
         const auto&whitelist=conf.actor_whitelist；
         bool是_subset=true；

         //快速范围检查，然后强制检查参与者
         if（*actors.cbegin（）>=*whitelist.cbegin（）&&*actors.crbegin（）<=*whitelist.crbegin（））
            auto lower_bound=whitelist.cbegin（）；
            用于（const auto&actor:actors）
               下限=std：：下限（下限，whitelist.cend（），actor）；

               //如果找不到参与者，则这不是子集
               if（lower_bound==whitelist.cend（）*lower_bound！{演员）{
                  是_subset=false；
                  断裂；
               }

               //如果找到演员，我们保证其他演员不在白名单中
               //或将出现在定义为[下一个参与者，结束]的范围内
               下限=std:：next（下限）；
            }
         }否则{
            是_subset=false；
         }

         //帮助lambda延迟计算错误消息传递的参与者
         static auto generate_missing_actors=[]（const flat_set<account_name>和actors，const flat_set<account_name>和whitelist）->vector<account_name>
            vector<account\u name>排除；
            excluded.reserve（actors.size（））；
            设置差异（actors.begin（），actors.end（），
                            whitelist.begin（），whitelist.end（），
                            标准：：背面插入器（不包括））；
            退货除外；
         }；

         eos断言（is_subset，actor_whitelist_exception，
                     “交易中的授权参与者不在参与者白名单中：$参与者”，
                     （“actors”，生成丢失的\u actors（actors，whitelist））。
                   ；
      else if（conf.actor_blacklist.size（）>0）
         //如果参与者与黑名单相交，则引发
         const auto&blacklist=conf.actor_blacklist；
         bool intersects=false；

         //快速范围检查，然后强力检查参与者
         if（*actors.cbegin（）<=*blacklist.crbegin（）&&*actors.crbegin（）>=*blacklist.cbegin（））
            auto lower_bound=blacklist.cbegin（）；
            用于（const auto&actor:actors）
               下限=std：：下限（下限，blacklist.cend（），actor）；

               //如果黑名单中的下界在末尾，则所有其他参与者都保证
               //黑名单中不存在
               if（lower_bound==blacklist.cend（））
                  断裂；
               }

               //如果演员的下界是演员，那么我们有一个交集
               if（*lower_bound==actor）
                  相交=真；
                  断裂；
               }
            }
         }

         //帮助lambda延迟计算错误消息传递的参与者
         static auto generate_blacklisted_actors=[]（const flat_set<account_name>&actors，const flat_set<account_name>&blacklist）->vector<account_name>
            vector<account\u name>blacklisted；
            黑名单.reserve（actors.size（））；
            设置交集（actors.begin（），actors.end（），
                              blacklist.begin（），黑名单.end（），
                              标准：：背面插入器（黑名单）
                            ；
            返回黑名单；
         }；

         EOSYAsScript（！）相交，演员黑名单例外，
                     “交易中的授权参与者在参与者黑名单上：$参与者”，
                     （“演员”，生成“黑名单”演员（演员，黑名单）
                   ；
      }
   }

   作废支票合同清单（账户名称代码）常量
      if（conf.contract_whitelist.size（）>0）
         eos断言（conf.contract\u whitelist.find（code）！=conf.contract_whitelist.end（），
                     合同\白名单\例外，
                     “账户”$代码“不在合同白名单中”，“（代码”，代码）
                   ；
      else if（conf.contract_blacklist.size（）>0）
         eos_assert（conf.contract_blacklist.find（code）==conf.contract_blacklist.end（），
                     合同黑名单例外，
                     “账户”$代码“在合同黑名单上”，“（代码”，代码）
                   ；
      }
   }

   void check_action_list（account_name code，action_name action）const_
      if（conf.action_blacklist.size（）>0）
         eos_assert（conf.action_blacklist.find（std:：make_pair（code，action））==conf.action_blacklist.end（），
                     行动黑名单例外，
                     “操作”$代码：：$操作在操作黑名单上”，
                     （“代码”，代码）（“操作”，操作）
                   ；
      }
   }

   void check_key_list（const public_key_type&key）const_
      if（conf.key_blacklist.size（）>0）
         eos_assert（conf.key_blacklist.find（key）==conf.key_blacklist.end（），
                     关键黑名单例外，
                     “公钥”$key“在密钥黑名单上”，
                     （“密钥”，密钥）
                   ；
      }
   }

   /*
   bool should_check_tapos（）const_return true；

   void validate_tapos（const transaction&trx）const_
      如果（！）应该检查返回；

      const auto&tapos_block_summary=db.get<block_summary_object>（（uint16_t）trx.ref_block_num）；

      //验证tapos块摘要具有正确的ID前缀，并且此块的时间没有超过过期时间
      eos_assert（trx.verify_reference_block（tapos_block_summary.block_id），无效的_ref_block_exception，
                 “事务的引用块不匹配。此事务是否来自不同的fork？”，
                 （“Tapos_Summary”，Tapos_Block_Summary））；
   }
   **/



   /*
    *在每个块的开头，我们会通知系统合同，其中包含一个传入的事务。
    *上一个块的块头（当前是我们的头块）
    **/

   signed_transaction get_on_block_transaction()
   {
      action on_block_act;
      on_block_act.account = config::system_account_name;
      on_block_act.name = N(onblock);
      on_block_act.authorization = vector<permission_level>{{config::system_account_name, config::active_name}};
      on_block_act.data = fc::raw::pack(self.head_block_header());

      signed_transaction trx;
      trx.actions.emplace_back(std::move(on_block_act));
      trx.set_reference_block(self.head_block_id());
trx.expiration = self.pending_block_time() + fc::microseconds(999'999); //四舍五入到最接近的秒，以避免出现过期
      return trx;
   }

}; ///控制器执行

const resource_limits_manager&   controller::get_resource_limits_manager()const
{
   return my->resource_limits;
}
resource_limits_manager&         controller::get_mutable_resource_limits_manager()
{
   return my->resource_limits;
}

const authorization_manager&   controller::get_authorization_manager()const
{
   return my->authorization;
}
authorization_manager&         controller::get_mutable_authorization_manager()
{
   return my->authorization;
}

controller::controller( const controller::config& cfg )
:my( new controller_impl( cfg, *this ) )
{
}

controller::~controller() {
   my->abort_block();
//在这里关闭fork_db，因为它可以向这个控制器生成“不可逆”信号，
//如果读取模式=不可逆，我们将应用最新的不可逆块。
//为此，我们需要“my”作为指向有效控制器impl的有效指针。
   my->fork_db.close();
}

void controller::add_indices() {
   my->add_indices();
}

void controller::startup( std::function<bool()> shutdown, const snapshot_reader_ptr& snapshot ) {
   my->head = my->fork_db.head();
   if( !my->head ) {
      elog( "No head block in fork db, perhaps we need to replay" );
   }
   my->init(shutdown, snapshot);
}

const chainbase::database& controller::db()const { return my->db; }

chainbase::database& controller::mutable_db()const { return my->db; }

const fork_database& controller::fork_db()const { return my->fork_db; }


void controller::start_block( block_timestamp_type when, uint16_t confirm_block_count) {
   validate_db_available_size();
   my->start_block(when, confirm_block_count, block_status::incomplete, optional<block_id_type>() );
}

void controller::finalize_block() {
   validate_db_available_size();
   my->finalize_block();
}

void controller::sign_block( const std::function<signature_type( const digest_type& )>& signer_callback ) {
   my->sign_block( signer_callback );
}

void controller::commit_block() {
   validate_db_available_size();
   validate_reversible_available_size();
   my->commit_block(true);
}

void controller::abort_block() {
   my->abort_block();
}

boost::asio::thread_pool& controller::get_thread_pool() {
   return my->thread_pool;
}

std::future<block_state_ptr> controller::create_block_state_future( const signed_block_ptr& b ) {
   return my->create_block_state_future( b );
}

void controller::push_block( std::future<block_state_ptr>& block_state_future ) {
   validate_db_available_size();
   validate_reversible_available_size();
   my->push_block( block_state_future );
}

transaction_trace_ptr controller::push_transaction( const transaction_metadata_ptr& trx, fc::time_point deadline, uint32_t billed_cpu_time_us ) {
   validate_db_available_size();
   EOS_ASSERT( get_read_mode() != chain::db_read_mode::READ_ONLY, transaction_type_exception, "push transaction not allowed in read-only mode" );
   EOS_ASSERT( trx && !trx->implicit && !trx->scheduled, transaction_type_exception, "Implicit/Scheduled transaction not allowed" );
   return my->push_transaction(trx, deadline, billed_cpu_time_us, billed_cpu_time_us > 0 );
}

transaction_trace_ptr controller::push_scheduled_transaction( const transaction_id_type& trxid, fc::time_point deadline, uint32_t billed_cpu_time_us )
{
   validate_db_available_size();
   return my->push_scheduled_transaction( trxid, deadline, billed_cpu_time_us, billed_cpu_time_us > 0 );
}

const flat_set<account_name>& controller::get_actor_whitelist() const {
   return my->conf.actor_whitelist;
}
const flat_set<account_name>& controller::get_actor_blacklist() const {
   return my->conf.actor_blacklist;
}
const flat_set<account_name>& controller::get_contract_whitelist() const {
   return my->conf.contract_whitelist;
}
const flat_set<account_name>& controller::get_contract_blacklist() const {
   return my->conf.contract_blacklist;
}
const flat_set< pair<account_name, action_name> >& controller::get_action_blacklist() const {
   return my->conf.action_blacklist;
}
const flat_set<public_key_type>& controller::get_key_blacklist() const {
   return my->conf.key_blacklist;
}

void controller::set_actor_whitelist( const flat_set<account_name>& new_actor_whitelist ) {
   my->conf.actor_whitelist = new_actor_whitelist;
}
void controller::set_actor_blacklist( const flat_set<account_name>& new_actor_blacklist ) {
   my->conf.actor_blacklist = new_actor_blacklist;
}
void controller::set_contract_whitelist( const flat_set<account_name>& new_contract_whitelist ) {
   my->conf.contract_whitelist = new_contract_whitelist;
}
void controller::set_contract_blacklist( const flat_set<account_name>& new_contract_blacklist ) {
   my->conf.contract_blacklist = new_contract_blacklist;
}
void controller::set_action_blacklist( const flat_set< pair<account_name, action_name> >& new_action_blacklist ) {
   for (auto& act: new_action_blacklist) {
      EOS_ASSERT(act.first != account_name(), name_type_exception, "Action blacklist - contract name should not be empty");
      EOS_ASSERT(act.second != action_name(), action_type_exception, "Action blacklist - action name should not be empty");
   }
   my->conf.action_blacklist = new_action_blacklist;
}
void controller::set_key_blacklist( const flat_set<public_key_type>& new_key_blacklist ) {
   my->conf.key_blacklist = new_key_blacklist;
}

uint32_t controller::head_block_num()const {
   return my->head->block_num;
}
time_point controller::head_block_time()const {
   return my->head->header.timestamp;
}
block_id_type controller::head_block_id()const {
   return my->head->id;
}
account_name  controller::head_block_producer()const {
   return my->head->header.producer;
}
const block_header& controller::head_block_header()const {
   return my->head->header;
}
block_state_ptr controller::head_block_state()const {
   return my->head;
}

uint32_t controller::fork_db_head_block_num()const {
   return my->fork_db.head()->block_num;
}

block_id_type controller::fork_db_head_block_id()const {
   return my->fork_db.head()->id;
}

time_point controller::fork_db_head_block_time()const {
   return my->fork_db.head()->header.timestamp;
}

account_name  controller::fork_db_head_block_producer()const {
   return my->fork_db.head()->header.producer;
}

block_state_ptr controller::pending_block_state()const {
   if( my->pending ) return my->pending->_pending_block_state;
   return block_state_ptr();
}
time_point controller::pending_block_time()const {
   EOS_ASSERT( my->pending, block_validate_exception, "no pending block" );
   return my->pending->_pending_block_state->header.timestamp;
}

optional<block_id_type> controller::pending_producer_block_id()const {
   EOS_ASSERT( my->pending, block_validate_exception, "no pending block" );
   return my->pending->_producer_block_id;
}

uint32_t controller::last_irreversible_block_num() const {
   return std::max(std::max(my->head->bft_irreversible_blocknum, my->head->dpos_irreversible_blocknum), my->snapshot_head_block);
}

block_id_type controller::last_irreversible_block_id() const {
   auto lib_num = last_irreversible_block_num();
   const auto& tapos_block_summary = db().get<block_summary_object>((uint16_t)lib_num);

   if( block_header::num_from_id(tapos_block_summary.block_id) == lib_num )
      return tapos_block_summary.block_id;

   return fetch_block_by_number(lib_num)->id();

}

const dynamic_global_property_object& controller::get_dynamic_global_properties()const {
  return my->db.get<dynamic_global_property_object>();
}
const global_property_object& controller::get_global_properties()const {
  return my->db.get<global_property_object>();
}

signed_block_ptr controller::fetch_block_by_id( block_id_type id )const {
   auto state = my->fork_db.get_block(id);
   if( state && state->block ) return state->block;
   auto bptr = fetch_block_by_number( block_header::num_from_id(id) );
   if( bptr && bptr->id() == id ) return bptr;
   return signed_block_ptr();
}

signed_block_ptr controller::fetch_block_by_number( uint32_t block_num )const  { try {
   auto blk_state = my->fork_db.get_block_in_current_chain_by_num( block_num );
   if( blk_state && blk_state->block ) {
      return blk_state->block;
   }

   return my->blog.read_block_by_num(block_num);
} FC_CAPTURE_AND_RETHROW( (block_num) ) }

block_state_ptr controller::fetch_block_state_by_id( block_id_type id )const {
   auto state = my->fork_db.get_block(id);
   return state;
}

block_state_ptr controller::fetch_block_state_by_number( uint32_t block_num )const  { try {
   auto blk_state = my->fork_db.get_block_in_current_chain_by_num( block_num );
   return blk_state;
} FC_CAPTURE_AND_RETHROW( (block_num) ) }

block_id_type controller::get_block_id_for_num( uint32_t block_num )const { try {
   auto blk_state = my->fork_db.get_block_in_current_chain_by_num( block_num );
   if( blk_state ) {
      return blk_state->id;
   }

   auto signed_blk = my->blog.read_block_by_num(block_num);

   EOS_ASSERT( BOOST_LIKELY( signed_blk != nullptr ), unknown_block_exception,
               "Could not find block: ${block}", ("block", block_num) );

   return signed_blk->id();
} FC_CAPTURE_AND_RETHROW( (block_num) ) }

sha256 controller::calculate_integrity_hash()const { try {
   return my->calculate_integrity_hash();
} FC_LOG_AND_RETHROW() }

void controller::write_snapshot( const snapshot_writer_ptr& snapshot ) const {
   EOS_ASSERT( !my->pending, block_validate_exception, "cannot take a consistent snapshot with a pending block" );
   return my->add_to_snapshot(snapshot);
}

void controller::pop_block() {
   my->pop_block();
}

int64_t controller::set_proposed_producers( vector<producer_key> producers ) {
   const auto& gpo = get_global_properties();
   auto cur_block_num = head_block_num() + 1;

   if( gpo.proposed_schedule_block_num.valid() ) {
      if( *gpo.proposed_schedule_block_num != cur_block_num )
return -1; //上一个块中已存在建议的计划集，请等待它变为挂起

      if( std::equal( producers.begin(), producers.end(),
                      gpo.proposed_schedule.producers.begin(), gpo.proposed_schedule.producers.end() ) )
return -1; //提议的生产商时间表不变
   }

   producer_schedule_type sch;

   decltype(sch.producers.cend()) end;
   decltype(end)                  begin;

   if( my->pending->_pending_block_state->pending_schedule.producers.size() == 0 ) {
      const auto& active_sch = my->pending->_pending_block_state->active_schedule;
      begin = active_sch.producers.begin();
      end   = active_sch.producers.end();
      sch.version = active_sch.version + 1;
   } else {
      const auto& pending_sch = my->pending->_pending_block_state->pending_schedule;
      begin = pending_sch.producers.begin();
      end   = pending_sch.producers.end();
      sch.version = pending_sch.version + 1;
   }

   if( std::equal( producers.begin(), producers.end(), begin, end ) )
return -1; //生产计划不会改变

   sch.producers = std::move(producers);

   int64_t version = sch.version;

   my->db.modify( gpo, [&]( auto& gp ) {
      gp.proposed_schedule_block_num = cur_block_num;
      gp.proposed_schedule = std::move(sch);
   });
   return version;
}

const producer_schedule_type&    controller::active_producers()const {
   if ( !(my->pending) )
      return  my->head->active_schedule;
   return my->pending->_pending_block_state->active_schedule;
}

const producer_schedule_type&    controller::pending_producers()const {
   if ( !(my->pending) )
      return  my->head->pending_schedule;
   return my->pending->_pending_block_state->pending_schedule;
}

optional<producer_schedule_type> controller::proposed_producers()const {
   const auto& gpo = get_global_properties();
   if( !gpo.proposed_schedule_block_num.valid() )
      return optional<producer_schedule_type>();

   return gpo.proposed_schedule;
}

bool controller::light_validation_allowed(bool replay_opts_disabled_by_policy) const {
   if (!my->pending || my->in_trx_requiring_checks) {
      return false;
   }

   const auto pb_status = my->pending->_block_status;

//在挂起的不可逆或以前验证的块中，我们强制执行所有检查
   const bool consider_skipping_on_replay = (pb_status == block_status::irreversible || pb_status == block_status::validated) && !replay_opts_disabled_by_policy;

//或在有符号的块和光验证模式中
   const bool consider_skipping_on_validate = (pb_status == block_status::complete &&
         (my->conf.block_validation_mode == validation_mode::LIGHT || my->trusted_producer_light_validation));

   return consider_skipping_on_replay || consider_skipping_on_validate;
}


bool controller::skip_auth_check() const {
   return light_validation_allowed(my->conf.force_all_checks);
}

bool controller::skip_db_sessions( block_status bs ) const {
   bool consider_skipping = bs == block_status::irreversible;
   return consider_skipping
      && !my->conf.disable_replay_opts
      && !my->in_trx_requiring_checks;
}

bool controller::skip_db_sessions( ) const {
   if (my->pending) {
      return skip_db_sessions(my->pending->_block_status);
   } else {
      return false;
   }
}

bool controller::skip_trx_checks() const {
   return light_validation_allowed(my->conf.disable_replay_opts);
}

bool controller::contracts_console()const {
   return my->conf.contracts_console;
}

chain_id_type controller::get_chain_id()const {
   return my->chain_id;
}

db_read_mode controller::get_read_mode()const {
   return my->read_mode;
}

validation_mode controller::get_validation_mode()const {
   return my->conf.block_validation_mode;
}

const apply_handler* controller::find_apply_handler( account_name receiver, account_name scope, action_name act ) const
{
   auto native_handler_scope = my->apply_handlers.find( receiver );
   if( native_handler_scope != my->apply_handlers.end() ) {
      auto handler = native_handler_scope->second.find( make_pair( scope, act ) );
      if( handler != native_handler_scope->second.end() )
         return &handler->second;
   }
   return nullptr;
}
wasm_interface& controller::get_wasm_interface() {
   return my->wasmif;
}

const account_object& controller::get_account( account_name name )const
{ try {
   return my->db.get<account_object, by_name>(name);
} FC_CAPTURE_AND_RETHROW( (name) ) }

vector<transaction_metadata_ptr> controller::get_unapplied_transactions() const {
   vector<transaction_metadata_ptr> result;
   if ( my->read_mode == db_read_mode::SPECULATIVE ) {
      result.reserve(my->unapplied_transactions.size());
      for ( const auto& entry: my->unapplied_transactions ) {
         result.emplace_back(entry.second);
      }
   } else {
EOS_ASSERT( my->unapplied_transactions.empty(), transaction_exception, "not empty unapplied_transactions in non-speculative mode" ); //不应该发生
   }
   return result;
}

void controller::drop_unapplied_transaction(const transaction_metadata_ptr& trx) {
   my->unapplied_transactions.erase(trx->signed_id);
}

void controller::drop_all_unapplied_transactions() {
   my->unapplied_transactions.clear();
}

vector<transaction_id_type> controller::get_scheduled_transactions() const {
   const auto& idx = db().get_index<generated_transaction_multi_index,by_delay>();

   vector<transaction_id_type> result;

   static const size_t max_reserve = 64;
   result.reserve(std::min(idx.size(), max_reserve));

   auto itr = idx.begin();
   while( itr != idx.end() && itr->delay_until <= pending_block_time() ) {
      result.emplace_back(itr->trx_id);
      ++itr;
   }
   return result;
}

bool controller::sender_avoids_whitelist_blacklist_enforcement( account_name sender )const {
   return my->sender_avoids_whitelist_blacklist_enforcement( sender );
}

void controller::check_actor_list( const flat_set<account_name>& actors )const {
   my->check_actor_list( actors );
}

void controller::check_contract_list( account_name code )const {
   my->check_contract_list( code );
}

void controller::check_action_list( account_name code, action_name action )const {
   my->check_action_list( code, action );
}

void controller::check_key_list( const public_key_type& key )const {
   my->check_key_list( key );
}

bool controller::is_producing_block()const {
   if( !my->pending ) return false;

   return (my->pending->_block_status == block_status::incomplete);
}

bool controller::is_ram_billing_in_notify_allowed()const {
   return !is_producing_block() || my->conf.allow_ram_billing_in_notify;
}

void controller::validate_expiration( const transaction& trx )const { try {
   const auto& chain_configuration = get_global_properties().configuration;

   EOS_ASSERT( time_point(trx.expiration) >= pending_block_time(),
               expired_tx_exception,
               "transaction has expired, "
               "expiration is ${trx.expiration} and pending block time is ${pending_block_time}",
               ("trx.expiration",trx.expiration)("pending_block_time",pending_block_time()));
   EOS_ASSERT( time_point(trx.expiration) <= pending_block_time() + fc::seconds(chain_configuration.max_transaction_lifetime),
               tx_exp_too_far_exception,
               "Transaction expiration is too far in the future relative to the reference time of ${reference_time}, "
               "expiration is ${trx.expiration} and the maximum transaction lifetime is ${max_til_exp} seconds",
               ("trx.expiration",trx.expiration)("reference_time",pending_block_time())
               ("max_til_exp",chain_configuration.max_transaction_lifetime) );
} FC_CAPTURE_AND_RETHROW((trx)) }

void controller::validate_tapos( const transaction& trx )const { try {
   const auto& tapos_block_summary = db().get<block_summary_object>((uint16_t)trx.ref_block_num);

//验证TAPOS块摘要具有正确的ID前缀，并且此块的时间没有超过过期时间
   EOS_ASSERT(trx.verify_reference_block(tapos_block_summary.block_id), invalid_ref_block_exception,
              "Transaction's reference block did not match. Is this transaction from a different fork?",
              ("tapos_summary", tapos_block_summary));
} FC_CAPTURE_AND_RETHROW() }

void controller::validate_db_available_size() const {
   const auto free = db().get_segment_manager()->get_free_memory();
   const auto guard = my->conf.state_guard_size;
   EOS_ASSERT(free >= guard, database_guard_exception, "database free: ${f}, guard size: ${g}", ("f", free)("g",guard));
}

void controller::validate_reversible_available_size() const {
   const auto free = my->reversible_blocks.get_segment_manager()->get_free_memory();
   const auto guard = my->conf.reversible_guard_size;
   EOS_ASSERT(free >= guard, reversible_guard_exception, "reversible free: ${f}, guard size: ${g}", ("f", free)("g",guard));
}

bool controller::is_known_unexpired_transaction( const transaction_id_type& id) const {
   return db().find<transaction_object, by_trx_id>(id);
}

void controller::set_subjective_cpu_leeway(fc::microseconds leeway) {
   my->subjective_cpu_leeway = leeway;
}

void controller::add_resource_greylist(const account_name &name) {
   my->conf.resource_greylist.insert(name);
}

void controller::remove_resource_greylist(const account_name &name) {
   my->conf.resource_greylist.erase(name);
}

bool controller::is_resource_greylisted(const account_name &name) const {
   return my->conf.resource_greylist.find(name) !=  my->conf.resource_greylist.end();
}

const flat_set<account_name> &controller::get_resource_greylist() const {
   return  my->conf.resource_greylist;
}

} } ///EOSIO：链
