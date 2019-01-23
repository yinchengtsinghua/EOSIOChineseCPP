
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once
#include <eosiolib/core_symbol.hpp>
#include <eosiolib/serialize.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/system.h>
#include <tuple>
#include <limits>

namespace eosio {

  /*
   *@defgroup符号api符号api
   *@brief定义用于管理符号的API
   *@ingroup contractdev公司
   **/


  /*
   *@defgroup符号cpp api符号cpp api
   *@brief定义了用于管理符号的%cpp api
   *@ingroup符号API
   *@
   **/


   /*
    *将字符串转换为符号的uint64_t表示形式
    *
    *@param precision-符号精度
    *@param str-符号的字符串表示形式
    **/

   static constexpr uint64_t string_to_symbol( uint8_t precision, const char* str ) {
      uint32_t len = 0;
      while( str[len] ) ++len;

      uint64_t result = 0;
      for( uint32_t i = 0; i < len; ++i ) {
         if( str[i] < 'A' || str[i] > 'Z' ) {
//错误？
         } else {
            result |= (uint64_t(str[i]) << (8*(1+i)));
         }
      }

      result |= uint64_t(precision);
      return result;
   }

   /*
    *用于将符号的字符串表示形式转换为字符表示形式的宏
    *
    *@param precision-符号精度
    *@param str-符号的字符串表示形式
    **/

   #define S(P,X) ::eosio::string_to_symbol(P,#X)

   /*
    *uint64表示符号名
    **/

   typedef uint64_t symbol_name;

   /*
    *检查提供的符号名称是否有效。
    *
    *@param sym-symbol-name类型的符号名
    *@返回真-如果符号有效
    **/

   static constexpr bool is_valid_symbol( symbol_name sym ) {
      sym >>= 8;
      for( int i = 0; i < 7; ++i ) {
         char c = (char)(sym & 0xff);
         if( !('A' <= c && c <= 'Z')  ) return false;
         sym >>= 8;
         if( !(sym & 0xff) ) {
            do {
              sym >>= 8;
              if( (sym & 0xff) ) return false;
              ++i;
            } while( i < 7 );
         }
      }
      return true;
   }

   /*
    *返回所提供符号的字符长度
    *
    *@param sym-检索长度的符号（uint64_t）
    *@返回长度-所提供符号的字符长度
    **/

   static constexpr uint32_t symbol_name_length( symbol_name sym ) {
sym >>= 8; ///跳过精度
      uint32_t length = 0;
      while( sym & 0xff && length <= 7) {
         ++length;
         sym >>= 8;
      }

      return length;
   }

   /*
    *\struct存储有关符号的信息
    *
    *@brief存储有关符号的信息
    **/

   struct symbol_type {
     /*
      *符号名称
      **/

      symbol_name value;

      symbol_type() { }

      /*
       *符号的类型
       **/

      symbol_type(symbol_name s): value(s) { }

      /*
       *这个符号有效吗？
       **/

      bool     is_valid()const  { return is_valid_symbol( value ); }

      /*
       *此符号的精度
       **/

      uint64_t precision()const { return value & 0xff; }

      /*
       *返回符号名的uint64表示形式
       **/

      uint64_t name()const      { return value >> 8;   }

      /*
       *该符号的长度
       **/

      uint32_t name_length()const { return symbol_name_length( value ); }

      /*
       *
       **/

      operator symbol_name()const { return value; }

      /*
       *%打印符号
       *
       *@brief%打印符号
       **/

      void print(bool show_precision=true)const {
         if( show_precision ){
            ::eosio::print(precision());
            prints(",");
         }

         auto sym = value;
         sym >>= 8;
         for( int i = 0; i < 7; ++i ) {
            char c = (char)(sym & 0xff);
            if( !c ) return;
            prints_l(&c, 1 );
            sym >>= 8;
         }
      }

      EOSLIB_SERIALIZE( symbol_type, (value) )
   };

   /*
    *\struct存储符号所有者信息的扩展资产
    *
    **/

   struct extended_symbol : public symbol_type
   {
     /*
      *符号的所有者
      *
      *@向符号所有者简要介绍
      **/

     account_name contract;

     extended_symbol( symbol_name sym = 0, account_name acc = 0 ):symbol_type{sym},contract(acc){}

      /*
       *%打印扩展符号
       *
       *@brief%打印扩展符号
       **/

      void print()const {
         symbol_type::print();
         prints("@");
         printn( contract );
      }


      /*
       *等价运算符。如果a==b（相同），则返回true
       *
       *@brief减法运算符
       *@param a-要减去的扩展资产
       *@param b-用于减去的扩展资产
       *@return boolean-如果提供的两个符号相同，则返回true
       **/

      friend bool operator == ( const extended_symbol& a, const extended_symbol& b ) {
        return std::tie( a.value, a.contract ) == std::tie( b.value, b.contract );
      }

      /*
       *反向等价运算符。如果是，则返回真！=b（不同）
       *
       *@brief减法运算符
       *@param a-要减去的扩展资产
       *@param b-用于减去的扩展资产
       *@return boolean-如果提供的两个符号相同，则返回true
       **/

      friend bool operator != ( const extended_symbol& a, const extended_symbol& b ) {
        return std::tie( a.value, a.contract ) != std::tie( b.value, b.contract );
      }

      friend bool operator < ( const extended_symbol& a, const extended_symbol& b ) {
        return std::tie( a.value, a.contract ) < std::tie( b.value, b.contract );
      }

      EOSLIB_SERIALIZE( extended_symbol, (value)(contract) )
   };

//}符号API

} ///命名空间eosio
