
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

#include <eosio/wallet_plugin/wallet_plugin.hpp>
#include <eosio/wallet_plugin/wallet_manager.hpp>
#include <eosio/wallet_plugin/yubihsm_wallet.hpp>
#include <eosio/chain/exceptions.hpp>
#include <boost/filesystem/path.hpp>
#include <chrono>

#include <fc/io/json.hpp>

namespace fc { class variant; }

namespace eosio {

static appbase::abstract_plugin& _wallet_plugin = app().register_plugin<wallet_plugin>();

wallet_plugin::wallet_plugin() {}

wallet_manager& wallet_plugin::get_wallet_manager() {
   return *wallet_manager_ptr;
}

void wallet_plugin::set_program_options(options_description& cli, options_description& cfg) {
   cfg.add_options()
         ("wallet-dir", bpo::value<boost::filesystem::path>()->default_value("."),
          "The path of the wallet files (absolute path or relative to application data dir)")
         ("unlock-timeout", bpo::value<int64_t>()->default_value(900),
          "Timeout for unlocked wallet in seconds (default 900 (15 minutes)). "
          "Wallets will automatically lock after specified number of seconds of inactivity. "
          "Activity is defined as any wallet command e.g. list-wallets.")
         ("yubihsm-url", bpo::value<string>()->value_name("URL"),
"Override default URL of http://本地主机：12345用于连接Yubihsm连接器）
         ("yubihsm-authkey", bpo::value<uint16_t>()->value_name("key_num"),
          "Enables YubiHSM support using given Authkey")
         ;
}

void wallet_plugin::plugin_initialize(const variables_map& options) {
   ilog("initializing wallet plugin");
   try {
      wallet_manager_ptr = std::make_unique<wallet_manager>();

      if (options.count("wallet-dir")) {
         auto dir = options.at("wallet-dir").as<boost::filesystem::path>();
         if (dir.is_relative())
            wallet_manager_ptr->set_dir(app().data_dir() / dir);
         else
            wallet_manager_ptr->set_dir(dir);
      }
      if (options.count("unlock-timeout")) {
         auto timeout = options.at("unlock-timeout").as<int64_t>();
         EOS_ASSERT(timeout > 0, chain::invalid_lock_timeout_exception, "Please specify a positive timeout ${t}", ("t", timeout));
         std::chrono::seconds t(timeout);
         wallet_manager_ptr->set_timeout(t);
      }
      if (options.count("yubihsm-authkey")) {
         uint16_t key = options.at("yubihsm-authkey").as<uint16_t>();
string connector_endpoint = "http://本地主机：12345“；
         if(options.count("yubihsm-url"))
            connector_endpoint = options.at("yubihsm-url").as<string>();
         try {
            wallet_manager_ptr->own_and_use_wallet("YubiHSM", make_unique<yubihsm_wallet>(connector_endpoint, key));
         }FC_LOG_AND_RETHROW()
      }
   } FC_LOG_AND_RETHROW()
}

} //命名空间EOSIO
