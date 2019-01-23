
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


#include <boost/test/unit_test.hpp>
#include <eosio/testing/tester.hpp>

using namespace eosio;
using namespace testing;
using namespace chain;

BOOST_AUTO_TEST_SUITE(block_tests)

BOOST_AUTO_TEST_CASE(block_with_invalid_tx_test)
{
   tester main;

//首先，我们用有效的事务创建一个有效的块
   main.create_account(N(newacc));
   auto b = main.produce_block();

//复制有效块并损坏事务
   auto copy_b = std::make_shared<signed_block>(std::move(*b));
   auto signed_tx = copy_b->transactions.back().trx.get<packed_transaction>().get_signed_transaction();
   auto& act = signed_tx.actions.back();
   auto act_data = act.data_as<newaccount>();
//通过使新帐户名与创建者名称相同，使事务无效
   act_data.name = act_data.creator;
   act.data = fc::raw::pack(act_data);
//重新签署交易
   signed_tx.signatures.clear();
   signed_tx.sign(main.get_private_key(config::system_account_name, "active"), main.control->get_chain_id());
//将有效事务替换为无效事务
   auto invalid_packed_tx = packed_transaction(signed_tx);
   copy_b->transactions.back().trx = invalid_packed_tx;

//重新签署区块
   auto header_bmroot = digest_type::hash( std::make_pair( copy_b->digest(), main.control->head_block_state()->blockroot_merkle.get_root() ) );
   auto sig_digest = digest_type::hash( std::make_pair(header_bmroot, main.control->head_block_state()->pending_schedule_hash) );
   copy_b->producer_signature = main.get_private_key(config::system_account_name, "active").sign(sig_digest);

//将具有无效事务的推送块推送到其他链
   tester validator;
   auto bs = validator.control->create_block_state_future( copy_b );
   validator.control->abort_block();
   BOOST_REQUIRE_EXCEPTION(validator.control->push_block( bs ), fc::exception ,
   [] (const fc::exception &e)->bool {
      return e.code() == account_name_exists_exception::code_value ;
   }) ;

}

std::pair<signed_block_ptr, signed_block_ptr> corrupt_trx_in_block(validating_tester& main, account_name act_name) {
//首先，我们用有效的事务创建一个有效的块
   main.create_account(act_name);
   signed_block_ptr b = main.produce_block_no_validation();

//复制有效块并损坏事务
   auto copy_b = std::make_shared<signed_block>(b->clone());
   const auto& packed_trx = copy_b->transactions.back().trx.get<packed_transaction>();
   auto signed_tx = packed_trx.get_signed_transaction();
//一个签名损坏
   signed_tx.signatures.clear();
   signed_tx.sign(main.get_private_key(act_name, "active"), main.control->get_chain_id());

//将有效事务替换为无效事务
   auto invalid_packed_tx = packed_transaction(signed_tx, packed_trx.get_compression());
   copy_b->transactions.back().trx = invalid_packed_tx;

//重新计算事务merkle
   vector<digest_type> trx_digests;
   const auto& trxs = copy_b->transactions;
   trx_digests.reserve( trxs.size() );
   for( const auto& a : trxs )
      trx_digests.emplace_back( a.digest() );
   copy_b->transaction_mroot = merkle( move(trx_digests) );

//重新签署区块
   auto header_bmroot = digest_type::hash( std::make_pair( copy_b->digest(), main.control->head_block_state()->blockroot_merkle.get_root() ) );
   auto sig_digest = digest_type::hash( std::make_pair(header_bmroot, main.control->head_block_state()->pending_schedule_hash) );
   copy_b->producer_signature = main.get_private_key(b->producer, "active").sign(sig_digest);
   return std::pair<signed_block_ptr, signed_block_ptr>(b, copy_b);
}

//验证具有错误签名的事务的块是否被可信生产者盲目接受
BOOST_AUTO_TEST_CASE(trusted_producer_test)
{
   flat_set<account_name> trusted_producers = { N(defproducera), N(defproducerc) };
   validating_tester main(trusted_producers);
//仅使用验证测试仪保持两条链同步，而不验证验证节点是否与主节点匹配，
//因为它不会
   main.skip_validate = true;

//首先，我们用有效的事务创建一个有效的块
   std::set<account_name> producers = { N(defproducera), N(defproducerb), N(defproducerc), N(defproducerd) };
   for (auto prod : producers)
       main.create_account(prod);
   auto b = main.produce_block();

   std::vector<account_name> schedule(producers.cbegin(), producers.cend());
   auto trace = main.set_producers(schedule);

   while (b->producer != N(defproducera)) {
      b = main.produce_block();
   }

   auto blocks = corrupt_trx_in_block(main, N(tstproducera));
   main.validate_push_block( blocks.second );
}

//类似于受信任的\生产者\测试，除非验证受信任的\生产者列表中的任何条目都被接受
BOOST_AUTO_TEST_CASE(trusted_producer_verify_2nd_test)
{
   flat_set<account_name> trusted_producers = { N(defproducera), N(defproducerc) };
   validating_tester main(trusted_producers);
//仅使用验证测试仪保持两条链同步，而不验证验证节点是否与主节点匹配，
//因为它不会
   main.skip_validate = true;

//首先，我们用有效的事务创建一个有效的块
   std::set<account_name> producers = { N(defproducera), N(defproducerb), N(defproducerc), N(defproducerd) };
   for (auto prod : producers)
       main.create_account(prod);
   auto b = main.produce_block();

   std::vector<account_name> schedule(producers.cbegin(), producers.cend());
   auto trace = main.set_producers(schedule);

   while (b->producer != N(defproducerc)) {
      b = main.produce_block();
   }

   auto blocks = corrupt_trx_in_block(main, N(tstproducera));
   main.validate_push_block( blocks.second );
}

//验证具有不正确签名的事务的块是否被拒绝（如果它不是来自受信任的生产者）
BOOST_AUTO_TEST_CASE(untrusted_producer_test)
{
   flat_set<account_name> trusted_producers = { N(defproducera), N(defproducerc) };
   validating_tester main(trusted_producers);
//仅使用验证测试仪保持两条链同步，而不验证验证节点是否与主节点匹配，
//因为它不会
   main.skip_validate = true;

//首先，我们用有效的事务创建一个有效的块
   std::set<account_name> producers = { N(defproducera), N(defproducerb), N(defproducerc), N(defproducerd) };
   for (auto prod : producers)
       main.create_account(prod);
   auto b = main.produce_block();

   std::vector<account_name> schedule(producers.cbegin(), producers.cend());
   auto trace = main.set_producers(schedule);

   while (b->producer != N(defproducerb)) {
      b = main.produce_block();
   }

   auto blocks = corrupt_trx_in_block(main, N(tstproducera));
   BOOST_REQUIRE_EXCEPTION(main.validate_push_block( blocks.second ), fc::exception ,
   [] (const fc::exception &e)->bool {
      return e.code() == unsatisfied_authorization::code_value ;
   }) ;
}

BOOST_AUTO_TEST_SUITE_END()
