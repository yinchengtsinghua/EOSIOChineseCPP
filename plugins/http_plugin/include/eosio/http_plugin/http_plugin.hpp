
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
#include <appbase/application.hpp>
#include <fc/exception/exception.hpp>

#include <fc/reflect/reflect.hpp>

namespace eosio {
   using namespace appbase;

   /*
    *@brief向URL处理程序提供回调函数
    *允许它指定HTTP响应代码和主体
    *
    *参数：响应\代码，响应\正文
    **/

   using url_response_callback = std::function<void(int,string)>;

   /*
    用于URL处理程序的*@brief回调类型
    *
    *URL处理程序具有此类型
    *
    *处理程序必须确保调用url_response_callback（）；
    *否则，连接将挂起并导致内存泄漏。
    *
    *参数：url，请求主体，响应回调
    */

   using url_handler = std::function<void(string,string,url_response_callback)>;

   /*
    *@简要介绍一个包含URL和处理程序的API
    *
    *API由多个调用组成，每个调用都有一个URL和
    *处理程序。URL是Web服务器上触发
    *调用，处理程序是实现API调用的函数
    **/

   using api_description = std::map<string, url_handler>;

   struct http_plugin_defaults {
//如果不为空，则此字符串将预先用于各种配置
//用于设置侦听地址的项
      string address_config_prefix;
//如果为空，将完全禁用Unix套接字支持。如果不是空的，
//使用给定的默认路径（相对处理）启用Unix套接字支持
//到DATADIR）
      string default_unix_socket_path;
//如果非0，则在给定端口号上默认启用HTTP。如果
//0，默认情况下不会启用HTTP
      uint16_t default_http_port{0};
   };

   /*
    *此插件启动HTTP服务器并将查询发送到
    *基于URL注册的句柄。处理程序通过
    *请求的URL和应
    *使用响应代码和正文调用。
    *
    *处理程序将从AppBase应用程序IO服务调用
    *线程。回调可以从任何线程调用，并且将
    *自动将调用传播到HTTP线程。
    *
    *HTTP服务将在自己的线程中运行，并使用自己的IO服务
    *请确保HTTP请求处理不会与其他请求交互。
    ＊插件。
    **/

   class http_plugin : public appbase::plugin<http_plugin>
   {
      public:
        http_plugin();
        virtual ~http_plugin();

//必须在初始化之前调用
        static void set_defaults(const http_plugin_defaults config);

        APPBASE_PLUGIN_REQUIRES()
        virtual void set_program_options(options_description&, options_description& cfg) override;

        void plugin_initialize(const variables_map& options);
        void plugin_startup();
        void plugin_shutdown();

        void add_handler(const string& url, const url_handler&);
        void add_api(const api_description& api) {
           for (const auto& call : api)
              add_handler(call.first, call.second);
        }

//API处理程序的标准异常处理
        static void handle_exception( const char *api_name, const char *call_name, const string& body, url_response_callback cb );

        bool is_on_loopback() const;
        bool is_secure() const;

        bool verbose_errors()const;

      private:
        std::unique_ptr<class http_plugin_impl> my;
   };

   /*
    用于创建JSON错误响应的*@brief结构
    **/

   struct error_results {
      uint16_t code;
      string message;

      struct error_info {
         int64_t code;
         string name;
         string what;

         struct error_detail {
            string message;
            string file;
            uint64_t line_number;
            string method;
         };

         vector<error_detail> details;

         static const uint8_t details_limit = 10;

         error_info() {};

         error_info(const fc::exception& exc, bool include_log) {
            code = exc.code();
            name = exc.name();
            what = exc.what();
            if (include_log) {
               for (auto itr = exc.get_log().begin(); itr != exc.get_log().end(); ++itr) {
//防止发送太大的跟踪
                  if (details.size() >= details_limit) break;
//追加错误
                  error_detail detail = {
                          itr->get_message(), itr->get_context().get_file(),
                          itr->get_context().get_line_number(), itr->get_context().get_method()
                  };
                  details.emplace_back(detail);
               }
            }
         }
      };

      error_info error;
   };
}

FC_REFLECT(eosio::error_results::error_info::error_detail, (message)(file)(line_number)(method))
FC_REFLECT(eosio::error_results::error_info, (code)(name)(what)(details))
FC_REFLECT(eosio::error_results, (code)(message)(error))
