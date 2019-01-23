
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once
#include <eosio/chain/config.hpp>

#include <stdint.h>
#include <fc/time.hpp>
#include <fc/variant.hpp>
#include <fc/string.hpp>
#include <fc/optional.hpp>
#include <fc/exception/exception.hpp>

namespace eosio { namespace chain {

   /*
   *该类在块头中用于表示块时间。
   *它是一个以毫秒为单位的参数化类，并且
   *和以毫秒为单位的间隔，并计算插槽数。
   */

   template<uint16_t IntervalMs, uint64_t EpochMs>
   class block_timestamp {
      public:
         explicit block_timestamp( uint32_t s=0 ) :slot(s){}

         block_timestamp(const fc::time_point& t) {
            set_time_point(t);
         }

         block_timestamp(const fc::time_point_sec& t) {
            set_time_point(t);
         }

         static block_timestamp maximum() { return block_timestamp( 0xffff ); }
         static block_timestamp min() { return block_timestamp(0); }

         block_timestamp next() const {
            EOS_ASSERT( std::numeric_limits<uint32_t>::max() - slot >= 1, fc::overflow_exception, "block timestamp overflow" );
            auto result = block_timestamp(*this);
            result.slot += 1;
            return result;
         }

         fc::time_point to_time_point() const {
            return (fc::time_point)(*this);
         }

         operator fc::time_point() const {
            int64_t msec = slot * (int64_t)IntervalMs;
            msec += EpochMs;
            return fc::time_point(fc::milliseconds(msec));
         }

         void operator = (const fc::time_point& t ) {
            set_time_point(t);
         }

         bool   operator > ( const block_timestamp& t )const   { return slot >  t.slot; }
         bool   operator >=( const block_timestamp& t )const   { return slot >= t.slot; }
         bool   operator < ( const block_timestamp& t )const   { return slot <  t.slot; }
         bool   operator <=( const block_timestamp& t )const   { return slot <= t.slot; }
         bool   operator ==( const block_timestamp& t )const   { return slot == t.slot; }
         bool   operator !=( const block_timestamp& t )const   { return slot != t.slot; }
         uint32_t slot;

      private:
      void set_time_point(const fc::time_point& t) {
         auto micro_since_epoch = t.time_since_epoch();
         auto msec_since_epoch  = micro_since_epoch.count() / 1000;
         slot = ( msec_since_epoch - EpochMs ) / IntervalMs;
      }

      void set_time_point(const fc::time_point_sec& t) {
         uint64_t  sec_since_epoch = t.sec_since_epoch();
         slot = (sec_since_epoch * 1000 - EpochMs) / IntervalMs;
      }
}; //封锁时间戳

   typedef block_timestamp<config::block_interval_ms,config::block_timestamp_epoch> block_timestamp_type; 

} } ///EOSIO：链


#include <fc/reflect/reflect.hpp>
FC_REFLECT(eosio::chain::block_timestamp_type, (slot))

namespace fc {
  template<uint16_t IntervalMs, uint64_t EpochMs>
  void to_variant(const eosio::chain::block_timestamp<IntervalMs,EpochMs>& t, fc::variant& v) {
     to_variant( (fc::time_point)t, v);
  }

  template<uint16_t IntervalMs, uint64_t EpochMs>
  void from_variant(const fc::variant& v, eosio::chain::block_timestamp<IntervalMs,EpochMs>& t) {
     t = v.as<fc::time_point>();
  }
}

#ifdef _MSC_VER
  #pragma warning (pop)
#endif ///ifdef诳msc诳ver
