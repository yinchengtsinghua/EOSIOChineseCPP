
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
#include <fc/network/http/http_client.hpp>

namespace eosio {
   using namespace appbase;
   using fc::http_client;

   class http_client_plugin : public appbase::plugin<http_client_plugin>
   {
      public:
        http_client_plugin();
        virtual ~http_client_plugin();

        APPBASE_PLUGIN_REQUIRES()
        virtual void set_program_options(options_description&, options_description& cfg) override;

        void plugin_initialize(const variables_map& options);
        void plugin_startup();
        void plugin_shutdown();

        http_client& get_client() {
           return *my;
        }

      private:
        std::unique_ptr<http_client> my;
   };

}
