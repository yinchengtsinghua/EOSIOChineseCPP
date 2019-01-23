
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <eosiolib/eosio.hpp>
#include <eosiolib/multi_index.hpp>

using namespace eosio;

namespace snapshot_test {

   struct main_record {
      uint64_t     id;
      double       index_f64  = 0.0;
      long double  index_f128 = 0.0L;
      uint64_t     index_i64  = 0ULL;
      uint128_t    index_i128 = 0ULL;
      key256       index_i256 = key256();

      auto primary_key() const { return id; }

      auto get_index_f64  () const { return index_f64 ; }
      auto get_index_f128 () const { return index_f128; }
      auto get_index_i64  () const { return index_i64 ; }
      auto get_index_i128 () const { return index_i128; }
      const key256& get_index_i256 () const { return index_i256; }

      EOSLIB_SERIALIZE( main_record, (id)(index_f64)(index_f128)(index_i64)(index_i128)(index_i256) )
   };

   struct increment {
      increment(): value(0) {}
      increment(uint32_t v): value(v) {}

      uint32_t value;

      EOSLIB_SERIALIZE(increment, (value))
   };

   using multi_index_type = eosio::multi_index<N(data), main_record,
      indexed_by< N(byf ),    const_mem_fun<main_record, double        ,&main_record::get_index_f64 >>,
      indexed_by< N(byff),    const_mem_fun<main_record, long double   ,&main_record::get_index_f128>>,
      indexed_by< N(byi ),    const_mem_fun<main_record, uint64_t      ,&main_record::get_index_i64 >>,
      indexed_by< N(byii),    const_mem_fun<main_record, uint128_t     ,&main_record::get_index_i128>>,
      indexed_by< N(byiiii),  const_mem_fun<main_record, const key256& ,&main_record::get_index_i256>>
   >;

   static void exec( uint64_t self, uint32_t value ) {
      multi_index_type data(self, self);
      auto current = data.begin( );
      if( current == data.end() ) {
         data.emplace( self, [&]( auto& r ) {
            r.id         = value;
            r.index_f64  = value;
            r.index_f128 = value;
            r.index_i64  = value;
            r.index_i128 = value;
            r.index_i256.data()[0] = value;
         });

      } else {
         data.modify( current, self, [&]( auto& r ) {
            r.index_f64  += value;
            r.index_f128 += value;
            r.index_i64  += value;
            r.index_i128 += value;
            r.index_i256.data()[0] += value;
         });
      }
   }

} ///多索引测试

namespace multi_index_test {
   extern "C" {
///apply方法实现将事件分派到此协定
      void apply( uint64_t self, uint64_t code, uint64_t action ) {
         require_auth(code);
         eosio_assert(action == N(increment), "unsupported action");
         snapshot_test::exec(self, unpack_action_data<snapshot_test::increment>().value);
      }
   }
}
