
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include <boost/test/unit_test.hpp>
#pragma GCC diagnostic pop
#include <boost/algorithm/string/predicate.hpp>
#include <eosio/testing/tester.hpp>
#include <eosio/chain/abi_serializer.hpp>

#include <payloadless/payloadless.wast.hpp>
#include <payloadless/payloadless.abi.hpp>

#include <Runtime/Runtime.h>

#include <fc/variant_object.hpp>
#include <fc/io/json.hpp>

#ifdef NON_VALIDATING_TEST
#define TESTER tester
#else
#define TESTER validating_tester
#endif

using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;

class payloadless_tester : public TESTER {

};

BOOST_AUTO_TEST_SUITE(payloadless_tests)

BOOST_FIXTURE_TEST_CASE( test_doit, payloadless_tester ) {
   
   create_accounts( {N(payloadless)} );
   set_code( N(payloadless), payloadless_wast );
   set_abi( N(payloadless), payloadless_abi );

   auto trace = push_action(N(payloadless), N(doit), N(payloadless), mutable_variant_object());
   auto msg = trace->action_traces.front().console;
   BOOST_CHECK_EQUAL(msg == "Im a payloadless action", true);
}

//TEST GH 3916-从Cleos调用时，没有参数的契约API操作失败
//当操作数据为空时，ABI序列化程序失败。
BOOST_FIXTURE_TEST_CASE( test_abi_serializer, payloadless_tester ) {

   create_accounts( {N(payloadless)} );
   set_code( N(payloadless), payloadless_wast );
   set_abi( N(payloadless), payloadless_abi );

   variant pretty_trx = fc::mutable_variant_object()
      ("actions", fc::variants({
         fc::mutable_variant_object()
            ("account", name(N(payloadless)))
            ("name", "doit")
            ("authorization", fc::variants({
               fc::mutable_variant_object()
                  ("actor", name(N(payloadless)))
                  ("permission", name(config::active_name))
            }))
            ("data", fc::mutable_variant_object()
            )
         })
     );

   signed_transaction trx;
//FROM变量是此测试的关键，因为ABI序列化程序显式不允许空的“数据”
   abi_serializer::from_variant(pretty_trx, trx, get_resolver(), abi_serializer_max_time);
   set_transaction_headers(trx);

   trx.sign( get_private_key( N(payloadless), "active" ), control->get_chain_id() );
   auto trace = push_transaction( trx );
   auto msg = trace->action_traces.front().console;
   BOOST_CHECK_EQUAL(msg == "Im a payloadless action", true);
}

BOOST_AUTO_TEST_SUITE_END()
