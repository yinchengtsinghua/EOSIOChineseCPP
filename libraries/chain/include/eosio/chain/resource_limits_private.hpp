
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once
#include <eosio/chain/types.hpp>
#include <eosio/chain/config.hpp>
#include <eosio/chain/exceptions.hpp>

#include "multi_index_includes.hpp"


namespace eosio { namespace chain { namespace resource_limits {

   namespace impl {
      template<typename T>
      ratio<T> make_ratio(T n, T d) {
         return ratio<T>{n, d};
      }

      template<typename T>
      T operator* (T value, const ratio<T>& r) {
         EOS_ASSERT(r.numerator == T(0) || std::numeric_limits<T>::max() / r.numerator >= value, rate_limiting_state_inconsistent, "Usage exceeds maximum value representable after extending for precision");
         return (value * r.numerator) / r.denominator;
      }

      template<typename UnsignedIntType>
      constexpr UnsignedIntType integer_divide_ceil(UnsignedIntType num, UnsignedIntType den ) {
         return (num / den) + ((num % den) > 0 ? 1 : 0);
      }


      template<typename LesserIntType, typename GreaterIntType>
      constexpr bool is_valid_downgrade_cast =
std::is_integral<LesserIntType>::value &&  //移除类型不是整型的重载
std::is_integral<GreaterIntType>::value && //移除类型不是整型的重载
(std::numeric_limits<LesserIntType>::max() <= std::numeric_limits<GreaterIntType>::max()); //删除升级而不是降级的重载

      /*
       *对匹配整数类型的签名进行专门化
       **/

      template<typename LesserIntType, typename GreaterIntType>
      constexpr auto downgrade_cast(GreaterIntType val) ->
         std::enable_if_t<is_valid_downgrade_cast<LesserIntType,GreaterIntType> && std::is_signed<LesserIntType>::value == std::is_signed<GreaterIntType>::value, LesserIntType>
      {
         const GreaterIntType max = std::numeric_limits<LesserIntType>::max();
         const GreaterIntType min = std::numeric_limits<LesserIntType>::min();
         EOS_ASSERT( val >= min && val <= max, rate_limiting_state_inconsistent, "Casting a higher bit integer value ${v} to a lower bit integer value which cannot contain the value, valid range is [${min}, ${max}]", ("v", val)("min", min)("max",max) );
         return LesserIntType(val);
      };

      /*
       *针对签名不匹配整数类型的专门化
       **/

      template<typename LesserIntType, typename GreaterIntType>
      constexpr auto downgrade_cast(GreaterIntType val) ->
         std::enable_if_t<is_valid_downgrade_cast<LesserIntType,GreaterIntType> && std::is_signed<LesserIntType>::value != std::is_signed<GreaterIntType>::value, LesserIntType>
      {
         const GreaterIntType max = std::numeric_limits<LesserIntType>::max();
         const GreaterIntType min = 0;
         EOS_ASSERT( val >= min && val <= max, rate_limiting_state_inconsistent, "Casting a higher bit integer value ${v} to a lower bit integer value which cannot contain the value, valid range is [${min}, ${max}]", ("v", val)("min", min)("max",max) );
         return LesserIntType(val);
      };

      /*
       *此类根据输入累积和指数移动平均值
       *该累加器假定输入数据没有下降。
       *
       *存储的值是精度乘以输入的总和。
       **/

      template<uint64_t Precision = config::rate_limiting_precision>
      struct exponential_moving_average_accumulator
      {
         static_assert( Precision > 0, "Precision must be positive" );
         static constexpr uint64_t max_raw_value = std::numeric_limits<uint64_t>::max() / Precision;

         exponential_moving_average_accumulator()
         : last_ordinal(0)
         , value_ex(0)
         , consumed(0)
         {
         }

uint32_t   last_ordinal;  ///<对平均值有贡献的最后一个期间的序号
uint64_t   value_ex;      ///<当前平均值预乘精度
uint64_t   consumed;       ///<上一个期间的平均值+目前为止当前期间的贡献

         /*
          *返回平均值
          **/

         uint64_t average() const {
            return integer_divide_ceil(value_ex, Precision);
         }

         /*d add（uint64_t units，uint32_t ordinal，uint32_t window_size/*必须为正数*/）
         {
            //在进行任何状态突变之前检查一些数值限制
            eos_assert（单位<=max_raw_value，rate_limiting_state_inconsisted，“用法超过了扩展精度后可表示的最大值”）；
            eos_assert（std:：numeric_limits<decltype（consumed）>：：max（）-consumed>=units，rate_limiting_state_inconsistent，“添加使用时跟踪使用溢出！”）；

            auto value_ex_contrib=降级_cast<uint64_t>（integer_divide_ceil（（uint128_t）units*precision，（uint128_t）window_size））；
            eos_assert（std:：numeric_limits<decltype（value_ex）>：：max（）-value_ex>=value_ex_contrib，rate_limiting_state_inconsistent，“添加用法时累积值溢出！”）；

            如果（最后一个）=序数）{
               eos_断言（ordinal>last_ordinal，resource_limit_exception，“new ordinal不能小于前一个ordinal”）；
               if（（uint64_t）Last_Ordinal+Window_Size>（uint64_t）Ordinal）
                  const auto delta=ordinal-last_ordinal；//clearly 0<delta<window_size
                  const auto decay=制造比率（
                          （uint64_t）窗口大小-增量，
                          （uint64_t）窗口大小
                  ；

                  value_ex=value_ex*衰减；
               }否则{
                  ValueSyx＝0；
               }

               最后一个序数=序数；
               消耗=平均值（）；
            }

            消耗+=单位；
            value_ex+=值_ex_contrib；
         }
      }；

   }

   使用_accumulator=impl：：指数_moving_average_accumulator<>

   /**
    *每一个授权交易的账户都会按交易的全部规模收费。这个对象
    *跟踪该帐户的平均使用情况。
    **/

   struct resource_limits_object : public chainbase::object<resource_limits_object_type, resource_limits_object> {

      OBJECT_CTOR(resource_limits_object)

      id_type id;
      account_name owner;
      bool pending = false;

      int64_t net_weight = -1;
      int64_t cpu_weight = -1;
      int64_t ram_bytes = -1;

   };

   struct by_owner;
   struct by_dirty;

   using resource_limits_index = chainbase::shared_multi_index_container<
      resource_limits_object,
      indexed_by<
         ordered_unique<tag<by_id>, member<resource_limits_object, resource_limits_object::id_type, &resource_limits_object::id>>,
         ordered_unique<tag<by_owner>,
            composite_key<resource_limits_object,
               BOOST_MULTI_INDEX_MEMBER(resource_limits_object, bool, pending),
               BOOST_MULTI_INDEX_MEMBER(resource_limits_object, account_name, owner)
            >
         >
      >
   >;

   struct resource_usage_object : public chainbase::object<resource_usage_object_type, resource_usage_object> {
      OBJECT_CTOR(resource_usage_object)

      id_type id;
      account_name owner;

      usage_accumulator        net_usage;
      usage_accumulator        cpu_usage;

      uint64_t                 ram_usage = 0;
   };

   using resource_usage_index = chainbase::shared_multi_index_container<
      resource_usage_object,
      indexed_by<
         ordered_unique<tag<by_id>, member<resource_usage_object, resource_usage_object::id_type, &resource_usage_object::id>>,
         ordered_unique<tag<by_owner>, member<resource_usage_object, account_name, &resource_usage_object::owner> >
      >
   >;

   class resource_limits_config_object : public chainbase::object<resource_limits_config_object_type, resource_limits_config_object> {
      OBJECT_CTOR(resource_limits_config_object);
      id_type id;

      static_assert( config::block_interval_ms > 0, "config::block_interval_ms must be positive" );
      static_assert( config::block_cpu_usage_average_window_ms >= config::block_interval_ms,
                     "config::block_cpu_usage_average_window_ms cannot be less than config::block_interval_ms" );
      static_assert( config::block_size_average_window_ms >= config::block_interval_ms,
                     "config::block_size_average_window_ms cannot be less than config::block_interval_ms" );


      elastic_limit_parameters cpu_limit_parameters = {EOS_PERCENT(config::default_max_block_cpu_usage, config::default_target_block_cpu_usage_pct), config::default_max_block_cpu_usage, config::block_cpu_usage_average_window_ms / config::block_interval_ms, 1000, {99, 100}, {1000, 999}};
      elastic_limit_parameters net_limit_parameters = {EOS_PERCENT(config::default_max_block_net_usage, config::default_target_block_net_usage_pct), config::default_max_block_net_usage, config::block_size_average_window_ms / config::block_interval_ms, 1000, {99, 100}, {1000, 999}};

      uint32_t account_cpu_usage_average_window = config::account_cpu_usage_average_window_ms / config::block_interval_ms;
      uint32_t account_net_usage_average_window = config::account_net_usage_average_window_ms / config::block_interval_ms;
   };

   using resource_limits_config_index = chainbase::shared_multi_index_container<
      resource_limits_config_object,
      indexed_by<
         ordered_unique<tag<by_id>, member<resource_limits_config_object, resource_limits_config_object::id_type, &resource_limits_config_object::id>>
      >
   >;

   class resource_limits_state_object : public chainbase::object<resource_limits_state_object_type, resource_limits_state_object> {
      OBJECT_CTOR(resource_limits_state_object);
      id_type id;

      /*
       *跟踪块的平均网络使用情况
       **/

      usage_accumulator average_block_net_usage;

      /*
       *跟踪块的平均CPU使用率
       **/

      usage_accumulator average_block_cpu_usage;

      void update_virtual_net_limit( const resource_limits_config_object& cfg );
      void update_virtual_cpu_limit( const resource_limits_config_object& cfg );

      uint64_t pending_net_usage = 0ULL;
      uint64_t pending_cpu_usage = 0ULL;

      uint64_t total_net_weight = 0ULL;
      uint64_t total_cpu_weight = 0ULL;
      uint64_t total_ram_bytes = 0ULL;

      /*
       *块大小平均窗口占用的虚拟字节数
       *如果所有块都处于其最大虚拟大小。这是虚拟的，因为
       *实际最大数据块较少，此虚拟数字仅用于速率限制用户。
       *
       *可能的最小值是max_block_size*block size_average_window_ms/block_interval
       *最高可能值是最低可能值的1000倍
       *
       *这意味着一个帐户在空闲期间最多可以消耗1000倍的带宽。
       *拥挤不堪。
       *
       *当平均块大小小于目标块大小时增加，当
       *平均块大小>目标块大小，上限为最大块大小的1000倍
       *和一个最大尺寸的地板；
       */

      uint64_t virtual_net_limit = 0ULL;

      /*
       *平均值增加
       **/

      uint64_t virtual_cpu_limit = 0ULL;

   };

   using resource_limits_state_index = chainbase::shared_multi_index_container<
      resource_limits_state_object,
      indexed_by<
         ordered_unique<tag<by_id>, member<resource_limits_state_object, resource_limits_state_object::id_type, &resource_limits_state_object::id>>
      >
   >;

} } } ///eosio：：链：：资源限制

CHAINBASE_SET_INDEX_TYPE(eosio::chain::resource_limits::resource_limits_object,        eosio::chain::resource_limits::resource_limits_index)
CHAINBASE_SET_INDEX_TYPE(eosio::chain::resource_limits::resource_usage_object,         eosio::chain::resource_limits::resource_usage_index)
CHAINBASE_SET_INDEX_TYPE(eosio::chain::resource_limits::resource_limits_config_object, eosio::chain::resource_limits::resource_limits_config_index)
CHAINBASE_SET_INDEX_TYPE(eosio::chain::resource_limits::resource_limits_state_object,  eosio::chain::resource_limits::resource_limits_state_index)

FC_REFLECT(eosio::chain::resource_limits::usage_accumulator, (last_ordinal)(value_ex)(consumed))

//@忽略未决
FC_REFLECT(eosio::chain::resource_limits::resource_limits_object, (owner)(net_weight)(cpu_weight)(ram_bytes))
FC_REFLECT(eosio::chain::resource_limits::resource_usage_object,  (owner)(net_usage)(cpu_usage)(ram_usage))
FC_REFLECT(eosio::chain::resource_limits::resource_limits_config_object, (cpu_limit_parameters)(net_limit_parameters)(account_cpu_usage_average_window)(account_net_usage_average_window))
FC_REFLECT(eosio::chain::resource_limits::resource_limits_state_object, (average_block_net_usage)(average_block_cpu_usage)(pending_net_usage)(pending_cpu_usage)(total_net_weight)(total_cpu_weight)(total_ram_bytes)(virtual_net_limit)(virtual_cpu_limit))
