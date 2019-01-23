
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

#include <eosio/wallet_plugin/wallet_plugin.hpp>
#include <eosio/http_plugin/http_plugin.hpp>

#include <appbase/application.hpp>

namespace eosio {

using namespace appbase;

class wallet_api_plugin : public plugin<wallet_api_plugin> {
public:
   APPBASE_PLUGIN_REQUIRES((wallet_plugin) (http_plugin))

   wallet_api_plugin() = default;
   wallet_api_plugin(const wallet_api_plugin&) = delete;
   wallet_api_plugin(wallet_api_plugin&&) = delete;
   wallet_api_plugin& operator=(const wallet_api_plugin&) = delete;
   wallet_api_plugin& operator=(wallet_api_plugin&&) = delete;
   virtual ~wallet_api_plugin() override = default;

   virtual void set_program_options(options_description& cli, options_description& cfg) override {}
   void plugin_initialize(const variables_map& vm);
   void plugin_startup();
   void plugin_shutdown() {}

private:
};

}
