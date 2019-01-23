
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
#include <eosio/chain/name.hpp>
#include <eosio/chain/chain_id_type.hpp>

#include <chainbase/chainbase.hpp>

#include <fc/container/flat_fwd.hpp>
#include <fc/io/varint.hpp>
#include <fc/io/enum_type.hpp>
#include <fc/crypto/sha224.hpp>
#include <fc/optional.hpp>
#include <fc/safe.hpp>
#include <fc/container/flat.hpp>
#include <fc/string.hpp>
#include <fc/io/raw.hpp>
#include <fc/static_variant.hpp>
#include <fc/smart_ref_fwd.hpp>
#include <fc/crypto/ripemd160.hpp>
#include <fc/fixed_string.hpp>
#include <fc/crypto/private_key.hpp>

#include <memory>
#include <vector>
#include <deque>
#include <cstdint>

#define OBJECT_CTOR1(NAME) \
    NAME() = delete; \
    public: \
    template<typename Constructor, typename Allocator> \
    NAME(Constructor&& c, chainbase::allocator<Allocator>) \
    { c(*this); }
#define OBJECT_CTOR2_MACRO(x, y, field) ,field(a)
#define OBJECT_CTOR2(NAME, FIELDS) \
    NAME() = delete; \
    public: \
    template<typename Constructor, typename Allocator> \
    NAME(Constructor&& c, chainbase::allocator<Allocator> a) \
    : id(0) BOOST_PP_SEQ_FOR_EACH(OBJECT_CTOR2_MACRO, _, FIELDS) \
    { c(*this); }
#define OBJECT_CTOR(...) BOOST_PP_OVERLOAD(OBJECT_CTOR, __VA_ARGS__)(__VA_ARGS__)

#define _V(n, v)  fc::mutable_variant_object(n, v)

namespace eosio { namespace chain {
   using                               std::map;
   using                               std::vector;
   using                               std::unordered_map;
   using                               std::string;
   using                               std::deque;
   using                               std::shared_ptr;
   using                               std::weak_ptr;
   using                               std::unique_ptr;
   using                               std::set;
   using                               std::pair;
   using                               std::make_pair;
   using                               std::enable_shared_from_this;
   using                               std::tie;
   using                               std::move;
   using                               std::forward;
   using                               std::to_string;
   using                               std::all_of;

   using                               fc::path;
   using                               fc::smart_ref;
   using                               fc::variant_object;
   using                               fc::variant;
   using                               fc::enum_type;
   using                               fc::optional;
   using                               fc::unsigned_int;
   using                               fc::signed_int;
   using                               fc::time_point_sec;
   using                               fc::time_point;
   using                               fc::safe;
   using                               fc::flat_map;
   using                               fc::flat_set;
   using                               fc::static_variant;
   using                               fc::ecc::range_proof_type;
   using                               fc::ecc::range_proof_info;
   using                               fc::ecc::commitment_type;

   using public_key_type  = fc::crypto::public_key;
   using private_key_type = fc::crypto::private_key;
   using signature_type   = fc::crypto::signature;

   struct void_t{};

   using chainbase::allocator;
   using shared_string = boost::interprocess::basic_string<char, std::char_traits<char>, allocator<char>>;
   template<typename T>
   using shared_vector = boost::interprocess::vector<T, allocator<T>>;
   template<typename T>
   using shared_set = boost::interprocess::set<T, std::less<T>, allocator<T>>;

   /*
    *对于boost进程间的bug，我们将blob数据移动到共享的字符串
    *这个包装器允许我们继续这样做，同时还可以对
    *序列化和到/来自变量
    **/

   class shared_blob : public shared_string {
      public:
         shared_blob() = delete;
         shared_blob(shared_blob&&) = default;

         shared_blob(const shared_blob& s)
         :shared_string(s.get_allocator())
         {
            assign(s.c_str(), s.size());
         }


         shared_blob& operator=(const shared_blob& s) {
            assign(s.c_str(), s.size());
            return *this;
         }

         shared_blob& operator=(shared_blob&& ) = default;

         template <typename InputIterator>
         shared_blob(InputIterator f, InputIterator l, const allocator_type& a)
         :shared_string(f,l,a)
         {}

         shared_blob(const allocator_type& a)
         :shared_string(a)
         {}
   };

   using action_name      = name;
   using scope_name       = name;
   using account_name     = name;
   using permission_name  = name;
   using table_name       = name;


   /*
    *在此列出所有命名空间中的所有对象类型，以便
    *易于在调试输出中反映和显示。如果是第三方
    *想要扩展核心代码，那么他们必须更改
    *压缩的_object:：type字段从enum_type到uint16以避免
    *将打包的对象转换为JSON或从JSON转换为JSON时出现警告。
    *
    *未使用的枚举可用于新用途，但其他情况下，偏移量
    *在此枚举中，可能会共享内存中断
    **/

   enum object_type
   {
      null_object_type = 0,
      account_object_type,
      account_sequence_object_type,
      permission_object_type,
      permission_usage_object_type,
      permission_link_object_type,
      UNUSED_action_code_object_type,
      key_value_object_type,
      index64_object_type,
      index128_object_type,
      index256_object_type,
      index_double_object_type,
      index_long_double_object_type,
      global_property_object_type,
      dynamic_global_property_object_type,
      block_summary_object_type,
      transaction_object_type,
      generated_transaction_object_type,
      producer_object_type,
      UNUSED_chain_property_object_type,
account_control_history_object_type,     ///<由历史\插件定义
      UNUSED_account_transaction_history_object_type,
      UNUSED_transaction_history_object_type,
public_key_history_object_type,          ///<由历史\插件定义
      UNUSED_balance_object_type,
      UNUSED_staked_balance_object_type,
      UNUSED_producer_votes_object_type,
      UNUSED_producer_schedule_object_type,
      UNUSED_proxy_vote_object_type,
      UNUSED_scope_sequence_object_type,
      table_id_object_type,
      resource_limits_object_type,
      resource_usage_object_type,
      resource_limits_state_object_type,
      resource_limits_config_object_type,
account_history_object_type,              ///<由历史\插件定义
action_history_object_type,               ///<由历史\插件定义
      reversible_block_object_type,
OBJECT_TYPE_COUNT ///<包含不同对象类型数量的Sentry值
   };

   class account_object;
   class producer_object;

   using block_id_type       = fc::sha256;
   using checksum_type       = fc::sha256;
   using checksum256_type    = fc::sha256;
   using checksum512_type    = fc::sha512;
   using checksum160_type    = fc::ripemd160;
   using transaction_id_type = checksum_type;
   using digest_type         = checksum_type;
   using weight_type         = uint16_t;
   using block_num_type      = uint32_t;
   using share_type          = int64_t;
   using int128_t            = __int128;
   using uint128_t           = unsigned __int128;
   using bytes               = vector<char>;


   /*
    *扩展名以类型作为前缀，是一个可以
    *由意识到的代码解释，被不意识到的代码忽略。
    **/

   typedef vector<std::pair<uint16_t,vector<char>>> extensions_type;


} }  //EOSIO：链

FC_REFLECT( eosio::chain::void_t, )
