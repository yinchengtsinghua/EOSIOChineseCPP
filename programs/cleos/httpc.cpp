
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
//
//同步客户端
//~~~~~~~~~~~~~~~~~~~~~
//
//版权所有（c）2003-2012 Christopher M.Kohlhoff（Chris at Kohlhoff.com）
//
//在Boost软件许可证1.0版下分发。（见附件
//文件许可证_1_0.txt或复制到http://www.boost.org/license_1_0.txt）
//

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <regex>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <fc/variant.hpp>
#include <fc/io/json.hpp>
#include <fc/network/platform_root_ca.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/http_plugin/http_plugin.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>
#include <boost/asio/ssl/rfc2818_verification.hpp>
#include "httpc.hpp"

using boost::asio::ip::tcp;
using namespace eosio::chain;
namespace eosio { namespace client { namespace http {

   namespace detail {
      class http_context_impl {
         public:
            boost::asio::io_service ios;
      };

      void http_context_deleter::operator()(http_context_impl* p) const {
         delete p;
      }
   }

   http_context create_http_context() {
      return http_context(new detail::http_context_impl, detail::http_context_deleter());
   }

   void do_connect(tcp::socket& sock, const resolved_url& url) {
//获取与服务器名称对应的终结点列表。
      vector<tcp::endpoint> endpoints;
      endpoints.reserve(url.resolved_addresses.size());
      for (const auto& addr: url.resolved_addresses) {
         endpoints.emplace_back(boost::asio::ip::make_address(addr), url.resolved_port);
      }
      boost::asio::connect(sock, endpoints);
   }

   template<class T>
   std::string do_txrx(T& socket, boost::asio::streambuf& request_buff, unsigned int& status_code) {
//发送请求。
      boost::asio::write(socket, request_buff);

//读取响应状态行。响应流buf将自动
//增长以适应整个生产线。增长可能会受到限制
//streambuf构造函数的最大大小。
      boost::asio::streambuf response;
      boost::asio::read_until(socket, response, "\r\n");

//检查响应是否正常。
      std::istream response_stream(&response);
      std::string http_version;
      response_stream >> http_version;
      response_stream >> status_code;

      EOS_ASSERT( status_code != 400, invalid_http_request, "The server has rejected the request as invalid!");

      std::string status_message;
      std::getline(response_stream, status_message);
      EOS_ASSERT( !(!response_stream || http_version.substr(0, 5) != "HTTP/"), invalid_http_response, "Invalid Response" );

//读取以空行结尾的响应头。
      boost::asio::read_until(socket, response, "\r\n\r\n");

//处理响应头。
      std::string header;
      int response_content_length = -1;
      std::regex clregex(R"xx(^content-length:\s+(\d+))xx", std::regex_constants::icase);
      while (std::getline(response_stream, header) && header != "\r") {
         std::smatch match;
         if(std::regex_search(header, match, clregex))
            response_content_length = std::stoi(match[1]);
      }
      EOS_ASSERT(response_content_length >= 0, invalid_http_response, "Invalid content-length response");

      std::stringstream re;
//写下我们已经要输出的任何内容。
      response_content_length -= response.size();
      if (response.size() > 0)
         re << &response;

      boost::asio::read(socket, response, boost::asio::transfer_exactly(response_content_length));
      re << &response;

      return re.str();
   }

   parsed_url parse_url( const string& server_url ) {
      parsed_url res;

//Unix套接字并不完全遵循经典的“url”规则，所以手动处理它
if(boost::algorithm::starts_with(server_url, "unix://“）
         res.scheme = "unix";
res.server = server_url.substr(strlen("unix://“）；
         return res;
      }

//通过rfc3986并修改位以吸出端口号
//遗憾的是，这不适用于IPv6地址
std::regex rgx(R"xx(^(([^:/?#]+):)?(//（？）（*）*（）：（\d+）？（？^？）（*）*（？）（[^×] *）？（α*（？*））？XX）；
      std::smatch match;
      if(std::regex_search(server_url.begin(), server_url.end(), match, rgx)) {
         res.scheme = match[2];
         res.server = match[4];
         res.port = match[6];
         res.path = match[7];
      }
      if(res.scheme != "http" && res.scheme != "https")
         EOS_THROW(fail_to_resolve_host, "Unrecognized URL scheme (${s}) in URL \"${u}\"", ("s", res.scheme)("u", server_url));
      if(res.server.empty())
         EOS_THROW(fail_to_resolve_host, "No server parsed from URL \"${u}\"", ("u", server_url));
      if(res.port.empty())
         res.port = res.scheme == "http" ? "80" : "443";
      boost::trim_right_if(res.path, boost::is_any_of("/"));
      return res;
   }

   resolved_url resolve_url( const http_context& context, const parsed_url& url ) {
      if(url.scheme == "unix")
         return resolved_url(url);

      tcp::resolver resolver(context->ios);
      boost::system::error_code ec;
      auto result = resolver.resolve(tcp::v4(), url.server, url.port, ec);
      if (ec) {
         EOS_THROW(fail_to_resolve_host, "Error resolving \"${server}:${port}\" : ${m}", ("server", url.server)("port",url.port)("m",ec.message()));
      }

//保证非错误结果返回非空范围
      vector<string> resolved_addresses;
      resolved_addresses.reserve(result.size());
      optional<uint16_t> resolved_port;
      bool is_loopback = true;

      for(const auto& r : result) {
         const auto& addr = r.endpoint().address();
         if (addr.is_v6()) continue;
         uint16_t port = r.endpoint().port();
         resolved_addresses.emplace_back(addr.to_string());
         is_loopback = is_loopback && addr.is_loopback();

         if (resolved_port) {
            EOS_ASSERT(*resolved_port == port, resolved_to_multiple_ports, "Service name \"${port}\" resolved to multiple ports and this is not supported!", ("port",url.port));
         } else {
            resolved_port = port;
         }
      }

      return resolved_url(url, std::move(resolved_addresses), *resolved_port, is_loopback);
   }

   string format_host_header(const resolved_url& url) {
//通常的做法是，只有当端口是非默认端口时，才将其显式
      if (
         (url.scheme == "https" && url.resolved_port == 443) ||
         (url.scheme == "http" && url.resolved_port == 80)
      ) {
         return url.server;
      } else {
         return url.server + ":" + url.port;
      }
   }

   fc::variant do_http_call( const connection_param& cp,
                             const fc::variant& postdata,
                             bool print_request,
                             bool print_response ) {
   std::string postjson;
   if( !postdata.is_null() ) {
      postjson = print_request ? fc::json::to_pretty_string( postdata ) : fc::json::to_string( postdata );
   }

   const auto& url = cp.url;

   boost::asio::streambuf request;
   std::ostream request_stream(&request);
   auto host_header_value = format_host_header(url);
   request_stream << "POST " << url.path << " HTTP/1.0\r\n";
   request_stream << "Host: " << host_header_value << "\r\n";
   request_stream << "content-length: " << postjson.size() << "\r\n";
   /*uest_stream<“accept:*/*\r\n”；
   请求流<“connection:close\r\n”；
   //附加更多自定义的标题
   std:：vector<string>：：迭代器itr；
   对于（itr=cp.headers.begin（）；itr！=cp.headers.end（）；itr++）
      请求流<<*itr<<\r\n；
   }
   请求流<\r\n；
   请求流<<postjson；

   如果（打印请求）
      字符串S（request.size（），'\0'）；
      buffer_copy（boost:：asio:：buffer，request.data（））；
      std:：cerr<“请求：”<<std:：endl
                <<“---------------------”<<std:：endl
                <<s<<std:：endl
                <<“----------------”<<std:：endl；
   }

   无符号int状态代码；
   STD：字符串RE；

   尝试{
      if（url.scheme==“Unix”）
         boost:：asio:：local:：stream_protocol:：socket unix_socket（cp.context->ios）；
         unix_socket.connect（boost:：asio:：local:：stream_protocol:：endpoint（url.server））；
         re=do_txrx（unix_socket，request，status_code）；
      }
      else if（url.scheme==“http”）
         tcp:：socket socket（cp.context->ios）；
         连接（socket，url）；
         re=do-txrx（插座、请求、状态代码）；
      }
      否则{//HTTPS
         boost:：asio:：ssl:：context ssl_context（boost:：asio:：ssl:：context:：sslv23_client）；
         fc：：将\u平台\u根目录\u添加到\u上下文（ssl \u上下文）；

         boost:：asio:：ssl:：stream<boost:：asio:：ip:：tcp:：socket>socket（cp.context->ios，ssl_context）；
         ssl_set_tlsext_host_name（socket.native_handle（），url.server.c_str（））；
         如果（cp.verify_cert）
            socket.set_verify_模式（boost:：asio:：ssl:：verify_peer）；
            socket.set_verify_callback（boost:：asio:：ssl:：rfc2818_verification（url.server））；
         }
         做连接（socket.next_layer（），url）；
         socket.handshake（boost:：asio:：ssl:：stream_base:：client）；
         re=do-txrx（插座、请求、状态代码）；
         //尝试执行干净的关闭；但如果失败，则吞咽（另一方可能已经向TCP提供了AX）
         尝试socket.shutdown（）；catch（…）
      }
   catch（无效的_http_请求&e）
      e.append_log（fc_log_message（info），请验证此URL是否有效：$url“，（url，url.scheme+“：/”+url.server+“：”+url.port+url.path））；
      e.append_log（fc_log_message（info），如果情况仍然存在，请联系服务器的RPC服务器管理员！，（“服务器”，url.server））；
      投掷；
   }

   const auto response_result=fc:：json:：from_string（re）；
   如果（打印响应）
      std:：cerr<“响应：”<<std:：endl
                <<“---------------------”<<std:：endl
                <<fc:：json:：to_pretty_string（response_result）<<std:：endl
                <<“----------------”<<std:：endl；
   }
   如果（状态代码==200状态代码==201状态代码==202）
      返回响应结果；
   否则，如果（状态代码==404）
      //未知终结点
      if（url.path.compare（0，chain_func_base.size（），chain_func_base）==0）
         throw chain：：缺少_chain_api_plugin_exception（fc_log_message（error，“chain api plugin is not enabled”）；
      else if（url.path.compare（0，wallet_func_base.size（），wallet_func_base）==0）
         throw chain：：缺少_wallet_api_plugin_exception（fc_log_message（错误，“wallet不可用”）；
      else if（url.path.compare（0，history_func_base.size（），history_func_base）==0）
         throw chain：：缺少_history_api_plugin_exception（fc_log_message（error，“history api plugin is not enabled”）；
      else if（url.path.compare（0，net_func_base.size（），net_func_base）==0）
         throw chain：：缺少_net_api_plugin_exception（fc_log_message（error，“net api plugin is not enabled”）；
      }
   }否则{
      auto&&error_info=response_result.as<eosio:：error_results>（）.error；
      //从错误构造fc异常
      const auto&error_details=error_info.details；

      fc：：记录消息日志；
      对于（auto itr=error_details.begin（）；itr！=错误_details.end（）；itr++）
         const auto&context=fc:：log_context（fc:：log_level:：error，itr->file.data（），itr->line_number，itr->method.data（））；
         logs.emplace_back（fc:：log_message（context，itr->message））；
      }

      抛出fc:：exception（日志、错误\u info.code、错误\u info.name、错误\u info.what）；
   }

   eos_assert（status_code==200，http_request_fail，“错误代码$c \n：$msg \n”，“（c”，status_code）（“msg”，re））；
   返回响应结果；
   }
} }
