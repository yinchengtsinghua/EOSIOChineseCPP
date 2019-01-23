
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

#include <fc/variant.hpp>
#include <fc/io/json.hpp>
#include <eosio/db_size_api_plugin/db_size_api_plugin.hpp>

namespace eosio {

static appbase::abstract_plugin& _db_size_api_plugin = app().register_plugin<db_size_api_plugin>();

using namespace eosio;

#define CALL(api_name, api_handle, call_name, INVOKE, http_response_code) \
{std::string("/v1/" #api_name "/" #call_name), \
   [api_handle](string, string body, url_response_callback cb) mutable { \
          try { \
             if (body.empty()) body = "{}"; \
             INVOKE \
             cb(http_response_code, fc::json::to_string(result)); \
          } catch (...) { \
             http_plugin::handle_exception(#api_name, #call_name, body, cb); \
          } \
       }}

#define INVOKE_R_V(api_handle, call_name) \
     auto result = api_handle->call_name();


void db_size_api_plugin::plugin_startup() {
   app().get_plugin<http_plugin>().add_api({
       CALL(db_size, this, get,
            INVOKE_R_V(this, get), 200),
   });
}

db_size_stats db_size_api_plugin::get() {
   const chainbase::database& db = app().get_plugin<chain_plugin>().chain().db();
   db_size_stats ret;

   ret.free_bytes = db.get_segment_manager()->get_free_memory();
   ret.size = db.get_segment_manager()->get_size();
   ret.used_bytes = ret.size - ret.free_bytes;

   chainbase::database::database_index_row_count_multiset indices = db.row_count_per_index();
   for(const auto& i : indices)
      ret.indices.emplace_back(db_size_index_count{i.second, i.first});

   return ret;
}

#undef INVOKE_R_V
#undef CALL

}
