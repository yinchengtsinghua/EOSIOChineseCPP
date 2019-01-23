
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include <websocketpp/transport/base/endpoint.hpp>
#include <websocketpp/transport/asio/connection.hpp>
#include <websocketpp/transport/asio/security/none.hpp>

#include <websocketpp/uri.hpp>
#include <websocketpp/logger/levels.hpp>

#include <websocketpp/common/functional.hpp>

#include <sstream>
#include <string>

namespace websocketpp {
namespace transport {
namespace asio {

namespace basic_socket {

class local_connection : public lib::enable_shared_from_this<local_connection> {
public:
///此连接套接字组件的类型
    typedef local_connection type;
///指向此连接套接字组件的共享指针的类型
    typedef lib::shared_ptr<type> ptr;

///指向正在使用的asio io_服务的指针类型
    typedef lib::asio::io_service* io_service_ptr;
///指向正在使用的asio io_服务流的指针类型
    typedef lib::shared_ptr<lib::asio::io_service::strand> strand_ptr;
///正在使用的ASIO套接字的类型
    typedef lib::asio::local::stream_protocol::socket socket_type;
///指向正在使用的套接字的共享指针的类型。
    typedef lib::shared_ptr<socket_type> socket_ptr;

    explicit local_connection() : m_state(UNINITIALIZED) {
    }

    ptr get_shared() {
        return shared_from_this();
    }

    bool is_secure() const {
        return false;
    }

    lib::asio::local::stream_protocol::socket & get_socket() {
        return *m_socket;
    }

    lib::asio::local::stream_protocol::socket & get_next_layer() {
        return *m_socket;
    }

    lib::asio::local::stream_protocol::socket & get_raw_socket() {
        return *m_socket;
    }

    std::string get_remote_endpoint(lib::error_code & ec) const {
        return "UNIX Socket Endpoint";
    }

   void pre_init(init_handler callback) {
      if (m_state != READY) {
         callback(socket::make_error_code(socket::error::invalid_state));
         return;
      }

      m_state = READING;

      callback(lib::error_code());
   }

   void post_init(init_handler callback) {
        callback(lib::error_code());
   }
protected:
    lib::error_code init_asio (io_service_ptr service, strand_ptr, bool)
    {
        if (m_state != UNINITIALIZED) {
            return socket::make_error_code(socket::error::invalid_state);
        }

        m_socket = lib::make_shared<lib::asio::local::stream_protocol::socket>(
            lib::ref(*service));

        m_state = READY;

        return lib::error_code();
    }

    void set_handle(connection_hdl hdl) {
        m_hdl = hdl;
    }

    lib::asio::error_code cancel_socket() {
        lib::asio::error_code ec;
        m_socket->cancel(ec);
        return ec;
    }

    void async_shutdown(socket::shutdown_handler h) {
        lib::asio::error_code ec;
        m_socket->shutdown(lib::asio::ip::tcp::socket::shutdown_both, ec);
        h(ec);
    }

    lib::error_code get_ec() const {
        return lib::error_code();
    }

    template <typename ErrorCodeType>
    lib::error_code translate_ec(ErrorCodeType) {
        return make_error_code(transport::error::pass_through);
    }
    
    lib::error_code translate_ec(lib::error_code ec) {
        return ec;
    }
private:
    enum state {
        UNINITIALIZED = 0,
        READY = 1,
        READING = 2
    };

    socket_ptr          m_socket;
    state               m_state;

    connection_hdl      m_hdl;
    socket_init_handler m_socket_init_handler;
};

class local_endpoint {
public:
///此终结点套接字组件的类型
    typedef local_endpoint type;

///相应连接套接字组件的类型
    typedef local_connection socket_con_type;
///指向相应连接套接字的共享指针的类型
//组件。
    typedef socket_con_type::ptr socket_con_ptr;

    explicit local_endpoint() {}

    bool is_secure() const {
        return false;
    }
};
}

///asio-based端点传输组件
/*
 *transport:：asio:：endpoint使用
 * Asio。
 **/

template <typename config>
class local_endpoint : public config::socket_type {
public:
///此终结点传输组件的类型
    typedef local_endpoint<config> type;

///并发策略的类型
    typedef typename config::concurrency_type concurrency_type;
///套接字策略的类型
    typedef typename config::socket_type socket_type;
///错误日志策略的类型
    typedef typename config::elog_type elog_type;
///访问日志策略的类型
    typedef typename config::alog_type alog_type;

///插座连接组件的类型
    typedef typename socket_type::socket_con_type socket_con_type;
///指向套接字连接组件的共享指针的类型
    typedef typename socket_con_type::ptr socket_con_ptr;

///与此关联的连接传输组件的类型
///endpoint传输组件
    typedef asio::connection<config> transport_con_type;
///指向连接传输组件的共享指针的类型
///与此终结点传输组件关联
    typedef typename transport_con_type::ptr transport_con_ptr;

///指向正在使用的asio io_服务的指针类型
    typedef lib::asio::io_service * io_service_ptr;
///指向正在使用的接受程序的共享指针的类型
    typedef lib::shared_ptr<lib::asio::local::stream_protocol::acceptor> acceptor_ptr;
///计时器句柄类型
    typedef lib::shared_ptr<lib::asio::steady_timer> timer_ptr;
///指向IO服务工作对象的共享指针类型
    typedef lib::shared_ptr<lib::asio::io_service::work> work_ptr;

//生成和管理我们自己的IO服务
    explicit local_endpoint()
      : m_io_service(NULL)
      , m_state(UNINITIALIZED)
    {
//std:：cout<“transport:：asio:：endpoint constructor”<<std:：endl；
    }

    ~local_endpoint() {
        if (m_acceptor && m_state == LISTENING)
            ::unlink(m_acceptor->local_endpoint().path().c_str());

//显式销毁本地对象
        m_acceptor.reset();
        m_work.reset();
    }

///TRANSPORT：：asio对象是可移动的，但不可复制或分配。
///以下代码根据我们是否
//有C++ 11支持或不支持
#ifdef _WEBSOCKETPP_DEFAULT_DELETE_FUNCTIONS_
    local_endpoint(const local_endpoint & src) = delete;
    local_endpoint& operator= (const local_endpoint & rhs) = delete;
#else
private:
    local_endpoint(const local_endpoint & src);
    local_endpoint & operator= (const local_endpoint & rhs);
public:
#endif //_websocketpp_默认值_删除_函数\u

#ifdef _WEBSOCKETPP_MOVE_SEMANTICS_
    local_endpoint (local_endpoint && src)
      : config::socket_type(std::move(src))
      , m_tcp_pre_init_handler(src.m_tcp_pre_init_handler)
      , m_tcp_post_init_handler(src.m_tcp_post_init_handler)
      , m_io_service(src.m_io_service)
      , m_acceptor(src.m_acceptor)
      , m_elog(src.m_elog)
      , m_alog(src.m_alog)
      , m_state(src.m_state)
    {
        src.m_io_service = NULL;
        src.m_acceptor = NULL;
        src.m_state = UNINITIALIZED;
    }

#endif //_websocketpp_move_语义\u

///返回终结点是否生成安全连接。
    bool is_secure() const {
        return socket_type::is_secure();
    }

///initialize asio transport with external io_service（无异常）
    /*
     *使用提供的
     *IO服务对象。必须在任何终结点上准确调用一次asio-init
     *在可以使用之前使用transport:：asio。
     *
     *@param ptr指向IO服务的指针，用于ASIO事件
     *@param ec设置为指示发生了什么错误（如果有）。
     **/

    void init_asio(io_service_ptr ptr, lib::error_code & ec) {
        if (m_state != UNINITIALIZED) {
            m_elog->write(log::elevel::library,
                "asio::init_asio called from the wrong state");
            using websocketpp::error::make_error_code;
            ec = make_error_code(websocketpp::error::invalid_state);
            return;
        }

        m_alog->write(log::alevel::devel,"asio::init_asio");

        m_io_service = ptr;
        m_acceptor = lib::make_shared<lib::asio::local::stream_protocol::acceptor>(
            lib::ref(*m_io_service));

        m_state = READY;
        ec = lib::error_code();
    }

///用外部IO服务初始化ASIO传输
    /*
     *使用提供的
     *IO服务对象。必须在任何终结点上准确调用一次asio-init
     *在可以使用之前使用transport:：asio。
     *
     *@param ptr指向IO服务的指针，用于ASIO事件
     **/

    void init_asio(io_service_ptr ptr) {
        lib::error_code ec;
        init_asio(ptr,ec);
        if (ec) { throw exception(ec); }
    }

///set tcp pre init处理程序
    /*
     *在原始TCP连接
     *已建立，但在任何其他包装器（代理连接、TLS）之前
     *握手等）。
     *
     *3.0
     *
     *@param h调用tcp pre init的处理程序。
     **/

    void set_tcp_pre_init_handler(tcp_init_handler h) {
        m_tcp_pre_init_handler = h;
    }

///set tcp pre init处理程序（已弃用）
    /*
     *在原始TCP连接
     *已建立，但在任何其他包装器（代理连接、TLS）之前
     *握手等）。
     *
     *@已弃用，请改用set_tcp_pre_init_handler
     *
     *@param h调用tcp pre init的处理程序。
     **/

    void set_tcp_init_handler(tcp_init_handler h) {
        set_tcp_pre_init_handler(h);
    }

///设置tcp post init处理程序
    /*
     *TCP连接完成后调用TCP Post Init处理程序。
     *已建立和所有附加包装（代理连接、TLS握手、
     *已执行ETC。在读取任何字节或任何
     *已执行WebSocket特定的握手逻辑。
     *
     *3.0
     *
     *@param h调用tcp post init的处理程序。
     **/

    void set_tcp_post_init_handler(tcp_init_handler h) {
        m_tcp_post_init_handler = h;
    }

///检索对终结点的IO服务的引用
    /*
     *IO服务可以是内部或外部的。这可以用来
     *未被
     ＊端点。
     *
     *此方法仅在使用初始化终结点后有效。
     *“IITITA ASIO”。否则不会返回任何错误。
     *
     *@返回对端点的IO服务的引用
     **/

    lib::asio::io_service & get_io_service() {
        return *m_io_service;
    }

///设置用于手动侦听的终结点（无异常）
    /*
     *使用指定的设置绑定内部接收器。端点
     *必须在侦听之前通过调用initou asio初始化。
     *
     *@param ep从中读取设置的端点
     *@param ec设置为指示发生了什么错误（如果有）。
     **/

    void listen(lib::asio::local::stream_protocol::endpoint const & ep, lib::error_code & ec)
    {
        if (m_state != READY) {
            m_elog->write(log::elevel::library,
                "asio::listen called from the wrong state");
            using websocketpp::error::make_error_code;
            ec = make_error_code(websocketpp::error::invalid_state);
            return;
        }

        m_alog->write(log::alevel::devel,"asio::listen");

        lib::asio::error_code bec;

        {
            boost::system::error_code test_ec;
            lib::asio::local::stream_protocol::socket test_socket(get_io_service());
            test_socket.connect(ep, test_ec);

//看起来服务已经在那个插座上运行了，可能是另一个keosd，不要碰它。
            if(test_ec == boost::system::errc::success)
               bec = boost::system::errc::make_error_code(boost::system::errc::address_in_use);
//插座已存在，但没有人在家，请继续拆卸并继续
            else if(test_ec == boost::system::errc::connection_refused)
               ::unlink(ep.path().c_str());
            else if(test_ec != boost::system::errc::no_such_file_or_directory)
               bec = test_ec;
        }

        if (!bec) {
            m_acceptor->open(ep.protocol(),bec);
        }
        if (!bec) {
            m_acceptor->bind(ep,bec);
        }
        if (!bec) {
            m_acceptor->listen(boost::asio::socket_base::max_listen_connections,bec);
        }
        if (bec) {
            if (m_acceptor->is_open()) {
                m_acceptor->close();
            }
            log_err(log::elevel::info,"asio listen",bec);
            ec = bec;
        } else {
            m_state = LISTENING;
            ec = lib::error_code();
        }
    }

///设置用于手动侦听的终结点
    /*
     *使用端点e指定的设置绑定内部接收器
     *
     *@param ep从中读取设置的端点
     **/

    void listen(lib::asio::local::stream_protocol::endpoint const & ep) {
        lib::error_code ec;
        listen(ep,ec);
        if (ec) { throw exception(ec); }
    }

///停止侦听（无异常）
    /*
     *停止收听和接受新连接。这不会结束任何
     *现有连接。
     *
     *@自0.3.0-alpha4起
     *@param ec表示错误（如果有）的状态代码。
     **/

    void stop_listening(lib::error_code & ec) {
        if (m_state != LISTENING) {
            m_elog->write(log::elevel::library,
                "asio::listen called from the wrong state");
            using websocketpp::error::make_error_code;
            ec = make_error_code(websocketpp::error::invalid_state);
            return;
        }

        ::unlink(m_acceptor->local_endpoint().path().c_str());
        m_acceptor->close();
        m_state = READY;
        ec = lib::error_code();
    }

///停止收听
    /*
     *停止收听和接受新连接。这不会结束任何
     *现有连接。
     *
     *@自0.3.0-alpha4起
     **/

    void stop_listening() {
        lib::error_code ec;
        stop_listening(ec);
        if (ec) { throw exception(ec); }
    }

///检查终结点是否正在侦听
    /*
     *@返回端点是否正在侦听。
     **/

    bool is_listening() const {
        return (m_state == LISTENING);
    }

///wrapps内部IO服务对象的运行方法
    std::size_t run() {
        return m_io_service->run();
    }

///wrapps内部IO服务对象的run-one方法
    /*
     *@自0.3.0-alpha4起
     **/

    std::size_t run_one() {
        return m_io_service->run_one();
    }

///wrapps内部IO服务对象的停止方法
    void stop() {
        m_io_service->stop();
    }

///wrapps内部IO服务对象的轮询方法
    std::size_t poll() {
        return m_io_service->poll();
    }

///wrapps轮询\内部IO服务对象的一个方法
    std::size_t poll_one() {
        return m_io_service->poll_one();
    }

///wrapps内部IO服务对象的重置方法
    void reset() {
        m_io_service->reset();
    }

///wrapps内部IO服务对象的已停止方法
    bool stopped() const {
        return m_io_service->stopped();
    }

///将终结点标记为永久性，空时停止它退出
    /*
     *将端点标记为永久性。永久终结点不会
     *当与进程的连接用完时自动退出。停止
     *一个永久端点调用'end_periodic'。
     *
     *任何线程都可以在任何时候永久标记端点。一定是
     *在终结点用完工作之前或之前调用
     *开始
     *
     *3.0
     **/

    void start_perpetual() {
        m_work = lib::make_shared<lib::asio::io_service::work>(
            lib::ref(*m_io_service)
        );
    }

///清除终结点的永久标志，使其在为空时退出
    /*
     *清除端点的永久标志。这将导致终结点运行
     *当连接耗尽时正常退出的方法。如果有
     *当前活动的连接在完成之前不会结束。
     *
     *3.0
     **/

    void stop_perpetual() {
        m_work.reset();
    }

///在一段时间后调用函数。
    /*
     *设置在指定的时间段后调用函数的计时器。
     *毫秒。返回可用于取消计时器的句柄。
     *取消的计时器将返回错误代码error:：operation_aborted
     *过期的计时器不会返回任何错误。
     *
     *@param duration等待时间的长度（毫秒）
     *@param回调计时器过期时要回调的函数
     *@返回一个句柄，该句柄可用于取消计时器（如果不再是计时器）
     *需要。
     **/

    timer_ptr set_timer(long duration, timer_handler callback) {
        timer_ptr new_timer = lib::make_shared<lib::asio::steady_timer>(
            *m_io_service,
             lib::asio::milliseconds(duration)
        );

        new_timer->async_wait(
            lib::bind(
                &type::handle_timer,
                this,
                new_timer,
                callback,
                lib::placeholders::_1
            )
        );

        return new_timer;
    }

//定时器处理器
    /*
     *包括计时器指针，以确保计时器在
     *到期后。
     *
     *@param t指向相关计时器的指针
     *@param回调要回调的函数
     *@param ec表示错误（如果有）的状态代码。
     **/

    void handle_timer(timer_ptr, timer_handler callback,
        lib::asio::error_code const & ec)
    {
        if (ec) {
            if (ec == lib::asio::error::operation_aborted) {
                callback(make_error_code(transport::error::operation_aborted));
            } else {
                m_elog->write(log::elevel::info,
                    "asio handle_timer error: "+ec.message());
                log_err(log::elevel::info,"asio handle_timer",ec);
                callback(ec);
            }
        } else {
            callback(lib::error_code());
        }
    }

///接受下一次连接尝试并将其分配给con（无异常）
    /*
     *@param tcon连接接受入。
     *@param回调操作完成后要调用的函数。
     *@param ec表示错误（如果有）的状态代码。
     **/

    void async_accept(transport_con_ptr tcon, accept_handler callback,
        lib::error_code & ec)
    {
        if (m_state != LISTENING) {
            using websocketpp::error::make_error_code;
            ec = make_error_code(websocketpp::error::async_accept_not_listening);
            return;
        }

        m_alog->write(log::alevel::devel, "asio::async_accept");

        if (config::enable_multithreading) {
            m_acceptor->async_accept(
                tcon->get_raw_socket(),
                tcon->get_strand()->wrap(lib::bind(
                    &type::handle_accept,
                    this,
                    callback,
                    lib::placeholders::_1
                ))
            );
        } else {
            m_acceptor->async_accept(
                tcon->get_raw_socket(),
                lib::bind(
                    &type::handle_accept,
                    this,
                    callback,
                    lib::placeholders::_1
                )
            );
        }
    }

///接受下一次连接尝试并将其分配给con。
    /*
     *@param tcon连接接受入。
     *@param回调操作完成后要调用的函数。
     **/

    void async_accept(transport_con_ptr tcon, accept_handler callback) {
        lib::error_code ec;
        async_accept(tcon,callback,ec);
        if (ec) { throw exception(ec); }
    }
protected:
///初始化日志记录
    /*
     *记录器位于主端点类中。这样，
     *运输部门无法直接访问。此方法被调用
     *由终结点构造函数允许从传输共享日志记录
     *组件。这些是指向端点成员变量的原始指针。
     *特别是，它们不能在传输构造函数中使用，因为它们
     *尚未建造，不能用于运输
     *销毁程序，因为它们将在那时被销毁。
     **/

    void init_logging(alog_type* a, elog_type* e) {
        m_alog = a;
        m_elog = e;
    }

    void handle_accept(accept_handler callback, lib::asio::error_code const & 
        asio_ec)
    {
        lib::error_code ret_ec;

        m_alog->write(log::alevel::devel, "asio::handle_accept");

        if (asio_ec) {
            if (asio_ec == lib::asio::errc::operation_canceled) {
                ret_ec = make_error_code(websocketpp::error::operation_canceled);
            } else {
                log_err(log::elevel::info,"asio handle_accept",asio_ec);
                ret_ec = asio_ec;
            }
        }

        callback(ret_ec);
    }

///asio连接超时处理程序
    /*
     *包括计时器指针，以确保计时器在
     *到期后。
     *
     *@param tcon指向正在连接的传输连接的指针
     *@param conou计时器指针指向相关计时器
     *@param回调要回调的函数
     *@param ec表示错误（如果有）的状态代码。
     **/

    void handle_connect_timeout(transport_con_ptr tcon, timer_ptr,
        connect_handler callback, lib::error_code const & ec)
    {
        lib::error_code ret_ec;

        if (ec) {
            if (ec == transport::error::operation_aborted) {
                m_alog->write(log::alevel::devel,
                    "asio handle_connect_timeout timer cancelled");
                return;
            }

            log_err(log::elevel::devel,"asio handle_connect_timeout",ec);
            ret_ec = ec;
        } else {
            ret_ec = make_error_code(transport::error::timeout);
        }

        m_alog->write(log::alevel::devel,"TCP connect timed out");
        tcon->cancel_socket_checked();
        callback(ret_ec);
    }

    void handle_connect(transport_con_ptr tcon, timer_ptr con_timer,
        connect_handler callback, lib::asio::error_code const & ec)
    {
        if (ec == lib::asio::error::operation_aborted ||
            lib::asio::is_neg(con_timer->expires_from_now()))
        {
            m_alog->write(log::alevel::devel,"async_connect cancelled");
            return;
        }

        con_timer->cancel();

        if (ec) {
            log_err(log::elevel::info,"asio async_connect",ec);
            callback(ec);
            return;
        }

        if (m_alog->static_test(log::alevel::devel)) {
            m_alog->write(log::alevel::devel,
                "Async connect to "+tcon->get_remote_endpoint()+" successful.");
        }

        callback(lib::error_code());
    }

///初始化连接
    /*
     *对于每个新创建的连接，一个端点调用一次init。
     *目的是让交通政策有机会执行
     *无法通过默认设置完成的特定于传输的初始化
     *构造函数。
     *
     *@param tcon连接传输部分的指针。
     *
     *@返回指示操作成功或失败的状态代码
     **/

    lib::error_code init(transport_con_ptr tcon) {
        m_alog->write(log::alevel::devel, "transport::asio::init");

        lib::error_code ec;

        ec = tcon->init_asio(m_io_service);
        if (ec) {return ec;}

        tcon->set_tcp_pre_init_handler(m_tcp_pre_init_handler);
        tcon->set_tcp_post_init_handler(m_tcp_post_init_handler);

        return lib::error_code();
    }
private:
///记录错误代码和消息的方便方法
    template <typename error_type>
    void log_err(log::level l, char const * msg, error_type const & ec) {
        std::stringstream s;
        s << msg << " error: " << ec << " (" << ec.message() << ")";
        m_elog->write(l,s.str());
    }

    enum state {
        UNINITIALIZED = 0,
        READY = 1,
        LISTENING = 2
    };

//处理程序
    tcp_init_handler    m_tcp_pre_init_handler;
    tcp_init_handler    m_tcp_post_init_handler;

//网络资源
    io_service_ptr      m_io_service;
    acceptor_ptr        m_acceptor;
    work_ptr            m_work;

    elog_type* m_elog;
    alog_type* m_alog;

//运输状态
    state               m_state;
};

} //命名空间ASIO
} //命名空间传输
} //命名空间websocketpp
