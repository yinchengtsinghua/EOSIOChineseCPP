
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <eosio/testing/tester.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/chain/wasm_eosio_constraints.hpp>
#include <eosio/chain/resource_limits.hpp>
#include <eosio/chain/exceptions.hpp>
#include <eosio/chain/wast_to_wasm.hpp>
#include <asserter/asserter.wast.hpp>
#include <asserter/asserter.abi.hpp>

#include <stltest/stltest.wast.hpp>
#include <stltest/stltest.abi.hpp>

#include <noop/noop.wast.hpp>
#include <noop/noop.abi.hpp>

#include <fc/io/fstream.hpp>

#include <Runtime/Runtime.h>

#include <fc/variant_object.hpp>
#include <fc/io/json.hpp>

#include "test_wasts.hpp"
#include "test_softfloat_wasts.hpp"

#include <array>
#include <utility>

#include "incbin.h"

#ifdef NON_VALIDATING_TEST
#define TESTER tester
#else
#define TESTER validating_tester
#endif

using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;


struct assertdef {
   int8_t      condition;
   string      message;

   static account_name get_account() {
      return N(asserter);
   }

   static action_name get_name() {
      return N(procassert);
   }
};

FC_REFLECT(assertdef, (condition)(message));

struct provereset {
   static account_name get_account() {
      return N(asserter);
   }

   static action_name get_name() {
      return N(provereset);
   }
};

FC_REFLECT_EMPTY(provereset);

BOOST_AUTO_TEST_SUITE(wasm_tests)

/*
 *证明行动阅读和断言有效
 **/

BOOST_FIXTURE_TEST_CASE( basic_test, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(asserter)} );
   produce_block();

   set_code(N(asserter), asserter_wast);
   produce_blocks(1);

   transaction_id_type no_assert_id;
   {
      signed_transaction trx;
      trx.actions.emplace_back( vector<permission_level>{{N(asserter),config::active_name}},
                                assertdef {1, "Should Not Assert!"} );
      trx.actions[0].authorization = {{N(asserter),config::active_name}};

      set_transaction_headers(trx);
      trx.sign( get_private_key( N(asserter), "active" ), control->get_chain_id() );
      auto result = push_transaction( trx );
      BOOST_CHECK_EQUAL(result->receipt->status, transaction_receipt::executed);
      BOOST_CHECK_EQUAL(result->action_traces.size(), 1);
      BOOST_CHECK_EQUAL(result->action_traces.at(0).receipt.receiver.to_string(),  name(N(asserter)).to_string() );
      BOOST_CHECK_EQUAL(result->action_traces.at(0).act.account.to_string(), name(N(asserter)).to_string() );
      BOOST_CHECK_EQUAL(result->action_traces.at(0).act.name.to_string(),  name(N(procassert)).to_string() );
      BOOST_CHECK_EQUAL(result->action_traces.at(0).act.authorization.size(),  1 );
      BOOST_CHECK_EQUAL(result->action_traces.at(0).act.authorization.at(0).actor.to_string(),  name(N(asserter)).to_string() );
      BOOST_CHECK_EQUAL(result->action_traces.at(0).act.authorization.at(0).permission.to_string(),  name(config::active_name).to_string() );
      no_assert_id = trx.id();
   }

   produce_blocks(1);

   BOOST_REQUIRE_EQUAL(true, chain_has_transaction(no_assert_id));
   const auto& receipt = get_transaction_receipt(no_assert_id);
   BOOST_CHECK_EQUAL(transaction_receipt::executed, receipt.status);

   transaction_id_type yes_assert_id;
   {
      signed_transaction trx;
      trx.actions.emplace_back( vector<permission_level>{{N(asserter),config::active_name}},
                                assertdef {0, "Should Assert!"} );

      set_transaction_headers(trx);
      trx.sign( get_private_key( N(asserter), "active" ), control->get_chain_id() );
      yes_assert_id = trx.id();

      BOOST_CHECK_THROW(push_transaction( trx ), eosio_assert_message_exception);
   }

   produce_blocks(1);

   auto has_tx = chain_has_transaction(yes_assert_id);
   BOOST_REQUIRE_EQUAL(false, has_tx);

} FC_LOG_AND_RETHROW() //基本测试

/*
 *证明对全局变量的修改在运行之间被擦除
 **/

BOOST_FIXTURE_TEST_CASE( prove_mem_reset, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(asserter)} );
   produce_block();

   set_code(N(asserter), asserter_wast);
   produce_blocks(1);

//每次操作处理程序检查预期的
//然后，默认值修改在下一次调用之前不应该存在的值
   for (int i = 0; i < 5; i++) {
      signed_transaction trx;
      trx.actions.emplace_back( vector<permission_level>{{N(asserter),config::active_name}},
                                provereset {} );

      set_transaction_headers(trx);
      trx.sign( get_private_key( N(asserter), "active" ), control->get_chain_id() );
      push_transaction( trx );
      produce_blocks(1);
      BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx.id()));
      const auto& receipt = get_transaction_receipt(trx.id());
      BOOST_CHECK_EQUAL(transaction_receipt::executed, receipt.status);
   }

} FC_LOG_AND_RETHROW() ///证明内存重置

/*
 *证明对全局变量的修改在运行之间被擦除
 **/

BOOST_FIXTURE_TEST_CASE( abi_from_variant, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(asserter)} );
   produce_block();

   set_code(N(asserter), asserter_wast);
   set_abi(N(asserter), asserter_abi);
   produce_blocks(1);

   auto resolver = [&,this]( const account_name& name ) -> optional<abi_serializer> {
      try {
         const auto& accnt  = this->control->db().get<account_object,by_name>( name );
         abi_def abi;
         if (abi_serializer::to_abi(accnt.abi, abi)) {
            return abi_serializer(abi, abi_serializer_max_time);
         }
         return optional<abi_serializer>();
      } FC_RETHROW_EXCEPTIONS(error, "Failed to find or parse ABI for ${name}", ("name", name))
   };

   variant pretty_trx = mutable_variant_object()
      ("actions", variants({
         mutable_variant_object()
            ("account", "asserter")
            ("name", "procassert")
            ("authorization", variants({
               mutable_variant_object()
                  ("actor", "asserter")
                  ("permission", name(config::active_name).to_string())
            }))
            ("data", mutable_variant_object()
               ("condition", 1)
               ("message", "Should Not Assert!")
            )
         })
      );

   signed_transaction trx;
   abi_serializer::from_variant(pretty_trx, trx, resolver, abi_serializer_max_time);
   set_transaction_headers(trx);
   trx.sign( get_private_key( N(asserter), "active" ), control->get_chain_id() );
   push_transaction( trx );
   produce_blocks(1);
   BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx.id()));
   const auto& receipt = get_transaction_receipt(trx.id());
   BOOST_CHECK_EQUAL(transaction_receipt::executed, receipt.status);

} FC_LOG_AND_RETHROW() ///证明内存重置

//测试Softfloat 32位操作
BOOST_FIXTURE_TEST_CASE( f32_tests, TESTER ) try {
   produce_blocks(2);
   create_accounts( {N(f32_tests)} );
   produce_block();
   {
      set_code(N(f32_tests), f32_test_wast);
      produce_blocks(10);

      signed_transaction trx;
      action act;
      act.account = N(f32_tests);
      act.name = N();
      act.authorization = vector<permission_level>{{N(f32_tests),config::active_name}};
      trx.actions.push_back(act);

      set_transaction_headers(trx);
      trx.sign(get_private_key( N(f32_tests), "active" ), control->get_chain_id());
      push_transaction(trx);
      produce_blocks(1);
      BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx.id()));
      const auto& receipt = get_transaction_receipt(trx.id());
   }
} FC_LOG_AND_RETHROW()
BOOST_FIXTURE_TEST_CASE( f32_test_bitwise, TESTER ) try {
   produce_blocks(2);
   create_accounts( {N(f32_tests)} );
   produce_block();
   {
      set_code(N(f32_tests), f32_bitwise_test_wast);
      produce_blocks(10);

      signed_transaction trx;
      action act;
      act.account = N(f32_tests);
      act.name = N();
      act.authorization = vector<permission_level>{{N(f32_tests),config::active_name}};
      trx.actions.push_back(act);

      set_transaction_headers(trx);
      trx.sign(get_private_key( N(f32_tests), "active" ), control->get_chain_id());
      push_transaction(trx);
      produce_blocks(1);
      BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx.id()));
      const auto& receipt = get_transaction_receipt(trx.id());
   }
} FC_LOG_AND_RETHROW()
BOOST_FIXTURE_TEST_CASE( f32_test_cmp, TESTER ) try {
   produce_blocks(2);
   create_accounts( {N(f32_tests)} );
   produce_block();
   {
      set_code(N(f32_tests), f32_cmp_test_wast);
      produce_blocks(10);

      signed_transaction trx;
      action act;
      act.account = N(f32_tests);
      act.name = N();
      act.authorization = vector<permission_level>{{N(f32_tests),config::active_name}};
      trx.actions.push_back(act);

      set_transaction_headers(trx);
      trx.sign(get_private_key( N(f32_tests), "active" ), control->get_chain_id());
      push_transaction(trx);
      produce_blocks(1);
      BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx.id()));
      const auto& receipt = get_transaction_receipt(trx.id());
   }
} FC_LOG_AND_RETHROW()

//测试Softfloat 64位操作
BOOST_FIXTURE_TEST_CASE( f64_tests, TESTER ) try {
   produce_blocks(2);
   create_accounts( {N(f_tests)} );
   produce_block();
   {
      set_code(N(f_tests), f64_test_wast);
      produce_blocks(10);

      signed_transaction trx;
      action act;
      act.account = N(f_tests);
      act.name = N();
      act.authorization = vector<permission_level>{{N(f_tests),config::active_name}};
      trx.actions.push_back(act);

      set_transaction_headers(trx);
      trx.sign(get_private_key( N(f_tests), "active" ), control->get_chain_id());
      push_transaction(trx);
      produce_blocks(1);
      BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx.id()));
      const auto& receipt = get_transaction_receipt(trx.id());
   }
} FC_LOG_AND_RETHROW()
BOOST_FIXTURE_TEST_CASE( f64_test_bitwise, TESTER ) try {
   produce_blocks(2);
   create_accounts( {N(f_tests)} );
   produce_block();
   {
      set_code(N(f_tests), f64_bitwise_test_wast);
      produce_blocks(10);

      signed_transaction trx;
      action act;
      act.account = N(f_tests);
      act.name = N();
      act.authorization = vector<permission_level>{{N(f_tests),config::active_name}};
      trx.actions.push_back(act);

      set_transaction_headers(trx);
      trx.sign(get_private_key( N(f_tests), "active" ), control->get_chain_id());
      push_transaction(trx);
      produce_blocks(1);
      BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx.id()));
      const auto& receipt = get_transaction_receipt(trx.id());
   }
} FC_LOG_AND_RETHROW()
BOOST_FIXTURE_TEST_CASE( f64_test_cmp, TESTER ) try {
   produce_blocks(2);
   create_accounts( {N(f_tests)} );
   produce_block();
   {
      set_code(N(f_tests), f64_cmp_test_wast);
      produce_blocks(10);

      signed_transaction trx;
      action act;
      act.account = N(f_tests);
      act.name = N();
      act.authorization = vector<permission_level>{{N(f_tests),config::active_name}};
      trx.actions.push_back(act);

      set_transaction_headers(trx);
      trx.sign(get_private_key( N(f_tests), "active" ), control->get_chain_id());
      push_transaction(trx);
      produce_blocks(1);
      BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx.id()));
      const auto& receipt = get_transaction_receipt(trx.id());
   }
} FC_LOG_AND_RETHROW()

//测试软浮点转换操作
BOOST_FIXTURE_TEST_CASE( f32_f64_conversion_tests, tester ) try {
   produce_blocks(2);

   create_accounts( {N(f_tests)} );
   produce_block();
   {
      set_code(N(f_tests), f32_f64_conv_wast);
      produce_blocks(10);

      signed_transaction trx;
      action act;
      act.account = N(f_tests);
      act.name = N();
      act.authorization = vector<permission_level>{{N(f_tests),config::active_name}};
      trx.actions.push_back(act);

      set_transaction_headers(trx);
      trx.sign(get_private_key( N(f_tests), "active" ), control->get_chain_id());
      push_transaction(trx);
      produce_blocks(1);
      BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx.id()));
      const auto& receipt = get_transaction_receipt(trx.id());
   }
} FC_LOG_AND_RETHROW()

//测试软浮点转换操作
BOOST_FIXTURE_TEST_CASE( f32_f64_overflow_tests, tester ) try {
   int count = 0;
   auto check = [&](const char *wast_template, const char *op, const char *param) -> bool {
      count+=16;
      create_accounts( {N(f_tests)+count} );
      produce_blocks(1);
      std::vector<char> wast;
      wast.resize(strlen(wast_template) + 128);
      sprintf(&(wast[0]), wast_template, op, param);
      set_code(N(f_tests)+count, &(wast[0]));
      produce_blocks(10);

      signed_transaction trx;
      action act;
      act.account = N(f_tests)+count;
      act.name = N();
      act.authorization = vector<permission_level>{{N(f_tests)+count,config::active_name}};
      trx.actions.push_back(act);

      set_transaction_headers(trx);
      trx.sign(get_private_key( N(f_tests)+count, "active" ), control->get_chain_id());

      try {
         push_transaction(trx);
         produce_blocks(1);
         BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx.id()));
         const auto& receipt = get_transaction_receipt(trx.id());
         return true;
      } catch (eosio::chain::wasm_execution_error &) {
         return false;
      }
   };
//
////float32=>int32
//2 ^ 31
   BOOST_REQUIRE_EQUAL(false, check(i32_overflow_wast, "i32_trunc_s_f32", "f32.const 2147483648"));
//IEEE float32中可表示的最大值低于2^31
   BOOST_REQUIRE_EQUAL(true, check(i32_overflow_wast, "i32_trunc_s_f32", "f32.const 2147483520"));
//- 2 ^ 31
   BOOST_REQUIRE_EQUAL(true, check(i32_overflow_wast, "i32_trunc_s_f32", "f32.const -2147483648"));
//IEEE float32中低于-2^31的最大值
   BOOST_REQUIRE_EQUAL(false, check(i32_overflow_wast, "i32_trunc_s_f32", "f32.const -2147483904"));

//
////float32=>uint32
   BOOST_REQUIRE_EQUAL(true, check(i32_overflow_wast, "i32_trunc_u_f32", "f32.const 0"));
   BOOST_REQUIRE_EQUAL(false, check(i32_overflow_wast, "i32_trunc_u_f32", "f32.const -1"));
//IEEE float32中最大值低于2^32
   BOOST_REQUIRE_EQUAL(true, check(i32_overflow_wast, "i32_trunc_u_f32", "f32.const 4294967040"));
   BOOST_REQUIRE_EQUAL(false, check(i32_overflow_wast, "i32_trunc_u_f32", "f32.const 4294967296"));

//
////双精度=>Int32
   BOOST_REQUIRE_EQUAL(false, check(i32_overflow_wast, "i32_trunc_s_f64", "f64.const 2147483648"));
   BOOST_REQUIRE_EQUAL(true, check(i32_overflow_wast, "i32_trunc_s_f64", "f64.const 2147483647"));
   BOOST_REQUIRE_EQUAL(true, check(i32_overflow_wast, "i32_trunc_s_f64", "f64.const -2147483648"));
   BOOST_REQUIRE_EQUAL(false, check(i32_overflow_wast, "i32_trunc_s_f64", "f64.const -2147483649"));

//
////双精度=>uint32
   BOOST_REQUIRE_EQUAL(true, check(i32_overflow_wast, "i32_trunc_u_f64", "f64.const 0"));
   BOOST_REQUIRE_EQUAL(false, check(i32_overflow_wast, "i32_trunc_u_f64", "f64.const -1"));
   BOOST_REQUIRE_EQUAL(true, check(i32_overflow_wast, "i32_trunc_u_f64", "f64.const 4294967295"));
   BOOST_REQUIRE_EQUAL(false, check(i32_overflow_wast, "i32_trunc_u_f64", "f64.const 4294967296"));


////float32=>int64
//2 ^ 63
   BOOST_REQUIRE_EQUAL(false, check(i64_overflow_wast, "i64_trunc_s_f32", "f32.const 9223372036854775808"));
//IEEE float32中可表示的最大值低于2^63
   BOOST_REQUIRE_EQUAL(true, check(i64_overflow_wast, "i64_trunc_s_f32", "f32.const 9223371487098961920"));
//- 2 ^ 63
   BOOST_REQUIRE_EQUAL(true, check(i64_overflow_wast, "i64_trunc_s_f32", "f32.const -9223372036854775808"));
//IEEE float32中-2^63以下的最大值
   BOOST_REQUIRE_EQUAL(false, check(i64_overflow_wast, "i64_trunc_s_f32", "f32.const -9223373136366403584"));

////float32=>uint64
   BOOST_REQUIRE_EQUAL(false, check(i64_overflow_wast, "i64_trunc_u_f32", "f32.const -1"));
   BOOST_REQUIRE_EQUAL(true, check(i64_overflow_wast, "i64_trunc_u_f32", "f32.const 0"));
//IEEE float32中最大值低于2^64
   BOOST_REQUIRE_EQUAL(true, check(i64_overflow_wast, "i64_trunc_u_f32", "f32.const 18446742974197923840"));
   BOOST_REQUIRE_EQUAL(false, check(i64_overflow_wast, "i64_trunc_u_f32", "f32.const 18446744073709551616"));

////双精度=>Int64
//2 ^ 63
   BOOST_REQUIRE_EQUAL(false, check(i64_overflow_wast, "i64_trunc_s_f64", "f64.const 9223372036854775808"));
//IEEE float64中可表示的最大值低于2^63
   BOOST_REQUIRE_EQUAL(true, check(i64_overflow_wast, "i64_trunc_s_f64", "f64.const 9223372036854774784"));
//- 2 ^ 63
   BOOST_REQUIRE_EQUAL(true, check(i64_overflow_wast, "i64_trunc_s_f64", "f64.const -9223372036854775808"));
//IEEE float64中-2^63以下的最大值
   BOOST_REQUIRE_EQUAL(false, check(i64_overflow_wast, "i64_trunc_s_f64", "f64.const -9223372036854777856"));

////双精度=>uint64
   BOOST_REQUIRE_EQUAL(false, check(i64_overflow_wast, "i64_trunc_u_f64", "f64.const -1"));
   BOOST_REQUIRE_EQUAL(true, check(i64_overflow_wast, "i64_trunc_u_f64", "f64.const 0"));
//IEEE float64中最大值低于2^64
   BOOST_REQUIRE_EQUAL(true, check(i64_overflow_wast, "i64_trunc_u_f64", "f64.const 18446744073709549568"));
   BOOST_REQUIRE_EQUAL(false, check(i64_overflow_wast, "i64_trunc_u_f64", "f64.const 18446744073709551616"));
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(misaligned_tests, tester ) try {
   produce_blocks(2);
   create_accounts( {N(aligncheck)} );
   produce_block();

   auto check_aligned = [&]( auto wast ) {
      set_code(N(aligncheck), wast);
      produce_blocks(10);

      signed_transaction trx;
      action act;
      act.account = N(aligncheck);
      act.name = N();
      act.authorization = vector<permission_level>{{N(aligncheck),config::active_name}};
      trx.actions.push_back(act);

      set_transaction_headers(trx);
      trx.sign(get_private_key( N(aligncheck), "active" ), control->get_chain_id());
      push_transaction(trx);
      produce_block();

      BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx.id()));
   };

   check_aligned(aligned_ref_wast);
   check_aligned(misaligned_ref_wast);
   check_aligned(aligned_const_ref_wast);
   check_aligned(misaligned_const_ref_wast);
} FC_LOG_AND_RETHROW()

//测试加权CPU限制
BOOST_FIXTURE_TEST_CASE(weighted_cpu_limit_tests, tester ) try {
//要提高此测试的健壮性。
   resource_limits_manager mgr = control->get_mutable_resource_limits_manager();
   create_accounts( {N(f_tests)} );
   create_accounts( {N(acc2)} );
   bool pass = false;

   std::string code = R"=====(
(module
  (import "env" "require_auth" (func $require_auth (param i64)))
  (import "env" "eosio_assert" (func $eosio_assert (param i32 i32)))
   (table 0 anyfunc)
   (memory $0 1)
   (export "apply" (func $apply))
   (func $i64_trunc_u_f64 (param $0 f64) (result i64) (i64.trunc_u/f64 (get_local $0)))
   (func $test (param $0 i64))
   (func $apply (param $0 i64)(param $1 i64)(param $2 i64)
   )=====";
   for (int i = 0; i < 1024; ++i) {
      code += "(call $test (call $i64_trunc_u_f64 (f64.const 1)))\n";
   }
   code += "))";

   produce_blocks(1);
   set_code(N(f_tests), code.c_str());
   produce_blocks(10);

   mgr.set_account_limits(N(f_tests), -1, -1, 1);
   int count = 0;
   while (count < 4) {
      signed_transaction trx;

      for (int i = 0; i < 2; ++i) {
         action act;
         act.account = N(f_tests);
         act.name = N() + (i * 16);
         act.authorization = vector<permission_level>{{N(f_tests),config::active_name}};
         trx.actions.push_back(act);
      }

      set_transaction_headers(trx);
      trx.sign(get_private_key( N(f_tests), "active" ), control->get_chain_id());

      try {
         push_transaction(trx, fc::time_point::maximum(), 0);
         produce_block();
         BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx.id()));
         pass = true;
         count++;
      } catch( eosio::chain::leeway_deadline_exception& ) {
         BOOST_REQUIRE_EQUAL(count, 3);
         break;
      }
      BOOST_REQUIRE_EQUAL(true, validate());

if (count == 2) { //在acc2上增加一个很大的权重，使f_测试失去资源
        mgr.set_account_limits(N(acc2), -1, -1, 100000000);
      }
   }
   BOOST_REQUIRE_EQUAL(count, 3);
} FC_LOG_AND_RETHROW()

/*
 *确保正确使用wasm“start”方法
 **/

BOOST_FIXTURE_TEST_CASE( check_entry_behavior, TESTER ) try {
   produce_blocks(2);
   create_accounts( {N(entrycheck)} );
   produce_block();

   set_code(N(entrycheck), entry_wast);
   produce_blocks(10);

   signed_transaction trx;
   action act;
   act.account = N(entrycheck);
   act.name = N();
   act.authorization = vector<permission_level>{{N(entrycheck),config::active_name}};
   trx.actions.push_back(act);

   set_transaction_headers(trx);
   trx.sign(get_private_key( N(entrycheck), "active" ), control->get_chain_id());
   push_transaction(trx);
   produce_blocks(1);
   BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx.id()));
   const auto& receipt = get_transaction_receipt(trx.id());
   BOOST_CHECK_EQUAL(transaction_receipt::executed, receipt.status);
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( check_entry_behavior_2, TESTER ) try {
   produce_blocks(2);
   create_accounts( {N(entrycheck)} );
   produce_block();

   set_code(N(entrycheck), entry_wast_2);
   produce_blocks(10);

   signed_transaction trx;
   action act;
   act.account = N(entrycheck);
   act.name = N();
   act.authorization = vector<permission_level>{{N(entrycheck),config::active_name}};
   trx.actions.push_back(act);

   set_transaction_headers(trx);
   trx.sign(get_private_key( N(entrycheck), "active" ), control->get_chain_id());
   push_transaction(trx);
   produce_blocks(1);
   BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx.id()));
   const auto& receipt = get_transaction_receipt(trx.id());
   BOOST_CHECK_EQUAL(transaction_receipt::executed, receipt.status);
} FC_LOG_AND_RETHROW()


/*
 *确保我们可以加载不带内存的WASM
 **/

BOOST_FIXTURE_TEST_CASE( simple_no_memory_check, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(nomem)} );
   produce_block();

   set_code(N(nomem), simple_no_memory_wast);
   produce_blocks(1);

//简单内存的apply func试图用线性内存指针调用本机func
   signed_transaction trx;
   action act;
   act.account = N(nomem);
   act.name = N();
   act.authorization = vector<permission_level>{{N(nomem),config::active_name}};
   trx.actions.push_back(act);
   trx.expiration = control->head_block_time();
   set_transaction_headers(trx);
   trx.sign(get_private_key( N(nomem), "active" ), control->get_chain_id());
   BOOST_CHECK_THROW(push_transaction( trx ), wasm_execution_error);
} FC_LOG_AND_RETHROW()

//确保全局值都重置为初始值
BOOST_FIXTURE_TEST_CASE( check_global_reset, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(globalreset)} );
   produce_block();

   set_code(N(globalreset), mutable_global_wast);
   produce_blocks(1);

   signed_transaction trx;
   {
   action act;
   act.account = N(globalreset);
   act.name = name(0ULL);
   act.authorization = vector<permission_level>{{N(globalreset),config::active_name}};
   trx.actions.push_back(act);
   }
   {
   action act;
   act.account = N(globalreset);
   act.name = 1ULL;
   act.authorization = vector<permission_level>{{N(globalreset),config::active_name}};
   trx.actions.push_back(act);
   }

   set_transaction_headers(trx);
   trx.sign(get_private_key( N(globalreset), "active" ), control->get_chain_id());
   push_transaction(trx);
   produce_blocks(1);
   BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx.id()));
   const auto& receipt = get_transaction_receipt(trx.id());
   BOOST_CHECK_EQUAL(transaction_receipt::executed, receipt.status);
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( stl_test, TESTER ) try {
    produce_blocks(2);

    create_accounts( {N(stltest), N(alice), N(bob)} );
    produce_block();

    set_code(N(stltest), stltest_wast);
    set_abi(N(stltest), stltest_abi);
    produce_blocks(1);

    const auto& accnt  = control->db().get<account_object,by_name>( N(stltest) );
    abi_def abi;
    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
    abi_serializer abi_ser(abi, abi_serializer_max_time);

//发送消息
    {
        signed_transaction trx;
        action msg_act;
        msg_act.account = N(stltest);
        msg_act.name = N(message);
        msg_act.authorization = {{N(stltest), config::active_name}};
        msg_act.data = abi_ser.variant_to_binary("message", mutable_variant_object()
                                             ("from", "bob")
                                             ("to", "alice")
                                             ("message","Hi Alice!"),
                                             abi_serializer_max_time
                                             );
        trx.actions.push_back(std::move(msg_act));

        set_transaction_headers(trx);
        trx.sign(get_private_key(N(stltest), "active"), control->get_chain_id());
        push_transaction(trx);
        produce_block();

        BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx.id()));
    }
} FC_LOG_AND_RETHROW() ///STLTEST

//确保我们可以创建一个最大页数的WASM，但不能增加任何页数。
BOOST_FIXTURE_TEST_CASE( big_memory, TESTER ) try {
   produce_blocks(2);


   create_accounts( {N(bigmem)} );
   produce_block();

   string biggest_memory_wast_f = fc::format_string(biggest_memory_wast, fc::mutable_variant_object(
                                          "MAX_WASM_PAGES", eosio::chain::wasm_constraints::maximum_linear_memory/(64*1024)));

   set_code(N(bigmem), biggest_memory_wast_f.c_str());
   produce_blocks(1);

   signed_transaction trx;
   action act;
   act.account = N(bigmem);
   act.name = N();
   act.authorization = vector<permission_level>{{N(bigmem),config::active_name}};
   trx.actions.push_back(act);

   set_transaction_headers(trx);
   trx.sign(get_private_key( N(bigmem), "active" ), control->get_chain_id());
//但不能超过最大页面
   push_transaction(trx);

   produce_blocks(1);

   string too_big_memory_wast_f = fc::format_string(too_big_memory_wast, fc::mutable_variant_object(
                                          "MAX_WASM_PAGES_PLUS_ONE", eosio::chain::wasm_constraints::maximum_linear_memory/(64*1024)+1));
   BOOST_CHECK_THROW(set_code(N(bigmem), too_big_memory_wast_f.c_str()), eosio::chain::wasm_execution_error);

} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( table_init_tests, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(tableinit)} );
   produce_block();

   set_code(N(tableinit), valid_sparse_table);
   produce_blocks(1);

   BOOST_CHECK_THROW(set_code(N(tableinit), too_big_table), eosio::chain::wasm_execution_error);

} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( memory_init_border, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(memoryborder)} );
   produce_block();

   set_code(N(memoryborder), memory_init_borderline);
   produce_blocks(1);

   BOOST_CHECK_THROW(set_code(N(memoryborder), memory_init_toolong), eosio::chain::wasm_execution_error);
   BOOST_CHECK_THROW(set_code(N(memoryborder), memory_init_negative), eosio::chain::wasm_execution_error);

} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( imports, TESTER ) try {
   try {
      produce_blocks(2);

      create_accounts( {N(imports)} );
      produce_block();

//这将无法链接，但这没关系；主要是为了确保约束
//当内存和表只作为导入存在时，系统不会阻塞
      BOOST_CHECK_THROW(set_code(N(imports), memory_table_import), fc::exception);
   } catch ( const fc::exception& e ) {

        edump((e.to_detail_string()));
        throw;
   }

} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( nested_limit_test, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(nested)} );
   produce_block();

//嵌套循环
   {
      std::stringstream ss;
      ss << "(module (export \"apply\" (func $apply)) (func $apply (param $0 i64) (param $1 i64) (param $2 i64)";
      for(unsigned int i = 0; i < 1023; ++i)
         ss << "(loop (drop (i32.const " <<  i << "))";
      for(unsigned int i = 0; i < 1023; ++i)
         ss << ")";
      ss << "))";
      set_code(N(nested), ss.str().c_str());
   }
   {
      std::stringstream ss;
      ss << "(module (export \"apply\" (func $apply)) (func $apply (param $0 i64) (param $1 i64) (param $2 i64)";
      for(unsigned int i = 0; i < 1024; ++i)
         ss << "(loop (drop (i32.const " <<  i << "))";
      for(unsigned int i = 0; i < 1024; ++i)
         ss << ")";
      ss << "))";
      BOOST_CHECK_THROW(set_code(N(nested), ss.str().c_str()), eosio::chain::wasm_execution_error);
   }

//嵌套块
   {
      std::stringstream ss;
      ss << "(module (export \"apply\" (func $apply)) (func $apply (param $0 i64) (param $1 i64) (param $2 i64)";
      for(unsigned int i = 0; i < 1023; ++i)
         ss << "(block (drop (i32.const " <<  i << "))";
      for(unsigned int i = 0; i < 1023; ++i)
         ss << ")";
      ss << "))";
      set_code(N(nested), ss.str().c_str());
   }
   {
      std::stringstream ss;
      ss << "(module (export \"apply\" (func $apply)) (func $apply (param $0 i64) (param $1 i64) (param $2 i64)";
      for(unsigned int i = 0; i < 1024; ++i)
         ss << "(block (drop (i32.const " <<  i << "))";
      for(unsigned int i = 0; i < 1024; ++i)
         ss << ")";
      ss << "))";
      BOOST_CHECK_THROW(set_code(N(nested), ss.str().c_str()), eosio::chain::wasm_execution_error);
   }
//嵌套IFS
   {
      std::stringstream ss;
      ss << "(module (export \"apply\" (func $apply)) (func $apply (param $0 i64) (param $1 i64) (param $2 i64)";
      for(unsigned int i = 0; i < 1023; ++i)
         ss << "(if (i32.wrap/i64 (get_local $0)) (then (drop (i32.const " <<  i << "))";
      for(unsigned int i = 0; i < 1023; ++i)
         ss << "))";
      ss << "))";
      set_code(N(nested), ss.str().c_str());
   }
   {
      std::stringstream ss;
      ss << "(module (export \"apply\" (func $apply)) (func $apply (param $0 i64) (param $1 i64) (param $2 i64)";
      for(unsigned int i = 0; i < 1024; ++i)
         ss << "(if (i32.wrap/i64 (get_local $0)) (then (drop (i32.const " <<  i << "))";
      for(unsigned int i = 0; i < 1024; ++i)
         ss << "))";
      ss << "))";
      BOOST_CHECK_THROW(set_code(N(nested), ss.str().c_str()), eosio::chain::wasm_execution_error);
   }
//混合嵌套
   {
      std::stringstream ss;
      ss << "(module (export \"apply\" (func $apply)) (func $apply (param $0 i64) (param $1 i64) (param $2 i64)";
      for(unsigned int i = 0; i < 223; ++i)
         ss << "(if (i32.wrap/i64 (get_local $0)) (then (drop (i32.const " <<  i << "))";
      for(unsigned int i = 0; i < 400; ++i)
         ss << "(block (drop (i32.const " <<  i << "))";
      for(unsigned int i = 0; i < 400; ++i)
         ss << "(loop (drop (i32.const " <<  i << "))";
      for(unsigned int i = 0; i < 800; ++i)
         ss << ")";
      for(unsigned int i = 0; i < 223; ++i)
         ss << "))";
      ss << "))";
      set_code(N(nested), ss.str().c_str());
   }
   {
      std::stringstream ss;
      ss << "(module (export \"apply\" (func $apply)) (func $apply (param $0 i64) (param $1 i64) (param $2 i64)";
      for(unsigned int i = 0; i < 224; ++i)
         ss << "(if (i32.wrap/i64 (get_local $0)) (then (drop (i32.const " <<  i << "))";
      for(unsigned int i = 0; i < 400; ++i)
         ss << "(block (drop (i32.const " <<  i << "))";
      for(unsigned int i = 0; i < 400; ++i)
         ss << "(loop (drop (i32.const " <<  i << "))";
      for(unsigned int i = 0; i < 800; ++i)
         ss << ")";
      for(unsigned int i = 0; i < 224; ++i)
         ss << "))";
      ss << "))";
      BOOST_CHECK_THROW(set_code(N(nested), ss.str().c_str()), eosio::chain::wasm_execution_error);
   }

} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( lotso_globals, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(globals)} );
   produce_block();

   std::stringstream ss;
   ss << "(module (export \"apply\" (func $apply)) (func $apply (param $0 i64) (param $1 i64) (param $2 i64))";
   for(unsigned int i = 0; i < 85; ++i)
      ss << "(global $g" << i << " (mut i32) (i32.const 0))" << "(global $g" << i+100 << " (mut i64) (i64.const 0))";
//这给了我们1020字节的可变全局
//为了更好的衡量，加上一些不变的
   for(unsigned int i = 0; i < 10; ++i)
      ss << "(global $g" << i+200 << " i32 (i32.const 0))";

   set_code(N(globals),
      string(ss.str() + ")")
   .c_str());
//1024应该通过
   set_code(N(globals),
      string(ss.str() + "(global $z (mut i32) (i32.const -12)))")
   .c_str());
//1028应该失败
   BOOST_CHECK_THROW(set_code(N(globals),
      string(ss.str() + "(global $z (mut i64) (i64.const -12)))")
   .c_str()), eosio::chain::wasm_execution_error);

} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( offset_check, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(offsets)} );
   produce_block();

   vector<string> loadops = {
      "i32.load", "i64.load", "f32.load", "f64.load", "i32.load8_s", "i32.load8_u",
      "i32.load16_s", "i32.load16_u", "i64.load8_s", "i64.load8_u", "i64.load16_s",
      "i64.load16_u", "i64.load32_s", "i64.load32_u"
   };
   vector<vector<string>> storeops = {
      {"i32.store",   "i32"},
      {"i64.store",   "i64"},
      {"f32.store",   "f32"},
      {"f64.store",   "f64"},
      {"i32.store8",  "i32"},
      {"i32.store16", "i32"},
      {"i64.store8",  "i64"},
      {"i64.store16", "i64"},
      {"i64.store32", "i64"},
   };

   for(const string& s : loadops) {
      std::stringstream ss;
      ss << "(module (memory $0 " << eosio::chain::wasm_constraints::maximum_linear_memory/(64*1024) << ") (func $apply (param $0 i64) (param $1 i64) (param $2 i64)";
      ss << "(drop (" << s << " offset=" << eosio::chain::wasm_constraints::maximum_linear_memory-2 << " (i32.const 0)))";
      ss << ") (export \"apply\" (func $apply)) )";

      set_code(N(offsets), ss.str().c_str());
      produce_block();
   }
   for(const vector<string>& o : storeops) {
      std::stringstream ss;
      ss << "(module (memory $0 " << eosio::chain::wasm_constraints::maximum_linear_memory/(64*1024) << ") (func $apply (param $0 i64) (param $1 i64) (param $2 i64)";
      ss << "(" << o[0] << " offset=" << eosio::chain::wasm_constraints::maximum_linear_memory-2 << " (i32.const 0) (" << o[1] << ".const 0))";
      ss << ") (export \"apply\" (func $apply)) )";

      set_code(N(offsets), ss.str().c_str());
      produce_block();
   }

   for(const string& s : loadops) {
      std::stringstream ss;
      ss << "(module (memory $0 " << eosio::chain::wasm_constraints::maximum_linear_memory/(64*1024) << ") (func $apply (param $0 i64) (param $1 i64) (param $2 i64)";
      ss << "(drop (" << s << " offset=" << eosio::chain::wasm_constraints::maximum_linear_memory+4 << " (i32.const 0)))";
      ss << ") (export \"apply\" (func $apply)) )";

      BOOST_CHECK_THROW(set_code(N(offsets), ss.str().c_str()), eosio::chain::wasm_execution_error);
      produce_block();
   }
   for(const vector<string>& o : storeops) {
      std::stringstream ss;
      ss << "(module (memory $0 " << eosio::chain::wasm_constraints::maximum_linear_memory/(64*1024) << ") (func $apply (param $0 i64) (param $1 i64) (param $2 i64)";
      ss << "(" << o[0] << " offset=" << eosio::chain::wasm_constraints::maximum_linear_memory+4 << " (i32.const 0) (" << o[1] << ".const 0))";
      ss << ") (export \"apply\" (func $apply)) )";

      BOOST_CHECK_THROW(set_code(N(offsets), ss.str().c_str()), eosio::chain::wasm_execution_error);
      produce_block();
   }

} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE(noop, TESTER) try {
   produce_blocks(2);
   create_accounts( {N(noop), N(alice)} );
   produce_block();

   set_code(N(noop), noop_wast);

   set_abi(N(noop), noop_abi);
   const auto& accnt  = control->db().get<account_object,by_name>(N(noop));
   abi_def abi;
   BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
   abi_serializer abi_ser(abi, abi_serializer_max_time);

   {
      produce_blocks(5);
      signed_transaction trx;
      action act;
      act.account = N(noop);
      act.name = N(anyaction);
      act.authorization = vector<permission_level>{{N(noop), config::active_name}};

      act.data = abi_ser.variant_to_binary("anyaction", mutable_variant_object()
                                           ("from", "noop")
                                           ("type", "some type")
                                           ("data", "some data goes here"),
                                           abi_serializer_max_time
                                           );

      trx.actions.emplace_back(std::move(act));

      set_transaction_headers(trx);
      trx.sign(get_private_key(N(noop), "active"), control->get_chain_id());
      push_transaction(trx);
      produce_block();

      BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx.id()));
   }

   {
      produce_blocks(5);
      signed_transaction trx;
      action act;
      act.account = N(noop);
      act.name = N(anyaction);
      act.authorization = vector<permission_level>{{N(alice), config::active_name}};

      act.data = abi_ser.variant_to_binary("anyaction", mutable_variant_object()
                                           ("from", "alice")
                                           ("type", "some type")
                                           ("data", "some data goes here"),
                                           abi_serializer_max_time
                                           );

      trx.actions.emplace_back(std::move(act));

      set_transaction_headers(trx);
      trx.sign(get_private_key(N(alice), "active"), control->get_chain_id());
      push_transaction(trx);
      produce_block();

      BOOST_REQUIRE_EQUAL(true, chain_has_transaction(trx.id()));
   }

 } FC_LOG_AND_RETHROW()

//ABI_序列化程序：：to_variant失败，因为eosio_系统通过set_abi修改了abi。
//此测试还验证链初始值设定项：：eos_contract_abi（）不冲突
//使用eosio系统时，不允许包含重复项。
BOOST_FIXTURE_TEST_CASE(eosio_abi, TESTER) try {
   produce_blocks(2);

   const auto& accnt  = control->db().get<account_object,by_name>(config::system_account_name);
   abi_def abi;
   BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
   abi_serializer abi_ser(abi, abi_serializer_max_time);

   signed_transaction trx;
   name a = N(alice);
   authority owner_auth =  authority( get_public_key( a, "owner" ) );
   trx.actions.emplace_back( vector<permission_level>{{config::system_account_name,config::active_name}},
                             newaccount{
                                   .creator  = config::system_account_name,
                                   .name     = a,
                                   .owner    = owner_auth,
                                   .active   = authority( get_public_key( a, "active" ) )
                             });
   set_transaction_headers(trx);
   trx.sign( get_private_key( config::system_account_name, "active" ), control->get_chain_id()  );
   auto result = push_transaction( trx );

   fc::variant pretty_output;
//验证到EOS本地合同类型：newaccount上的变量工作
//请参阅abi_serializer:：to_abi（）。
   abi_serializer::to_variant(*result, pretty_output, get_resolver(), abi_serializer_max_time);

   BOOST_TEST(fc::json::to_string(pretty_output).find("newaccount") != std::string::npos);

   produce_block();
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( check_big_deserialization, TESTER ) try {
   produce_blocks(2);
   create_accounts( {N(cbd)} );
   produce_block();

   std::stringstream ss;
   ss << "(module ";
   ss << "(export \"apply\" (func $apply))";
   ss << "  (func $apply  (param $0 i64)(param $1 i64)(param $2 i64))";
   for(unsigned int i = 0; i < wasm_constraints::maximum_section_elements-2; i++)
      ss << "  (func " << "$AA_" << i << ")";
   ss << ")";

   set_code(N(cbd), ss.str().c_str());
   produce_blocks(1);

   produce_blocks(1);

   ss.str("");
   ss << "(module ";
   ss << "(export \"apply\" (func $apply))";
   ss << "  (func $apply  (param $0 i64)(param $1 i64)(param $2 i64))";
   for(unsigned int i = 0; i < wasm_constraints::maximum_section_elements; i++)
      ss << "  (func " << "$AA_" << i << ")";
   ss << ")";

   BOOST_CHECK_THROW(set_code(N(cbd), ss.str().c_str()), wasm_serialization_error);
   produce_blocks(1);

   ss.str("");
   ss << "(module ";
   ss << "(export \"apply\" (func $apply))";
   ss << "  (func $apply  (param $0 i64)(param $1 i64)(param $2 i64))";
   ss << "  (func $aa ";
   for(unsigned int i = 0; i < wasm_constraints::maximum_code_size; i++)
      ss << "  (drop (i32.const 3))";
   ss << "))";

BOOST_CHECK_THROW(set_code(N(cbd), ss.str().c_str()), fc::assert_exception); //这首先由数组检查的最大值捕获
   produce_blocks(1);

   ss.str("");
   ss << "(module ";
   ss << "(memory $0 1)";
   ss << "(data (i32.const 20) \"";
   for(unsigned int i = 0; i < wasm_constraints::maximum_func_local_bytes-1; i++)
      ss << 'a';
   ss << "\")";
   ss << "(export \"apply\" (func $apply))";
   ss << "  (func $apply  (param $0 i64)(param $1 i64)(param $2 i64))";
   ss << "  (func $aa ";
      ss << "  (drop (i32.const 3))";
   ss << "))";

   set_code(N(cbd), ss.str().c_str());
   produce_blocks(1);

   ss.str("");
   ss << "(module ";
   ss << "(memory $0 1)";
   ss << "(data (i32.const 20) \"";
   for(unsigned int i = 0; i < wasm_constraints::maximum_func_local_bytes; i++)
      ss << 'a';
   ss << "\")";
   ss << "(export \"apply\" (func $apply))";
   ss << "  (func $apply  (param $0 i64)(param $1 i64)(param $2 i64))";
   ss << "  (func $aa ";
      ss << "  (drop (i32.const 3))";
   ss << "))";

   BOOST_CHECK_THROW(set_code(N(cbd), ss.str().c_str()), wasm_serialization_error);
   produce_blocks(1);

} FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE( check_table_maximum, TESTER ) try {
   produce_blocks(2);
   create_accounts( {N(tbl)} );
   produce_block();

   set_code(N(tbl), table_checker_wast);
   produce_blocks(1);
   {
   signed_transaction trx;
   action act;
act.name = 555ULL<<32 | 0ULL;       //前32位是我们断言的，后32位是间接调用索引
   act.account = N(tbl);
   act.authorization = vector<permission_level>{{N(tbl),config::active_name}};
   trx.actions.push_back(act);
      set_transaction_headers(trx);
   trx.sign(get_private_key( N(tbl), "active" ), control->get_chain_id());
   push_transaction(trx);
   }

   produce_blocks(1);

   {
   signed_transaction trx;
   action act;
act.name = 555ULL<<32 | 1022ULL;       //前32位是我们断言的，后32位是间接调用索引
   act.account = N(tbl);
   act.authorization = vector<permission_level>{{N(tbl),config::active_name}};
   trx.actions.push_back(act);
      set_transaction_headers(trx);
   trx.sign(get_private_key( N(tbl), "active" ), control->get_chain_id());
   push_transaction(trx);
   }

   produce_blocks(1);

   {
   signed_transaction trx;
   action act;
act.name = 7777ULL<<32 | 1023ULL;       //前32位是我们断言的，后32位是间接调用索引
   act.account = N(tbl);
   act.authorization = vector<permission_level>{{N(tbl),config::active_name}};
   trx.actions.push_back(act);
      set_transaction_headers(trx);
   trx.sign(get_private_key( N(tbl), "active" ), control->get_chain_id());
   push_transaction(trx);
   }

   produce_blocks(1);

   {
   signed_transaction trx;
   action act;
act.name = 7778ULL<<32 | 1023ULL;       //前32位是我们断言的，后32位是间接调用索引
   act.account = N(tbl);
   act.authorization = vector<permission_level>{{N(tbl),config::active_name}};
   trx.actions.push_back(act);
      set_transaction_headers(trx);
   trx.sign(get_private_key( N(tbl), "active" ), control->get_chain_id());

//如果失败，则检查以确保正确评估WASM中的assert（）。
   BOOST_CHECK_THROW(push_transaction(trx), eosio_assert_message_exception);
   }

   produce_blocks(1);

   {
   signed_transaction trx;
   action act;
act.name = 133ULL<<32 | 5ULL;       //前32位是我们断言的，后32位是间接调用索引
   act.account = N(tbl);
   act.authorization = vector<permission_level>{{N(tbl),config::active_name}};
   trx.actions.push_back(act);
      set_transaction_headers(trx);
   trx.sign(get_private_key( N(tbl), "active" ), control->get_chain_id());

//如果失败，则此元素索引（5）不存在
   BOOST_CHECK_THROW(push_transaction(trx), eosio::chain::wasm_execution_error);
   }

   produce_blocks(1);

   {
   signed_transaction trx;
   action act;
   act.name = eosio::chain::wasm_constraints::maximum_table_elements+54334;
   act.account = N(tbl);
   act.authorization = vector<permission_level>{{N(tbl),config::active_name}};
   trx.actions.push_back(act);
      set_transaction_headers(trx);
   trx.sign(get_private_key( N(tbl), "active" ), control->get_chain_id());

//如果失败，则此元素索引超出范围
   BOOST_CHECK_THROW(push_transaction(trx), eosio::chain::wasm_execution_error);
   }

   produce_blocks(1);

//用新的、正确的语法运行一些测试，调用
   set_code(N(tbl), table_checker_proper_syntax_wast);
   produce_blocks(1);

   {
   signed_transaction trx;
   action act;
act.name = 555ULL<<32 | 1022ULL;       //前32位是我们断言的，后32位是间接调用索引
   act.account = N(tbl);
   act.authorization = vector<permission_level>{{N(tbl),config::active_name}};
   trx.actions.push_back(act);
      set_transaction_headers(trx);
   trx.sign(get_private_key( N(tbl), "active" ), control->get_chain_id());
   push_transaction(trx);
   }

   produce_blocks(1);
   {
   signed_transaction trx;
   action act;
act.name = 7777ULL<<32 | 1023ULL;       //前32位是我们断言的，后32位是间接调用索引
   act.account = N(tbl);
   act.authorization = vector<permission_level>{{N(tbl),config::active_name}};
   trx.actions.push_back(act);
   set_transaction_headers(trx);
   trx.sign(get_private_key( N(tbl), "active" ), control->get_chain_id());
   push_transaction(trx);
   }
   set_code(N(tbl), table_checker_small_wast);
   produce_blocks(1);

   {
   signed_transaction trx;
   action act;
   act.name = 888ULL;
   act.account = N(tbl);
   act.authorization = vector<permission_level>{{N(tbl),config::active_name}};
   trx.actions.push_back(act);
   set_transaction_headers(trx);
   trx.sign(get_private_key( N(tbl), "active" ), control->get_chain_id());

//超出范围且也没有mmap访问权限的元素（应该是被捕获的segv）
   BOOST_CHECK_EXCEPTION(push_transaction(trx), eosio::chain::wasm_execution_error, [](const eosio::chain::wasm_execution_error &e) {return true;});
   }
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( protected_globals, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(gob)} );
   produce_block();

   BOOST_CHECK_THROW(set_code(N(gob), global_protection_none_get_wast), fc::exception);
   produce_blocks(1);

   BOOST_CHECK_THROW(set_code(N(gob), global_protection_some_get_wast), fc::exception);
   produce_blocks(1);

   BOOST_CHECK_THROW(set_code(N(gob), global_protection_none_set_wast), fc::exception);
   produce_blocks(1);

   BOOST_CHECK_THROW(set_code(N(gob), global_protection_some_set_wast), fc::exception);
   produce_blocks(1);

//保持清醒，确保我得到了一般的二元结构，好吗？
   set_code(N(gob), global_protection_okay_get_wasm);
   produce_blocks(1);

   BOOST_CHECK_THROW(set_code(N(gob), global_protection_none_get_wasm), fc::exception);
   produce_blocks(1);

   BOOST_CHECK_THROW(set_code(N(gob), global_protection_some_get_wasm), fc::exception);
   produce_blocks(1);

   set_code(N(gob), global_protection_okay_set_wasm);
   produce_blocks(1);

   BOOST_CHECK_THROW(set_code(N(gob), global_protection_some_set_wasm), fc::exception);
   produce_blocks(1);
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( lotso_stack_1, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(stackz)} );
   produce_block();

   {
   std::stringstream ss;
   ss << "(module ";
   ss << "(export \"apply\" (func $apply))";
   ss << "  (func $apply  (param $0 i64)(param $1 i64)(param $2 i64))";
   ss << "  (func ";
   for(unsigned int i = 0; i < wasm_constraints::maximum_func_local_bytes; i+=4)
      ss << "(local i32)";
   ss << "  )";
   ss << ")";
   set_code(N(stackz), ss.str().c_str());
   produce_blocks(1);
   }
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( lotso_stack_2, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(stackz)} );
   produce_block();
   {
   std::stringstream ss;
   ss << "(module ";
   ss << "(import \"env\" \"require_auth\" (func $require_auth (param i64)))";
   ss << "(export \"apply\" (func $apply))";
   ss << "  (func $apply  (param $0 i64)(param $1 i64)(param $2 i64) (call $require_auth (i64.const 14288945783897063424)))";
   ss << "  (func ";
   for(unsigned int i = 0; i < wasm_constraints::maximum_func_local_bytes; i+=8)
      ss << "(local f64)";
   ss << "  )";
   ss << ")";
   set_code(N(stackz), ss.str().c_str());
   produce_blocks(1);
   }
} FC_LOG_AND_RETHROW()
BOOST_FIXTURE_TEST_CASE( lotso_stack_3, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(stackz)} );
   produce_block();

//尝试使用与这许多本地人的契约（以便实际编译它）。注意
//此时，没有apply（）是可以接受的非错误。
   {
   signed_transaction trx;
   action act;
   act.account = N(stackz);
   act.name = N();
   act.authorization = vector<permission_level>{{N(stackz),config::active_name}};
   trx.actions.push_back(act);

      set_transaction_headers(trx);
   trx.sign(get_private_key( N(stackz), "active" ), control->get_chain_id());
   push_transaction(trx);
   }
} FC_LOG_AND_RETHROW()
BOOST_FIXTURE_TEST_CASE( lotso_stack_4, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(stackz)} );
   produce_block();
//当地人太多了！验证失败
   {
   std::stringstream ss;
   ss << "(module ";
   ss << "(export \"apply\" (func $apply))";
   ss << "  (func $apply  (param $0 i64) (param $1 i64) (param $2 i64))";
   ss << "  (func ";
   for(unsigned int i = 0; i < wasm_constraints::maximum_func_local_bytes+4; i+=4)
      ss << "(local i32)";
   ss << "  )";
   ss << ")";
   BOOST_CHECK_THROW(set_code(N(stackz), ss.str().c_str()), fc::exception);
   produce_blocks(1);
   }
} FC_LOG_AND_RETHROW()
BOOST_FIXTURE_TEST_CASE( lotso_stack_5, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(stackz)} );
   produce_block();

//重试，但使用参数
   {
   std::stringstream ss;
   ss << "(module ";
   ss << "(import \"env\" \"require_auth\" (func $require_auth (param i64)))";
   ss << "(export \"apply\" (func $apply))";
   ss << "  (func $apply  (param $0 i64)(param $1 i64)(param $2 i64) (call $require_auth (i64.const 14288945783897063424)))";
   ss << "  (func ";
   for(unsigned int i = 0; i < wasm_constraints::maximum_func_local_bytes; i+=4)
      ss << "(param i32)";
   ss << "  )";
   ss << ")";
   set_code(N(stackz), ss.str().c_str());
   produce_blocks(1);
   }
} FC_LOG_AND_RETHROW()
BOOST_FIXTURE_TEST_CASE( lotso_stack_6, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(stackz)} );
   produce_block();

//尝试使用具有这么多参数的契约
   {
   signed_transaction trx;
   action act;
   act.account = N(stackz);
   act.name = N();
   act.authorization = vector<permission_level>{{N(stackz),config::active_name}};
   trx.actions.push_back(act);

      set_transaction_headers(trx);
   trx.sign(get_private_key( N(stackz), "active" ), control->get_chain_id());
   push_transaction(trx);
   }
} FC_LOG_AND_RETHROW()
BOOST_FIXTURE_TEST_CASE( lotso_stack_7, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(stackz)} );
   produce_block();

//参数太多！
   {
   std::stringstream ss;
   ss << "(module ";
   ss << "(export \"apply\" (func $apply))";
   ss << "  (func $apply  (param $0 i64) (param $1 i64) (param $2 i64))";
   ss << "  (func ";
   for(unsigned int i = 0; i < wasm_constraints::maximum_func_local_bytes+4; i+=4)
      ss << "(param i32)";
   ss << "  )";
   ss << ")";
   BOOST_CHECK_THROW(set_code(N(stackz), ss.str().c_str()), fc::exception);
   produce_blocks(1);
   }
} FC_LOG_AND_RETHROW()
BOOST_FIXTURE_TEST_CASE( lotso_stack_8, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(stackz)} );
   produce_block();

//让我们混合参数和本地变量，确保它在混合情况下正确计数。
   {
   std::stringstream ss;
   ss << "(module ";
   ss << "(export \"apply\" (func $apply))";
   ss << "  (func $apply  (param $0 i64) (param $1 i64) (param $2 i64))";
   ss << "  (func (param i64) (param f32) ";
   for(unsigned int i = 12; i < wasm_constraints::maximum_func_local_bytes; i+=4)
      ss << "(local i32)";
   ss << "  )";
   ss << ")";
   set_code(N(stackz), ss.str().c_str());
   produce_blocks(1);
   }
} FC_LOG_AND_RETHROW()
BOOST_FIXTURE_TEST_CASE( lotso_stack_9, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(stackz)} );
   produce_block();

   {
   std::stringstream ss;
   ss << "(module ";
   ss << "(export \"apply\" (func $apply))";
   ss << "  (func $apply  (param $0 i64) (param $1 i64) (param $2 i64))";
   ss << "  (func (param i64) (param f32) ";
   for(unsigned int i = 12; i < wasm_constraints::maximum_func_local_bytes+4; i+=4)
      ss << "(local f32)";
   ss << "  )";
   ss << ")";
   BOOST_CHECK_THROW(set_code(N(stackz), ss.str().c_str()), fc::exception);
   produce_blocks(1);
   }
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( apply_export_and_signature, TESTER ) try {
   produce_blocks(2);
   create_accounts( {N(bbb)} );
   produce_block();

   BOOST_CHECK_THROW(set_code(N(bbb), no_apply_wast), fc::exception);
   produce_blocks(1);

   BOOST_CHECK_THROW(set_code(N(bbb), no_apply_2_wast), fc::exception);
   produce_blocks(1);

   BOOST_CHECK_THROW(set_code(N(bbb), no_apply_3_wast), fc::exception);
   produce_blocks(1);

   BOOST_CHECK_THROW(set_code(N(bbb), apply_wrong_signature_wast), fc::exception);
   produce_blocks(1);
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( trigger_serialization_errors, TESTER) try {
   produce_blocks(2);
   const vector<uint8_t> proper_wasm = { 0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x0d, 0x02, 0x60, 0x03, 0x7f, 0x7f, 0x7f,
                                         0x00, 0x60, 0x03, 0x7e, 0x7e, 0x7e, 0x00, 0x02, 0x0e, 0x01, 0x03, 0x65, 0x6e, 0x76, 0x06, 0x73,
                                         0x68, 0x61, 0x32, 0x35, 0x36, 0x00, 0x00, 0x03, 0x02, 0x01, 0x01, 0x04, 0x04, 0x01, 0x70, 0x00,
                                         0x00, 0x05, 0x03, 0x01, 0x00, 0x20, 0x07, 0x09, 0x01, 0x05, 0x61, 0x70, 0x70, 0x6c, 0x79, 0x00,
                                         0x01, 0x0a, 0x0c, 0x01, 0x0a, 0x00, 0x41, 0x04, 0x41, 0x05, 0x41, 0x10, 0x10, 0x00, 0x0b, 0x0b,
                                         0x0b, 0x01, 0x00, 0x41, 0x04, 0x0b, 0x05, 0x68, 0x65, 0x6c, 0x6c, 0x6f };

   const vector<uint8_t> malformed_wasm = { 0x00, 0x61, 0x03, 0x0d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x0d, 0x02, 0x60, 0x03, 0x7f, 0x7f, 0x7f,
                                            0x00, 0x60, 0x03, 0x7e, 0x7e, 0x7e, 0x00, 0x02, 0x0e, 0x01, 0x03, 0x65, 0x6e, 0x76, 0x06, 0x73,
                                            0x68, 0x61, 0x32, 0x38, 0x36, 0x00, 0x00, 0x03, 0x03, 0x01, 0x01, 0x04, 0x04, 0x01, 0x70, 0x00,
                                            0x00, 0x05, 0x03, 0x01, 0x00, 0x20, 0x07, 0x09, 0x01, 0x05, 0x61, 0x70, 0x70, 0x6c, 0x79, 0x00,
                                            0x01, 0x0a, 0x0c, 0x01, 0x0a, 0x00, 0x41, 0x04, 0x41, 0x05, 0x41, 0x10, 0x10, 0x00, 0x0b, 0x0b,
                                            0x0b, 0x01, 0x00, 0x41, 0x04, 0x0b, 0x05, 0x68, 0x65, 0x6c, 0x6c, 0x6f };

   create_accounts( {N(bbb)} );
   produce_block();

   set_code(N(bbb), proper_wasm);
   BOOST_CHECK_THROW(set_code(N(bbb), malformed_wasm), wasm_serialization_error);
   produce_blocks(1);
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( protect_injected, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(inj)} );
   produce_block();

   BOOST_CHECK_THROW(set_code(N(inj), import_injected_wast), fc::exception);
   produce_blocks(1);
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( mem_growth_memset, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(grower)} );
   produce_block();

   action act;
   act.account = N(grower);
   act.name = N();
   act.authorization = vector<permission_level>{{N(grower),config::active_name}};

   set_code(N(grower), memory_growth_memset_store);
   {
      signed_transaction trx;
      trx.actions.push_back(act);
      set_transaction_headers(trx);
      trx.sign(get_private_key( N(grower), "active" ), control->get_chain_id());
      push_transaction(trx);
   }

   produce_blocks(1);
   set_code(N(grower), memory_growth_memset_test);
   {
      signed_transaction trx;
      trx.actions.push_back(act);
      set_transaction_headers(trx);
      trx.sign(get_private_key( N(grower), "active" ), control->get_chain_id());
      push_transaction(trx);
   }
} FC_LOG_AND_RETHROW()

INCBIN(fuzz1, "fuzz1.wasm");
INCBIN(fuzz2, "fuzz2.wasm");
INCBIN(fuzz3, "fuzz3.wasm");
INCBIN(fuzz4, "fuzz4.wasm");
INCBIN(fuzz5, "fuzz5.wasm");
INCBIN(fuzz6, "fuzz6.wasm");
INCBIN(fuzz7, "fuzz7.wasm");
INCBIN(fuzz8, "fuzz8.wasm");
INCBIN(fuzz9, "fuzz9.wasm");
INCBIN(fuzz10, "fuzz10.wasm");
INCBIN(fuzz11, "fuzz11.wasm");
INCBIN(fuzz12, "fuzz12.wasm");
INCBIN(fuzz13, "fuzz13.wasm");
INCBIN(fuzz14, "fuzz14.wasm");
INCBIN(fuzz15, "fuzz15.wasm");
//incbin（fuzz13，“fuzz13.wasm”）；
INCBIN(big_allocation, "big_allocation.wasm");
INCBIN(crash_section_size_too_big, "crash_section_size_too_big.wasm");
INCBIN(leak_no_destructor, "leak_no_destructor.wasm");
INCBIN(leak_readExports, "leak_readExports.wasm");
INCBIN(leak_readFunctions, "leak_readFunctions.wasm");
INCBIN(leak_readFunctions_2, "leak_readFunctions_2.wasm");
INCBIN(leak_readFunctions_3, "leak_readFunctions_3.wasm");
INCBIN(leak_readGlobals, "leak_readGlobals.wasm");
INCBIN(leak_readImports, "leak_readImports.wasm");
INCBIN(leak_wasm_binary_cpp_L1249, "leak_wasm_binary_cpp_L1249.wasm");
INCBIN(readFunctions_slowness_out_of_memory, "readFunctions_slowness_out_of_memory.wasm");
INCBIN(locals_yc, "locals-yc.wasm");
INCBIN(locals_s, "locals-s.wasm");
INCBIN(slowwasm_localsets, "slowwasm_localsets.wasm");
INCBIN(getcode_deepindent, "getcode_deepindent.wasm");
INCBIN(indent_mismatch, "indent-mismatch.wasm");
INCBIN(deep_loops_ext_report, "deep_loops_ext_report.wasm");
INCBIN(80k_deep_loop_with_ret, "80k_deep_loop_with_ret.wasm");
INCBIN(80k_deep_loop_with_void, "80k_deep_loop_with_void.wasm");

BOOST_FIXTURE_TEST_CASE( fuzz, TESTER ) try {
   produce_blocks(2);

   create_accounts( {N(fuzzy)} );
   produce_block();

   {
      vector<uint8_t> wasm(gfuzz1Data, gfuzz1Data + gfuzz1Size);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), fc::exception);
   }
   {
      vector<uint8_t> wasm(gfuzz2Data, gfuzz2Data + gfuzz2Size);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), fc::exception);
   }
   {
      vector<uint8_t> wasm(gfuzz3Data, gfuzz3Data + gfuzz3Size);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), fc::exception);
   }
   {
      vector<uint8_t> wasm(gfuzz4Data, gfuzz4Data + gfuzz4Size);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), fc::exception);
   }
   {
      vector<uint8_t> wasm(gfuzz5Data, gfuzz5Data + gfuzz5Size);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), fc::exception);
   }
   {
      vector<uint8_t> wasm(gfuzz6Data, gfuzz6Data + gfuzz6Size);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), fc::exception);
   }
   {
      vector<uint8_t> wasm(gfuzz7Data, gfuzz7Data + gfuzz7Size);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), fc::exception);
   }
   {
      vector<uint8_t> wasm(gfuzz8Data, gfuzz8Data + gfuzz8Size);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), fc::exception);
   }
   {
      vector<uint8_t> wasm(gfuzz9Data, gfuzz9Data + gfuzz9Size);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), fc::exception);
   }
   {
      vector<uint8_t> wasm(gfuzz10Data, gfuzz10Data + gfuzz10Size);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), fc::exception);
   }
   {
      vector<uint8_t> wasm(gfuzz11Data, gfuzz11Data + gfuzz11Size);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), fc::exception);
   }
   {
      vector<uint8_t> wasm(gfuzz12Data, gfuzz12Data + gfuzz12Size);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), fc::exception);
   }
   {
      vector<uint8_t> wasm(gfuzz13Data, gfuzz13Data + gfuzz13Size);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), fc::exception);
   }
   {
      vector<uint8_t> wasm(gfuzz14Data, gfuzz14Data + gfuzz14Size);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), fc::exception);
   }
      {
      vector<uint8_t> wasm(gfuzz15Data, gfuzz15Data + gfuzz15Size);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), fc::exception);
   }
   /*TODO:更新wasm使apply（…）然后调用，声明是
    *在wavm下需要1.6秒…
   {
      自动启动=fc:：time_point:：now（）；
      vector<uint8_t>wasm（gfuzz13data，gfuzz13data+gfuzz13size）；
      设置代码（n（模糊），wasm）；
      boost_check_throw（set_code（n（fuzzy），wasm），fc:：exception）；
      自动结束=fc:：time_point:：now（）；
      edump（（结束-开始））；
   }
   **/


   {
      vector<uint8_t> wasm(gbig_allocationData, gbig_allocationData + gbig_allocationSize);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), wasm_serialization_error);
   }
   {
      vector<uint8_t> wasm(gcrash_section_size_too_bigData, gcrash_section_size_too_bigData + gcrash_section_size_too_bigSize);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), wasm_serialization_error);
   }
   {
      vector<uint8_t> wasm(gleak_no_destructorData, gleak_no_destructorData + gleak_no_destructorSize);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), wasm_serialization_error);
   }
   {
      vector<uint8_t> wasm(gleak_readExportsData, gleak_readExportsData + gleak_readExportsSize);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), wasm_serialization_error);
   }
   {
      vector<uint8_t> wasm(gleak_readFunctionsData, gleak_readFunctionsData + gleak_readFunctionsSize);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), wasm_serialization_error);
   }
   {
      vector<uint8_t> wasm(gleak_readFunctions_2Data, gleak_readFunctions_2Data + gleak_readFunctions_2Size);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), wasm_serialization_error);
   }
   {
      vector<uint8_t> wasm(gleak_readFunctions_3Data, gleak_readFunctions_3Data + gleak_readFunctions_3Size);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), wasm_serialization_error);
   }
   {
      vector<uint8_t> wasm(gleak_readGlobalsData, gleak_readGlobalsData + gleak_readGlobalsSize);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), wasm_serialization_error);
   }
   {
      vector<uint8_t> wasm(gleak_readImportsData, gleak_readImportsData + gleak_readImportsSize);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), wasm_serialization_error);
   }
   {
      vector<uint8_t> wasm(gleak_wasm_binary_cpp_L1249Data, gleak_wasm_binary_cpp_L1249Data + gleak_wasm_binary_cpp_L1249Size);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), wasm_serialization_error);
   }
   {
      vector<uint8_t> wasm(greadFunctions_slowness_out_of_memoryData, greadFunctions_slowness_out_of_memoryData + greadFunctions_slowness_out_of_memorySize);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), wasm_serialization_error);
   }
   {
      vector<uint8_t> wasm(glocals_ycData, glocals_ycData + glocals_ycSize);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), wasm_serialization_error);
   }
   {
      vector<uint8_t> wasm(glocals_sData, glocals_sData + glocals_sSize);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), wasm_serialization_error);
   }
   {
      vector<uint8_t> wasm(gslowwasm_localsetsData, gslowwasm_localsetsData + gslowwasm_localsetsSize);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), wasm_serialization_error);
   }
   {
      vector<uint8_t> wasm(gdeep_loops_ext_reportData, gdeep_loops_ext_reportData + gdeep_loops_ext_reportSize);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), wasm_execution_error);
   }
   {
      vector<uint8_t> wasm(g80k_deep_loop_with_retData, g80k_deep_loop_with_retData + g80k_deep_loop_with_retSize);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), wasm_execution_error);
   }
   {
      vector<uint8_t> wasm(g80k_deep_loop_with_voidData, g80k_deep_loop_with_voidData + g80k_deep_loop_with_voidSize);
      BOOST_CHECK_THROW(set_code(N(fuzzy), wasm), wasm_execution_error);
   }

   produce_blocks(1);
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( getcode_checks, TESTER ) try {
   vector<uint8_t> wasm(ggetcode_deepindentData, ggetcode_deepindentData + ggetcode_deepindentSize);
   wasm_to_wast( wasm.data(), wasm.size(), true );
   vector<uint8_t> wasmx(gindent_mismatchData, gindent_mismatchData + gindent_mismatchSize);
   wasm_to_wast( wasmx.data(), wasmx.size(), true );
} FC_LOG_AND_RETHROW()


//TODO:还原网络使用情况测试
#if 0
BOOST_FIXTURE_TEST_CASE(net_usage_tests, tester ) try {
   int count = 0;
   auto check = [&](int coderepeat, int max_net_usage)-> bool {
      account_name account = N(f_tests) + (count++) * 16;
      create_accounts({account});

      std::string code = R"=====(
   (module
   (import "env" "require_auth" (func $require_auth (param i64)))
   (import "env" "eosio_assert" (func $eosio_assert (param i32 i32)))
      (table 0 anyfunc)
      (memory $0 1)
      (export "apply" (func $apply))
      (func $i64_trunc_u_f64 (param $0 f64) (result i64) (i64.trunc_u/f64 (get_local $0)))
      (func $test (param $0 i64))
      (func $apply (param $0 i64)(param $1 i64)(param $2 i64)
      )=====";
      for (int i = 0; i < coderepeat; ++i) {
         code += "(call $test (call $i64_trunc_u_f64 (f64.const 1)))\n";
      }
      code += "))";
      produce_blocks(1);
      signed_transaction trx;
      auto wasm = ::eosio::chain::wast_to_wasm(code);
      trx.actions.emplace_back( vector<permission_level>{{account,config::active_name}},
                              setcode{
                                 .account    = account,
                                 .vmtype     = 0,
                                 .vmversion  = 0,
                                 .code       = bytes(wasm.begin(), wasm.end())
                              });
      set_transaction_headers(trx);
      if (max_net_usage) trx.max_net_usage_words = max_net_usage;
      trx.sign( get_private_key( account, "active" ), control->get_chain_id()  );
      try {
         packed_transaction ptrx(trx);
         push_transaction(ptrx);
         produce_blocks(1);
         return true;
      } catch (tx_net_usage_exceeded &) {
         return false;
      } catch (transaction_exception &) {
         return false;
      }
   };
BOOST_REQUIRE_EQUAL(true, check(1024, 0)); //默认行为
BOOST_REQUIRE_EQUAL(false, check(1024, 100)); //事务最大\净\使用量太小
BOOST_REQUIRE_EQUAL(false, check(10240, 0)); //大于全局最大值

} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(weighted_net_usage_tests, tester ) try {
   account_name account = N(f_tests);
   account_name acc2 = N(acc2);
   create_accounts({account, acc2});
   int ver = 0;
   auto check = [&](int coderepeat)-> bool {
      std::string code = R"=====(
   (module
   (import "env" "require_auth" (func $require_auth (param i64)))
   (import "env" "eosio_assert" (func $eosio_assert (param i32 i32)))
      (table 0 anyfunc)
      (memory $0 1)
      (export "apply" (func $apply))
      (func $i64_trunc_u_f64 (param $0 f64) (result i64) (i64.trunc_u/f64 (get_local $0)))
      (func $test (param $0 i64))
      (func $apply (param $0 i64)(param $1 i64)(param $2 i64)
      )=====";
      for (int i = 0; i < coderepeat; ++i) {
         code += "(call $test (call $i64_trunc_u_f64 (f64.const ";
         code += (char)('0' + ver);
         code += ")))\n";
      }
      code += "))"; ver++;
      produce_blocks(1);
      signed_transaction trx;
      auto wasm = ::eosio::chain::wast_to_wasm(code);
      trx.actions.emplace_back( vector<permission_level>{{account,config::active_name}},
                              setcode{
                                 .account    = account,
                                 .vmtype     = 0,
                                 .vmversion  = 0,
                                 .code       = bytes(wasm.begin(), wasm.end())
                              });
      set_transaction_headers(trx);
      trx.sign( get_private_key( account, "active" ), control->get_chain_id()  );
      try {
         packed_transaction ptrx(trx);
         push_transaction(ptrx );
         produce_blocks(1);
         return true;
      } catch (tx_net_usage_exceeded &) {
         return false;
      }
   };
BOOST_REQUIRE_EQUAL(true, check(128)); //没有限制，应该通过

   resource_limits_manager mgr = control->get_mutable_resource_limits_manager();
mgr.set_account_limits(account, -1, 1, -1); //设置权重=1表示科目

   BOOST_REQUIRE_EQUAL(true, check(128));

mgr.set_account_limits(acc2, -1, 1000, -1); //为其他帐户设置一个很大的权重
   BOOST_REQUIRE_EQUAL(false, check(128));

} FC_LOG_AND_RETHROW()
#endif

BOOST_AUTO_TEST_SUITE_END()
