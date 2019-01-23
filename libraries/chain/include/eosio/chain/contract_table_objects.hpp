
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

#include <eosio/chain/database_utils.hpp>
#include <eosio/chain/contract_types.hpp>
#include <eosio/chain/multi_index_includes.hpp>

#include <array>
#include <type_traits>

namespace eosio { namespace chain {

   /*
    *@brief table_id_object类跟踪（scope、code、table）到不透明标识符的映射
    **/

   class table_id_object : public chainbase::object<table_id_object_type, table_id_object> {
      OBJECT_CTOR(table_id_object)

      id_type        id;
      account_name   code;
      scope_name     scope;
      table_name     table;
      account_name   payer;
uint32_t       count = 0; ///表中的元素数
   };

   struct by_code_scope_table;

   using table_id_multi_index = chainbase::shared_multi_index_container<
      table_id_object,
      indexed_by<
         ordered_unique<tag<by_id>,
            member<table_id_object, table_id_object::id_type, &table_id_object::id>
         >,
         ordered_unique<tag<by_code_scope_table>,
            composite_key< table_id_object,
               member<table_id_object, account_name, &table_id_object::code>,
               member<table_id_object, scope_name,   &table_id_object::scope>,
               member<table_id_object, table_name,   &table_id_object::table>
            >
         >
      >
   >;

   using table_id = table_id_object::id_type;

   struct by_scope_primary;
   struct by_scope_secondary;
   struct by_scope_tertiary;


   struct key_value_object : public chainbase::object<key_value_object_type, key_value_object> {
      OBJECT_CTOR(key_value_object, (value))

      typedef uint64_t key_type;
      static const int number_of_keys = 1;

      id_type               id;
      table_id              t_id;
      uint64_t              primary_key;
      account_name          payer = 0;
      shared_blob           value;
   };

   using key_value_index = chainbase::shared_multi_index_container<
      key_value_object,
      indexed_by<
         ordered_unique<tag<by_id>, member<key_value_object, key_value_object::id_type, &key_value_object::id>>,
         ordered_unique<tag<by_scope_primary>,
            composite_key< key_value_object,
               member<key_value_object, table_id, &key_value_object::t_id>,
               member<key_value_object, uint64_t, &key_value_object::primary_key>
            >,
            composite_key_compare< std::less<table_id>, std::less<uint64_t> >
         >
      >
   >;

   struct by_primary;
   struct by_secondary;

   template<typename SecondaryKey, uint64_t ObjectTypeId, typename SecondaryKeyLess = std::less<SecondaryKey> >
   struct secondary_index
   {
      struct index_object : public chainbase::object<ObjectTypeId,index_object> {
         OBJECT_CTOR(index_object)
         typedef SecondaryKey secondary_key_type;

         typename chainbase::object<ObjectTypeId,index_object>::id_type       id;
         table_id      t_id;
         uint64_t      primary_key;
         account_name  payer = 0;
         SecondaryKey  secondary_key;
      };


      typedef chainbase::shared_multi_index_container<
         index_object,
         indexed_by<
            ordered_unique<tag<by_id>, member<index_object, typename index_object::id_type, &index_object::id>>,
            ordered_unique<tag<by_primary>,
               composite_key< index_object,
                  member<index_object, table_id, &index_object::t_id>,
                  member<index_object, uint64_t, &index_object::primary_key>
               >,
               composite_key_compare< std::less<table_id>, std::less<uint64_t> >
            >,
            ordered_unique<tag<by_secondary>,
               composite_key< index_object,
                  member<index_object, table_id, &index_object::t_id>,
                  member<index_object, SecondaryKey, &index_object::secondary_key>,
                  member<index_object, uint64_t, &index_object::primary_key>
               >,
               composite_key_compare< std::less<table_id>, SecondaryKeyLess, std::less<uint64_t> >
            >
         >
      > index_index;
   };

   typedef secondary_index<uint64_t,index64_object_type>::index_object   index64_object;
   typedef secondary_index<uint64_t,index64_object_type>::index_index    index64_index;

   typedef secondary_index<uint128_t,index128_object_type>::index_object index128_object;
   typedef secondary_index<uint128_t,index128_object_type>::index_index  index128_index;

   typedef std::array<uint128_t, 2> key256_t;
   typedef secondary_index<key256_t,index256_object_type>::index_object index256_object;
   typedef secondary_index<key256_t,index256_object_type>::index_index  index256_index;

   struct soft_double_less {
      bool operator()( const float64_t& lhs, const float64_t& rhs ) const {
         return f64_lt( lhs, rhs );
      }
   };

   struct soft_long_double_less {
      bool operator()( const float128_t& lhs, const float128_t& rhs ) const {
         return f128_lt( lhs, rhs );
      }
   };

   /*
    *此索引支持确定性软件实现double作为辅助密钥。
    *
    *软件双重实现使用伯克利Softfloat库（第3版）。
    **/


   typedef secondary_index<float64_t,index_double_object_type,soft_double_less>::index_object  index_double_object;
   typedef secondary_index<float64_t,index_double_object_type,soft_double_less>::index_index   index_double_index;

   /*
    *此索引支持以long double作为辅助键的确定性软件实现。
    *
    *软件long double实现使用的是Berkeley Softfloat库（第3版）。
    **/

   typedef secondary_index<float128_t,index_long_double_object_type,soft_long_double_less>::index_object  index_long_double_object;
   typedef secondary_index<float128_t,index_long_double_object_type,soft_long_double_less>::index_index   index_long_double_index;

   template<typename T>
   struct secondary_key_traits {
      using value_type = std::enable_if_t<std::is_integral<T>::value, T>;

      static_assert( std::numeric_limits<value_type>::is_specialized, "value_type does not have specialized numeric_limits" );

      static constexpr value_type true_lowest() { return std::numeric_limits<value_type>::lowest(); }
      static constexpr value_type true_highest() { return std::numeric_limits<value_type>::max(); }
   };

   template<size_t N>
   struct secondary_key_traits<std::array<uint128_t, N>> {
   private:
      static constexpr uint128_t max_uint128 = (static_cast<uint128_t>(std::numeric_limits<uint64_t>::max()) << 64) | std::numeric_limits<uint64_t>::max();
      static_assert( std::numeric_limits<uint128_t>::max() == max_uint128, "numeric_limits for uint128_t is not properly defined" );

   public:
      using value_type = std::array<uint128_t, N>;

      static value_type true_lowest() {
         value_type arr;
         return arr;
      }

      static value_type true_highest() {
         value_type arr;
         for( auto& v : arr ) {
            v = std::numeric_limits<uint128_t>::max();
         }
         return arr;
      }
   };

   template<>
   struct secondary_key_traits<float64_t> {
      using value_type = float64_t;

      static value_type true_lowest() {
         return f64_negative_infinity();
      }

      static value_type true_highest() {
         return f64_positive_infinity();
      }
   };

   template<>
   struct secondary_key_traits<float128_t> {
      using value_type = float128_t;

      static value_type true_lowest() {
         return f128_negative_infinity();
      }

      static value_type true_highest() {
         return f128_positive_infinity();
      }
   };

   /*
    *要从索引类型映射到最佳标记的帮助器模板
    *按表ID遍历时使用
    **/

   template<typename T>
   struct object_to_table_id_tag;

#define DECLARE_TABLE_ID_TAG( object, tag ) \
   template<> \
   struct object_to_table_id_tag<object> { \
      using tag_type = tag;\
   };

   DECLARE_TABLE_ID_TAG(key_value_object, by_scope_primary)
   DECLARE_TABLE_ID_TAG(index64_object, by_primary)
   DECLARE_TABLE_ID_TAG(index128_object, by_primary)
   DECLARE_TABLE_ID_TAG(index256_object, by_primary)
   DECLARE_TABLE_ID_TAG(index_double_object, by_primary)
   DECLARE_TABLE_ID_TAG(index_long_double_object, by_primary)

   template<typename T>
   using object_to_table_id_tag_t = typename object_to_table_id_tag<T>::tag_type;

namespace config {
   template<>
   struct billable_size<table_id_object> {
static const uint64_t overhead = overhead_per_row_per_index_ram_bytes * 2;  ///<2X索引内部键和代码、作用域、表的开销
static const uint64_t value = 44 + overhead; ///<44字节，用于常量字段+开销
   };

   template<>
   struct billable_size<key_value_object> {
static const uint64_t overhead = overhead_per_row_per_index_ram_bytes * 2;  ///<潜在的单行表、2X索引内部键和主键的开销
static const uint64_t value = 32 + 8 + 4 + overhead; ///<32字节用于常量大小字段，8字节用于指向矢量数据的指针，4字节用于向量大小+开销
   };

   template<>
   struct billable_size<index64_object> {
static const uint64_t overhead = overhead_per_row_per_index_ram_bytes * 3;  ///<潜在的单行表、3x索引内部键、主键和主键+辅助键的开销
static const uint64_t value = 24 + 8 + overhead; //固定字段：/<24字节+键8字节+开销
   };

   template<>
   struct billable_size<index128_object> {
static const uint64_t overhead = overhead_per_row_per_index_ram_bytes * 3;  ///<潜在的单行表、3x索引内部键、主键和主键+辅助键的开销
static const uint64_t value = 24 + 16 + overhead; //固定字段：/<24字节+16字节键+开销
   };

   template<>
   struct billable_size<index256_object> {
static const uint64_t overhead = overhead_per_row_per_index_ram_bytes * 3;  ///<潜在的单行表、3x索引内部键、主键和主键+辅助键的开销
static const uint64_t value = 24 + 32 + overhead; //固定字段：/<24字节+32字节键+开销
   };

   template<>
   struct billable_size<index_double_object> {
static const uint64_t overhead = overhead_per_row_per_index_ram_bytes * 3;  ///<潜在的单行表、3x索引内部键、主键和主键+辅助键的开销
static const uint64_t value = 24 + 8 + overhead; //固定字段：/<24字节+键8字节+开销
   };

   template<>
   struct billable_size<index_long_double_object> {
static const uint64_t overhead = overhead_per_row_per_index_ram_bytes * 3;  ///<潜在的单行表、3x索引内部键、主键和主键+辅助键的开销
static const uint64_t value = 24 + 16 + overhead; //固定字段：/<24字节+16字节键+开销
   };

} //命名空间配置

} }  //命名空间eosio:：chain

CHAINBASE_SET_INDEX_TYPE(eosio::chain::table_id_object, eosio::chain::table_id_multi_index)
CHAINBASE_SET_INDEX_TYPE(eosio::chain::key_value_object, eosio::chain::key_value_index)

CHAINBASE_SET_INDEX_TYPE(eosio::chain::index64_object, eosio::chain::index64_index)
CHAINBASE_SET_INDEX_TYPE(eosio::chain::index128_object, eosio::chain::index128_index)
CHAINBASE_SET_INDEX_TYPE(eosio::chain::index256_object, eosio::chain::index256_index)
CHAINBASE_SET_INDEX_TYPE(eosio::chain::index_double_object, eosio::chain::index_double_index)
CHAINBASE_SET_INDEX_TYPE(eosio::chain::index_long_double_object, eosio::chain::index_long_double_index)

FC_REFLECT(eosio::chain::table_id_object, (code)(scope)(table)(payer)(count) )
FC_REFLECT(eosio::chain::key_value_object, (primary_key)(payer)(value) )

#define REFLECT_SECONDARY(type)\
  FC_REFLECT(type, (primary_key)(payer)(secondary_key) )

REFLECT_SECONDARY(eosio::chain::index64_object)
REFLECT_SECONDARY(eosio::chain::index128_object)
REFLECT_SECONDARY(eosio::chain::index256_object)
REFLECT_SECONDARY(eosio::chain::index_double_object)
REFLECT_SECONDARY(eosio::chain::index_long_double_object)
