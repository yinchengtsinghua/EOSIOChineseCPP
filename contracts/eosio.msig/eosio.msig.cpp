
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <eosio.msig/eosio.msig.hpp>
#include <eosiolib/action.hpp>
#include <eosiolib/permission.hpp>

namespace eosio {

/*
propose函数手动分析输入数据（而不是从调度器获取已分析的参数）
因为在调度程序中分析数据会占用太多的CPU，以防提议的事务太大。

如果我们使用Dispatcher，函数签名应该是：

void multisig:：propose（帐户名proposer，
                        名称建议\名称，
                        Vector<Permission_Level>已请求，
                        事务处理trx）
**/


void multisig::propose() {
   constexpr size_t max_stack_buffer_size = 512;
   size_t size = action_data_size();
   char* buffer = (char*)( max_stack_buffer_size < size ? malloc(size) : alloca(size) );
   read_action_data( buffer, size );

   account_name proposer;
   name proposal_name;
   vector<permission_level> requested;
   transaction_header trx_header;

   datastream<const char*> ds( buffer, size );
   ds >> proposer >> proposal_name >> requested;

   size_t trx_pos = ds.tellp();
   ds >> trx_header;

   require_auth( proposer );
   eosio_assert( trx_header.expiration >= eosio::time_point_sec(now()), "transaction expired" );
//eosio_assert（trx_header.actions.size（）>0，“事务必须至少有一个操作”）；

   proposals proptable( _self, proposer );
   eosio_assert( proptable.find( proposal_name ) == proptable.end(), "proposal with the same name exists" );

   bytes packed_requested = pack(requested);
   auto res = ::check_transaction_authorization( buffer+trx_pos, size-trx_pos,
                                                 (const char*)0, 0,
                                                 packed_requested.data(), packed_requested.size()
                                               );
   eosio_assert( res > 0, "transaction authorization failed" );

   proptable.emplace( proposer, [&]( auto& prop ) {
      prop.proposal_name       = proposal_name;
      prop.packed_transaction  = bytes( buffer+trx_pos, buffer+size );
   });

   approvals apptable(  _self, proposer );
   apptable.emplace( proposer, [&]( auto& a ) {
      a.proposal_name       = proposal_name;
      a.requested_approvals = std::move(requested);
   });
}

void multisig::approve( account_name proposer, name proposal_name, permission_level level ) {
   require_auth( level );

   approvals apptable(  _self, proposer );
   auto& apps = apptable.get( proposal_name, "proposal not found" );

   auto itr = std::find( apps.requested_approvals.begin(), apps.requested_approvals.end(), level );
   eosio_assert( itr != apps.requested_approvals.end(), "approval is not on the list of requested approvals" );

   apptable.modify( apps, proposer, [&]( auto& a ) {
      a.provided_approvals.push_back( level );
      a.requested_approvals.erase( itr );
   });
}

void multisig::unapprove( account_name proposer, name proposal_name, permission_level level ) {
   require_auth( level );

   approvals apptable(  _self, proposer );
   auto& apps = apptable.get( proposal_name, "proposal not found" );
   auto itr = std::find( apps.provided_approvals.begin(), apps.provided_approvals.end(), level );
   eosio_assert( itr != apps.provided_approvals.end(), "no approval previously granted" );

   apptable.modify( apps, proposer, [&]( auto& a ) {
      a.requested_approvals.push_back(level);
      a.provided_approvals.erase(itr);
   });
}

void multisig::cancel( account_name proposer, name proposal_name, account_name canceler ) {
   require_auth( canceler );

   proposals proptable( _self, proposer );
   auto& prop = proptable.get( proposal_name, "proposal not found" );

   if( canceler != proposer ) {
      eosio_assert( unpack<transaction_header>( prop.packed_transaction ).expiration < eosio::time_point_sec(now()), "cannot cancel until expiration" );
   }

   approvals apptable(  _self, proposer );
   auto& apps = apptable.get( proposal_name, "proposal not found" );

   proptable.erase(prop);
   apptable.erase(apps);
}

void multisig::exec( account_name proposer, name proposal_name, account_name executer ) {
   require_auth( executer );

   proposals proptable( _self, proposer );
   auto& prop = proptable.get( proposal_name, "proposal not found" );

   approvals apptable(  _self, proposer );
   auto& apps = apptable.get( proposal_name, "proposal not found" );

   transaction_header trx_header;
   datastream<const char*> ds( prop.packed_transaction.data(), prop.packed_transaction.size() );
   ds >> trx_header;
   eosio_assert( trx_header.expiration >= eosio::time_point_sec(now()), "transaction expired" );

   bytes packed_provided_approvals = pack(apps.provided_approvals);
   auto res = ::check_transaction_authorization( prop.packed_transaction.data(), prop.packed_transaction.size(),
                                                 (const char*)0, 0,
                                                 packed_provided_approvals.data(), packed_provided_approvals.size()
                                               );
   eosio_assert( res > 0, "transaction authorization failed" );

   send_deferred( (uint128_t(proposer) << 64) | proposal_name, executer, prop.packed_transaction.data(), prop.packed_transaction.size() );

   proptable.erase(prop);
   apptable.erase(apps);
}

} ///命名空间eosio

EOSIO_ABI( eosio::multisig, (propose)(approve)(unapprove)(cancel)(exec) )
