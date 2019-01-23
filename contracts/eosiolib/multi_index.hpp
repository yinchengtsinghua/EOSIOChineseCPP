
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

#include <vector>
#include <tuple>
#include <boost/hana.hpp>
#include <functional>
#include <utility>
#include <type_traits>
#include <iterator>
#include <limits>
#include <algorithm>
#include <memory>

#include <boost/multi_index/mem_fun.hpp>

#include <eosiolib/action.h>
#include <eosiolib/types.hpp>
#include <eosiolib/serialize.hpp>
#include <eosiolib/datastream.hpp>
#include <eosiolib/db.h>
#include <eosiolib/fixed_key.hpp>

namespace eosio {

using boost::multi_index::const_mem_fun;

#define WRAP_SECONDARY_SIMPLE_TYPE(IDX, TYPE)\
template<>\
struct secondary_index_db_functions<TYPE> {\
   static int32_t db_idx_next( int32_t iterator, uint64_t* primary )          { return db_##IDX##_next( iterator, primary ); }\
   static int32_t db_idx_previous( int32_t iterator, uint64_t* primary )      { return db_##IDX##_previous( iterator, primary ); }\
   static void    db_idx_remove( int32_t iterator  )                          { db_##IDX##_remove( iterator ); }\
   static int32_t db_idx_end( uint64_t code, uint64_t scope, uint64_t table ) { return db_##IDX##_end( code, scope, table ); }\
   static int32_t db_idx_store( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const TYPE& secondary ) {\
      return db_##IDX##_store( scope, table, payer, id, &secondary );\
   }\
   static void    db_idx_update( int32_t iterator, uint64_t payer, const TYPE& secondary ) {\
      db_##IDX##_update( iterator, payer, &secondary );\
   }\
   static int32_t db_idx_find_primary( uint64_t code, uint64_t scope, uint64_t table, uint64_t primary, TYPE& secondary ) {\
      return db_##IDX##_find_primary( code, scope, table, &secondary, primary );\
   }\
   static int32_t db_idx_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const TYPE& secondary, uint64_t& primary ) {\
      return db_##IDX##_find_secondary( code, scope, table, &secondary, &primary );\
   }\
   static int32_t db_idx_lowerbound( uint64_t code, uint64_t scope, uint64_t table, TYPE& secondary, uint64_t& primary ) {\
      return db_##IDX##_lowerbound( code, scope, table, &secondary, &primary );\
   }\
   static int32_t db_idx_upperbound( uint64_t code, uint64_t scope, uint64_t table, TYPE& secondary, uint64_t& primary ) {\
      return db_##IDX##_upperbound( code, scope, table, &secondary, &primary );\
   }\
};

#define WRAP_SECONDARY_ARRAY_TYPE(IDX, TYPE)\
template<>\
struct secondary_index_db_functions<TYPE> {\
   static int32_t db_idx_next( int32_t iterator, uint64_t* primary )          { return db_##IDX##_next( iterator, primary ); }\
   static int32_t db_idx_previous( int32_t iterator, uint64_t* primary )      { return db_##IDX##_previous( iterator, primary ); }\
   static void    db_idx_remove( int32_t iterator )                           { db_##IDX##_remove( iterator ); }\
   static int32_t db_idx_end( uint64_t code, uint64_t scope, uint64_t table ) { return db_##IDX##_end( code, scope, table ); }\
   static int32_t db_idx_store( uint64_t scope, uint64_t table, uint64_t payer, uint64_t id, const TYPE& secondary ) {\
      return db_##IDX##_store( scope, table, payer, id, secondary.data(), TYPE::num_words() );\
   }\
   static void    db_idx_update( int32_t iterator, uint64_t payer, const TYPE& secondary ) {\
      db_##IDX##_update( iterator, payer, secondary.data(), TYPE::num_words() );\
   }\
   static int32_t db_idx_find_primary( uint64_t code, uint64_t scope, uint64_t table, uint64_t primary, TYPE& secondary ) {\
      return db_##IDX##_find_primary( code, scope, table, secondary.data(), TYPE::num_words(), primary );\
   }\
   static int32_t db_idx_find_secondary( uint64_t code, uint64_t scope, uint64_t table, const TYPE& secondary, uint64_t& primary ) {\
      return db_##IDX##_find_secondary( code, scope, table, secondary.data(), TYPE::num_words(), &primary );\
   }\
   static int32_t db_idx_lowerbound( uint64_t code, uint64_t scope, uint64_t table, TYPE& secondary, uint64_t& primary ) {\
      return db_##IDX##_lowerbound( code, scope, table, secondary.data(), TYPE::num_words(), &primary );\
   }\
   static int32_t db_idx_upperbound( uint64_t code, uint64_t scope, uint64_t table, TYPE& secondary, uint64_t& primary ) {\
      return db_##IDX##_upperbound( code, scope, table, secondary.data(), TYPE::num_words(), &primary );\
   }\
};

#define MAKE_TRAITS_FOR_ARITHMETIC_SECONDARY_KEY(TYPE)\
template<>\
struct secondary_key_traits<TYPE> {\
   static constexpr  TYPE lowest() { return std::numeric_limits<TYPE>::lowest(); }\
};

namespace _multi_index_detail {

   namespace hana = boost::hana;

   template<typename T>
   struct secondary_index_db_functions;

   template<typename T>
   struct secondary_key_traits;

   WRAP_SECONDARY_SIMPLE_TYPE(idx64,  uint64_t)
   MAKE_TRAITS_FOR_ARITHMETIC_SECONDARY_KEY(uint64_t)

   WRAP_SECONDARY_SIMPLE_TYPE(idx128, uint128_t)
   MAKE_TRAITS_FOR_ARITHMETIC_SECONDARY_KEY(uint128_t)

   WRAP_SECONDARY_SIMPLE_TYPE(idx_double, double)
   MAKE_TRAITS_FOR_ARITHMETIC_SECONDARY_KEY(double)

   WRAP_SECONDARY_SIMPLE_TYPE(idx_long_double, long double)
   MAKE_TRAITS_FOR_ARITHMETIC_SECONDARY_KEY(long double)

   WRAP_SECONDARY_ARRAY_TYPE(idx256, key256)
   template<>
   struct secondary_key_traits<key256> {
      static constexpr key256 lowest() { return key256(); }
   };

}

/*
 *索引结构用于实例化多索引表的索引。在eosio中，最多可以指定16个二级索引。
 *@brief索引结构用于实例化多索引表的索引。在eosio中，最多可以指定16个二级索引。
 *
 *@tparam index name-是索引的名称。名称必须以eosio base32编码的64位整数提供，并且必须符合eosio命名要求，最多13个字符，前12个来自小写字符a-z，数字0-5和“.”，如果有第13个字符，则仅限于小写字符a-p和“.”。
 *@tparam提取器-是一个函数调用运算符，它接受对表对象类型的常量引用，并返回辅助键类型或对辅助键类型的引用。建议使用“eosio:：const_mem_fun”模板，该模板是“boost:：multi_index:：const_mem_fun”的类型别名。有关详细信息，请参阅boost'const\u mem\u fun'密钥提取程序的文档。
 *
 *实例：
       *
*
 *@代码
 *包括<eosiolib/eosio.hpp>
 *使用名称空间eosio；
 *我方合同类：eosio:：contract
 *结构记录
 *uint64主要；
 *uint128_t二级；
 *uint64_t primary_key（）const返回primary；
 *uint64_t get_secondary（）const返回secondary；
 *eoslib串行（记录，（主要）（次要）
 *}；
 ＊公众：
 *mycontract（account_name self）：contract（self）
 *void myaction（）
 *自动编码=_self；
 *自动范围=_self；
 *多索引<n（mytable），记录，
 *indexed_by<n（by secondary），const_mem_fun<record，uint128_t，&record:：get_secondary>>table（code，scope）；
 *}
 *}
 *eosioou abi（mycontract，（myaction））。
 *@终结码
 **/

template<uint64_t IndexName, typename Extractor>
struct indexed_by {
   enum constants { index_name   = IndexName };
   typedef Extractor secondary_extractor_type;
};

/*
 *@defgroup多索引多索引表
 *@brief定义eosio多索引表
 *@ingroup数据库cpp
 *
 *
 *
 ＊EOSIO多索引API为EOSIO数据库提供C++接口。它是在Boost多索引容器之后形成的。
 *eosio多索引表需要一个uint64_t主键。为了表能够检索主键，
 *存储在表中的对象需要有一个名为primary_key（）的常量成员函数，该函数返回uint64_t。
 *eosio多索引表还支持多达16个二级索引。二级指数的类型可以是：
 *-UIT64 64
 *-UITN128YT
 *-UTIN 256It
 *双
 *长双人床
 *
 *@tparam table name-表名
 *@tparam t-表中存储的数据类型
 *@tparam indexs-表的二级索引，这里最多支持16个索引
 *
 *实例：
       *
 *@代码
 *包括<eosiolib/eosio.hpp>
 *使用名称空间eosio；
 *我方合同类别：合同
 *结构记录
 *uint64主要；
 *uint64_t次要_1；
 *uint128_t次要_2；
 *uint256_t次要_3；
 *双二级\4；
 *长双二次_5；
 *uint64_t primary_key（）const返回primary；
 *uint64_t get_secondary_1（）const返回secondary_1；
 *uint128_t get_secondary_2（）const返回secondary_2；
 *uint256_t get_secondary_3（）const返回secondary_3；
 *double get_secondary_4（）const返回secondary_4；
 *long double get_secondary_5（）const返回secondary_5；
 *eoslib_serialize（record，（primary）（secondary_1）（secondary_2）（secondary_3）（secondary_4）（secondary_5））。
 *}；
 ＊公众：
 *mycontract（account_name self）：contract（self）
 *void myaction（）
 *自动编码=_self；
 *自动范围=_self；
 *多索引<n（mytable），记录，
 *索引_by<n（by secondary 1），const_mem_fun<record，uint64_t，&record:：get_secondary_1>>，
 *indexed_by<n（by secondary 2），const_mem_fun<record，uint128_t，&record:：get_secondary_2>>，
 *索引_by<n（by secondary 3），const_mem_fun<record，uint256_t，&record:：get_secondary_3>>，
 *索引_by<n（by secondary 4），const_mem_fun<record，double，&record:：get_secondary_4>>，
 *索引_by<n（by secondary 5），const_mem_fun<record，long double，&record:：get_secondary_5>>
 *>表（代码、范围）；
 *}
 *}
 *eosioou abi（mycontract，（myaction））。
 *@终结码
 *@
 **/


template<uint64_t TableName, typename T, typename... Indices>
class multi_index
{
   private:

      static_assert( sizeof...(Indices) <= 16, "multi_index only supports a maximum of 16 secondary indices" );

      constexpr static bool validate_table_name( uint64_t n ) {
//将表名限制为12个字符，以便最后一个字符（4位）可用于区分辅助索引。
         return (n & 0x000000000000000FULL) == 0;
      }

      constexpr static size_t max_stack_buffer_size = 512;

      static_assert( validate_table_name(TableName), "multi_index does not support table names with a length greater than 12");

      uint64_t _code;
      uint64_t _scope;

      mutable uint64_t _next_primary_key;

      enum next_primary_key_tags : uint64_t {
no_available_primary_key = static_cast<uint64_t>(-2), //必须是与所有其他标记相比的最小uint64_t值
         unset_next_primary_key = static_cast<uint64_t>(-1)
      };

      struct item : public T
      {
         template<typename Constructor>
         item( const multi_index* idx, Constructor&& c )
         :__idx(idx){
            c(*this);
         }

         const multi_index* __idx;
         int32_t            __primary_itr;
         int32_t            __iters[sizeof...(Indices)+(sizeof...(Indices)==0)];
      };

      struct item_ptr
      {
         item_ptr(std::unique_ptr<item>&& i, uint64_t pk, int32_t pitr)
         : _item(std::move(i)), _primary_key(pk), _primary_itr(pitr) {}

         std::unique_ptr<item> _item;
         uint64_t              _primary_key;
         int32_t               _primary_itr;
      };

      mutable std::vector<item_ptr> _items_vector;

      template<uint64_t IndexName, typename Extractor, uint64_t Number, bool IsConst>
      struct index {
         public:
            typedef Extractor  secondary_extractor_type;
            typedef typename std::decay<decltype( Extractor()(nullptr) )>::type secondary_key_type;

            constexpr static bool validate_index_name( uint64_t n ) {
return n != 0 && n != N(primary); //primary是保留索引名称。
            }

            static_assert( validate_index_name(IndexName), "invalid index name used in multi_index" );

            enum constants {
               table_name   = TableName,
               index_name   = IndexName,
               index_number = Number,
index_table_name = (TableName & 0xFFFFFFFFFFFFFFF0ULL) | (Number & 0x000000000000000FULL) //假设不允许超过16个二级指数
            };

            constexpr static uint64_t name()   { return index_table_name; }
            constexpr static uint64_t number() { return Number; }

            struct const_iterator : public std::iterator<std::bidirectional_iterator_tag, const T> {
               public:
                  friend bool operator == ( const const_iterator& a, const const_iterator& b ) {
                     return a._item == b._item;
                  }
                  friend bool operator != ( const const_iterator& a, const const_iterator& b ) {
                     return a._item != b._item;
                  }

                  const T& operator*()const { return *static_cast<const T*>(_item); }
                  const T* operator->()const { return static_cast<const T*>(_item); }

                  const_iterator operator++(int){
                     const_iterator result(*this);
                     ++(*this);
                     return result;
                  }

                  const_iterator operator--(int){
                     const_iterator result(*this);
                     --(*this);
                     return result;
                  }

                  const_iterator& operator++() {
                     using namespace _multi_index_detail;

                     eosio_assert( _item != nullptr, "cannot increment end iterator" );

                     if( _item->__iters[Number] == -1 ) {
                        secondary_key_type temp_secondary_key;
                        auto idxitr = secondary_index_db_functions<secondary_key_type>::db_idx_find_primary(_idx->get_code(), _idx->get_scope(), _idx->name(), _item->primary_key(), temp_secondary_key);
                        auto& mi = const_cast<item&>( *_item );
                        mi.__iters[Number] = idxitr;
                     }

                     uint64_t next_pk = 0;
                     auto next_itr = secondary_index_db_functions<secondary_key_type>::db_idx_next( _item->__iters[Number], &next_pk );
                     if( next_itr < 0 ) {
                        _item = nullptr;
                        return *this;
                     }

                     const T& obj = *_idx->_multidx->find( next_pk );
                     auto& mi = const_cast<item&>( static_cast<const item&>(obj) );
                     mi.__iters[Number] = next_itr;
                     _item = &mi;

                     return *this;
                  }

                  const_iterator& operator--() {
                     using namespace _multi_index_detail;

                     uint64_t prev_pk = 0;
                     int32_t  prev_itr = -1;

                     if( !_item ) {
                        auto ei = secondary_index_db_functions<secondary_key_type>::db_idx_end(_idx->get_code(), _idx->get_scope(), _idx->name());
                        eosio_assert( ei != -1, "cannot decrement end iterator when the index is empty" );
                        prev_itr = secondary_index_db_functions<secondary_key_type>::db_idx_previous( ei , &prev_pk );
                        eosio_assert( prev_itr >= 0, "cannot decrement end iterator when the index is empty" );
                     } else {
                        if( _item->__iters[Number] == -1 ) {
                           secondary_key_type temp_secondary_key;
                           auto idxitr = secondary_index_db_functions<secondary_key_type>::db_idx_find_primary(_idx->get_code(), _idx->get_scope(), _idx->name(), _item->primary_key(), temp_secondary_key);
                           auto& mi = const_cast<item&>( *_item );
                           mi.__iters[Number] = idxitr;
                        }
                        prev_itr = secondary_index_db_functions<secondary_key_type>::db_idx_previous( _item->__iters[Number], &prev_pk );
                        eosio_assert( prev_itr >= 0, "cannot decrement iterator at beginning of index" );
                     }

                     const T& obj = *_idx->_multidx->find( prev_pk );
                     auto& mi = const_cast<item&>( static_cast<const item&>(obj) );
                     mi.__iters[Number] = prev_itr;
                     _item = &mi;

                     return *this;
                  }

                  const_iterator():_item(nullptr){}
               private:
                  friend struct index;
                  const_iterator( const index* idx, const item* i = nullptr )
                  : _idx(idx), _item(i) {}

                  const index* _idx;
                  const item*  _item;
}; ///struct multi_index:：index:：const_迭代器

            typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

            const_iterator cbegin()const {
               using namespace _multi_index_detail;
               return lower_bound( secondary_key_traits<secondary_key_type>::lowest() );
            }
            const_iterator begin()const  { return cbegin(); }

            const_iterator cend()const   { return const_iterator( this ); }
            const_iterator end()const    { return cend(); }

            const_reverse_iterator crbegin()const { return std::make_reverse_iterator(cend()); }
            const_reverse_iterator rbegin()const  { return crbegin(); }

            const_reverse_iterator crend()const   { return std::make_reverse_iterator(cbegin()); }
            const_reverse_iterator rend()const    { return crend(); }

            const_iterator find( secondary_key_type&& secondary )const {
               return find( secondary );
            }

            const_iterator find( const secondary_key_type& secondary )const {
               auto lb = lower_bound( secondary );
               auto e = cend();
               if( lb == e ) return e;

               if( secondary != secondary_extractor_type()(*lb) )
                  return e;
               return lb;
            }

            const_iterator require_find( secondary_key_type&& secondary, const char* error_msg = "unable to find secondary key" )const {
               return require_find( secondary, error_msg );
            }

            const_iterator require_find( const secondary_key_type& secondary, const char* error_msg = "unable to find secondary key" )const {
               auto lb = lower_bound( secondary );
               eosio_assert( lb != cend(), error_msg );
               eosio_assert( secondary == secondary_extractor_type()(*lb), error_msg );
               return lb;
            }

            const T& get( secondary_key_type&& secondary, const char* error_msg = "unable to find secondary key" )const {
               return get( secondary, error_msg );
            }

//在次键不唯一的情况下获取具有最小主键的对象。
            const T& get( const secondary_key_type& secondary, const char* error_msg = "unable to find secondary key" )const {
               auto result = find( secondary );
               eosio_assert( result != cend(), error_msg );
               return *result;
            }

            const_iterator lower_bound( secondary_key_type&& secondary )const {
               return lower_bound( secondary );
            }
            const_iterator lower_bound( const secondary_key_type& secondary )const {
               using namespace _multi_index_detail;

               uint64_t primary = 0;
               secondary_key_type secondary_copy(secondary);
               auto itr = secondary_index_db_functions<secondary_key_type>::db_idx_lowerbound( get_code(), get_scope(), name(), secondary_copy, primary );
               if( itr < 0 ) return cend();

               const T& obj = *_multidx->find( primary );
               auto& mi = const_cast<item&>( static_cast<const item&>(obj) );
               mi.__iters[Number] = itr;

               return {this, &mi};
            }

            const_iterator upper_bound( secondary_key_type&& secondary )const {
               return upper_bound( secondary );
            }
            const_iterator upper_bound( const secondary_key_type& secondary )const {
               using namespace _multi_index_detail;

               uint64_t primary = 0;
               secondary_key_type secondary_copy(secondary);
               auto itr = secondary_index_db_functions<secondary_key_type>::db_idx_upperbound( get_code(), get_scope(), name(), secondary_copy, primary );
               if( itr < 0 ) return cend();

               const T& obj = *_multidx->find( primary );
               auto& mi = const_cast<item&>( static_cast<const item&>(obj) );
               mi.__iters[Number] = itr;

               return {this, &mi};
            }

            const_iterator iterator_to( const T& obj ) {
               using namespace _multi_index_detail;

               const auto& objitem = static_cast<const item&>(obj);
               eosio_assert( objitem.__idx == _multidx, "object passed to iterator_to is not in multi_index" );

               if( objitem.__iters[Number] == -1 ) {
                  secondary_key_type temp_secondary_key;
                  auto idxitr = secondary_index_db_functions<secondary_key_type>::db_idx_find_primary(get_code(), get_scope(), name(), objitem.primary_key(), temp_secondary_key);
                  auto& mi = const_cast<item&>( objitem );
                  mi.__iters[Number] = idxitr;
               }

               return {this, &objitem};
            }

            template<typename Lambda>
            void modify( const_iterator itr, uint64_t payer, Lambda&& updater ) {
               eosio_assert( itr != cend(), "cannot pass end iterator to modify" );

               _multidx->modify( *itr, payer, std::forward<Lambda&&>(updater) );
            }

            const_iterator erase( const_iterator itr ) {
               eosio_assert( itr != cend(), "cannot pass end iterator to erase" );

               const auto& obj = *itr;
               ++itr;

               _multidx->erase(obj);

               return itr;
            }

            uint64_t get_code()const  { return _multidx->get_code(); }
            uint64_t get_scope()const { return _multidx->get_scope(); }

            static auto extract_secondary_key(const T& obj) { return secondary_extractor_type()(obj); }

         private:
            friend class multi_index;

            index( typename std::conditional<IsConst, const multi_index*, multi_index*>::type midx )
            :_multidx(midx){}

            typename std::conditional<IsConst, const multi_index*, multi_index*>::type _multidx;
}; ///struct多索引：：索引

      template<uint64_t I>
      struct intc { enum e{ value = I }; operator uint64_t()const{ return I; }  };

      static constexpr auto transform_indices( ) {
         using namespace _multi_index_detail;

         typedef decltype( hana::zip_shortest(
                             hana::make_tuple( intc<0>(), intc<1>(), intc<2>(), intc<3>(), intc<4>(), intc<5>(),
                                               intc<6>(), intc<7>(), intc<8>(), intc<9>(), intc<10>(), intc<11>(),
                                               intc<12>(), intc<13>(), intc<14>(), intc<15>() ),
                             hana::tuple<Indices...>() ) ) indices_input_type;

         return hana::transform( indices_input_type(), [&]( auto&& idx ){
             typedef typename std::decay<decltype(hana::at_c<0>(idx))>::type num_type;
             typedef typename std::decay<decltype(hana::at_c<1>(idx))>::type idx_type;
             return hana::make_tuple( hana::type_c<index<idx_type::index_name,
                                                         typename idx_type::secondary_extractor_type,
                                                         num_type::e::value, false> >,
                                      hana::type_c<index<idx_type::index_name,
                                                         typename idx_type::secondary_extractor_type,
                                                         num_type::e::value, true> > );

         });
      }

      typedef decltype( multi_index::transform_indices() ) indices_type;

      indices_type _indices;

      const item& load_object_by_primary_iterator( int32_t itr )const {
         using namespace _multi_index_detail;

         auto itr2 = std::find_if(_items_vector.rbegin(), _items_vector.rend(), [&](const item_ptr& ptr) {
            return ptr._primary_itr == itr;
         });
         if( itr2 != _items_vector.rend() )
            return *itr2->_item;

         auto size = db_get_i64( itr, nullptr, 0 );
         eosio_assert( size >= 0, "error reading iterator" );

//在这里使用malloc/free可能不是异常安全的，尽管wasm不支持异常
         void* buffer = max_stack_buffer_size < size_t(size) ? malloc(size_t(size)) : alloca(size_t(size));

         db_get_i64( itr, buffer, uint32_t(size) );

         datastream<const char*> ds( (char*)buffer, uint32_t(size) );

         if ( max_stack_buffer_size < size_t(size) ) {
            free(buffer);
         }

         auto itm = std::make_unique<item>( this, [&]( auto& i ) {
            T& val = static_cast<T&>(i);
            ds >> val;

            i.__primary_itr = itr;
            hana::for_each( _indices, [&]( auto& idx ) {
               typedef typename decltype(+hana::at_c<1>(idx))::type index_type;

               i.__iters[ index_type::number() ] = -1;
            });
         });

         const item* ptr = itm.get();
         auto pk   = itm->primary_key();
         auto pitr = itm->__primary_itr;

         _items_vector.emplace_back( std::move(itm), pk, pitr );

         return *ptr;
} ///通过主迭代器加载\u对象\u

   public:
      /*
       *构造多索引表的实例。
       *@brief构造多索引表的实例。
       *
       *@param code-拥有表的帐户
       *@param scope-代码层次结构中的作用域标识符
       *
       *@pre-code和scope成员属性已初始化
       *@post初始化了每个辅助索引表
       *@post secondary indexs会更新以引用新添加的对象。如果辅助索引表不存在，则会创建它们。
       *@post付款人将收取新对象的存储使用费，如果必须创建表（和辅助索引表），则支付创建表的开销。
       *
       ＊注释
       *eosio:：multi_index`模板具有模板参数`<uint64_t tablename，typename t，typename…索引>`，其中：
       *-`table name`是表的名称，最长12个字符，名称中包含小写字母、数字1到5以及“.”（句点）字符；
       *-`T`是对象类型（即行定义）；
       *-`indexs`是最多16个二级索引的列表。
       *-每个都必须是默认的可构造类或结构
       *-每个函数都必须有一个函数调用运算符，该运算符接受对表对象类型的常量引用，并返回辅助键类型或对辅助键类型的引用。
       *建议使用eosio:：const_mem_fun模板，该模板是boost:：multi_index:：const_mem_fun的类型别名。有关更多详细信息，请参阅Boost Const_Mem_Fun密钥提取程序的文档。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint64_t primary_key（）const返回帐户_name；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      multi_index( uint64_t code, uint64_t scope )
      :_code(code),_scope(scope),_next_primary_key(unset_next_primary_key)
      {}

      /*
       *返回“code”成员属性。
       *@brief返回“code”成员属性。
       *
       *@返回主表所属代码的帐户名。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint64_t primary_key（）const返回帐户_name；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address>address_index；
       *void myaction（）
       *地址_index addresses（n（dan），n（dan））；//代码，作用域
       *eosio_assert（addresses.get_code（）==n（dan），“codes don't match.”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      uint64_t get_code()const  { return _code; }

      /*
       *返回“scope”成员属性。
       *@brief返回“scope”成员属性。
       *
       *@返回当前接收器代码内的作用域ID，在该代码下可以找到所需的主表实例。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint64_t primary_key（）const返回帐户_name；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address>address_index；
       *void myaction（）
       *地址_index addresses（n（dan），n（dan））；//代码，作用域
       *eosio_assert（addresses.get_code（）==n（dan），“作用域不匹配”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      uint64_t get_scope()const { return _scope; }

      struct const_iterator : public std::iterator<std::bidirectional_iterator_tag, const T> {
         friend bool operator == ( const const_iterator& a, const const_iterator& b ) {
            return a._item == b._item;
         }
         friend bool operator != ( const const_iterator& a, const const_iterator& b ) {
            return a._item != b._item;
         }

         const T& operator*()const { return *static_cast<const T*>(_item); }
         const T* operator->()const { return static_cast<const T*>(_item); }

         const_iterator operator++(int) {
            const_iterator result(*this);
            ++(*this);
            return result;
         }

         const_iterator operator--(int) {
            const_iterator result(*this);
            --(*this);
            return result;
         }

         const_iterator& operator++() {
            eosio_assert( _item != nullptr, "cannot increment end iterator" );

            uint64_t next_pk;
            auto next_itr = db_next_i64( _item->__primary_itr, &next_pk );
            if( next_itr < 0 )
               _item = nullptr;
            else
               _item = &_multidx->load_object_by_primary_iterator( next_itr );
            return *this;
         }
         const_iterator& operator--() {
            uint64_t prev_pk;
            int32_t  prev_itr = -1;

            if( !_item ) {
               auto ei = db_end_i64(_multidx->get_code(), _multidx->get_scope(), TableName);
               eosio_assert( ei != -1, "cannot decrement end iterator when the table is empty" );
               prev_itr = db_previous_i64( ei , &prev_pk );
               eosio_assert( prev_itr >= 0, "cannot decrement end iterator when the table is empty" );
            } else {
               prev_itr = db_previous_i64( _item->__primary_itr, &prev_pk );
               eosio_assert( prev_itr >= 0, "cannot decrement iterator at beginning of table" );
            }

            _item = &_multidx->load_object_by_primary_iterator( prev_itr );
            return *this;
         }

         private:
            const_iterator( const multi_index* mi, const item* i = nullptr )
            :_multidx(mi),_item(i){}

            const multi_index* _multidx;
            const item*        _item;
            friend class multi_index;
}; ///struct multi_index:：const_迭代器

      typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

      /*
       *返回指向多索引表中主键值最低的对象类型的迭代器。
       *@brief返回指向多索引表中主键值最低的对象类型的迭代器。
       *
       *@返回指向多索引表中主键值最低的对象类型的迭代器。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint64_t primary_key（）const返回帐户_name；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（_self，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *}；
       *auto itr=地址。查找（n（dan））；
       *eosio_assert（itr==addresses.cbegin（），“only address is not at front.”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      const_iterator cbegin()const {
         return lower_bound(std::numeric_limits<uint64_t>::lowest());
      }

      /*
       *返回指向多索引表中主键值最低的对象类型的迭代器。
       *@brief返回指向多索引表中主键值最低的对象类型的迭代器。
       *
       *@返回指向多索引表中主键值最低的对象类型的迭代器。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint64_t primary_key（）const返回帐户_name；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（_self，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *}；
       *auto itr=地址。查找（n（dan））；
       *eosio_assert（itr==addresses.begin（），“only address is not at front.”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      const_iterator begin()const  { return cbegin(); }

      /*
       *返回一个迭代器，指向多索引表中主键值最高的“对象类型”。
       *@brief返回一个迭代器，指向多索引表中主键值最高的“object_type”。
       *
       *@返回一个迭代器，指向多索引表中主键值最高的“对象类型”。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint64_t primary_key（）const返回帐户_name；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（_self，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *}；
       *auto itr=地址。查找（n（dan））；
       *EOSIO声明（ITR！=addresses.cend（），“账户地址不存在”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      const_iterator cend()const   { return const_iterator( this ); }

      /*
       *返回一个迭代器，指向多索引表中主键值最高的“对象类型”。
       *@brief返回一个迭代器，指向多索引表中主键值最高的“object_type”。
       *
       *@返回一个迭代器，指向多索引表中主键值最高的“对象类型”。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint64_t primary_key（）const返回帐户_name；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（_self，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *}；
       *auto itr=地址。查找（n（dan））；
       *EOSIO声明（ITR！=addresses.end（），“账户地址不存在”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      const_iterator end()const    { return cend(); }

      /*
       *返回一个反向迭代器，指向多索引表中主键值最高的“对象类型”。
       *@brief返回一个反向迭代器，指向多索引表中主键值最高的“object_type”。
       *
       *@返回一个反向迭代器，指向多索引表中主键值最高的“object_type”。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint64_t primary_key（）const返回帐户_name；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（payer，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *}；
       *地址.emplace（payer，[&]（auto&address）
       *address.account_name=n（布兰登）；
       *address.first_name=“brendan”；
       *address.last_name=“blumer”；
       *address.street=“1 eos路”；
       *地址=城市“香港”；
       *address.state=“香港”；
       *}；
       *auto itr=addresses.crbegin（）；
       *eosio_assert（itr->account_name==n（dan），“不正确的最后记录”）；
       ＊ITR++；
       *eosio_assert（itr->account_name==n（brendan），“不正确的第二条最后记录”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      const_reverse_iterator crbegin()const { return std::make_reverse_iterator(cend()); }

      /*
       *返回一个反向迭代器，指向多索引表中主键值最高的“对象类型”。
       *@brief返回一个反向迭代器，指向多索引表中主键值最高的“object_type”。
       *
       *@返回一个反向迭代器，指向多索引表中主键值最高的“object_type”。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint64_t primary_key（）const返回帐户_name；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（payer，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *}；
       *地址.emplace（payer，[&]（auto&address）
       *address.account_name=n（布兰登）；
       *address.first_name=“brendan”；
       *address.last_name=“blumer”；
       *address.street=“1 eos路”；
       *地址=城市“香港”；
       *address.state=“香港”；
       *}；
       *auto itr=地址.rbegin（）；
       *eosio_assert（itr->account_name==n（dan），“不正确的最后记录”）；
       ＊ITR++；
       *eosio_assert（itr->account_name==n（brendan），“不正确的第二条最后记录”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      const_reverse_iterator rbegin()const  { return crbegin(); }

      /*
       *返回一个迭代器，指向多索引表中主键值最低的“object\u type”。
       *@brief返回一个迭代器，指向多索引表中主键值最低的“object_type”。
       *
       *@返回一个迭代器，指向多索引表中主键值最低的“object_type”。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint64_t primary_key（）const返回帐户_name；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（payer，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *}；
       *地址.emplace（payer，[&]（auto&address）
       *address.account_name=n（布兰登）；
       *address.first_name=“brendan”；
       *address.last_name=“blumer”；
       *address.street=“1 eos路”；
       *地址=城市“香港”；
       *address.state=“香港”；
       *}；
       *auto itr=地址.crend（）；
       ＊ITR；
       *eosio_assert（itr->account_name==n（brendan），“第一条记录不正确”）；
       ＊ITR；
       *eosio_assert（itr->account_name==n（dan），“第二条记录不正确”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      const_reverse_iterator crend()const   { return std::make_reverse_iterator(cbegin()); }

      /*
       *返回一个迭代器，指向多索引表中主键值最低的“object\u type”。
       *@brief返回一个迭代器，指向多索引表中主键值最低的“object_type”。
       *
       *@返回一个迭代器，指向多索引表中主键值最低的“object_type”。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint64_t primary_key（）const返回帐户_name；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（payer，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *}；
       *地址.emplace（payer，[&]（auto&address）
       *address.account_name=n（布兰登）；
       *address.first_name=“brendan”；
       *address.last_name=“blumer”；
       *address.street=“1 eos路”；
       *地址=城市“香港”；
       *address.state=“香港”；
       *}；
       *auto itr=地址.rend（）；
       ＊ITR；
       *eosio_assert（itr->account_name==n（brendan），“第一条记录不正确”）；
       ＊ITR；
       *eosio_assert（itr->account_name==n（dan），“第二条记录不正确”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      const_reverse_iterator rend()const    { return crend(); }

      /*
       *搜索具有最低主键的“object_type”，该主键大于或等于给定的主键。
       *@brief搜索具有最低主键的“object\u type”，该主键大于或等于给定的主键。
       *
       *@param primary-为下限搜索建立目标值的主键。
       *
       *@返回一个迭代器，该迭代器指向的“object_type”具有大于或等于“primary”的最低主键。如果找不到对象，它将返回“end”迭代器。如果表不存在**，它将返回`-1`。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint32_t zip=0；
       *uint64_t primary_key（）const返回帐户_name；
       *uint64_t by_zip（）const返回zip；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州）（zip））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address，indexed_by<n（zip），const_mem_fun<address，uint64_t，&address:：by_zip>>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（payer，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *地址.zip=93446；
       *}；
       *地址.emplace（payer，[&]（auto&address）
       *address.account_name=n（布兰登）；
       *address.first_name=“brendan”；
       *address.last_name=“blumer”；
       *address.street=“1 eos路”；
       *地址=城市“香港”；
       *address.state=“香港”；
       *地址.zip=93445；
       *}；
       *uint32_t zipnumb=93445；
       *auto-zip_index=addresses.get_index<n（zip）>（）；
       *auto itr=zip_index.lower_bound（zipnumb）；
       *eosio_assert（itr->account_name==n（brendan），“第一个下限记录不正确”）；
       ＊ITR++；
       *eosio_assert（itr->account_name==n（dan），“第二个下界记录不正确”）；
       ＊ITR++；
       *eosio_assert（itr==zip_index.end（），“迭代器结尾不正确”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      const_iterator lower_bound( uint64_t primary )const {
         auto itr = db_lowerbound_i64( _code, _scope, TableName, primary );
         if( itr < 0 ) return end();
         const auto& obj = load_object_by_primary_iterator( itr );
         return {this, &obj};
      }

      /*
       *搜索具有最高主键小于或等于给定主键的“对象类型”。
       *@brief搜索具有最高主键小于或等于给定主键的“object\u type”。
       *
       *@param primary-为上限搜索建立目标值的主键
       *
       *@返回指向“object_type”的迭代器，该类型具有小于或等于“primary”的最高主键。如果找不到对象，它将返回“end”迭代器。如果表不存在**，它将返回`-1`。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint32_t zip=0；
       *uint64不喜欢=0；
       *uint64_t primary_key（）const返回帐户_name；
       *uint64_t by_zip（）const返回zip；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州）（zip））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address，indexed_by<n（zip），const_mem_fun<address，uint64_t，&address:：by_zip>>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（payer，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *地址.zip=93446；
       *}；
       *地址.emplace（payer，[&]（auto&address）
       *address.account_name=n（布兰登）；
       *address.first_name=“brendan”；
       *address.last_name=“blumer”；
       *address.street=“1 eos路”；
       *地址=城市“香港”；
       *address.state=“香港”；
       *地址.zip=93445；
       *}；
       *uint32_t zipnumb=93445；
       *auto-zip_index=addresses.get_index<n（zip）>（）；
       *auto itr=zip_index.upper_bound（zipnumb）；
       *eosio_assert（itr->account_name==n（dan），“第一个上限记录不正确”）；
       ＊ITR++；
       *eosio_assert（itr==zip_index.end（），“迭代器结尾不正确”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      const_iterator upper_bound( uint64_t primary )const {
         auto itr = db_upperbound_i64( _code, _scope, TableName, primary );
         if( itr < 0 ) return end();
         const auto& obj = load_object_by_primary_iterator( itr );
         return {this, &obj};
      }

      /*
       *返回可用的主键。
       *@brief返回可用的主键。
       *
       *@返回一个可用（未使用）的主键值。
       *
       *注释：
       *用于表中，表的主键严格打算自动递增，因此合同不会将其设置为自定义值。违反此预期可能会导致表由于无法分配可用的主键而看起来已满。
       *理想情况下，此方法仅用于确定要在添加到表中的新对象中使用的适当主键，在该表中，表的主键从一开始就严格计划为自动增量，因此合同不会将其设置为自定义任意值。如果违反此协议，则可能导致表在实际剩余空间较大时显示为满。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64_t键；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint64_t primary_key（）const回车键；
       *eoslib-serialize（地址，（键）（名字）（姓氏）（街道）（城市）（州））
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（payer，[&]（auto&address）
       *address.key=addresses.available_primary_key（）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *}；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      uint64_t available_primary_key()const {
         if( _next_primary_key == unset_next_primary_key ) {
//这是首次为此多索引实例调用可用的\u primary\u key（）。
if( begin() == end() ) { //空表
               _next_primary_key = 0;
            } else {
auto itr = --end(); //按主键排序的表的最后一行
auto pk = itr->primary_key(); //表中当前最大的主键
if( pk >= no_available_primary_key ) //保留标签
                  _next_primary_key = no_available_primary_key;
               else
                  _next_primary_key = pk + 1;
            }
         }

         eosio_assert( _next_primary_key < no_available_primary_key, "next primary key in table is at autoincrement limit");
         return _next_primary_key;
      }

      /*
       *返回适当类型的辅助索引。
       *@brief返回一个适当类型的辅助索引。
       *
       *@tparam indexname-所需辅助索引的ID
       *
       *@返回相应类型的索引：primitive 64位无符号整数键（idx64）、primitive 128位无符号整数键（idx128）、128位固定大小词典编辑键（idx128）、256位固定大小词典编辑键（idx256）、浮点键、双精度浮点键、长双精度（四倍）浮点键
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint32_t zip=0；
       *uint64_t primary_key（）const返回帐户_name；
       *uint64_t by_zip（）const返回zip；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州）（zip））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address，indexed_by<n（zip），const_mem_fun<address，uint64_t，&address:：by_zip>>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（payer，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *地址.zip=93446；
       *}；
       *uint32_t zipnumb=93446；
       *auto-zip_index=addresses.get_index<n（zip）>（）；
       *auto itr=zip_index.find（zipnumb）；
       *eosio_assert（itr->account_name==n（dan），“错误记录”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      template<uint64_t IndexName>
      auto get_index() {
         using namespace _multi_index_detail;

         auto res = hana::find_if( _indices, []( auto&& in ) {
            return std::integral_constant<bool, std::decay<typename decltype(+hana::at_c<0>(in))::type>::type::index_name == IndexName>();
         });

         static_assert( res != hana::nothing, "name provided is not the name of any secondary index within multi_index" );

         return typename decltype(+hana::at_c<0>(res.value()))::type(this);
      }

      /*
       *返回适当类型的辅助索引。
       *@brief返回一个适当类型的辅助索引。
       *
       *@tparam indexname-所需辅助索引的ID
       *
       *@返回相应类型的索引：primitive 64位无符号整数键（idx64）、primitive 128位无符号整数键（idx128）、128位固定大小词典编辑键（idx128）、256位固定大小词典编辑键（idx256）、浮点键、双精度浮点键、长双精度（四倍）浮点键
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint32_t zip=0；
       *uint64_t primary_key（）const返回帐户_name；
       *uint64_t by_zip（）const返回zip；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州）（zip））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address，indexed_by<n（zip），const_mem_fun<address，uint64_t，&address:：by_zip>>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（payer，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *地址.zip=93446；
       *}；
       *地址.emplace（payer，[&]（auto&address）
       *address.account_name=n（布兰登）；
       *address.first_name=“brendan”；
       *address.last_name=“blumer”；
       *address.street=“1 eos路”；
       *地址=城市“香港”；
       *address.state=“香港”；
       *地址.zip=93445；
       *}；
       *uint32_t zipnumb=93445；
       *auto-zip_index=addresses.get_index<n（zip）>（）；
       *auto itr=zip_index.upper_bound（zipnumb）；
       *eosio_assert（itr->account_name==n（dan），“第一个上限记录不正确”）；
       ＊ITR++；
       *eosio_assert（itr==zip_index.end（），“迭代器结尾不正确”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      template<uint64_t IndexName>
      auto get_index()const {
         using namespace _multi_index_detail;

         auto res = hana::find_if( _indices, []( auto&& in ) {
            return std::integral_constant<bool, std::decay<typename decltype(+hana::at_c<1>(in))::type>::type::index_name == IndexName>();
         });

         static_assert( res != hana::nothing, "name provided is not the name of any secondary index within multi_index" );

         return typename decltype(+hana::at_c<1>(res.value()))::type(this);
      }

      /*
       *返回多索引表中给定对象的迭代器。
       *@brief返回多索引表中给定对象的迭代器。
       *
       *@param obj-对所需对象的引用
       *
       *@返回给定对象的迭代器
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint32_t zip=0；
       *uint64_t primary_key（）const返回帐户_name；
       *uint64_t by_zip（）const返回zip；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州）（zip））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address，indexed_by<n（zip），const_mem_fun<address，uint64_t，&address:：by_zip>>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（payer，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *地址.zip=93446；
       *}；
       *地址.emplace（payer，[&]（auto&address）
       *address.account_name=n（布兰登）；
       *address.first_name=“brendan”；
       *address.last_name=“blumer”；
       *address.street=“1 eos路”；
       *地址=城市“香港”；
       *address.state=“香港”；
       *地址.zip=93445；
       *}；
       *auto user=addresses.get（n（dan））；
       *auto itr=地址。查找（n（dan））；
       *eosio_assert（iterator_to（user）==itr，“无效的iterator”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      const_iterator iterator_to( const T& obj )const {
         const auto& objitem = static_cast<const item&>(obj);
         eosio_assert( objitem.__idx == this, "object passed to iterator_to is not in multi_index" );
         return {this, &objitem};
      }
      /*
       *向表中添加新对象（即行）。
       *@brief向表中添加了一个新对象（即行）。
       *
       *@param payer-新对象存储使用的付款人的帐户名
       *@param constructor-对要在表中创建的对象进行就地初始化的lambda函数
       *
       *@pre已实例化多索引表
       *@post在多索引表中创建一个新对象，具有唯一的主键（如对象中指定的那样）。对象被序列化并写入表。如果表不存在，则创建该表。
       *@post secondary indexs会更新以引用新添加的对象。如果辅助索引表不存在，则会创建它们。
       *@post付款人将收取新对象的存储使用费，如果必须创建表（和辅助索引表），则支付创建表的开销。
       *
       *@向新创建的对象返回主键迭代器
       *
       *例外-帐户无权写入表。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint64_t primary_key（）const返回帐户_name；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（_self，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *}；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      template<typename Lambda>
      const_iterator emplace( uint64_t payer, Lambda&& constructor ) {
         using namespace _multi_index_detail;

eosio_assert( _code == current_receiver(), "cannot create objects in table of another contract" ); //使用不允许突变的多重索引对数据库进行快速修复。真正的修复可以在RC2中进行。

         auto itm = std::make_unique<item>( this, [&]( auto& i ){
            T& obj = static_cast<T&>(i);
            constructor( obj );

            size_t size = pack_size( obj );

//在这里使用malloc/free可能不是异常安全的，尽管wasm不支持异常
            void* buffer = max_stack_buffer_size < size ? malloc(size) : alloca(size);

            datastream<char*> ds( (char*)buffer, size );
            ds << obj;

            auto pk = obj.primary_key();

            i.__primary_itr = db_store_i64( _scope, TableName, payer, pk, buffer, size );

            if ( max_stack_buffer_size < size ) {
               free(buffer);
            }

            if( pk >= _next_primary_key )
               _next_primary_key = (pk >= no_available_primary_key) ? no_available_primary_key : (pk + 1);

            hana::for_each( _indices, [&]( auto& idx ) {
               typedef typename decltype(+hana::at_c<0>(idx))::type index_type;

               i.__iters[index_type::number()] = secondary_index_db_functions<typename index_type::secondary_key_type>::db_idx_store( _scope, index_type::name(), payer, obj.primary_key(), index_type::extract_secondary_key(obj) );
            });
         });

         const item* ptr = itm.get();
         auto pk   = itm->primary_key();
         auto pitr = itm->__primary_itr;

         _items_vector.emplace_back( std::move(itm), pk, pitr );

         return {this, ptr};
      }

      /*
       *修改表中的现有对象。
       *@brief修改表中的现有对象。
       *
       *@param itr-指向要更新的对象的迭代器
       *@param payer-更新行存储使用的付款人的帐户名
       *@param updater-更新目标对象的lambda函数
       *
       *@pre-itr指向现有元素
       *@pre-payer是一个有效的帐户，有权执行该操作，并按存储使用计费。
       *
       *@post将对修改后的对象进行序列化，然后替换表中现有的对象。
       *@post secondary indexs已更新；更新对象的主键未更改。
       *@post付款方对更新对象的存储使用收取费用。
       *@post如果付款人与现有付款人相同，则付款人只支付现有和更新对象之间的使用差异（如果此差异为负数，则退款）。
       *@post如果付款人与现有付款人不同，现有付款人将退还现有对象的存储使用费。
       *
       ＊例外情况：
       *如果使用无效的前提条件调用，则中止执行。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint64_t primary_key（）const返回帐户_name；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（_self，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *}；
       *auto itr=地址。查找（n（dan））；
       *EOSIO声明（ITR！=addresses.end（），“找不到账户地址”）；
       *地址.修改（ITR，帐户付款人，[&]（自动和地址）
       *address.city=“圣路易斯奥比斯波”；
       *address.state=“CA”；
       *}；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      template<typename Lambda>
      void modify( const_iterator itr, uint64_t payer, Lambda&& updater ) {
         eosio_assert( itr != end(), "cannot pass end iterator to modify" );

         modify( *itr, payer, std::forward<Lambda&&>(updater) );
      }

      /*
       *修改表中的现有对象。
       *@brief修改表中的现有对象。
       *
       *@param obj-对要更新的对象的引用
       *@param payer-更新行存储使用的付款人的帐户名
       *@param updater-更新目标对象的lambda函数
       *
       *@pre obj是表中现有的对象
       *@pre-payer是一个有效的帐户，有权执行该操作，并按存储使用计费。
       *
       *@post将对修改后的对象进行序列化，然后替换表中现有的对象。
       *@post secondary indexs已更新；更新对象的主键未更改。
       *@post付款方对更新对象的存储使用收取费用。
       *@post如果付款人与现有付款人相同，则付款人只支付现有和更新对象之间的使用差异（如果此差异为负数，则退款）。
       *@post如果付款人与现有付款人不同，现有付款人将退还现有对象的存储使用费。
       *
       ＊例外情况：
       *如果使用无效的前提条件调用，则中止执行。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint64_t primary_key（）const返回帐户_name；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（_self，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *}；
       *auto itr=地址。查找（n（dan））；
       *EOSIO声明（ITR！=addresses.end（），“找不到账户地址”）；
       *地址。修改（*itr，付款人，[&]（自动和地址）
       *address.city=“圣路易斯奥比斯波”；
       *address.state=“CA”；
       *}；
       *eosio_assert（itr->city==“San Luis Obispo”，“地址未修改”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      template<typename Lambda>
      void modify( const T& obj, uint64_t payer, Lambda&& updater ) {
         using namespace _multi_index_detail;

         const auto& objitem = static_cast<const item&>(obj);
         eosio_assert( objitem.__idx == this, "object passed to modify is not in multi_index" );
         auto& mutableitem = const_cast<item&>(objitem);
eosio_assert( _code == current_receiver(), "cannot modify objects in table of another contract" ); //使用不允许突变的多重索引对数据库进行快速修复。真正的修复可以在RC2中进行。

         auto secondary_keys = hana::transform( _indices, [&]( auto&& idx ) {
            typedef typename decltype(+hana::at_c<0>(idx))::type index_type;

            return index_type::extract_secondary_key( obj );
         });

         auto pk = obj.primary_key();

auto& mutableobj = const_cast<T&>(obj); //不要忘记自动的，否则它会生成一个副本，因此根本不会更新。
         updater( mutableobj );

         eosio_assert( pk == obj.primary_key(), "updater cannot change primary key when modifying an object" );

         size_t size = pack_size( obj );
//在这里使用malloc/free可能不是异常安全的，尽管wasm不支持异常
         void* buffer = max_stack_buffer_size < size ? malloc(size) : alloca(size);

         datastream<char*> ds( (char*)buffer, size );
         ds << obj;

         db_update_i64( objitem.__primary_itr, payer, buffer, size );

         if ( max_stack_buffer_size < size ) {
            free( buffer );
         }

         if( pk >= _next_primary_key )
            _next_primary_key = (pk >= no_available_primary_key) ? no_available_primary_key : (pk + 1);

         hana::for_each( _indices, [&]( auto& idx ) {
            typedef typename decltype(+hana::at_c<0>(idx))::type index_type;

            auto secondary = index_type::extract_secondary_key( obj );
            if( memcmp( &hana::at_c<index_type::index_number>(secondary_keys), &secondary, sizeof(secondary) ) != 0 ) {
               auto indexitr = mutableitem.__iters[index_type::number()];

               if( indexitr < 0 ) {
                  typename index_type::secondary_key_type temp_secondary_key;
                  indexitr = mutableitem.__iters[index_type::number()]
                           = secondary_index_db_functions<typename index_type::secondary_key_type>::db_idx_find_primary( _code, _scope, index_type::name(), pk,  temp_secondary_key );
               }

               secondary_index_db_functions<typename index_type::secondary_key_type>::db_idx_update( indexitr, payer, secondary );
            }
         });
      }

      /*
       *使用表的主键从表中检索现有对象。
       *@brief使用其主键从表中检索现有对象。
       *
       *@param primary-对象的主键值
       *@返回对包含指定主键的对象的常量引用。
       *
       *异常-没有与给定键匹配的对象
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint64_t primary_key（）const返回帐户_name；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（_self，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *}；
       *auto user=addresses.get（n（dan））；
       *eosio_assert（user.first_name=“daniel”，“找不到他。”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      const T& get( uint64_t primary, const char* error_msg = "unable to find key" )const {
         auto result = find( primary );
         eosio_assert( result != cend(), error_msg );
         return *result;
      }

      /*
       *使用表的主键搜索表中的现有对象。
       *@brief使用主键搜索表中的现有对象。
       *
       *@param primary-对象的主键值
       *@如果找不到主键为“primary”的对象，则将迭代器返回到找到的主键等于被引用表的“primary”或“end”迭代器的对象。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint64_t primary_key（）const返回帐户_name；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（_self，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *}；
       *auto itr=地址。查找（n（dan））；
       *EOSIO声明（ITR！=addresses.end（），“找不到他。”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      const_iterator find( uint64_t primary )const {
         auto itr2 = std::find_if(_items_vector.rbegin(), _items_vector.rend(), [&](const item_ptr& ptr) {
            return ptr._item->primary_key() == primary;
         });
         if( itr2 != _items_vector.rend() )
            return iterator_to(*(itr2->_item));

         auto itr = db_find_i64( _code, _scope, TableName, primary );
         if( itr < 0 ) return end();

         const item& i = load_object_by_primary_iterator( itr );
         return iterator_to(static_cast<const T&>(i));
      }

      /*
       *使用表的主键搜索表中的现有对象。
       *@brief使用主键搜索表中的现有对象。
       *
       *@param primary-对象的主键值
       *@param error_msg-如果找不到主键为'primary'的对象，则显示错误消息。
       *@向找到的主键等于“primary”的对象返回迭代器，如果找不到主键为“primary”的对象，则抛出异常。
       **/


      const_iterator require_find( uint64_t primary, const char* error_msg = "unable to find key" )const {
         auto itr2 = std::find_if(_items_vector.rbegin(), _items_vector.rend(), [&](const item_ptr& ptr) {
               return ptr._item->primary_key() == primary;
            });
         if( itr2 != _items_vector.rend() )
            return iterator_to(*(itr2->_item));

         auto itr = db_find_i64( _code, _scope, TableName, primary );
         eosio_assert( itr >= 0,  error_msg );

         const item& i = load_object_by_primary_iterator( itr );
         return iterator_to(static_cast<const T&>(i));
      }

      /*
       *使用主键从表中删除现有对象。
       *@brief使用主键从表中删除现有对象。
       *
       *@param itr-指向要删除对象的迭代器
       *
       *@pre-itr指向现有元素
       *@post从表中删除对象，并回收所有相关的存储。
       *@post与表关联的二级索引已更新。
       *@post现有支付对象存储使用费的人将退还已删除对象的表和辅助索引使用费，如果删除了表和索引，则退还相关开销。
       *
       *@return对于带有“const_迭代器”的签名，返回一个指向已删除对象后面的对象的指针。
       *
       ＊例外情况：
       *要删除的对象不在表中。
       *此操作无权修改表。
       *给定的迭代器无效。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint64_t primary_key（）const返回帐户_name；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（_self，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *}；
       *auto itr=地址。查找（n（dan））；
       *EOSIO声明（ITR！=addresses.end（），“找不到账户地址”）；
       *地址.擦除（ITR）；
       *EOSIO声明（ITR！=addresses.end（），“未正确删除地址”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      const_iterator erase( const_iterator itr ) {
         eosio_assert( itr != end(), "cannot pass end iterator to erase" );

         const auto& obj = *itr;
         ++itr;

         erase(obj);

         return itr;
      }

      /*
       *使用主键从表中删除现有对象。
       *@brief使用主键从表中删除现有对象。
       *
       *@param obj-要删除的对象
       *
       *@pre obj是表中现有的对象
       *@post从表中删除对象，并回收所有相关的存储。
       *@post与表关联的二级索引已更新。
       *@post现有支付对象存储使用费的人将退还已删除对象的表和辅助索引使用费，如果删除了表和索引，则退还相关开销。
       *
       ＊例外情况：
       *要删除的对象不在表中。
       *此操作无权修改表。
       *给定的迭代器无效。
       *
       *实例：
       *
       *@代码
       *包括<eosiolib/eosio.hpp>
       *使用名称空间eosio；
       *使用命名空间std；
       *班级通讯簿：合同
       *结构地址
       *uint64账户名称；
       *字符串名；
       *字符串姓氏；
       *弦街；
       *字符串城市；
       *字符串状态；
       *uint64_t primary_key（）const返回帐户_name；
       *eoslib-serialize（地址，（帐户名）（名）（姓）（街）（市）（州））。
       *}；
       ＊公众：
       *通讯录（帐户名self）：合同（self）
       *typedef eosio:：multi_index<n（address），address>address_index；
       *void myaction（）
       *address_index addresses（_self，_self）；//代码，作用域
       *//添加到表中，第一个参数是要用于存储的帐单帐户
       *地址.emplace（_self，[&]（auto&address）
       *address.account_name=n（dan）；
       *address.first_name=“daniel”；
       *address.last_name=“larimer”；
       *address.street=“1 eos路”；
       *address.city=“布莱克斯堡”；
       *address.state=“va”；
       *}；
       *auto itr=地址。查找（n（dan））；
       *EOSIO声明（ITR！=addresses.end（），“找不到记录”）；
       *地址。删除（*itr）；
       *itr=地址。查找（n（dan））；
       *eosio_assert（itr==addresses.end（），“记录未删除”）；
       *}
       *}
       *eosioou abi（通讯录，（myaction））。
       *@终结码
       **/

      void erase( const T& obj ) {
         using namespace _multi_index_detail;

         const auto& objitem = static_cast<const item&>(obj);
         eosio_assert( objitem.__idx == this, "object passed to erase is not in multi_index" );
eosio_assert( _code == current_receiver(), "cannot erase objects in table of another contract" ); //使用不允许突变的多重索引对数据库进行快速修复。真正的修复可以在RC2中进行。

         auto pk = objitem.primary_key();
         auto itr2 = std::find_if(_items_vector.rbegin(), _items_vector.rend(), [&](const item_ptr& ptr) {
            return ptr._item->primary_key() == pk;
         });

         eosio_assert( itr2 != _items_vector.rend(), "attempt to remove object that was not in multi_index" );

         _items_vector.erase(--(itr2.base()));

         db_remove_i64( objitem.__primary_itr );

         hana::for_each( _indices, [&]( auto& idx ) {
            typedef typename decltype(+hana::at_c<0>(idx))::type index_type;

            auto i = objitem.__iters[index_type::number()];
            if( i < 0 ) {
              typename index_type::secondary_key_type secondary;
              i = secondary_index_db_functions<typename index_type::secondary_key_type>::db_idx_find_primary( _code, _scope, index_type::name(), objitem.primary_key(),  secondary );
            }
            if( i >= 0 )
               secondary_index_db_functions<typename index_type::secondary_key_type>::db_idx_remove( i );
         });
      }

};
///@
}  ///EOSIO
