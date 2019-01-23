
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <eosio/chain/transaction_metadata.hpp>
#include <eosio/chain/thread_utils.hpp>
#include <boost/asio/thread_pool.hpp>

namespace eosio { namespace chain {


const flat_set<public_key_type>& transaction_metadata::recover_keys( const chain_id_type& chain_id ) {
//不太可能在一个节点实例中使用多个链ID
   if( !signing_keys || signing_keys->first != chain_id ) {
      if( signing_keys_future.valid() ) {
         std::tuple<chain_id_type, fc::microseconds, flat_set<public_key_type>> sig_keys = signing_keys_future.get();
         if( std::get<0>( sig_keys ) == chain_id ) {
            sig_cpu_usage = std::get<1>( sig_keys );
            signing_keys.emplace( std::get<0>( sig_keys ), std::move( std::get<2>( sig_keys )));
            return signing_keys->second;
         }
      }
      flat_set<public_key_type> recovered_pub_keys;
      sig_cpu_usage = packed_trx->get_signed_transaction().get_signature_keys( chain_id, fc::time_point::maximum(), recovered_pub_keys );
      signing_keys.emplace( chain_id, std::move( recovered_pub_keys ));
   }
   return signing_keys->second;
}

void transaction_metadata::create_signing_keys_future( const transaction_metadata_ptr& mtrx,
      boost::asio::thread_pool& thread_pool, const chain_id_type& chain_id, fc::microseconds time_limit ) {
if( mtrx->signing_keys.valid() ) //已经创建
      return;

   std::weak_ptr<transaction_metadata> mtrx_wp = mtrx;
   mtrx->signing_keys_future = async_thread_pool( thread_pool, [time_limit, chain_id, mtrx_wp]() {
      fc::time_point deadline = time_limit == fc::microseconds::maximum() ?
            fc::time_point::maximum() : fc::time_point::now() + time_limit;
      auto mtrx = mtrx_wp.lock();
      fc::microseconds cpu_usage;
      flat_set<public_key_type> recovered_pub_keys;
      if( mtrx ) {
         const signed_transaction& trn = mtrx->packed_trx->get_signed_transaction();
         cpu_usage = trn.get_signature_keys( chain_id, deadline, recovered_pub_keys );
      }
      return std::make_tuple( chain_id, cpu_usage, std::move( recovered_pub_keys ));
   } );
}


} } //EOSIO：链
