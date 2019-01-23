
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

#include <eosio/chain_plugin/chain_plugin.hpp>

namespace fc { class variant; }

namespace eosio {
   using chain::transaction_id_type;
   using std::shared_ptr;
   using namespace appbase;
   using chain::name;
   using fc::optional;
   using chain::uint128_t;

   typedef shared_ptr<class history_plugin_impl> history_ptr;
   typedef shared_ptr<const class history_plugin_impl> history_const_ptr;

namespace history_apis {

class read_only {
   history_const_ptr history;

   public:
      read_only(history_const_ptr&& history)
         : history(history) {}


      struct get_actions_params {
         chain::account_name account_name;
optional<int32_t>   pos; ///A absolute sequence position-1是结束/最后一个操作
optional<int32_t>   offset; ///<相对于位置的操作数，负数返回[位置偏移量，位置]，正数返回[位置，位置+偏移量]
      };

      struct ordered_action_result {
         uint64_t                     global_action_seq = 0;
         int32_t                      account_action_seq = 0;
         uint32_t                     block_num;
         chain::block_timestamp_type  block_time;
         fc::variant                  action_trace;
      };

      struct get_actions_result {
         vector<ordered_action_result> actions;
         uint32_t                      last_irreversible_block;
         optional<bool>                time_limit_exceeded_error;
      };


      get_actions_result get_actions( const get_actions_params& )const;


      struct get_transaction_params {
         string                        id;
         optional<uint32_t>            block_num_hint;
      };

      struct get_transaction_result {
         transaction_id_type                   id;
         fc::variant                           trx;
         chain::block_timestamp_type           block_time;
         uint32_t                              block_num = 0;
         uint32_t                              last_irreversible_block = 0;
         vector<fc::variant>                   traces;
      };

      get_transaction_result get_transaction( const get_transaction_params& )const;




      /*
      结构订单交易结果
         uint32序列号；
         chain:：transaction_id_type transaction_id；
         fc：：变量事务；
      }；

      获取交易结果获取交易（const get_transactions&params）const；
      **/



      struct get_key_accounts_params {
         chain::public_key_type     public_key;
      };
      struct get_key_accounts_results {
         vector<chain::account_name> account_names;
      };
      get_key_accounts_results get_key_accounts(const get_key_accounts_params& params) const;


      struct get_controlled_accounts_params {
         chain::account_name     controlling_account;
      };
      struct get_controlled_accounts_results {
         vector<chain::account_name> controlled_accounts;
      };
      get_controlled_accounts_results get_controlled_accounts(const get_controlled_accounts_params& params) const;
};


} //命名空间历史记录\API


/*
 *此插件跟踪与一组已配置帐户关联的所有操作和密钥。它使
 *用于对历史查询分页的钱包。
 *
 *如果有以下任何一种情况，将在帐户历史记录中包括一项操作：
 *接收器
 *-身份验证列表中指定的任何帐户
 *
 *如果密钥是在updateauth或newaccount的权限中引用的，则密钥将链接到帐户。
 **/

class history_plugin : public plugin<history_plugin> {
   public:
      APPBASE_PLUGIN_REQUIRES((chain_plugin))

      history_plugin();
      virtual ~history_plugin();

      virtual void set_program_options(options_description& cli, options_description& cfg) override;

      void plugin_initialize(const variables_map& options);
      void plugin_startup();
      void plugin_shutdown();

      history_apis::read_only  get_read_only_api()const { return history_apis::read_only(history_const_ptr(my)); }

   private:
      history_ptr my;
};

} ///命名空间eosio

FC_REFLECT( eosio::history_apis::read_only::get_actions_params, (account_name)(pos)(offset) )
FC_REFLECT( eosio::history_apis::read_only::get_actions_result, (actions)(last_irreversible_block)(time_limit_exceeded_error) )
FC_REFLECT( eosio::history_apis::read_only::ordered_action_result, (global_action_seq)(account_action_seq)(block_num)(block_time)(action_trace) )

FC_REFLECT( eosio::history_apis::read_only::get_transaction_params, (id)(block_num_hint) )
FC_REFLECT( eosio::history_apis::read_only::get_transaction_result, (id)(trx)(block_time)(block_num)(last_irreversible_block)(traces) )
/*
fc_reflect（eosio:：history_apis:：read_only:：get_transaction_params，（transaction_id））。
fc_reflect（eosio:：history_apis:：read_only:：get_transaction_results，（transaction_id）（transaction））。
fc_reflect（eosio:：history_apis:：read_only:：get_transactions_params，（account_name）（skip_seq）（num_seq））。
fc_reflect（eosio:：history_apis:：read_only:：ordered_transaction_results，（seq_num）（transaction_id）（transaction））。
fc_reflect（eosio:：history_apis:：read_only:：get_transactions_results，（transactions）（超时\u error））。
**/

FC_REFLECT(eosio::history_apis::read_only::get_key_accounts_params, (public_key) )
FC_REFLECT(eosio::history_apis::read_only::get_key_accounts_results, (account_names) )
FC_REFLECT(eosio::history_apis::read_only::get_controlled_accounts_params, (controlling_account) )
FC_REFLECT(eosio::history_apis::read_only::get_controlled_accounts_results, (controlled_accounts) )
