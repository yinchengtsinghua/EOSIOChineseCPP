
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

#include <eosio/http_plugin/http_plugin.hpp>
#include <eosio/chain_plugin/chain_plugin.hpp>

#include <appbase/application.hpp>

namespace eosio {

using namespace appbase;

struct db_size_index_count {
   string   index;
   uint64_t row_count;
};

struct db_size_stats {
   uint64_t                    free_bytes;
   uint64_t                    used_bytes;
   uint64_t                    size;
   vector<db_size_index_count> indices;
};

class db_size_api_plugin : public plugin<db_size_api_plugin> {
public:
   APPBASE_PLUGIN_REQUIRES((http_plugin) (chain_plugin))

   db_size_api_plugin() = default;
   db_size_api_plugin(const db_size_api_plugin&) = delete;
   db_size_api_plugin(db_size_api_plugin&&) = delete;
   db_size_api_plugin& operator=(const db_size_api_plugin&) = delete;
   db_size_api_plugin& operator=(db_size_api_plugin&&) = delete;
   virtual ~db_size_api_plugin() override = default;

   virtual void set_program_options(options_description& cli, options_description& cfg) override {}
   void plugin_initialize(const variables_map& vm) {}
   void plugin_startup();
   void plugin_shutdown() {}

   db_size_stats get();

private:
};

}

FC_REFLECT( eosio::db_size_index_count, (index)(row_count) )
FC_REFLECT( eosio::db_size_stats, (free_bytes)(used_bytes)(size)(indices) )