
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
/*
 *@文件
 *@eosio/license.txt中定义的版权
 **/

#include <appbase/application.hpp>

#include <eosio/chain_plugin/chain_plugin.hpp>
#include <eosio/http_plugin/http_plugin.hpp>
#include <eosio/net_plugin/net_plugin.hpp>
#include <eosio/producer_plugin/producer_plugin.hpp>

#include <fc/log/logger_config.hpp>
#include <fc/log/appender.hpp>
#include <fc/exception/exception.hpp>

#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include "config.hpp"

using namespace appbase;
using namespace eosio;

namespace fc {
   std::unordered_map<std::string,appender::ptr>& get_appender_map();
}

namespace detail {

void configure_logging(const bfs::path& config_path)
{
   try {
      try {
         fc::configure_logging(config_path);
      } catch (...) {
         elog("Error reloading logging.json");
         throw;
      }
   } catch (const fc::exception& e) {
      elog("${e}", ("e",e.to_detail_string()));
   } catch (const boost::exception& e) {
      elog("${e}", ("e",boost::diagnostic_information(e)));
   } catch (const std::exception& e) {
      elog("${e}", ("e",e.what()));
   } catch (...) {
//空的
   }
}

} //命名空间详细信息

void logging_conf_loop()
{
   std::shared_ptr<boost::asio::signal_set> sighup_set(new boost::asio::signal_set(app().get_io_service(), SIGHUP));
   /*hup_set->async_wait（[sighup_set]（const boost:：system:：error_code&err，int/*num*/）
      如果（！）错误）
      {
         ILOG（“收到的Hup.正在重新加载日志配置。“）；
         auto config_path=app（）.get_logging_conf（）；
         如果（fc:：exists（config_path））。
            ：：详细信息：：配置日志记录（配置路径）；
         for（auto-iter:fc:：get_appender_map（））
            iter.second->initialize（app（）.get_io_service（））；
         logging_conf_loop（）；
      }
   （}）；
}

void初始化日志（）。
{
   auto config_path=app（）.get_logging_conf（）；
   如果（fc:：exists（config_path））。
     fc:：configure_logging（config_path）；//故意允许异常转义
   for（auto-iter:fc:：get_appender_map（））
     iter.second->initialize（app（）.get_io_service（））；

   logging_conf_loop（）；
}

枚举返回_代码_
   其他故障=-2，
   初始化失败=-1，
   成功=0，
   错误的分配=1，
   数据库脏=2，
   固定_可逆=3，
   提取出的成因=4，
   节点管理成功=5
}；

int main（int argc，char**argv）
{
   尝试{
      app（）。设置_版本（eosio:：nodeos:：config:：version）；

      自动根=fc：：app_path（）；
      app（）。设置“默认数据目录”（root/“eosio/nodeos/data”）；
      app（）。设置_default_config_dir（root/“eosio/nodeos/config”）；
      http_plugin：：设置_默认值（
         .address_config_prefix=“”，
         .default_unix_socket_path=“”，
         .default_http_port=8888
      （}）；
      如果（！）app（）。初始化<chain_plugin，http_plugin，net_plugin，producer_plugin>（argc，argv））
         返回初始化失败；
      初始化_logging（）；
      ilog（“nodeos版本$ver”，“（ver），app（）.version_string（））；
      ilog（“eosio root is$root”，“root”，root.string（））；
      ilog（“nodeos使用配置文件$c，”，“（c”，app（）.full_config_file_path（）.string（））；
      ilog（“nodeos data directory is$d”，（“d”，app（）.data_dir（）.string（））；
      app（）.startup（）；
      App.（）
   捕获（const extract_genesis_state_exception&e）
      回采-成因；
   catch（const fixed_reversale_db_exception&e）
      返回固定\可逆；
   catch（const节点_management_success&e）
      返回节点管理成功；
   catch（const fc:：exception&e）
      if（e.code（）==fc:：std_exception_code）
         如果（e.top_message（）.find（“数据库脏标志集”）！=std：：字符串：：npos）
            ELOG（“数据库脏标志集（可能由于关闭不干净）：需要重播”）；
            返回数据库脏；
         否则，如果（e.top_message（）.find（“数据库元数据脏标志集”）！=std：：字符串：：npos）
            ELOG（“数据库元数据脏标志集（可能由于关闭不干净）：需要重播”）；
            返回数据库脏；
         }
      }
      ELOG（“$e”，（“e”，e.to_detail_string（））；
      退回其他故障；
   catch（const boost:：interprocess:：badou alloc&e）
      ELOG（“不良分配”）；
      返回坏分配；
   catch（const boost:：exception&e）
      ELOG（“$e”，（“e”，Boost：：诊断信息（e））；
      退回其他故障；
   catch（const std:：runtime_error&e）
      if（std:：string（e.what（））=“数据库脏标志集”）；
         ELOG（“数据库脏标志集（可能由于关闭不干净）：需要重播”）；
         返回数据库脏；
      else if（std:：string（e.what（））=“数据库元数据脏标志集”）
         ELOG（“数据库元数据脏标志集（可能由于关闭不干净）：需要重播”）；
         返回数据库脏；
      }否则{
         ELOG（“$e”，（“e”，e.what（））；
      }
      退回其他故障；
   catch（const std:：exception&e）
      ELOG（“$e”，（“e”，e.what（））；
      退回其他故障；
   抓住（…）{
      ELOG（“未知异常”）；
      退回其他故障；
   }

   回归成功；
}
