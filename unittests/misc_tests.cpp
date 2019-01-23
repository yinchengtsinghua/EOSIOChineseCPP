
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

#include <eosio/chain/chain_config.hpp>
#include <eosio/chain/authority_checker.hpp>
#include <eosio/chain/authority.hpp>
#include <eosio/chain/types.hpp>
#include <eosio/chain/asset.hpp>
#include <eosio/testing/tester.hpp>

#include <fc/io/json.hpp>

#include <boost/test/unit_test.hpp>

#ifdef NON_VALIDATING_TEST
#define TESTER tester
#else
#define TESTER validating_tester
#endif

using namespace eosio::chain;
using namespace eosio::testing;

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

namespace eosio
{
using namespace chain;
using namespace std;

static constexpr uint64_t name_suffix( uint64_t n ) {
   uint32_t remaining_bits_after_last_actual_dot = 0;
   uint32_t tmp = 0;
for( int32_t remaining_bits = 59; remaining_bits >= 4; remaining_bits -= 5 ) { //注意：剩余的位必须保持有符号整数
//按从左到右的顺序逐个获取名称中的字符（不包括第13个字符）
      auto c = (n >> remaining_bits) & 0x1Full;
if( !c ) { //如果这个字符是一个点
         tmp = static_cast<uint32_t>(remaining_bits);
} else { //如果这个字符不是点
         remaining_bits_after_last_actual_dot = tmp;
      }
   }

   uint64_t thirteenth_character = n & 0x0Full;
if( thirteenth_character ) { //如果第13个字符不是点
      remaining_bits_after_last_actual_dot = tmp;
   }

if( remaining_bits_after_last_actual_dot == 0 ) //除了潜在的前导点之外，名称中没有实际的点
      return n;

//在这一点上，最后一个点之后的剩余位必须在4到59的范围内（并且限制为5的增量）。

//除4个最低有效位（对应于第13个字符）外，与最后一个实际点之后的字符对应的剩余位的掩码。
   uint64_t mask = (1ull << remaining_bits_after_last_actual_dot) - 16;
   uint32_t shift = 64 - remaining_bits_after_last_actual_dot;

   return ( ((n & mask) << shift) + (thirteenth_character << (shift-1)) );
}

BOOST_AUTO_TEST_SUITE(misc_tests)

BOOST_AUTO_TEST_CASE(name_suffix_tests)
{
   BOOST_CHECK_EQUAL( name{name_suffix(0)}, name{0} );
   BOOST_CHECK_EQUAL( name{name_suffix(N(abcdehijklmn))}, name{N(abcdehijklmn)} );
   BOOST_CHECK_EQUAL( name{name_suffix(N(abcdehijklmn1))}, name{N(abcdehijklmn1)} );
   BOOST_CHECK_EQUAL( name{name_suffix(N(abc.def))}, name{N(def)} );
   BOOST_CHECK_EQUAL( name{name_suffix(N(.abc.def))}, name{N(def)} );
   BOOST_CHECK_EQUAL( name{name_suffix(N(..abc.def))}, name{N(def)} );
   BOOST_CHECK_EQUAL( name{name_suffix(N(abc..def))}, name{N(def)} );
   BOOST_CHECK_EQUAL( name{name_suffix(N(abc.def.ghi))}, name{N(ghi)} );
   BOOST_CHECK_EQUAL( name{name_suffix(N(.abcdefghij))}, name{N(abcdefghij)} );
   BOOST_CHECK_EQUAL( name{name_suffix(N(.abcdefghij.1))}, name{N(1)} );
   BOOST_CHECK_EQUAL( name{name_suffix(N(a.bcdefghij))}, name{N(bcdefghij)} );
   BOOST_CHECK_EQUAL( name{name_suffix(N(a.bcdefghij.1))}, name{N(1)} );
   BOOST_CHECK_EQUAL( name{name_suffix(N(......a.b.c))}, name{N(c)} );
   BOOST_CHECK_EQUAL( name{name_suffix(N(abcdefhi.123))}, name{N(123)} );
   BOOST_CHECK_EQUAL( name{name_suffix(N(abcdefhij.123))}, name{N(123)} );
}

///不平衡串的测试处理
BOOST_AUTO_TEST_CASE(json_from_string_test)
{
  bool exc_found = false;
  try {
    auto val = fc::json::from_string("{\"}");
  } catch(...) {
    exc_found = true;
  }
  BOOST_CHECK_EQUAL(exc_found, true);

  exc_found = false;
  try {
    auto val = fc::json::from_string("{\"block_num_or_id\":5");
  } catch(...) {
    exc_found = true;
  }
  BOOST_CHECK_EQUAL(exc_found, true);
}

//在asset:：from_string中测试溢出处理
BOOST_AUTO_TEST_CASE(asset_from_string_overflow)
{
   asset a;

//精度=19，量级<2^61
   BOOST_CHECK_EXCEPTION( asset::from_string("0.1000000000000000000 CUR") , symbol_type_exception, [](const auto& e) {
      return expect_assert_message(e, "precision 19 should be <= 18");
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("-0.1000000000000000000 CUR") , symbol_type_exception, [](const auto& e) {
      return expect_assert_message(e, "precision 19 should be <= 18");
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("1.0000000000000000000 CUR") , symbol_type_exception, [](const auto& e) {
      return expect_assert_message(e, "precision 19 should be <= 18");
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("-1.0000000000000000000 CUR") , symbol_type_exception, [](const auto& e) {
      return expect_assert_message(e, "precision 19 should be <= 18");
   });

//精度=18，量级<2^58
   a = asset::from_string("0.100000000000000000 CUR");
   BOOST_CHECK_EQUAL(a.get_amount(), 100000000000000000L);
   a = asset::from_string("-0.100000000000000000 CUR");
   BOOST_CHECK_EQUAL(a.get_amount(), -100000000000000000L);

//精度=18，量级=2^62
   BOOST_CHECK_EXCEPTION( asset::from_string("4.611686018427387904 CUR") , asset_type_exception, [](const asset_type_exception& e) {
      return expect_assert_message(e, "magnitude of asset amount must be less than 2^62");
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("-4.611686018427387904 CUR") , asset_type_exception, [](const asset_type_exception& e) {
      return expect_assert_message(e, "magnitude of asset amount must be less than 2^62");
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("4611686018427387.904 CUR") , asset_type_exception, [](const asset_type_exception& e) {
      return expect_assert_message(e, "magnitude of asset amount must be less than 2^62");
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("-4611686018427387.904 CUR") , asset_type_exception, [](const asset_type_exception& e) {
      return expect_assert_message(e, "magnitude of asset amount must be less than 2^62");
   });

//精度=18，量级=2^62-1
   a = asset::from_string("4.611686018427387903 CUR");
   BOOST_CHECK_EQUAL(a.get_amount(), 4611686018427387903L);
   a = asset::from_string("-4.611686018427387903 CUR");
   BOOST_CHECK_EQUAL(a.get_amount(), -4611686018427387903L);

//精度=0，量级=2^62
   BOOST_CHECK_EXCEPTION( asset::from_string("4611686018427387904 CUR") , asset_type_exception, [](const asset_type_exception& e) {
      return expect_assert_message(e, "magnitude of asset amount must be less than 2^62");
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("-4611686018427387904 CUR") , asset_type_exception, [](const asset_type_exception& e) {
      return expect_assert_message(e, "magnitude of asset amount must be less than 2^62");
   });

//精度=0，量级=2^62-1
   a = asset::from_string("4611686018427387903 CUR");
   BOOST_CHECK_EQUAL(a.get_amount(), 4611686018427387903L);
   a = asset::from_string("-4611686018427387903 CUR");
   BOOST_CHECK_EQUAL(a.get_amount(), -4611686018427387903L);

//精度=18，量级=2^65
   BOOST_CHECK_EXCEPTION( asset::from_string("36.893488147419103232 CUR") , overflow_exception, [](const overflow_exception& e) {
      return true;
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("-36.893488147419103232 CUR") , underflow_exception, [](const underflow_exception& e) {
      return true;
   });

//精度=14，量级>2^76
   BOOST_CHECK_EXCEPTION( asset::from_string("1000000000.00000000000000 CUR") , overflow_exception, [](const overflow_exception& e) {
      return true;
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("-1000000000.00000000000000 CUR") , underflow_exception, [](const underflow_exception& e) {
      return true;
   });

//精度=0，量级>2^76
   BOOST_CHECK_EXCEPTION( asset::from_string("100000000000000000000000 CUR") , parse_error_exception, [](const parse_error_exception& e) {
      return expect_assert_message(e, "Couldn't parse int64_t");
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("-100000000000000000000000 CUR") , parse_error_exception, [](const parse_error_exception& e) {
      return expect_assert_message(e, "Couldn't parse int64_t");
   });

//精度=20，量级>2^142
   BOOST_CHECK_EXCEPTION( asset::from_string("100000000000000000000000.00000000000000000000 CUR") , symbol_type_exception, [](const auto& e) {
      return expect_assert_message(e, "precision 20 should be <= 18");
   });
   BOOST_CHECK_EXCEPTION( asset::from_string("-100000000000000000000000.00000000000000000000 CUR") , symbol_type_exception, [](const auto& e) {
      return expect_assert_message(e, "precision 20 should be <= 18");
   });
}

struct permission_visitor {
   std::vector<permission_level> permissions;
   std::vector<size_t> size_stack;
   bool _log;

   permission_visitor(bool log = false) : _log(log) {}

   void operator()(const permission_level& permission) {
      permissions.push_back(permission);
   }

   void operator()(const permission_level& permission, bool repeat ) {}

   void push_undo() {
      if( _log )
         ilog("push_undo called");
      size_stack.push_back(permissions.size());
   }

   void pop_undo() {
      if( _log )
         ilog("pop_undo called");
      FC_ASSERT( size_stack.back() <= permissions.size() && size_stack.size() >= 1,
                 "invariant failure in test permission_visitor" );
      permissions.erase( permissions.begin() + size_stack.back(), permissions.end() );
      size_stack.pop_back();
   }

   void squash_undo() {
      if( _log )
         ilog("squash_undo called");
      FC_ASSERT( size_stack.size() >= 1, "invariant failure in test permission_visitor" );
      size_stack.pop_back();
   }

};

BOOST_AUTO_TEST_CASE(authority_checker)
{ try {
   testing::TESTER test;
   auto a = test.get_public_key("a", "active");
   auto b = test.get_public_key("b", "active");
   auto c = test.get_public_key("c", "active");

   auto GetNullAuthority = [](auto){abort(); return authority();};

   auto A = authority(2, {key_weight{a, 1}, key_weight{b, 1}});
   {
      auto checker = make_auth_checker(GetNullAuthority, 2, {a, b});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 2);
      BOOST_TEST(checker.unused_keys().size() == 0);
   }
   {
      auto checker = make_auth_checker(GetNullAuthority, 2, {a, c});
      BOOST_TEST(!checker.satisfied(A));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 0);
      BOOST_TEST(checker.unused_keys().size() == 2);
   }
   {
      auto checker = make_auth_checker(GetNullAuthority, 2, {a, b, c});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 2);
      BOOST_TEST(checker.used_keys().count(a) == 1);
      BOOST_TEST(checker.used_keys().count(b) == 1);
      BOOST_TEST(checker.unused_keys().size() == 1);
      BOOST_TEST(checker.unused_keys().count(c) == 1);
   }
   {
      auto checker = make_auth_checker(GetNullAuthority, 2, {b, c});
      BOOST_TEST(!checker.satisfied(A));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 0);
   }

   A = authority(3, {key_weight{a, 1}, key_weight{b, 1}, key_weight{c, 1}});
   BOOST_TEST(make_auth_checker(GetNullAuthority, 2, {c, b, a}).satisfied(A));
   BOOST_TEST(!make_auth_checker(GetNullAuthority, 2, {a, b}).satisfied(A));
   BOOST_TEST(!make_auth_checker(GetNullAuthority, 2, {a, c}).satisfied(A));
   BOOST_TEST(!make_auth_checker(GetNullAuthority, 2, {b, c}).satisfied(A));

   A = authority(1, {key_weight{a, 1}, key_weight{b, 1}});
   BOOST_TEST(make_auth_checker(GetNullAuthority, 2, {a}).satisfied(A));
   BOOST_TEST(make_auth_checker(GetNullAuthority, 2, {b}).satisfied(A));
   BOOST_TEST(!make_auth_checker(GetNullAuthority, 2, {c}).satisfied(A));

   A = authority(1, {key_weight{a, 2}, key_weight{b, 1}});
   BOOST_TEST(make_auth_checker(GetNullAuthority, 2, {a}).satisfied(A));
   BOOST_TEST(make_auth_checker(GetNullAuthority, 2, {b}).satisfied(A));
   BOOST_TEST(!make_auth_checker(GetNullAuthority, 2, {c}).satisfied(A));

   auto GetCAuthority = [c](auto){
      return authority(1, {key_weight{c, 1}});
   };

   A = authority(2, {key_weight{a, 2}, key_weight{b, 1}}, {permission_level_weight{{"hello",  "world"}, 1}});
   {
      auto checker = make_auth_checker(GetCAuthority, 2, {a});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(checker.all_keys_used());
   }
   {
      auto checker = make_auth_checker(GetCAuthority, 2, {b});
      BOOST_TEST(!checker.satisfied(A));
      BOOST_TEST(checker.used_keys().size() == 0);
      BOOST_TEST(checker.unused_keys().size() == 1);
      BOOST_TEST(checker.unused_keys().count(b) == 1);
   }
   {
      auto checker = make_auth_checker(GetCAuthority, 2, {c});
      BOOST_TEST(!checker.satisfied(A));
      BOOST_TEST(checker.used_keys().size() == 0);
      BOOST_TEST(checker.unused_keys().size() == 1);
      BOOST_TEST(checker.unused_keys().count(c) == 1);
   }
   {
      auto checker = make_auth_checker(GetCAuthority, 2, {b, c});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 2);
      BOOST_TEST(checker.unused_keys().size() == 0);
      BOOST_TEST(checker.used_keys().count(b) == 1);
      BOOST_TEST(checker.used_keys().count(c) == 1);
   }
   {
      auto checker = make_auth_checker(GetCAuthority, 2, {b, c, a});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 1);
      BOOST_TEST(checker.used_keys().count(a) == 1);
      BOOST_TEST(checker.unused_keys().size() == 2);
      BOOST_TEST(checker.unused_keys().count(b) == 1);
      BOOST_TEST(checker.unused_keys().count(c) == 1);
   }

   A = authority(3, {key_weight{a, 2}, key_weight{b, 1}}, {permission_level_weight{{"hello",  "world"}, 3}});
   {
      auto checker = make_auth_checker(GetCAuthority, 2, {a, b});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(checker.all_keys_used());
   }
   {
      auto checker = make_auth_checker(GetCAuthority, 2, {a, b, c});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 1);
      BOOST_TEST(checker.used_keys().count(c) == 1);
      BOOST_TEST(checker.unused_keys().size() == 2);
      BOOST_TEST(checker.unused_keys().count(a) == 1);
      BOOST_TEST(checker.unused_keys().count(b) == 1);
   }

   A = authority(2, {key_weight{a, 1}, key_weight{b, 1}}, {permission_level_weight{{"hello",  "world"}, 1}});
   BOOST_TEST(!make_auth_checker(GetCAuthority, 2, {a}).satisfied(A));
   BOOST_TEST(!make_auth_checker(GetCAuthority, 2, {b}).satisfied(A));
   BOOST_TEST(!make_auth_checker(GetCAuthority, 2, {c}).satisfied(A));
   BOOST_TEST(make_auth_checker(GetCAuthority, 2, {a, b}).satisfied(A));
   BOOST_TEST(make_auth_checker(GetCAuthority, 2, {b, c}).satisfied(A));
   BOOST_TEST(make_auth_checker(GetCAuthority, 2, {a, c}).satisfied(A));
   {
      auto checker = make_auth_checker(GetCAuthority, 2, {a, b, c});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 2);
      BOOST_TEST(checker.unused_keys().size() == 1);
      BOOST_TEST(checker.unused_keys().count(c) == 1);
   }

   A = authority(2, {key_weight{a, 1}, key_weight{b, 1}}, {permission_level_weight{{"hello",  "world"}, 2}});
   BOOST_TEST(make_auth_checker(GetCAuthority, 2, {a, b}).satisfied(A));
   BOOST_TEST(make_auth_checker(GetCAuthority, 2, {c}).satisfied(A));
   BOOST_TEST(!make_auth_checker(GetCAuthority, 2, {a}).satisfied(A));
   BOOST_TEST(!make_auth_checker(GetCAuthority, 2, {b}).satisfied(A));
   {
      auto checker = make_auth_checker(GetCAuthority, 2, {a, b, c});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 1);
      BOOST_TEST(checker.unused_keys().size() == 2);
      BOOST_TEST(checker.used_keys().count(c) == 1);
   }

   auto d = test.get_public_key("d", "active");
   auto e = test.get_public_key("e", "active");

   auto GetAuthority = [d, e] (const permission_level& perm) {
      if (perm.actor == "top")
         return authority(2, {key_weight{d, 1}}, {permission_level_weight{{"bottom",  "bottom"}, 1}});
      return authority{1, {{e, 1}}, {}};
   };

   A = authority(5, {key_weight{a, 2}, key_weight{b, 2}, key_weight{c, 2}}, {permission_level_weight{{"top",  "top"}, 5}});
   {
      auto checker = make_auth_checker(GetAuthority, 2, {d, e});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(checker.all_keys_used());
   }
   {
      auto checker = make_auth_checker(GetAuthority, 2, {a, b, c, d, e});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 2);
      BOOST_TEST(checker.unused_keys().size() == 3);
      BOOST_TEST(checker.used_keys().count(d) == 1);
      BOOST_TEST(checker.used_keys().count(e) == 1);
   }
   {
      auto checker = make_auth_checker(GetAuthority, 2, {a, b, c, e});
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.used_keys().size() == 3);
      BOOST_TEST(checker.unused_keys().size() == 1);
      BOOST_TEST(checker.used_keys().count(a) == 1);
      BOOST_TEST(checker.used_keys().count(b) == 1);
      BOOST_TEST(checker.used_keys().count(c) == 1);
   }
   BOOST_TEST(make_auth_checker(GetAuthority, 1, {a, b, c}).satisfied(A));
//由于短递归深度限制而失败
   BOOST_TEST(!make_auth_checker(GetAuthority, 1, {d, e}).satisfied(A));

   BOOST_TEST(b < a);
   BOOST_TEST(b < c);
   BOOST_TEST(a < c);
   {
//有效键顺序：b<a<c
      A = authority(2, {key_weight{b, 1}, key_weight{a, 1}, key_weight{c, 1}});
//有效键顺序：b<c
      auto B = authority(1, {key_weight{b, 1}, key_weight{c, 1}});
//无效的键顺序：c>b
      auto C = authority(1, {key_weight{b, 1}, key_weight{c, 1}, key_weight{b, 1}});
//无效的键顺序：重复的C
      auto D = authority(1, {key_weight{b, 1}, key_weight{c, 1}, key_weight{c, 1}});
//无效的键顺序：重复b
      auto E = authority(1, {key_weight{b, 1}, key_weight{b, 1}, key_weight{c, 1}});
//无效：重量不足
      auto F = authority(4, {key_weight{b, 1}, key_weight{a, 1}, key_weight{c, 1}});

      auto checker = make_auth_checker(GetNullAuthority, 2, {a, b, c});
      BOOST_TEST(validate(A));
      BOOST_TEST(validate(B));
      BOOST_TEST(!validate(C));
      BOOST_TEST(!validate(D));
      BOOST_TEST(!validate(E));
      BOOST_TEST(!validate(F));

      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.unused_keys().count(b) == 1);
      BOOST_TEST(checker.unused_keys().count(a) == 1);
      BOOST_TEST(checker.unused_keys().count(c) == 1);
      BOOST_TEST(checker.satisfied(A));
      BOOST_TEST(checker.satisfied(B));
      BOOST_TEST(!checker.all_keys_used());
      BOOST_TEST(checker.unused_keys().count(b) == 0);
      BOOST_TEST(checker.unused_keys().count(a) == 0);
      BOOST_TEST(checker.unused_keys().count(c) == 1);
   }
   {
      auto A2 = authority(4, {key_weight{b, 1}, key_weight{a, 1}, key_weight{c, 1}},
                          { permission_level_weight{{"a",  "world"},     1},
                            permission_level_weight{{"hello",  "world"}, 1},
                            permission_level_weight{{"hi",  "world"},    1}
                          });
      auto B2 = authority(4, {key_weight{b, 1}, key_weight{a, 1}, key_weight{c, 1}},
                          {permission_level_weight{{"hello",  "world"}, 1}
                          });
      auto C2 = authority(4, {key_weight{b, 1}, key_weight{a, 1}, key_weight{c, 1}},
                          { permission_level_weight{{"hello",  "there"}, 1},
                            permission_level_weight{{"hello",  "world"}, 1}
                          });
//无效：重复
      auto D2 = authority(4, {key_weight{b, 1}, key_weight{a, 1}, key_weight{c, 1}},
                          { permission_level_weight{{"hello",  "world"}, 1},
                            permission_level_weight{{"hello",  "world"}, 2}
                          });
//无效：顺序错误
      auto E2 = authority(4, {key_weight{b, 1}, key_weight{a, 1}, key_weight{c, 1}},
                          { permission_level_weight{{"hello",  "world"}, 2},
                            permission_level_weight{{"hello",  "there"}, 1}
                          });
//无效：顺序错误
      auto F2 = authority(4, {key_weight{b, 1}, key_weight{a, 1}, key_weight{c, 1}},
                          { permission_level_weight{{"hi",  "world"}, 2},
                            permission_level_weight{{"hello",  "world"}, 1}
                          });

//无效：重量不足
      auto G2 = authority(7, {key_weight{b, 1}, key_weight{a, 1}, key_weight{c, 1}},
                             { permission_level_weight{{"a",  "world"},     1},
                               permission_level_weight{{"hello",  "world"}, 1},
                               permission_level_weight{{"hi",  "world"},    1}
                             });

      BOOST_TEST(validate(A2));
      BOOST_TEST(validate(B2));
      BOOST_TEST(validate(C2));
      BOOST_TEST(!validate(D2));
      BOOST_TEST(!validate(E2));
      BOOST_TEST(!validate(F2));
      BOOST_TEST(!validate(G2));
   }
} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_CASE(alphabetic_sort)
{ try {

  vector<string> words = {
    "com.o",
    "te",
    "a.....5",
    "a...4",
    ".va.ku",
    "gh",
    "1ho.la",
    "g1",
    "g",
    "a....2",
    "gg",
    "va",
    "lale.....12b",
    "a....3",
    "a....1",
    "..g",
    ".g",
    "....g",
    "a....y",
    "...g",
    "lale.....333",
  };

  std::sort(words.begin(), words.end(), std::less<string>());

  vector<uint64_t> uwords;
  for(const auto w: words) {
    auto n = name(w.c_str());
    uwords.push_back(n.value);
  }

  std::sort(uwords.begin(), uwords.end(), std::less<uint64_t>());

  vector<string> tmp;
  for(const auto uw: uwords) {
    auto str = name(uw).to_string();
    tmp.push_back(str);
  }

  for(int i = 0; i < words.size(); ++i ) {
    BOOST_TEST(tmp[i] == words[i]);
  }

} FC_LOG_AND_RETHROW() }


BOOST_AUTO_TEST_CASE(transaction_test) { try {

   testing::TESTER test;
   signed_transaction trx;

   variant pretty_trx = fc::mutable_variant_object()
      ("actions", fc::variants({
         fc::mutable_variant_object()
            ("account", "eosio")
            ("name", "reqauth")
            ("authorization", fc::variants({
               fc::mutable_variant_object()
                  ("actor", "eosio")
                  ("permission", "active")
            }))
            ("data", fc::mutable_variant_object()
               ("from", "eosio")
            )
         })
      )
//我们还可以推动一个上下文无关的操作，多链测试随后还将包括一个上下文无关的操作。
      ("context_free_actions", fc::variants({
         fc::mutable_variant_object()
            ("account", "eosio")
            ("name", "nonce")
            ("data", fc::raw::pack(std::string("dummy")))
         })
      );

   abi_serializer::from_variant(pretty_trx, trx, test.get_resolver(), test.abi_serializer_max_time);

   test.set_transaction_headers(trx);

   trx.expiration = fc::time_point::now();
   trx.validate();
   BOOST_CHECK_EQUAL(0, trx.signatures.size());
   ((const signed_transaction &)trx).sign( test.get_private_key( config::system_account_name, "active" ), test.control->get_chain_id());
   BOOST_CHECK_EQUAL(0, trx.signatures.size());
   trx.sign( test.get_private_key( config::system_account_name, "active" ), test.control->get_chain_id()  );
   BOOST_CHECK_EQUAL(1, trx.signatures.size());
   trx.validate();

   packed_transaction pkt(trx, packed_transaction::none);

   packed_transaction pkt2(trx, packed_transaction::zlib);

   BOOST_CHECK_EQUAL(true, trx.expiration ==  pkt.expiration());
   BOOST_CHECK_EQUAL(true, trx.expiration == pkt2.expiration());

   BOOST_CHECK_EQUAL(trx.id(), pkt.id());
   BOOST_CHECK_EQUAL(trx.id(), pkt2.id());

   bytes raw = pkt.get_raw_transaction();
   bytes raw2 = pkt2.get_raw_transaction();
   BOOST_CHECK_EQUAL(raw.size(), raw2.size());

} FC_LOG_AND_RETHROW() }

BOOST_AUTO_TEST_SUITE_END()

} //命名空间EOSIO
