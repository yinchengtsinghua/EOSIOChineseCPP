
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

#include <eosio/producer_plugin/producer_plugin.hpp>
#include <eosio/http_plugin/http_plugin.hpp>

#include <appbase/application.hpp>

namespace eosio {

using namespace appbase;

class producer_api_plugin : public plugin<producer_api_plugin> {
   public:
      APPBASE_PLUGIN_REQUIRES((producer_plugin) (http_plugin))

      producer_api_plugin() = default;
      producer_api_plugin(const producer_api_plugin&) = delete;
      producer_api_plugin(producer_api_plugin&&) = delete;
      producer_api_plugin& operator=(const producer_api_plugin&) = delete;
      producer_api_plugin& operator=(producer_api_plugin&&) = delete;
      virtual ~producer_api_plugin() override = default;

      virtual void set_program_options(options_description& cli, options_description& cfg) override {}
      void plugin_initialize(const variables_map& vm);
      void plugin_startup();
      void plugin_shutdown() {}

   private:
};

}
