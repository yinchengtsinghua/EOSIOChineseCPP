
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
/*
 *本协议的目的是同步（并保持同步）两个
 *区块链使用非常简单的算法：
 *
 * 1。找到远程对等机知道的本地链上的最后一个块ID
 * 2。如果我们有下一个街区，把它寄给他们
 * 3。如果没有下一个数据块，请向他们发送最旧的未过期事务
 *
 *有几个输入事件：
 *
 * 1。本地链接受的新块
 * 2。局部链认为不可逆的块
 * 3。本地链接受的新块头
 * 4。本地链接受的交易
 * 5。从远程对等机接收的块
 * 6。从远程对等端接收的事务
 * 7。准备好下次写入的套接字
 *
 *每个会议负责维护以下内容：
 *
 * 1。我们目前所知的最好的连锁店中最近的一个街区
 *可以确定远程对等机拥有。
 *-这可能是对等端最后一个不可逆块
 *-对等端通知我们的lib之后的块ID
 *-我们已发送给远程对等机的块
 *-对等方发送给我们的块
 * 2。我们从远程对等机收到的块ID，以便
 *如果其中一个块被认为无效，我们可以断开对等的连接。
 *-一旦块可逆，我们就可以清除这些ID
 * 3。我们从远程对等机收到的事务，以便
 *我们不会向他们发送他们已经知道的信息。
 *-这包括作为块的一部分发送的事务
 *-在应用了一个
 *包括交易，因为我们知道控制者
 *不应再通知我们（他们会被欺骗）
 *
 ＊假设：
 * 1。我们发送给对等机的所有块都是有效的，将保留在
 *对等分叉数据库，直到它们不可逆或被替换为止。
 *采用不可逆的替代方法。
 * 2。我们不在乎对方在干什么，只要我们知道他们在干什么。
 *我们要发送的块之前的块。同伴会解决的
 *有了它的fork数据库，我们很有希望得出结论。
 * 3。对等方将以相同的基础向我们发送数据块
 *
 **/


#include <eosio/bnet_plugin/bnet_plugin.hpp>
#include <eosio/chain/controller.hpp>
#include <eosio/chain/trace.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>

#include <fc/io/json.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <eosio/chain/plugin_interface.hpp>

using tcp = boost::asio::ip::tcp;
namespace ws  = boost::beast::websocket;

namespace eosio {
   using namespace chain;

   static appbase::abstract_plugin& _bnet_plugin = app().register_plugin<bnet_plugin>();

} ///命名空间eosio

namespace fc {
   extern std::unordered_map<std::string,logger>& get_logger_map();
}

const fc::string logger_name("bnet_plugin");
fc::logger plugin_logger;
std::string peer_log_format;

#define peer_dlog( PEER, FORMAT, ... ) \
  FC_MULTILINE_MACRO_BEGIN \
   if( plugin_logger.is_enabled( fc::log_level::debug ) ) \
      plugin_logger.log( FC_LOG_MESSAGE( debug, peer_log_format + FORMAT, __VA_ARGS__ (PEER->get_logger_variant()) ) ); \
  FC_MULTILINE_MACRO_END

#define peer_ilog( PEER, FORMAT, ... ) \
  FC_MULTILINE_MACRO_BEGIN \
   if( plugin_logger.is_enabled( fc::log_level::info ) ) \
      plugin_logger.log( FC_LOG_MESSAGE( info, peer_log_format + FORMAT, __VA_ARGS__ (PEER->get_logger_variant()) ) ); \
  FC_MULTILINE_MACRO_END

#define peer_wlog( PEER, FORMAT, ... ) \
  FC_MULTILINE_MACRO_BEGIN \
   if( plugin_logger.is_enabled( fc::log_level::warn ) ) \
      plugin_logger.log( FC_LOG_MESSAGE( warn, peer_log_format + FORMAT, __VA_ARGS__ (PEER->get_logger_variant()) ) ); \
  FC_MULTILINE_MACRO_END

#define peer_elog( PEER, FORMAT, ... ) \
  FC_MULTILINE_MACRO_BEGIN \
   if( plugin_logger.is_enabled( fc::log_level::error ) ) \
      plugin_logger.log( FC_LOG_MESSAGE( error, peer_log_format + FORMAT, __VA_ARGS__ (PEER->get_logger_variant())) ); \
  FC_MULTILINE_MACRO_END


using eosio::public_key_type;
using eosio::chain_id_type;
using eosio::block_id_type;
using eosio::block_timestamp_type;
using std::string;
using eosio::sha256;
using eosio::signed_block_ptr;
using eosio::packed_transaction_ptr;
using std::vector;

struct hello {
   public_key_type               peer_id;
   string                        network_version;
   string                        agent;
   string                        protocol_version = "1.0.1";
   string                        user;
   string                        password;
   chain_id_type                 chain_id;
   bool                          request_transactions = false;
   uint32_t                      last_irr_block_num = 0;
   vector<block_id_type>         pending_block_ids;
};
FC_REFLECT( hello, (peer_id)(network_version)(user)(password)(agent)(protocol_version)(chain_id)(request_transactions)(last_irr_block_num)(pending_block_ids) )

struct hello_extension_irreversible_only {};

FC_REFLECT( hello_extension_irreversible_only, BOOST_PP_SEQ_NIL )

using hello_extension = fc::static_variant<hello_extension_irreversible_only>;

/*
 *此消息是在成功应用推测性事务后发送的。
 *并通知同伴不要发送此消息。
 **/

struct trx_notice {
vector<sha256>  signed_trx_id; ///<trx+sigs的哈希
};

FC_REFLECT( trx_notice, (signed_trx_id) )

/*
 *此消息在成功将事务添加到fork数据库后发送。
 *并通知远程对等机不需要发送此块。
 **/

struct block_notice {
   vector<block_id_type> block_ids;
};

FC_REFLECT( block_notice, (block_ids) );

struct ping {
   fc::time_point sent;
   fc::sha256     code;
uint32_t       lib; ///<最后一个不可逆块
};
FC_REFLECT( ping, (sent)(code)(lib) )

struct pong {
   fc::time_point sent;
   fc::sha256     code;
};
FC_REFLECT( pong, (sent)(code) )

using bnet_message = fc::static_variant<hello,
                                        trx_notice,
                                        block_notice,
                                        signed_block_ptr,
                                        packed_transaction_ptr,
                                        ping, pong
                                        >;


struct by_id;
struct by_num;
struct by_received;
struct by_expired;

namespace eosio {
  using namespace chain::plugin_interface;

  class bnet_plugin_impl;

  template <typename Strand>
  void verify_strand_in_this_thread(const Strand& strand, const char* func, int line) {
     if( !strand.running_in_this_thread() ) {
        elog( "wrong strand: ${f} : line ${n}, exiting", ("f", func)("n", line) );
        app().quit();
     }
  }

  /*
   *假定每个会话在其自身的流中运行，以便
   *操作可以并行执行。
   **/

  class session : public std::enable_shared_from_this<session>
  {
     public:
        enum session_state {
           hello_state,
           sending_state,
           idle_state
        };

        struct block_status {
           block_status( block_id_type i, bool kby_peer, bool rfrom_peer)
           {
              known_by_peer      = kby_peer;
              received_from_peer = rfrom_peer;
              id = i;
           }

bool known_by_peer  = false;         ///<我们向对等方发送了块或对等方向我们发送了通知
bool received_from_peer = false;     ///<peer向我们发送了此块并认为完整块有效
block_id_type id;                    ///<块ID；
//block_id_type prev；///<prev block id

//shared_ptr<vector<char>>block_msg；//<packed bnet_message for this block

           uint32_t block_num()const { return block_header::num_from_id(id); }
        };

        typedef boost::multi_index_container<block_status,
           indexed_by<
              ordered_unique< tag<by_id>,  member<block_status,block_id_type,&block_status::id> >,
              ordered_non_unique< tag<by_num>,  const_mem_fun<block_status,uint32_t,&block_status::block_num> >
           >
        > block_status_index;


        struct transaction_status {
           time_point                 received;
time_point                 expired; //距上次接受时间/5秒
           transaction_id_type        id;
           transaction_metadata_ptr   trx;

           void mark_known_by_peer() { received = fc::time_point::maximum(); trx.reset();  }
           bool known_by_peer()const { return received == fc::time_point::maximum(); }
        };

        typedef boost::multi_index_container<transaction_status,
           indexed_by<
              ordered_unique< tag<by_id>,  member<transaction_status,transaction_id_type,&transaction_status::id> >,
              ordered_non_unique< tag<by_received>,  member<transaction_status,time_point,&transaction_status::received> >,
              ordered_non_unique< tag<by_expired>,  member<transaction_status,time_point,&transaction_status::expired> >
           >
        > transaction_status_index;

        block_status_index        _block_status;
        transaction_status_index  _transaction_status;
const uint32_t            _max_block_status_range = 2048; //限制跟踪块的状态

        public_key_type    _local_peer_id;
        uint32_t           _local_lib             = 0;
        block_id_type      _local_lib_id;
        uint32_t           _local_head_block_num  = 0;
block_id_type      _local_head_block_id; ///在本地通道上接收的最后一个块ID


        public_key_type    _remote_peer_id;
        uint32_t           _remote_lib            = 0;
        block_id_type      _remote_lib_id;
        bool               _remote_request_trx    = false;
        bool               _remote_request_irreversible_only = false;

        uint32_t           _last_sent_block_num   = 0;
block_id_type      _last_sent_block_id; ///发送的最后一个块的ID
        bool               _recv_remote_hello     = false;
        bool               _sent_remote_hello     = false;


        fc::sha256         _current_code;
        fc::time_point     _last_recv_ping_time = fc::time_point::now();
        ping               _last_recv_ping;
        ping               _last_sent_ping;


        int                                                            _session_num = 0;
        session_state                                                  _state = hello_state;
        tcp::resolver                                                  _resolver;
        bnet_ptr                                                       _net_plugin;
        boost::asio::io_service&                                       _ios;
        unique_ptr<ws::stream<tcp::socket>>                            _ws;
        boost::asio::strand< boost::asio::io_context::executor_type>   _strand;
        boost::asio::io_service&                                       _app_ios;

        methods::get_block_by_number::method_type& _get_block_by_number;


        string                                                         _peer;
        string                                                         _remote_host;
        string                                                         _remote_port;

        vector<char>                                                  _out_buffer;
//boost：：beast：：缓冲区中有多个缓冲区；
        boost::beast::flat_buffer                                     _in_buffer;
        flat_set<block_id_type>                                       _block_header_notices;
        fc::optional<fc::variant_object>                              _logger_variant;


        int next_session_id()const {
           static std::atomic<int> session_count(0);
           return ++session_count;
        }

        /*
         *从服务器套接字接受创建会话
         **/

        explicit session( tcp::socket socket, bnet_ptr net_plug )
        :_resolver(socket.get_io_service()),
         _net_plugin( std::move(net_plug) ),
         _ios(socket.get_io_service()),
         _ws( new ws::stream<tcp::socket>(move(socket)) ),
         _strand(_ws->get_executor() ),
         _app_ios( app().get_io_service() ),
         _get_block_by_number( app().get_method<methods::get_block_by_number>() )
        {
            _session_num = next_session_id();
            set_socket_options();
            _ws->binary(true);
            wlog( "open session ${n}",("n",_session_num) );
        }


        /*
         *创建传出会话
         **/

        explicit session( boost::asio::io_context& ioc, bnet_ptr net_plug )
        :_resolver(ioc),
         _net_plugin( std::move(net_plug) ),
         _ios(ioc),
         _ws( new ws::stream<tcp::socket>(ioc) ),
         _strand( _ws->get_executor() ),
         _app_ios( app().get_io_service() ),
         _get_block_by_number( app().get_method<methods::get_block_by_number>() )
        {
           _session_num = next_session_id();
           _ws->binary(true);
           wlog( "open session ${n}",("n",_session_num) );
        }

        ~session();


        void set_socket_options() {
           try {
            /*在发送短消息时最小化延迟*/
            _ws->next_layer().set_option( boost::asio::ip::tcp::no_delay(true) );

            /*为了在发送大型1MB块时最小化延迟，发送缓冲区不必
             *等待“确认”，使其变大可能导致较小紧急事件的延迟更高。
             *消息。
             **/

            _ws->next_layer().set_option( boost::asio::socket_base::send_buffer_size( 1024*1024 ) );
            _ws->next_layer().set_option( boost::asio::socket_base::receive_buffer_size( 1024*1024 ) );
           } catch ( ... ) {
              elog( "uncaught exception on set socket options" );
           }
        }

        void run() {
           _ws->async_accept( boost::asio::bind_executor(
                             _strand,
                             std::bind( &session::on_accept,
                                        shared_from_this(),
                                        std::placeholders::_1) ) );
        }

        void run( const string& peer ) {
           auto c = peer.find(':');
           auto host = peer.substr( 0, c );
           auto port = peer.substr( c+1, peer.size() );

          _peer = peer;
          _remote_host = host;
          _remote_port = port;

          _resolver.async_resolve( _remote_host, _remote_port,
                                   boost::asio::bind_executor( _strand,
                                   std::bind( &session::on_resolve,
                                              shared_from_this(),
                                              std::placeholders::_1,
                                              std::placeholders::_2 ) ) );
        }

        void on_resolve( boost::system::error_code ec,
                         tcp::resolver::results_type results ) {
           if( ec ) return on_fail( ec, "resolve" );

           boost::asio::async_connect( _ws->next_layer(),
                                       results.begin(), results.end(),
                                       boost::asio::bind_executor( _strand,
                                          std::bind( &session::on_connect,
                                                     shared_from_this(),
                                                     std::placeholders::_1 ) ) );
        }

        void on_connect( boost::system::error_code ec ) {
           if( ec ) return on_fail( ec, "connect" );

           set_socket_options();

           _ws->async_handshake( _remote_host, "/",
                                boost::asio::bind_executor( _strand,
                                std::bind( &session::on_handshake,
                                           shared_from_this(),
                                           std::placeholders::_1 ) ) );
        }

        void on_handshake( boost::system::error_code ec ) {
           if( ec ) return on_fail( ec, "handshake" );

           do_hello();
           do_read();
        }

        /*
         *这将被称为“每次”接受发生的交易。
         *在推测块（可能有几个这样的块）上，当一个块
         *包含应用事务和/或切换分叉时的。
         *
         *我们将把它添加到事务状态表中，作为“Received Now”作为
         *发送给对等方的基础。当我们把它发送给对等方时“现在收到”
         *将被设置为无限未来，以将其标记为已发送，因此我们不会重新发送它。
         *再次接受时。
         *
         *每次交易被“接受”时，我们会将缓存时间延长
         *5秒后。每次应用程序块时，我们都清除所有已接受的
         *在没有新“接受”的情况下达到5秒的事务。
         **/

        void on_accepted_transaction( transaction_metadata_ptr t ) {
//国际劳工组织（“已接受的$t”，“（t”，t->id））；
           auto itr = _transaction_status.find( t->id );
           if( itr != _transaction_status.end() ) {
              if( !itr->known_by_peer() ) {
                 _transaction_status.modify( itr, [&]( auto& stat ) {
                    stat.expired = std::min<fc::time_point>( fc::time_point::now() + fc::seconds(5), t->packed_trx->expiration() );
                 });
              }
              return;
           }

           transaction_status stat;
           stat.received = fc::time_point::now();
           stat.expired  = stat.received + fc::seconds(5);
           stat.id       = t->id;
           stat.trx      = t;
           _transaction_status.insert( stat );

           maybe_send_next_message();
        }

        /*
         *从缓存中删除以前过期的所有事务
         **/

        void purge_transaction_cache() {
           auto& idx = _transaction_status.get<by_expired>();
           auto itr = idx.begin();
           auto now = fc::time_point::now();
           while( itr != idx.end() && itr->expired < now ) {
              idx.erase(itr);
              itr = idx.begin();
           }
        }

        /*
         *当我们的本地lib升级时，我们可以清除已知的历史，直到
         *LIB或远程对等机已知的最后一个块。
         **/

        void on_new_lib( block_state_ptr s ) {
           verify_strand_in_this_thread(_strand, __func__, __LINE__);
           _local_lib = s->block_num;
           _local_lib_id = s->id;

           auto purge_to = std::min( _local_lib, _last_sent_block_num );

           auto& idx = _block_status.get<by_num>();
           auto itr = idx.begin();
           while( itr != idx.end() && itr->block_num() < purge_to ) {
              idx.erase(itr);
              itr = idx.begin();
           }

           if( _remote_request_irreversible_only ) {
              auto bitr = _block_status.find(s->id);
              if ( bitr == _block_status.end() || !bitr->received_from_peer ) {
                 _block_header_notices.insert(s->id);
              }
           }

           maybe_send_next_message();
        }


        void on_bad_block( signed_block_ptr b ) {
           verify_strand_in_this_thread(_strand, __func__, __LINE__);
           try {
              auto id = b->id();
              auto itr = _block_status.find( id );
              if( itr == _block_status.end() ) return;
              if( itr->received_from_peer ) {
                 peer_elog(this, "bad signed_block_ptr : unknown" );
                 elog( "peer sent bad block #${b} ${i}, disconnect", ("b", b->block_num())("i",b->id())  );
                 _ws->next_layer().close();
              }
           } catch ( ... ) {
              elog( "uncaught exception" );
           }
        }

        void on_accepted_block_header( const block_state_ptr& s ) {
           verify_strand_in_this_thread(_strand, __func__, __LINE__);
//ilog（“接受的块头$n，”，“（n”，s->block_num））；
           const auto& id = s->id;

           if( fc::time_point::now() - s->block->timestamp  < fc::seconds(6) ) {
//ilog（“排队通知同行我们有这个块，希望他们不会发送给我们”）；
              auto itr = _block_status.find( id );
              if( !_remote_request_irreversible_only && ( itr == _block_status.end() || !itr->received_from_peer ) ) {
                 _block_header_notices.insert( id );
              }
              if( itr == _block_status.end() ) {
                 _block_status.insert( block_status(id, false, false) );
              }
           }
        }

        void on_accepted_block( const block_state_ptr& s ) {
           verify_strand_in_this_thread(_strand, __func__, __LINE__);
//idump（（_block_status.size（））（_transaction_status.size（））；
//ILOG（“已接受的数据块$n，”，“（n”，s->block_num））；

           const auto& id = s->id;

           _local_head_block_id = id;
           _local_head_block_num = block_header::num_from_id(id);

           if( _local_head_block_num < _last_sent_block_num ) {
              _last_sent_block_num = _local_lib;
              _last_sent_block_id  = _local_lib_id;
           }

           purge_transaction_cache();

           /*从缓存中清除所有事务，我将作为块的一部分发送它们
            *以后，除非同行告诉我他们已经有阻塞。
            **/

           for( const auto& receipt : s->block->transactions ) {
              if( receipt.trx.which() == 1 ) {
                 const auto& pt = receipt.trx.get<packed_transaction>();
                 const auto& tid = pt.id();
                 auto itr = _transaction_status.find( tid );
                 if( itr != _transaction_status.end() )
                    _transaction_status.erase(itr);
              }
           }

maybe_send_next_message(); ///如果空闲，尝试发送
        }


        template<typename L>
        void async_get_pending_block_ids( L&& callback ) {
///send peer从链插件中读取的头块状态
           _app_ios.post( [self = shared_from_this(),callback]{
              auto& control = app().get_plugin<chain_plugin>().chain();
              auto lib = control.last_irreversible_block_num();
              auto head = control.fork_db_head_block_id();
              auto head_num = block_header::num_from_id(head);


              std::vector<block_id_type> ids;
              if( lib > 0 ) {
                 ids.reserve((head_num-lib)+1);
                 for( auto i = lib; i <= head_num; ++i ) {
                   ids.emplace_back(control.get_block_id_for_num(i));
                 }
              }
              self->_ios.post( boost::asio::bind_executor(
                                    self->_strand,
                                    [callback,ids,lib](){
                                       callback(ids,lib);
                                    }
                                ));
           });
        }

        template<typename L>
        void async_get_block_num( uint32_t blocknum, L&& callback ) {
           _app_ios.post( [self = shared_from_this(), blocknum, callback]{
              auto& control = app().get_plugin<chain_plugin>().chain();
              signed_block_ptr sblockptr;
              try {
//ilog（“获取块$n”，“n”，“blocknum”）；
                 sblockptr = control.fetch_block_by_number( blocknum );
              } catch ( const fc::exception& e ) {
                 edump((e.to_detail_string()));
              }

              self->_ios.post( boost::asio::bind_executor(
                    self->_strand,
                    [callback,sblockptr](){
                       callback(sblockptr);
                    }
              ));
           });
        }

        void do_hello();


        void send( const bnet_message& msg ) { try {
           auto ps = fc::raw::pack_size(msg);
           _out_buffer.resize(ps);
           fc::datastream<char*> ds(_out_buffer.data(), ps);
           fc::raw::pack(ds, msg);
           send();
        } FC_LOG_AND_RETHROW() }

        template<class T>
        void send( const bnet_message& msg, const T& ex ) { try {
           auto ex_size = fc::raw::pack_size(ex);
           auto ps = fc::raw::pack_size(msg) + fc::raw::pack_size(unsigned_int(ex_size)) + ex_size;
           _out_buffer.resize(ps);
           fc::datastream<char*> ds(_out_buffer.data(), ps);
           fc::raw::pack( ds, msg );
           fc::raw::pack( ds, unsigned_int(ex_size) );
           fc::raw::pack( ds, ex );
           send();
        } FC_LOG_AND_RETHROW() }

        void send() { try {
           verify_strand_in_this_thread(_strand, __func__, __LINE__);

           _state = sending_state;
           _ws->async_write( boost::asio::buffer(_out_buffer),
                             boost::asio::bind_executor(
                                _strand,
                               std::bind( &session::on_write,
                                          shared_from_this(),
                                          std::placeholders::_1,
                                          std::placeholders::_2 ) ) );
        } FC_LOG_AND_RETHROW() }

        void mark_block_status( const block_id_type& id, bool known_by_peer, bool recv_from_peer ) {
           auto itr = _block_status.find(id);
           if( itr == _block_status.end() ) {
//优化以避免将块发送到已经知道它们的节点
//为了避免无限内存增长限制跟踪的数量
              const auto min_block_num = std::min( _local_lib, _last_sent_block_num );
              const auto max_block_num = min_block_num + _max_block_status_range;
              const auto block_num = block_header::num_from_id( id );
              if( block_num > min_block_num && block_num < max_block_num && _block_status.size() < _max_block_status_range )
                 _block_status.insert( block_status( id, known_by_peer, recv_from_peer ) );
           } else {
              _block_status.modify( itr, [&]( auto& item ) {
                 item.known_by_peer = known_by_peer;
                 if (recv_from_peer) item.received_from_peer = true;
              });
           }
        }

        /*
         *此方法将确定
         *输出队列，如果返回。否则它就决定了最好的
         *要发送的消息。
         **/

        void maybe_send_next_message() {
           verify_strand_in_this_thread(_strand, __func__, __LINE__);
if( _state == sending_state ) return; ///正在发送
if( _out_buffer.size() ) return; ///正在发送
           if( !_recv_remote_hello || !_sent_remote_hello ) return;

           clear_expired_trx();

           if( send_block_notice() ) return;
           if( send_pong() ) return;
           if( send_ping() ) return;

///我们不知道我们在哪里（等待接受块localhost）
           if( _local_head_block_id == block_id_type() ) return ;
           if( send_next_block() ) return;
           if( send_next_trx() ) return;
        }

        bool send_block_notice() {
           if( _block_header_notices.size() == 0 )
              return false;

           block_notice notice;
           notice.block_ids.reserve( _block_header_notices.size() );
           for( auto& id : _block_header_notices )
              notice.block_ids.emplace_back(id);
           send(notice);
           _block_header_notices.clear();
           return true;
        }

        bool send_pong() {
           if( _last_recv_ping.code == fc::sha256() )
              return false;

           send( pong{ fc::time_point::now(), _last_recv_ping.code } );
           _last_recv_ping.code = fc::sha256();
           return true;
        }

        bool send_ping() {
           auto delta_t = fc::time_point::now() - _last_sent_ping.sent;
           if( delta_t < fc::seconds(3) ) return false;

           if( _last_sent_ping.code == fc::sha256() ) {
              _last_sent_ping.sent = fc::time_point::now();
_last_sent_ping.code = fc::sha256::hash(_last_sent_ping.sent); ///TODO:使其更随机
              _last_sent_ping.lib  = _local_lib;
              send( _last_sent_ping );
           }

///我们希望对方每3秒钟给我们发送一次ping，所以如果我们还没有收到
///在过去6秒钟内，连接可能挂起。不幸的是，我们不能
///use the round-trip time of ping/pong to measure latency because during syncing the
///remote peer可能无法执行CPU密集型任务，这些任务会阻止其读取
///缓冲器。这个缓冲区被大约100个块填满，每个块需要1秒
///a总处理时间为10+秒。也就是说，同伴应该上来呼吸空气
///every.1 seconds，所以应该仍然能够每3秒发出一次ping。
//
//我们不想等待每个块的RTT，因为这也会减慢
//空块…
//
//if（fc:：time_point:：now（）-_last_recv_ping_time>fc:：seconds（6））
//再见（“最近6秒钟内没有对等机的ping….”）；
//}
           return true;
        }

        bool is_known_by_peer( block_id_type id ) {
           auto itr = _block_status.find(id);
           if( itr == _block_status.end() ) return false;
           return itr->known_by_peer;
        }

        void clear_expired_trx() {
           auto& idx = _transaction_status.get<by_expired>();
           auto itr = idx.begin();
           while( itr != idx.end() && itr->expired < fc::time_point::now() ) {
              idx.erase(itr);
              itr = idx.begin();
           }
        }

        bool send_next_trx() { try {
           if( !_remote_request_trx  ) return false;

           auto& idx = _transaction_status.get<by_received>();
           auto start = idx.begin();
           if( start == idx.end() || start->known_by_peer() )
              return false;


           auto ptrx_ptr = start->trx->packed_trx;

           idx.modify( start, [&]( auto& stat ) {
              stat.mark_known_by_peer();
           });

//wlog（“发送trx$id”，“id”，“start->id”）；
           send(ptrx_ptr);

           return true;

        } FC_LOG_AND_RETHROW() }

        void on_async_get_block( const signed_block_ptr& nextblock ) {
           verify_strand_in_this_thread(_strand, __func__, __LINE__);
            if( !nextblock)  {
               _state = idle_state;
               maybe_send_next_message();
               return;
            }

///如果有更改，下一个块不会链接到最后一个块
///block我们发送，本地链必须已切换分叉
            if( nextblock->previous != _last_sent_block_id && _last_sent_block_id != block_id_type() ) {
                if( !is_known_by_peer( nextblock->previous ) ) {
                  _last_sent_block_id  = _local_lib_id;
                  _last_sent_block_num = _local_lib;
                  _state = idle_state;
                  maybe_send_next_message();
                  return;
                }
            }

///此时，我们知道对等方可以链接此块

            auto next_id = nextblock->id();

///如果同伴已经知道这个块，很好，不需要
///发送，将其标记为“已发送”，然后继续。
            if( is_known_by_peer( next_id ) ) {
               _last_sent_block_id  = next_id;
               _last_sent_block_num = nextblock->block_num();

               _state = idle_state;
               maybe_send_next_message();
               return;
            }

            mark_block_status( next_id, true, false );

            _last_sent_block_id  = next_id;
            _last_sent_block_num = nextblock->block_num();

            send( nextblock );
            status( "sending block " + std::to_string( block_header::num_from_id(next_id) ) );

            if( nextblock->timestamp > (fc::time_point::now() - fc::seconds(5)) ) {
               mark_block_transactions_known_by_peer( nextblock );
            }
        }

        /*
         *在当前fork中的最后一个块之后发送下一个块
         *我们知道远程对等机知道。
         **/

        bool send_next_block() {

           if ( _remote_request_irreversible_only && _last_sent_block_id == _local_lib_id ) {
              return false;
           }

if( _last_sent_block_id == _local_head_block_id ) ///我们被抓到了
              return false;

///<设置发送状态，因为此回调可能导致发送消息
           _state = sending_state;
           async_get_block_num( _last_sent_block_num + 1,
                [self=shared_from_this()]( auto sblockptr ) {
                      self->on_async_get_block( sblockptr );
           });

           return true;
        }

        void on_fail( boost::system::error_code ec, const char* what ) {
           try {
              verify_strand_in_this_thread(_strand, __func__, __LINE__);
              elog( "${w}: ${m}", ("w", what)("m", ec.message() ) );
              _ws->next_layer().close();
           } catch ( ... ) {
              elog( "uncaught exception on close" );
           }
        }

        void on_accept( boost::system::error_code ec ) {
           if( ec ) {
              return on_fail( ec, "accept" );
           }

           do_hello();
           do_read();
        }

        void do_read() {
           _ws->async_read( _in_buffer,
                           boost::asio::bind_executor(
                              _strand,
                              std::bind( &session::on_read,
                                         shared_from_this(),
                                         std::placeholders::_1,
                                         std::placeholders::_2)));
        }

        void on_read( boost::system::error_code ec, std::size_t bytes_transferred ) {
           boost::ignore_unused(bytes_transferred);

           if( ec == ws::error::closed )
              return on_fail( ec, "close on read" );

           if( ec ) {
              return on_fail( ec, "read" );;
           }

           try {
              auto d = boost::asio::buffer_cast<char const*>(boost::beast::buffers_front(_in_buffer.data()));
              auto s = boost::asio::buffer_size(_in_buffer.data());
              fc::datastream<const char*> ds(d,s);

              bnet_message msg;
              fc::raw::unpack( ds, msg );
              on_message( msg, ds );
              _in_buffer.consume( ds.tellp() );

              wait_on_app();
              return;

           } catch ( ... ) {
              wlog( "close bad payload" );
           }
           try {
              _ws->close( boost::beast::websocket::close_code::bad_payload );
           } catch ( ... ) {
              elog( "uncaught exception on close" );
           }
        }

        /*如果我们在这里调用do-read，那么这个线程可能会在
         *主线程，相反，我们将事件发布到主线程，然后
         *准备好后发布新的读取事件。
         *
         *这也使“共享指针”在回调中保持活动状态，防止
         *连接被关闭。
         **/

        void wait_on_app() {
            app().get_io_service().post( 
                boost::asio::bind_executor( _strand, [self=shared_from_this()]{ self->do_read(); } )
            );
        }

        void on_message( const bnet_message& msg, fc::datastream<const char*>& ds ) {
           try {
              switch( msg.which() ) {
                 case bnet_message::tag<hello>::value:
                    on( msg.get<hello>(), ds );
                    break;
                 case bnet_message::tag<block_notice>::value:
                    on( msg.get<block_notice>() );
                    break;
                 case bnet_message::tag<signed_block_ptr>::value:
                    on( msg.get<signed_block_ptr>() );
                    break;
                 case bnet_message::tag<packed_transaction_ptr>::value:
                    on( msg.get<packed_transaction_ptr>() );
                    break;
                 case bnet_message::tag<ping>::value:
                    on( msg.get<ping>() );
                    break;
                 case bnet_message::tag<pong>::value:
                    on( msg.get<pong>() );
                    break;
                 default:
                    wlog( "bad message received" );
                    _ws->close( boost::beast::websocket::close_code::bad_payload );
                    return;
              }
              maybe_send_next_message();
           } catch( const fc::exception& e ) {
              elog( "${e}", ("e",e.to_detail_string()));
              _ws->close( boost::beast::websocket::close_code::bad_payload );
           }
        }

        void on( const block_notice& notice ) {
           peer_ilog(this, "received block_notice");
           for( const auto& id : notice.block_ids ) {
              status( "received notice " + std::to_string( block_header::num_from_id(id) ) );
              mark_block_status( id, true, false );
           }
        }

        void on( const hello& hi, fc::datastream<const char*>& ds );

        void on( const ping& p ) {
           peer_ilog(this, "received ping");
           _last_recv_ping = p;
           _remote_lib     = p.lib;
           _last_recv_ping_time = fc::time_point::now();
        }

        void on( const pong& p ) {
           peer_ilog(this, "received pong");
           if( p.code != _last_sent_ping.code ) {
              peer_elog(this, "bad ping : invalid pong code");
              return do_goodbye( "invalid pong code" );
           }
           _last_sent_ping.code = fc::sha256();
        }

        void do_goodbye( const string& reason ) {
           try {
              status( "goodbye - " + reason );
              _ws->next_layer().close();
           } catch ( ... ) {
              elog( "uncaught exception on close" );
           }
        }

        void check_for_redundant_connection();

        void on( const signed_block_ptr& b ) {
           peer_ilog(this, "received signed_block_ptr");
           if (!b) {
              peer_elog(this, "bad signed_block_ptr : null pointer");
              EOS_THROW(block_validate_exception, "bad block" );
           }
           status( "received block " + std::to_string(b->block_num()) );
//ilog（“recv block$n，”，“（n”，b->block_num（））；
           auto id = b->id();
           mark_block_status( id, true, true );

           app().get_channel<incoming::channels::block>().publish(b);

           mark_block_transactions_known_by_peer( b );
        }

        void mark_block_transactions_known_by_peer( const signed_block_ptr& b ) {
           for( const auto& receipt : b->transactions ) {
              if( receipt.trx.which() == 1 ) {
                 const auto& pt = receipt.trx.get<packed_transaction>();
                 const auto& id = pt.id();
                 mark_transaction_known_by_peer(id);
              }
           }
        }

        /*
         *@如果本地主机知道trx，则返回true；如果此主机是新的，则返回false
         **/

        bool mark_transaction_known_by_peer( const transaction_id_type& id ) {
           auto itr = _transaction_status.find( id );
           if( itr != _transaction_status.end() ) {
              _transaction_status.modify( itr, [&]( auto& stat ) {
                 stat.mark_known_by_peer();
              });
              return true;
           } else {
              transaction_status stat;
              stat.id = id;
              stat.mark_known_by_peer();
              stat.expired = fc::time_point::now()+fc::seconds(5);
              _transaction_status.insert(stat);
           }
           return false;
        }

        void on( const packed_transaction_ptr& p );

        void on_write( boost::system::error_code ec, std::size_t bytes_transferred ) {
           boost::ignore_unused(bytes_transferred);
           verify_strand_in_this_thread(_strand, __func__, __LINE__);
           if( ec ) {
              _ws->next_layer().close();
              return on_fail( ec, "write" );
           }
           _state = idle_state;
           _out_buffer.resize(0);
           maybe_send_next_message();
        }

        void status( const string& msg ) {
//ilog（“$remote_peer：$msg”，（“remote_peer”，fc:：variant（_remote_peer）.as_string（）.substr（3,5））（“msg”，msg））；
        }

        const fc::variant_object& get_logger_variant()  {
           if (!_logger_variant) {
              boost::system::error_code ec;
              auto rep = _ws->lowest_layer().remote_endpoint(ec);
              string ip = ec ? "<unknown>" : rep.address().to_string();
              string port = ec ? "<unknown>" : std::to_string(rep.port());

              auto lep = _ws->lowest_layer().local_endpoint(ec);
              string lip = ec ? "<unknown>" : lep.address().to_string();
              string lport = ec ? "<unknown>" : std::to_string(lep.port());

              _logger_variant.emplace(fc::mutable_variant_object()
                 ("_name", _peer)
                 ("_id", _remote_peer_id)
                 ("_ip", ip)
                 ("_port", port)
                 ("_lip", lip)
                 ("_lport", lport)
              );
           }
           return *_logger_variant;
        }
  };


  /*
   *接受传入连接并启动会话
   **/

  class listener : public std::enable_shared_from_this<listener> {
     private:
        tcp::acceptor         _acceptor;
        tcp::socket           _socket;
        bnet_ptr              _net_plugin;

     public:
        listener( boost::asio::io_context& ioc, tcp::endpoint endpoint, bnet_ptr np  )
        :_acceptor(ioc), _socket(ioc), _net_plugin(std::move(np))
        {
           boost::system::error_code ec;

           _acceptor.open( endpoint.protocol(), ec );
           if( ec ) { on_fail( ec, "open" ); return; }

           _acceptor.set_option( boost::asio::socket_base::reuse_address(true) );

           _acceptor.bind( endpoint, ec );
           if( ec ) { on_fail( ec, "bind" ); return; }

           _acceptor.listen( boost::asio::socket_base::max_listen_connections, ec );
           if( ec ) on_fail( ec, "listen" );
        }

        void run() {
           EOS_ASSERT( _acceptor.is_open(), plugin_exception, "unable top open listen socket" );
           do_accept();
        }

        void do_accept() {
           _acceptor.async_accept( _socket, [self=shared_from_this()]( auto ec ){ self->on_accept(ec); } );
        }

        void on_fail( boost::system::error_code ec, const char* what ) {
           elog( "${w}: ${m}", ("w", what)("m", ec.message() ) );
        }

        void on_accept( boost::system::error_code ec );
  };


   class bnet_plugin_impl : public std::enable_shared_from_this<bnet_plugin_impl> {
      public:
         bnet_plugin_impl() = default;

const private_key_type  _peer_pk = fc::crypto::private_key::generate(); ///一次性随机键来标识此进程
         public_key_type         _peer_id = _peer_pk.get_public_key();
         string                                                 _bnet_endpoint_address = "0.0.0.0";
         uint16_t                                               _bnet_endpoint_port = 4321;
         bool                                                   _request_trx = true;
         bool                                                   _follow_irreversible = false;

std::vector<std::string>                               _connect_to_peers; ///要连接到的对等方列表
         std::vector<std::thread>                               _socket_threads;
         int32_t                                                _num_threads = 1;

std::unique_ptr<boost::asio::io_context>               _ioc; //由bnet插件的共享ptr保护的生存期impl
         std::shared_ptr<listener>                              _listener;
std::shared_ptr<boost::asio::deadline_timer>           _timer;    //仅访问应用程序IO服务
std::map<const session*, std::weak_ptr<session> >      _sessions; //仅访问应用程序IO服务

         channels::irreversible_block::channel_type::handle     _on_irb_handle;
         channels::accepted_block::channel_type::handle         _on_accepted_block_handle;
         channels::accepted_block_header::channel_type::handle  _on_accepted_block_header_handle;
         channels::rejected_block::channel_type::handle         _on_bad_block_handle;
         channels::accepted_transaction::channel_type::handle   _on_appled_trx_handle;

         void async_add_session( std::weak_ptr<session> wp ) {
            app().get_io_service().post( [wp,this]{
               if( auto l = wp.lock() ) {
                  _sessions[l.get()] = wp;
               }
            });
         }

         void on_session_close( const session* s ) {
            verify_strand_in_this_thread(app().get_io_service().get_executor(), __func__, __LINE__);
            auto itr = _sessions.find(s);
            if( _sessions.end() != itr )
               _sessions.erase(itr);
         }

         template<typename Call>
         void for_each_session( Call callback ) {
            app().get_io_service().post([this, callback = callback] {
               for (const auto& item : _sessions) {
                  if (auto ses = item.second.lock()) {
                     ses->_ios.post(boost::asio::bind_executor(
                           ses->_strand,
                           [ses, cb = callback]() { cb(ses); }
                     ));
                  }
               }
            });
         }

         void on_accepted_transaction( transaction_metadata_ptr trx ) {
            if( trx->implicit || trx->scheduled ) return;
            for_each_session( [trx]( auto ses ){ ses->on_accepted_transaction( trx ); } );
         }

         /*
          *通知新的不可逆块的所有活动连接，以便
          *可以清除其块缓存
          **/

         void on_irreversible_block( block_state_ptr s ) {
            for_each_session( [s]( auto ses ){ ses->on_new_lib( s ); } );
         }

         /*
          *通知新接受块的所有活动连接，以便
          *他们可以转播。此方法还预打包块
          *作为打包的bnet_消息，因此连接可以简单地中继
          *打开。
          **/

         void on_accepted_block( block_state_ptr s ) {
_ioc->post( [s,this] { ///post将其发布到线程池，因为打包可能很密集。
               for_each_session( [s]( auto ses ){ ses->on_accepted_block( s ); } );
            });
         }

         void on_accepted_block_header( block_state_ptr s ) {
_ioc->post( [s,this] { ///post将其发布到线程池，因为打包可能很密集。
               for_each_session( [s]( auto ses ){ ses->on_accepted_block_header( s ); } );
            });
         }

         /*
          *我们收到了一个坏块
          * 1。没有链接到已知链
          * 2。违反共识规则
          *
          *向我们发送此块的任何对等机（未注意到）
          *应该断开连接，因为它们在客观上是不好的
          **/

         void on_bad_block( signed_block_ptr s ) {
            for_each_session( [s]( auto ses ) { ses->on_bad_block(s); } );
         };

         void on_reconnect_peers() {
             verify_strand_in_this_thread(app().get_io_service().get_executor(), __func__, __LINE__);
             for( const auto& peer : _connect_to_peers ) {
                bool found = false;
                for( const auto& con : _sessions ) {
                   auto ses = con.second.lock();
                   if( ses && (ses->_peer == peer) ) {
                      found = true;
                      break;
                   }
                }

                if( !found ) {
                   wlog( "attempt to connect to ${p}", ("p",peer) );
                   auto s = std::make_shared<session>( *_ioc, shared_from_this() );
                   s->_local_peer_id = _peer_id;
                   _sessions[s.get()] = s;
                   s->run( peer );
                }
             }

             start_reconnect_timer();
         }


         void start_reconnect_timer() {
///添加一些随机延迟，这样我的所有同事都不会尝试重新连接到我。
///同时关闭..
            _timer->expires_from_now( boost::posix_time::microseconds( 1000000*(10+rand()%5) ) );
            _timer->async_wait([=](const boost::system::error_code& ec) {
                if( ec ) { return; }
                on_reconnect_peers();
            });
         }
   };


   void listener::on_accept( boost::system::error_code ec ) {
     if( ec ) {
        if( ec == boost::system::errc::too_many_files_open )
           do_accept();
        return;
     }
     std::shared_ptr<session> newsession;
     try {
        newsession = std::make_shared<session>( move( _socket ), _net_plugin );
     }
     catch( std::exception& e ) {
//创建会话会创建一个std:：random_设备的实例，该设备可能会打开/dev/urandom
//例如。不幸的是，唯一定义的错误是std:：exception派生
        _socket.close();
     }
     if( newsession ) {
        _net_plugin->async_add_session( newsession );
        newsession->_local_peer_id = _net_plugin->_peer_id;
        newsession->run();
     }
     do_accept();
   }


   bnet_plugin::bnet_plugin()
   :my(std::make_shared<bnet_plugin_impl>()) {
   }

   bnet_plugin::~bnet_plugin() {
   }

   void bnet_plugin::set_program_options(options_description& cli, options_description& cfg) {
      cfg.add_options()
         ("bnet-endpoint", bpo::value<string>()->default_value("0.0.0.0:4321"), "the endpoint upon which to listen for incoming connections" )
         ("bnet-follow-irreversible", bpo::value<bool>()->default_value(false), "this peer will request only irreversible blocks from other nodes" )
         ("bnet-threads", bpo::value<uint32_t>(), "the number of threads to use to process network messages" )
         ("bnet-connect", bpo::value<vector<string>>()->composing(), "remote endpoint of other node to connect to; Use multiple bnet-connect options as needed to compose a network" )
         ("bnet-no-trx", bpo::bool_switch()->default_value(false), "this peer will request no pending transactions from other nodes" )
         ("bnet-peer-log-format", bpo::value<string>()->default_value( "[\"${_name}\" ${_ip}:${_port}]" ),
           "The string used to format peers when logging messages about them.  Variables are escaped with ${<variable name>}.\n"
           "Available Variables:\n"
           "   _name  \tself-reported name\n\n"
           "   _id    \tself-reported ID (Public Key)\n\n"
           "   _ip    \tremote IP address of peer\n\n"
           "   _port  \tremote port number of peer\n\n"
           "   _lip   \tlocal IP address connected to peer\n\n"
           "   _lport \tlocal port number connected to peer\n\n")
         ;
   }

   void bnet_plugin::plugin_initialize(const variables_map& options) {
      ilog( "Initialize bnet plugin" );

      try {
         peer_log_format = options.at( "bnet-peer-log-format" ).as<string>();

         if( options.count( "bnet-endpoint" )) {
            auto ip_port = options.at( "bnet-endpoint" ).as<string>();

//自动主机=boost:：asio:：ip:：host_name（ip_port）；
            auto port = ip_port.substr( ip_port.find( ':' ) + 1, ip_port.size());
            auto host = ip_port.substr( 0, ip_port.find( ':' ));
            my->_bnet_endpoint_address = host;
            my->_bnet_endpoint_port = std::stoi( port );
            idump((ip_port)( host )( port )( my->_follow_irreversible ));
         }
         if( options.count( "bnet-follow-irreversible" )) {
            my->_follow_irreversible = options.at( "bnet-follow-irreversible" ).as<bool>();
         }


         if( options.count( "bnet-connect" )) {
            my->_connect_to_peers = options.at( "bnet-connect" ).as<vector<string>>();
         }
         if( options.count( "bnet-threads" )) {
            my->_num_threads = options.at( "bnet-threads" ).as<uint32_t>();
            if( my->_num_threads > 8 )
               my->_num_threads = 8;
         }
         my->_request_trx = !options.at( "bnet-no-trx" ).as<bool>();

      } FC_LOG_AND_RETHROW()
   }

   void bnet_plugin::plugin_startup() {
      if(fc::get_logger_map().find(logger_name) != fc::get_logger_map().end())
         plugin_logger = fc::get_logger_map()[logger_name];

      wlog( "bnet startup " );

      auto& chain = app().get_plugin<chain_plugin>().chain();
      FC_ASSERT ( chain.get_read_mode() != chain::db_read_mode::IRREVERSIBLE, "bnet is not compatible with \"irreversible\" read_mode");

      my->_on_appled_trx_handle = app().get_channel<channels::accepted_transaction>()
                                .subscribe( [this]( transaction_metadata_ptr t ){
                                       my->on_accepted_transaction(t);
                                });

      my->_on_irb_handle = app().get_channel<channels::irreversible_block>()
                                .subscribe( [this]( block_state_ptr s ){
                                       my->on_irreversible_block(s);
                                });

      my->_on_accepted_block_handle = app().get_channel<channels::accepted_block>()
                                         .subscribe( [this]( block_state_ptr s ){
                                                my->on_accepted_block(s);
                                         });

      my->_on_accepted_block_header_handle = app().get_channel<channels::accepted_block_header>()
                                         .subscribe( [this]( block_state_ptr s ){
                                                my->on_accepted_block_header(s);
                                         });

      my->_on_bad_block_handle = app().get_channel<channels::rejected_block>()
                                .subscribe( [this]( signed_block_ptr b ){
                                       my->on_bad_block(b);
                                });


      if( app().get_plugin<chain_plugin>().chain().get_read_mode() == chain::db_read_mode::READ_ONLY ) {
         if (my->_request_trx) {
            my->_request_trx = false;
            ilog( "forced bnet-no-trx to true since in read-only mode" );
         }
      }

      const auto address = boost::asio::ip::make_address( my->_bnet_endpoint_address );
      my->_ioc.reset( new boost::asio::io_context{my->_num_threads} );


      auto& ioc = *my->_ioc;
      my->_timer = std::make_shared<boost::asio::deadline_timer>( app().get_io_service() );

      my->start_reconnect_timer();

      my->_listener = std::make_shared<listener>( ioc,
                                                  tcp::endpoint{ address, my->_bnet_endpoint_port },
                                                  my );
      my->_listener->run();

      my->_socket_threads.reserve( my->_num_threads );
      for( auto i = 0; i < my->_num_threads; ++i ) {
         my->_socket_threads.emplace_back( [&ioc]{ wlog( "start thread" ); ioc.run(); wlog( "end thread" ); } );
      }

      for( const auto& peer : my->_connect_to_peers ) {
         auto s = std::make_shared<session>( ioc, my );
         s->_local_peer_id = my->_peer_id;
         my->_sessions[s.get()] = s;
         s->run( peer );
      }
   }

   void bnet_plugin::plugin_shutdown() {
      try {
         my->_timer->cancel();
         my->_timer.reset();
      } catch ( ... ) {
         elog( "exception thrown on timer shutdown" );
      }

///关闭所有线程并关闭所有连接

      my->for_each_session([](auto ses){
         ses->do_goodbye( "shutting down" );
      });

      my->_listener.reset();
      my->_ioc->stop();

      wlog( "joining bnet threads" );
      for( auto& t : my->_socket_threads ) {
         t.join();
      }
      wlog( "done joining threads" );

      my->for_each_session([](auto ses){
         EOS_ASSERT( false, plugin_exception, "session ${ses} still active", ("ses", ses->_session_num) );
      });

//bnet插件impl的共享ptr保护IOC的生命周期
   }


   session::~session() {
     wlog( "close session ${n}",("n",_session_num) );
     std::weak_ptr<bnet_plugin_impl> netp = _net_plugin;
     _app_ios.post( [netp,ses=this]{
        if( auto net = netp.lock() )
           net->on_session_close(ses);
     });
   }

   void session::do_hello() {
///TODO:找到更有效的方法在分叉事件中移动大量ID数组。
      async_get_pending_block_ids( [self = shared_from_this() ]( const vector<block_id_type>& ids, uint32_t lib ){
          hello hello_msg;
          hello_msg.peer_id = self->_local_peer_id;
          hello_msg.last_irr_block_num = lib;
          hello_msg.pending_block_ids  = ids;
          hello_msg.request_transactions = self->_net_plugin->_request_trx;
hello_msg.chain_id = app().get_plugin<chain_plugin>().get_chain_id(); //托多：快点解决。也许需要更好的解决方案。

          self->_local_lib = lib;
          if ( self->_net_plugin->_follow_irreversible ) {
             self->send( hello_msg, hello_extension(hello_extension_irreversible_only()) );
          } else {
             self->send( hello_msg );
          }
          self->_sent_remote_hello = true;
      });
   }

   void session::check_for_redundant_connection() {
     app().get_io_service().post( [self=shared_from_this()]{
       self->_net_plugin->for_each_session( [self]( auto ses ){
         if( ses != self && ses->_remote_peer_id == self->_remote_peer_id ) {
           self->do_goodbye( "redundant connection" );
         }
       });
     });
   }

   void session::on( const hello& hi, fc::datastream<const char*>& ds ) {
      peer_ilog(this, "received hello");
      _recv_remote_hello     = true;

if( hi.chain_id != app().get_plugin<chain_plugin>().get_chain_id() ) { //托多：快点解决。也许需要更好的解决方案。
         peer_elog(this, "bad hello : wrong chain id");
         return do_goodbye( "disconnecting due to wrong chain id" );
      }

      if( hi.peer_id == _local_peer_id ) {
         return do_goodbye( "connected to self" );
      }

      if ( _net_plugin->_follow_irreversible && hi.protocol_version <= "1.0.0") {
         return do_goodbye( "need newer protocol version that supports sending only irreversible blocks" );
      }

      if ( hi.protocol_version >= "1.0.1" ) {
//可选扩展名
         while ( 0 < ds.remaining() ) {
            unsigned_int size;
fc::raw::unpack( ds, size ); //下一个扩展名大小
            auto ex_start = ds.pos();
            fc::datastream<const char*> dsw( ex_start, size );
            unsigned_int wich;
            fc::raw::unpack( dsw, wich );
            hello_extension ex;
if ( wich < ex.count() ) { //知拓
fc::datastream<const char*> dsx( ex_start, size ); //解包需要再次读取静态变量标记
               fc::raw::unpack( dsx, ex );
               if ( ex.which() == hello_extension::tag<hello_extension_irreversible_only>::value ) {
                  _remote_request_irreversible_only = true;
               }
            } else {
//不支持的扩展名，我们只是忽略它
//另一方知道我们的协议版本，即它知道我们支持哪些扩展
//所以，一些扩展是至关重要的，另一方将关闭连接
            }
ds.skip(size); //移到下一个扩展名
         }
      }

      _last_sent_block_num   = hi.last_irr_block_num;
      _remote_request_trx    = hi.request_transactions;
      _remote_peer_id        = hi.peer_id;
      _remote_lib            = hi.last_irr_block_num;

      for( const auto& id : hi.pending_block_ids )
         mark_block_status( id, true, false );

      check_for_redundant_connection();

   }

   void session::on( const packed_transaction_ptr& p ) {
      peer_ilog(this, "received packed_transaction_ptr");
      if (!p) {
        peer_elog(this, "bad packed_transaction_ptr : null pointer");
        EOS_THROW(transaction_exception, "bad transaction");
      }
      if( !_net_plugin->_request_trx )
        return;

//ilog（“recv trx$n”，“n”，id））；
      if( p->expiration() < fc::time_point::now() ) return;

      const auto& id = p->id();

      if( mark_transaction_known_by_peer( id ) )
        return;

      auto ptr = std::make_shared<transaction_metadata>(p);

      app().get_channel<incoming::channels::transaction>().publish(ptr);
   }
} ///命名空间eosio
