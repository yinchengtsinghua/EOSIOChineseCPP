
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once
#include <eosiolib/multi_index.hpp>
#include <eosiolib/system.h>

namespace  eosio {

   /*
    *@defgroup singleton singleton表
    *@brief定义eosio singleton表
    *@ingroup数据库cpp
    *@
    **/


   /*
    *此包装器使用单个表存储各种类型的命名对象。
    *
    *@tparam singleton name-此singleton变量的名称
    *@tparam t-单件的类型
    **/

   template<uint64_t SingletonName, typename T>
   class singleton
   {
      /*
       *单件表中数据的主键
       *
       *@brief数据单件表的主键
       **/

      constexpr static uint64_t pk_value = SingletonName;

      /*
       *单例表中的数据结构
       *
       *@singleton表中数据的简要结构
       **/

      struct row {
         /*
          *要存储在singleton表中的值
          *
          *@brief值存储在singleton表中
          **/

         T value;

         /*
          *获取数据的主键
          *
          *@brief获取数据的主键
          *@return uint64_t-主键
          **/

         uint64_t primary_key() const { return pk_value; }

         EOSLIB_SERIALIZE( row, (value) )
      };

      typedef eosio::multi_index<SingletonName, row> table;

      public:

         /*
          *根据表的所有者和作用域构造一个新的singleton对象。
          *
          *@brief构造一个新的singleton对象
          *@param code-表的所有者
          *@param scope-表的作用域
          **/

         singleton( account_name code, scope_name scope ) : _t( code, scope ) {}

         /*
          *检查singleton表是否存在
          *
          *@brief检查singleton表是否存在
          *@返回真-如果存在
          *@返回false-否则
          **/

         bool exists() {
            return _t.find( pk_value ) != _t.end();
         }

         /*
          *获取存储在singleton表中的值。如果不存在将引发异常
          *
          *@brief获取存储在singleton表中的值
          *@返回t-存储的值
          **/

         T get() {
            auto itr = _t.find( pk_value );
            eosio_assert( itr != _t.end(), "singleton does not exist" );
            return itr->value;
         }

         /*
          *获取存储在singleton表中的值。如果不存在，则返回指定的默认值
          *
          *@brief获取存储在singleton表中的值，或者返回指定的默认值（如果不存在的话）
          *@param def-数据不存在时返回的默认值
          *@返回t-存储的值
          **/

         T get_or_default( const T& def = T() ) {
            auto itr = _t.find( pk_value );
            return itr != _t.end() ? itr->value : def;
         }

         /*
          *获取存储在singleton表中的值。如果它不存在，它将使用指定的默认值创建一个新的
          *
          *@brief获取存储在singleton表中的值，如果不存在，则使用指定的默认值创建一个新值。
          *@param bill_to_account-如果数据不存在，新创建的数据的账单账户
          *@param def-数据不存在时要创建的默认值
          *@返回t-存储的值
          **/

         T get_or_create( account_name bill_to_account, const T& def = T() ) {
            auto itr = _t.find( pk_value );
            return itr != _t.end() ? itr->value
               : _t.emplace(bill_to_account, [&](row& r) { r.value = def; })->value;
         }

         /*
          *为singleton表设置新值
          *
          *@brief为singleton表设置新值
          *
          *@param value-要设置的新值
          *@param bill_to_account-支付新值的帐户
          **/

         void set( const T& value, account_name bill_to_account ) {
            auto itr = _t.find( pk_value );
            if( itr != _t.end() ) {
               _t.modify(itr, bill_to_account, [&](row& r) { r.value = value; });
            } else {
               _t.emplace(bill_to_account, [&](row& r) { r.value = value; });
            }
         }

         /*
          *删除singleton表中的唯一数据
          *
          *@brief删除singleton表中的唯一数据
          **/

         void remove( ) {
            auto itr = _t.find( pk_value );
            if( itr != _t.end() ) {
               _t.erase(itr);
            }
         }

      private:
         table _t;
   };

///}} SuntLon
} ///命名空间eosio
