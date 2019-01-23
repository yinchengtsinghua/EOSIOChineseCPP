
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <boost/test/unit_test.hpp>
#include <eosio/testing/tester.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/chain/contract_table_objects.hpp>
#include <eosio/chain/fixed_key.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <chainbase/chainbase.hpp>

#include <identity/identity.wast.hpp>
#include <identity/identity.abi.hpp>
#include <identity/test/identity_test.wast.hpp>
#include <identity/test/identity_test.abi.hpp>

#include <Runtime/Runtime.h>

#include <fc/variant_object.hpp>
#include <fc/io/json.hpp>

#include <algorithm>

#ifdef NON_VALIDATING_TEST
#define TESTER tester
#else
#define TESTER validating_tester
#endif

using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;

class identity_tester : public TESTER {
public:

   identity_tester() {
      produce_blocks(2);

      create_accounts( {N(identity), N(identitytest), N(alice), N(bob), N(carol)} );
      produce_blocks(1000);

      set_code(N(identity), identity_wast);
      set_abi(N(identity), identity_abi);
      set_code(N(identitytest), identity_test_wast);
      set_abi(N(identitytest), identity_test_abi);
      produce_blocks(1);

      const auto& accnt = control->db().get<account_object,by_name>( N(identity) );
      abi_def abi;
      BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
      abi_ser.set_abi(abi, abi_serializer_max_time);

      const auto& acnt_test = control->db().get<account_object,by_name>( N(identitytest) );
      abi_def abi_test;
      BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(acnt_test.abi, abi_test), true);
      abi_ser_test.set_abi(abi_test, abi_serializer_max_time);

      const auto& ap = control->active_producers();
      FC_ASSERT(0 < ap.producers.size(), "No producers");
      producer_name = (string)ap.producers.front().producer_name;
   }

   uint64_t get_result_uint64() {
      const auto& db = control->db();
      const auto* t_id = db.find<table_id_object, by_code_scope_table>(boost::make_tuple(N(identitytest), 0, N(result)));
      FC_ASSERT(t_id != 0, "Table id not found");

      const auto& idx = db.get_index<key_value_index, by_scope_primary>();

      auto itr = idx.lower_bound(boost::make_tuple(t_id->id));
      FC_ASSERT( itr != idx.end() && itr->t_id == t_id->id, "lower_bound failed");

      FC_ASSERT( N(result) == itr->primary_key, "row with result not found");
      FC_ASSERT( sizeof(uint64_t) == itr->value.size(), "unexpected result size");
      return *reinterpret_cast<const uint64_t*>(itr->value.data());
   }

   uint64_t get_owner_for_identity(uint64_t identity)
   {
      action get_owner_act;
      get_owner_act.account = N(identitytest);
      get_owner_act.name = N(getowner);
      get_owner_act.data = abi_ser_test.variant_to_binary("getowner", mutable_variant_object()
                                                          ("identity", identity),
                                                          abi_serializer_max_time
      );
      BOOST_REQUIRE_EQUAL(success(), push_action(std::move(get_owner_act), N(alice)));
      return get_result_uint64();
   }

   uint64_t get_identity_for_account(const string& account)
   {
      action get_identity_act;
      get_identity_act.account = N(identitytest);
      get_identity_act.name = N(getidentity);
      get_identity_act.data = abi_ser_test.variant_to_binary("getidentity", mutable_variant_object()
                                                          ("account", account),
                                                          abi_serializer_max_time
      );
      BOOST_REQUIRE_EQUAL(success(), push_action(std::move(get_identity_act), N(alice)));
      return get_result_uint64();
   }

   action_result create_identity(const string& account_name, uint64_t identity, bool auth = true) {
      action create_act;
      create_act.account = N(identity);
      create_act.name = N(create);
      create_act.data = abi_ser.variant_to_binary("create", mutable_variant_object()
                                                  ("creator", account_name)
                                                  ("identity", identity),
                                                  abi_serializer_max_time
      );
      return push_action( std::move(create_act), (auth ? string_to_name(account_name.c_str()) : (string_to_name(account_name.c_str()) == N(bob)) ? N(alice) : N(bob)));
   }

   fc::variant get_identity(uint64_t idnt) {
      const auto& db = control->db();
      const auto* t_id = db.find<table_id_object, by_code_scope_table>(boost::make_tuple(N(identity), N(identity), N(ident)));
      FC_ASSERT(t_id != 0, "object not found");

      const auto& idx = db.get_index<key_value_index, by_scope_primary>();

      auto itr = idx.lower_bound(boost::make_tuple(t_id->id, idnt));
      FC_ASSERT( itr != idx.end() && itr->t_id == t_id->id, "lower_bound failed");
      BOOST_REQUIRE_EQUAL(idnt, itr->primary_key);

      vector<char> data;
      copy_row(*itr, data);
      return abi_ser.binary_to_variant("identrow", data, abi_serializer_max_time);
   }

   action_result certify(const string& certifier, uint64_t identity, const vector<fc::variant>& fields, bool auth = true) {
      action cert_act;
      cert_act.account = N(identity);
      cert_act.name = N(certprop);
      cert_act.data = abi_ser.variant_to_binary("certprop", mutable_variant_object()
                                                ("bill_storage_to", certifier)
                                                ("certifier", certifier)
                                                ("identity", identity)
                                                ("value", fields),
                                                abi_serializer_max_time
      );
      return push_action( std::move(cert_act), (auth ? string_to_name(certifier.c_str()) : (string_to_name(certifier.c_str()) == N(bob)) ? N(alice) : N(bob)));
   }

   fc::variant get_certrow(uint64_t identity, const string& property, uint64_t trusted, const string& certifier) {
      const auto& db = control->db();
      const auto* t_id = db.find<table_id_object, by_code_scope_table>(boost::make_tuple(N(identity), identity, N( certs )));
      if ( !t_id ) {
         return fc::variant(nullptr);
      }

      const auto& idx = db.get_index<index256_index, by_secondary>();
      auto key = key256::make_from_word_sequence<uint64_t>(string_to_name(property.c_str()), trusted, string_to_name(certifier.c_str()));

      auto itr = idx.lower_bound(boost::make_tuple(t_id->id, key.get_array()));
      if (itr != idx.end() && itr->t_id == t_id->id && itr->secondary_key == key.get_array()) {
         auto primary_key = itr->primary_key;
         const auto& idx = db.get_index<key_value_index, by_scope_primary>();

         auto itr = idx.lower_bound(boost::make_tuple(t_id->id, primary_key));
         FC_ASSERT( itr != idx.end() && itr->t_id == t_id->id && primary_key == itr->primary_key,
                    "Record found in secondary index, but not found in primary index."
         );
         vector<char> data;
         copy_row(*itr, data);
         return abi_ser.binary_to_variant("certrow", data, abi_serializer_max_time);
      } else {
         return fc::variant(nullptr);
      }
   }

   fc::variant get_accountrow(const string& account) {
      const auto& db = control->db();
      uint64_t acnt = string_to_name(account.c_str());
      const auto* t_id = db.find<table_id_object, by_code_scope_table>(boost::make_tuple(N(identity), acnt, N(account)));
      if (!t_id) {
         return fc::variant(nullptr);
      }
      const auto& idx = db.get_index<key_value_index, by_scope_primary>();
      auto itr = idx.lower_bound(boost::make_tuple(t_id->id, N(account)));
      if( itr != idx.end() && itr->t_id == t_id->id && N(account) == itr->primary_key) {
         vector<char> data;
         copy_row(*itr, data);
         return abi_ser.binary_to_variant("accountrow", data, abi_serializer_max_time);
      } else {
         return fc::variant(nullptr);
      }
   }

   action_result settrust(const string& trustor, const string& trusting, uint64_t trust, bool auth = true)
   {
      signed_transaction trx;
      action settrust_act;
      settrust_act.account = N(identity);
      settrust_act.name = N(settrust);
      settrust_act.data = abi_ser.variant_to_binary("settrust", mutable_variant_object()
                                                    ("trustor", trustor)
                                                    ("trusting", trusting)
                                                    ("trust", trust),
                                                    abi_serializer_max_time
      );
      auto tr = string_to_name(trustor.c_str());
      return push_action( std::move(settrust_act), (auth ? tr : 0) );
   }

   bool get_trust(const string& trustor, const string& trusting) {
      const auto& db = control->db();
      const auto* t_id = db.find<table_id_object, by_code_scope_table>(boost::make_tuple(N(identity), string_to_name(trustor.c_str()), N(trust)));
      if (!t_id) {
         return false;
      }

      uint64_t tng = string_to_name(trusting.c_str());
      const auto& idx = db.get_index<key_value_index, by_scope_primary>();
      auto itr = idx.lower_bound(boost::make_tuple(t_id->id, tng));
return ( itr != idx.end() && itr->t_id == t_id->id && tng == itr->primary_key ); //如果找到的话是真的
   }

public:
   abi_serializer abi_ser;
   abi_serializer abi_ser_test;
   std::string producer_name;
};

constexpr uint64_t identity_val = 0xffffffffffffffff; //64位值

BOOST_AUTO_TEST_SUITE(identity_tests)

BOOST_FIXTURE_TEST_CASE( identity_create, identity_tester ) try {

   BOOST_REQUIRE_EQUAL(success(), create_identity("alice", identity_val));
   fc::variant idnt = get_identity(identity_val);
   BOOST_REQUIRE_EQUAL( identity_val, idnt["identity"].as_uint64());
   BOOST_REQUIRE_EQUAL( "alice", idnt["creator"].as_string());

//尝试创建现有标识应失败
   BOOST_REQUIRE_EQUAL(wasm_assert_msg("identity already exists"), create_identity("alice", identity_val));
   BOOST_REQUIRE_EQUAL(wasm_assert_msg("identity already exists"), create_identity("bob", identity_val));

//Alice可以创建更多的身份
   BOOST_REQUIRE_EQUAL(success(), create_identity("alice", 2));
   fc::variant idnt2 = get_identity(2);
   BOOST_REQUIRE_EQUAL( 2, idnt2["identity"].as_uint64());
   BOOST_REQUIRE_EQUAL( "alice", idnt2["creator"].as_string());

//Bob也可以创建身份
   BOOST_REQUIRE_EQUAL(success(), create_identity("bob", 1));

//标识==0有特殊意义，不可能创建
   BOOST_REQUIRE_EQUAL(wasm_assert_msg("identity=0 is not allowed"), create_identity("alice", 0));

//不允许在没有身份验证的情况下创建附件
   BOOST_REQUIRE_EQUAL(error("missing authority of alice"), create_identity("alice", 3, false));

   fc::variant idnt_bob = get_identity(1);
   BOOST_REQUIRE_EQUAL( 1, idnt_bob["identity"].as_uint64());
   BOOST_REQUIRE_EQUAL( "bob", idnt_bob["creator"].as_string());

//以前创建的标识应该仍然存在
   idnt = get_identity(identity_val);
   BOOST_REQUIRE_EQUAL( identity_val, idnt["identity"].as_uint64());

} FC_LOG_AND_RETHROW() //标识创建

BOOST_FIXTURE_TEST_CASE( certify_decertify, identity_tester ) try {
   BOOST_REQUIRE_EQUAL(success(), create_identity("alice", identity_val));

//Alice（标识的创建者）证明1个属性
   BOOST_REQUIRE_EQUAL(success(), certify("alice", identity_val, vector<fc::variant>{ mutable_variant_object()
                                                                                     ("property", "name")
                                                                                     ("type", "string")
                                                                                     ("data", to_uint8_vector("Alice Smith"))
                                                                                     ("memo", "")
                                                                                     ("confidence", 100)
               }));

   auto obj = get_certrow(identity_val, "name", 0, "alice");
   BOOST_REQUIRE_EQUAL(true, obj.is_object());
   BOOST_REQUIRE_EQUAL( "name", obj["property"].as_string() );
   BOOST_REQUIRE_EQUAL( 0, obj["trusted"].as_uint64() );
   BOOST_REQUIRE_EQUAL( "alice", obj["certifier"].as_string() );
   BOOST_REQUIRE_EQUAL( 100, obj["confidence"].as_uint64() );
   BOOST_REQUIRE_EQUAL( "string", obj["type"].as_string() );
   BOOST_REQUIRE_EQUAL( "Alice Smith", to_string(obj["data"]) );

//检查是否没有具有相同数据的受信任行
   BOOST_REQUIRE_EQUAL(true, get_certrow(identity_val, "name", 1, "alice").is_null());

//Bob认证2个属性
   vector<fc::variant> fields = { mutable_variant_object()
                                  ("property", "email")
                                  ("type", "string")
                                  ("data", to_uint8_vector("alice@alice.name"))
                                  ("memo", "official email")
                                  ("confidence", 95),
                                  mutable_variant_object()
                                  ("property", "address")
                                  ("type", "string")
                                  ("data", to_uint8_vector("1750 Kraft Drive SW, Blacksburg, VA 24060"))
                                  ("memo", "official address")
                                  ("confidence", 80)
   };

//没有授权就不能认证
   BOOST_REQUIRE_EQUAL(error("missing authority of bob"), certify("bob", identity_val, fields, false));

//不允许验证不存在的标识
   uint64_t non_existent = 11;
   BOOST_REQUIRE_EQUAL(wasm_assert_msg("identity does not exist"),
                       certify("alice", non_existent, vector<fc::variant>{ mutable_variant_object()
                                ("property", "name")
                                ("type", "string")
                                ("data", to_uint8_vector("Alice Smith"))
                                ("memo", "")
                                ("confidence", 100)
                                })
   );

//参数“type”不应超过32个字节
   BOOST_REQUIRE_EQUAL(wasm_assert_msg("certrow::type should be not longer than 32 bytes"),
                       certify("alice", identity_val, vector<fc::variant>{ mutable_variant_object()
                                ("property", "height")
                                ("type", "super_long_type_name_wich_is_not_allowed")
                                ("data", to_uint8_vector("Alice Smith"))
                                ("memo", "")
                                ("confidence", 100)
                                })
   );

//Bob还应该能够证明
   BOOST_REQUIRE_EQUAL(success(), certify("bob", identity_val, fields));

   obj = get_certrow(identity_val, "email", 0, "bob");
   BOOST_REQUIRE_EQUAL(true, obj.is_object());
   BOOST_REQUIRE_EQUAL( "email", obj["property"].as_string() );
   BOOST_REQUIRE_EQUAL( 0, obj["trusted"].as_uint64() );
   BOOST_REQUIRE_EQUAL( "bob", obj["certifier"].as_string() );
   BOOST_REQUIRE_EQUAL( 95, obj["confidence"].as_uint64() );
   BOOST_REQUIRE_EQUAL( "string", obj["type"].as_string() );
   BOOST_REQUIRE_EQUAL( "alice@alice.name", to_string(obj["data"]) );

   obj = get_certrow(identity_val, "address", 0, "bob");
   BOOST_REQUIRE_EQUAL(true, obj.is_object());
   BOOST_REQUIRE_EQUAL( "address", obj["property"].as_string() );
   BOOST_REQUIRE_EQUAL( 0, obj["trusted"].as_uint64() );
   BOOST_REQUIRE_EQUAL( "bob", obj["certifier"].as_string() );
   BOOST_REQUIRE_EQUAL( 80, obj["confidence"].as_uint64() );
   BOOST_REQUIRE_EQUAL( "string", obj["type"].as_string() );
   BOOST_REQUIRE_EQUAL( "1750 Kraft Drive SW, Blacksburg, VA 24060", to_string(obj["data"]) );

//现在Alice认证了另一封电子邮件
   BOOST_REQUIRE_EQUAL(success(), certify("alice", identity_val, vector<fc::variant>{ mutable_variant_object()
                                                                                     ("property", "email")
                                                                                     ("type", "string")
                                                                                     ("data", to_uint8_vector("alice.smith@gmail.com"))
                                                                                     ("memo", "")
                                                                                     ("confidence", 100)
               }));
   obj = get_certrow(identity_val, "email", 0, "alice");
   BOOST_REQUIRE_EQUAL(true, obj.is_object());
   BOOST_REQUIRE_EQUAL( "email", obj["property"].as_string() );
   BOOST_REQUIRE_EQUAL( 0, obj["trusted"].as_uint64() );
   BOOST_REQUIRE_EQUAL( "alice", obj["certifier"].as_string() );
   BOOST_REQUIRE_EQUAL( 100, obj["confidence"].as_uint64() );
   BOOST_REQUIRE_EQUAL( "string", obj["type"].as_string() );
   BOOST_REQUIRE_EQUAL( "alice.smith@gmail.com", to_string(obj["data"]) );

//Bob的电子邮件认证应该仍然有效
   obj = get_certrow(identity_val, "email", 0, "bob");
   BOOST_REQUIRE_EQUAL( "bob", obj["certifier"].as_string() );
   BOOST_REQUIRE_EQUAL( "alice@alice.name", to_string(obj["data"]) );

//删除Alice的电子邮件证书
   BOOST_REQUIRE_EQUAL(success(), certify("alice", identity_val, vector<fc::variant>{ mutable_variant_object()
                                                                                     ("property", "email")
                                                                                     ("type", "string")
                                                                                     ("data", to_uint8_vector(""))
                                                                                     ("memo", "")
                                                                                     ("confidence", 0)
               }));
   BOOST_REQUIRE_EQUAL(true, get_certrow(identity_val, "email", 0, "alice").is_null());

//Bob的电子邮件认证仍然有效
   obj = get_certrow(identity_val, "email", 0, "bob");
   BOOST_REQUIRE_EQUAL( "bob", obj["certifier"].as_string() );
   BOOST_REQUIRE_EQUAL( "alice@alice.name", to_string(obj["data"]) );

//Alice的姓名认证仍然有效
   obj = get_certrow(identity_val, "name", 0, "alice");
   BOOST_REQUIRE_EQUAL(true, obj.is_object());
   BOOST_REQUIRE_EQUAL( "Alice Smith", to_string(obj["data"]) );

} FC_LOG_AND_RETHROW() //认证

BOOST_FIXTURE_TEST_CASE( trust_untrust, identity_tester ) try {
   BOOST_REQUIRE_EQUAL(success(), settrust("bob", "alice", 1));
   BOOST_REQUIRE_EQUAL(true, get_trust("bob", "alice"));

//相反方向的信任关系不应存在
   BOOST_REQUIRE_EQUAL(false, get_trust("alice", "bob"));

//移除信任
   BOOST_REQUIRE_EQUAL(success(), settrust("bob", "alice", 0));
   BOOST_REQUIRE_EQUAL(false, get_trust("bob", "alice"));

} FC_LOG_AND_RETHROW() //信托不信任

BOOST_FIXTURE_TEST_CASE( certify_decertify_owner, identity_tester ) try {
   BOOST_REQUIRE_EQUAL(success(), create_identity("alice", identity_val));

//Bob证明Alice的所有权
   BOOST_REQUIRE_EQUAL(success(), certify("bob", identity_val, vector<fc::variant>{ mutable_variant_object()
                                                                              ("property", "owner")
                                                                              ("type", "account")
                                                                              ("data", to_uint8_vector(N(alice)))
                                                                              ("memo", "claiming onwership")
                                                                              ("confidence", 100)
               }));
   BOOST_REQUIRE_EQUAL( true, get_certrow(identity_val, "owner", 0, "bob").is_object() );
//它不应该影响Alice范围内的“account”singleton，因为它不是自认证的。
   BOOST_REQUIRE_EQUAL( true, get_accountrow("alice").is_null() );
//它也不应该影响Bob范围内的“帐户”单例，因为他证明自己不是
   BOOST_REQUIRE_EQUAL( true, get_accountrow("bob").is_null() );

//Alice证明了她的所有权，应该填充“account”singleton
   BOOST_REQUIRE_EQUAL(success(), certify("alice", identity_val, vector<fc::variant>{ mutable_variant_object()
                                                                              ("property", "owner")
                                                                              ("type", "account")
                                                                              ("data", to_uint8_vector(N(alice)))
                                                                              ("memo", "claiming onwership")
                                                                              ("confidence", 100)
               }));
   fc::variant certrow = get_certrow(identity_val, "owner", 0, "alice");
   BOOST_REQUIRE_EQUAL( true, certrow.is_object() );
   BOOST_REQUIRE_EQUAL( "owner", certrow["property"].as_string() );
   BOOST_REQUIRE_EQUAL( 0, certrow["trusted"].as_uint64() );
   BOOST_REQUIRE_EQUAL( "alice", certrow["certifier"].as_string() );
   BOOST_REQUIRE_EQUAL( 100, certrow["confidence"].as_uint64() );
   BOOST_REQUIRE_EQUAL( "account", certrow["type"].as_string() );
   BOOST_REQUIRE_EQUAL( N(alice), to_uint64(certrow["data"]) );

//检查alice作用域中的singleton“account”是否包含标识
   fc::variant acntrow = get_accountrow("alice");
   BOOST_REQUIRE_EQUAL( true, certrow.is_object() );
   BOOST_REQUIRE_EQUAL( identity_val, acntrow["identity"].as_uint64() );

//所有权由Alice认证，但不是由区块生产商或区块生产商信任的人认证。
   BOOST_REQUIRE_EQUAL(0, get_owner_for_identity(identity_val));
   BOOST_REQUIRE_EQUAL(0, get_identity_for_account("alice"));

//删除Bob的证书
   BOOST_REQUIRE_EQUAL(success(), certify("bob", identity_val, vector<fc::variant>{ mutable_variant_object()
               ("property", "owner")
               ("type", "account")
               ("data", to_uint8_vector(N(alice)))
               ("memo", "claiming onwership")
               ("confidence", 0)
               }));
//alice作用域中的singleton“account”仍应包含标识
   acntrow = get_accountrow("alice");
   BOOST_REQUIRE_EQUAL( true, certrow.is_object() );
   BOOST_REQUIRE_EQUAL( identity_val, acntrow["identity"].as_uint64() );

//删除所有者证书
   BOOST_REQUIRE_EQUAL(success(), certify("alice", identity_val, vector<fc::variant>{ mutable_variant_object()
                                                                              ("property", "owner")
                                                                              ("type", "account")
                                                                              ("data", to_uint8_vector(N(alice)))
                                                                              ("memo", "claiming onwership")
                                                                              ("confidence", 0)
               }));
   certrow = get_certrow(identity_val, "owner", 0, "alice");
   BOOST_REQUIRE_EQUAL(true, certrow.is_null());

//检查alice作用域中的singleton“account”是否不包含标识
   acntrow = get_accountrow("alice");
   BOOST_REQUIRE_EQUAL(true, certrow.is_null());

} FC_LOG_AND_RETHROW() //证明业主

BOOST_FIXTURE_TEST_CASE( owner_certified_by_producer, identity_tester ) try {
   BOOST_REQUIRE_EQUAL(success(), create_identity("alice", identity_val));

//由块生产商验证所有者，应生成可信的证书
   BOOST_REQUIRE_EQUAL(success(), certify( producer_name, identity_val, vector<fc::variant>{ mutable_variant_object()
               ("property", "owner")
               ("type", "account")
               ("data", to_uint8_vector(N(alice)))
               ("memo", "")
               ("confidence", 100)
               }));
   fc::variant certrow = get_certrow(identity_val, "owner", 1, producer_name);
   BOOST_REQUIRE_EQUAL( true, certrow.is_object() );
   BOOST_REQUIRE_EQUAL( "owner", certrow["property"].as_string() );
   BOOST_REQUIRE_EQUAL( 1, certrow["trusted"].as_uint64() );
   BOOST_REQUIRE_EQUAL( producer_name, certrow["certifier"].as_string() );
   BOOST_REQUIRE_EQUAL( N(alice), to_uint64(certrow["data"]) );

//那一行的未认证副本不应该存在
   BOOST_REQUIRE_EQUAL( true, get_certrow(identity_val, "owner", 0, producer_name).is_null());

//爱丽丝还没有认领身份——她还不是正式的主人。
   BOOST_REQUIRE_EQUAL(0, get_owner_for_identity(identity_val));
   BOOST_REQUIRE_EQUAL(0, get_identity_for_account("alice"));


//爱丽丝声称
   BOOST_REQUIRE_EQUAL(success(), certify("alice", identity_val, vector<fc::variant>{ mutable_variant_object()
               ("property", "owner")
               ("type", "account")
               ("data", to_uint8_vector(N(alice)))
               ("memo", "claiming onwership")
               ("confidence", 100)
               }));
   BOOST_REQUIRE_EQUAL( true, get_certrow(identity_val, "owner", 0, "alice").is_object());

//现在爱丽丝应该是正式的主人了
   BOOST_REQUIRE_EQUAL(N(alice), get_owner_for_identity(identity_val));
   BOOST_REQUIRE_EQUAL(identity_val, get_identity_for_account("alice"));

//块生产商取消所有权
   BOOST_REQUIRE_EQUAL(success(), certify(producer_name, identity_val, vector<fc::variant>{ mutable_variant_object()
               ("property", "owner")
               ("type", "account")
               ("data", to_uint8_vector(N(alice)))
               ("memo", "")
               ("confidence", 0)
               }));
//Alice的自我认证仍然存在
   BOOST_REQUIRE_EQUAL( true, get_certrow(identity_val, "owner", 0, "alice").is_object());
//但现在她不是正式的主人了
   BOOST_REQUIRE_EQUAL(0, get_owner_for_identity(identity_val));
   BOOST_REQUIRE_EQUAL(0, get_identity_for_account("alice"));

} FC_LOG_AND_RETHROW() //所有人经生产商认证

BOOST_FIXTURE_TEST_CASE( owner_certified_by_trusted_account, identity_tester ) try {
   BOOST_REQUIRE_EQUAL(success(), create_identity("bob", identity_val));

//艾丽丝声称鲍勃创造的身份
   BOOST_REQUIRE_EQUAL(success(), certify("alice", identity_val, vector<fc::variant>{ mutable_variant_object()
               ("property", "owner")
               ("type", "account")
               ("data", to_uint8_vector(N(alice)))
               ("memo", "claiming onwership")
               ("confidence", 100)
               }));
   BOOST_REQUIRE_EQUAL( true, get_certrow(identity_val, "owner", 0, "alice").is_object());
//艾丽斯认领了身份，但还没有得到证明——她不是正式的主人。
   BOOST_REQUIRE_EQUAL(0, get_owner_for_identity(identity_val));
   BOOST_REQUIRE_EQUAL(0, get_identity_for_account("alice"));

//块状生产商信任Bob
   BOOST_REQUIRE_EQUAL(success(), settrust(producer_name, "bob", 1));
   BOOST_REQUIRE_EQUAL(true, get_trust(producer_name, "bob"));

//Bob（可信帐户）证明Alice的所有权，它应该导致可信证书
   BOOST_REQUIRE_EQUAL(success(), certify("bob", identity_val, vector<fc::variant>{ mutable_variant_object()
               ("property", "owner")
               ("type", "account")
               ("data", to_uint8_vector(N(alice)))
               ("memo", "")
               ("confidence", 100)
               }));
   BOOST_REQUIRE_EQUAL( true, get_certrow(identity_val, "owner", 1, "bob").is_object() );
//不应存在不受信任的行
   BOOST_REQUIRE_EQUAL( true, get_certrow(identity_val, "owner", 0, "bob").is_null() );

//现在爱丽丝应该是正式的主人了
   BOOST_REQUIRE_EQUAL(N(alice), get_owner_for_identity(identity_val));
   BOOST_REQUIRE_EQUAL(identity_val, get_identity_for_account("alice"));

//阻止生成器停止信任Bob
   BOOST_REQUIRE_EQUAL(success(), settrust(producer_name, "bob", 0));
   BOOST_REQUIRE_EQUAL(false, get_trust(producer_name, "bob"));

//Bob的认证仍然被标记为可信
   BOOST_REQUIRE_EQUAL( true, get_certrow(identity_val, "owner", 1, "bob").is_object() );

//但是现在爱丽丝不应该是正式的主人
   BOOST_REQUIRE_EQUAL(0, get_owner_for_identity(identity_val));
   BOOST_REQUIRE_EQUAL(0, get_identity_for_account("alice"));

} FC_LOG_AND_RETHROW() //所有者\通过\可信\帐户认证\u

BOOST_FIXTURE_TEST_CASE( owner_certification_becomes_trusted, identity_tester ) try {
   BOOST_REQUIRE_EQUAL(success(), create_identity("bob", identity_val));

//Bob（目前还不可信）证明Alice的所有权，它应该导致不可信的认证
   BOOST_REQUIRE_EQUAL(success(), certify("bob", identity_val, vector<fc::variant>{ mutable_variant_object()
               ("property", "owner")
               ("type", "account")
               ("data", to_uint8_vector(N(alice)))
               ("memo", "")
               ("confidence", 100)
               }));
   BOOST_REQUIRE_EQUAL( true, get_certrow(identity_val, "owner", 0, "bob").is_object() );
//不应存在受信任的行
   BOOST_REQUIRE_EQUAL( true, get_certrow(identity_val, "owner", 1, "bob").is_null() );

//艾丽丝声称鲍勃创造的身份
   BOOST_REQUIRE_EQUAL(success(), certify("alice", identity_val, vector<fc::variant>{ mutable_variant_object()
               ("property", "owner")
               ("type", "account")
               ("data", to_uint8_vector(N(alice)))
               ("memo", "claiming onwership")
               ("confidence", 100)
               }));
   BOOST_REQUIRE_EQUAL( true, get_certrow(identity_val, "owner", 0, "alice").is_object());
//爱丽丝认领了身份，但只有不受信任的账户才能证明身份——她不是官方所有者。
   BOOST_REQUIRE_EQUAL(0, get_owner_for_identity(identity_val));
   BOOST_REQUIRE_EQUAL(0, get_identity_for_account("alice"));

//块状生产商信任Bob
   BOOST_REQUIRE_EQUAL(success(), settrust(producer_name, "bob", 1));
   BOOST_REQUIRE_EQUAL(true, get_trust(producer_name, "bob"));

//鲍勃的旧认证仍然不应该被视为可信的
   BOOST_REQUIRE_EQUAL( true, get_certrow(identity_val, "owner", 0, "bob").is_object() );

//但实际上，鲍勃的认证应该得到信任
   BOOST_REQUIRE_EQUAL(N(alice), get_owner_for_identity(identity_val));
   BOOST_REQUIRE_EQUAL(identity_val, get_identity_for_account("alice"));

} FC_LOG_AND_RETHROW() //拥有者\认证\成为\信任

BOOST_FIXTURE_TEST_CASE( ownership_contradiction, identity_tester ) try {
   BOOST_REQUIRE_EQUAL(success(), create_identity("alice", identity_val));

//爱丽丝声称身份
   BOOST_REQUIRE_EQUAL(success(), certify("alice", identity_val, vector<fc::variant>{ mutable_variant_object()
               ("property", "owner")
               ("type", "account")
               ("data", to_uint8_vector(N(alice)))
               ("memo", "claiming onwership")
               ("confidence", 100)
               }));

//块生产商证明爱丽丝的所有权
   BOOST_REQUIRE_EQUAL(success(), certify(producer_name, identity_val, vector<fc::variant>{ mutable_variant_object()
               ("property", "owner")
               ("type", "account")
               ("data", to_uint8_vector(N(alice)))
               ("memo", "")
               ("confidence", 100)
               }));
   BOOST_REQUIRE_EQUAL( true, get_certrow(identity_val, "owner", 1, producer_name).is_object() );

//现在爱丽丝是身份的正式拥有者
   BOOST_REQUIRE_EQUAL(N(alice), get_owner_for_identity(identity_val));
   BOOST_REQUIRE_EQUAL(identity_val, get_identity_for_account("alice"));

//鲍勃声称身份
   BOOST_REQUIRE_EQUAL(success(), certify("bob", identity_val, vector<fc::variant>{ mutable_variant_object()
               ("property", "owner")
               ("type", "account")
               ("data", to_uint8_vector(N(bob)))
               ("memo", "claiming onwership")
               ("confidence", 100)
               }));


//布洛克制片人信托卡罗尔
   BOOST_REQUIRE_EQUAL(success(), settrust(producer_name, "carol", 1));
   BOOST_REQUIRE_EQUAL(true, get_trust(producer_name, "carol"));

//另一个受信任的委托人证明Bob的身份（对已经向Alice证明的身份）
   BOOST_REQUIRE_EQUAL(success(), certify("carol", identity_val, vector<fc::variant>{ mutable_variant_object()
               ("property", "owner")
               ("type", "account")
               ("data", to_uint8_vector(N(bob)))
               ("memo", "")
               ("confidence", 100)
               }));
   BOOST_REQUIRE_EQUAL( true, get_certrow(identity_val, "owner", 1, producer_name).is_object() );

//现在，爱丽丝和鲍勃都不是官方所有者，因为我们有两个相互矛盾的可信证书
   BOOST_REQUIRE_EQUAL(0, get_owner_for_identity(identity_val));
   BOOST_REQUIRE_EQUAL(0, get_identity_for_account("alice"));

} FC_LOG_AND_RETHROW() //所有权矛盾

BOOST_AUTO_TEST_SUITE_END()
