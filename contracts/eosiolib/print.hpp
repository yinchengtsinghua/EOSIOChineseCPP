
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
#include <eosiolib/print.h>
#include <eosiolib/types.hpp>
#include <eosiolib/fixed_key.hpp>
#include <utility>
#include <string>

namespace eosio {

   static_assert( sizeof(long) == sizeof(int), "unexpected size difference" );

   /*
    *打印字符串
    *
    *@brief打印字符串
    *@param ptr-以空结尾的字符串
    **/

   inline void print( const char* ptr ) {
      prints(ptr);
   }

   inline void print( const std::string& s) {
      prints_l( s.c_str(), s.size() );
   }

   inline void print( std::string& s) {
      prints_l( s.c_str(), s.size() );
   }

   inline void print( const char c ) {
      prints_l( &c, 1 );
   }

   /*
    *打印有符号整数
    *
    *@brief将有符号整数打印为64位有符号整数
    *@param num待打印
    **/

   inline void print( int num ) {
      printi(num);
   }

   /*
    *打印32位有符号整数
    *
    *@brief将32位有符号整数打印为64位有符号整数
    *@param num待打印
    **/

   inline void print( int32_t num ) {
      printi(num);
   }

   /*
    *打印64位有符号整数
    *
    *@brief将64位有符号整数打印为64位有符号整数
    *@param num待打印
    **/

   inline void print( int64_t num ) {
      printi(num);
   }


   /*
    *打印无符号整数
    *
    *@brief将无符号整数打印为64位无符号整数
    *@param num待打印
    **/

   inline void print( unsigned int num ) {
      printui(num);
   }

   /*
    *打印32位无符号整数
    *
    *@brief将32位无符号整数打印为64位无符号整数
    *@param num待打印
    **/

   inline void print( uint32_t num ) {
      printui(num);
   }

   /*
    *打印64位无符号整数
    *
    *@brief将64位无符号整数打印为64位无符号整数
    *@param num待打印
    **/

   inline void print( uint64_t num ) {
      printui(num);
   }

   /*
    *打印128位有符号整数
    *
    *@brief打印128位有符号整数
    *@param num待打印
    **/

   inline void print( int128_t num ) {
      printi128(&num);
   }

   /*
    *打印128位无符号整数
    *
    *@brief打印128位无符号整数
    *@param num待打印
    **/

   inline void print( uint128_t num ) {
      printui128(&num);
   }


   /*
    *打印单精度浮点数
    *
    *@brief打印单精度浮点数（即float）
    *@param num待打印
    **/

   inline void print( float num ) { printsf( num ); }

   /*
    *打印双精度浮点数
    *
    *@brief打印双精度浮点数（即双精度）
    *@param num待打印
    **/

   inline void print( double num ) { printdf( num ); }

   /*
    *打印四倍精度浮点数
    *
    *@brief打印四倍精度浮点数（即长双精度）
    *@param num待打印
    **/

   inline void print( long double num ) { printqf( &num ); }


   /*
    *将固定键打印为十六进制字符串
    *
    *@brief以十六进制字符串形式打印固定\键
    *@param val待打印
    **/

   template<size_t Size>
   inline void print( const fixed_key<Size>& val ) {
      auto arr = val.extract_as_byte_array();
      prints("0x");
      printhex(static_cast<const void*>(arr.data()), arr.size());
   }

  /*
    *将固定键打印为十六进制字符串
    *
    *@brief以十六进制字符串形式打印固定\键
    *@param val待打印
    **/

   template<size_t Size>
   inline void print( fixed_key<Size>& val ) {
      print(static_cast<const fixed_key<Size>&>(val));
   }

   /*
    *将64位名称打印为base32编码字符串
    *
    *@brief将64位名称打印为base32编码字符串
    *@param name要打印的64位名称
    **/

   inline void print( name name ) {
      printn(name.value);
   }

  /*
    *打印布尔
    *
    *brief打印bool
    *@param val待打印
    **/

   inline void print( bool val ) {
      prints(val?"true":"false");
   }


  /*
    *打印类对象
    *
    *@brief打印类对象
    *@param t待打印
    *@pre t必须实现print（）函数
    **/

   template<typename T>
   inline void print( T&& t ) {
      t.print();
   }

   /*
    *打印以空结尾的字符串
    *
    *@brief打印以空结尾的字符串
    *@param s要打印的以空结尾的字符串
    **/

   inline void print_f( const char* s ) {
      prints(s);
   }

 /*
    *DeGuff-COSOLeCPPAPI控制台C++ API
    *@ingroup控制台API
    *@简单定义C++包装器来记录/打印文本消息
    *
    *这个API使用C++变量模板和类型检测到
    *方便打印任何本机类型。你甚至可以超载
    *您自己的自定义类型的“print（）”方法。
    *
    ***例子：**
    ＊` `
    *打印（“你好，世界，这是一个数字：”，5）；
    ＊` `
    *
    *@section重写类型的覆盖打印
    *
    *有两种方法可以过载打印：
    * 1。实现void print（const&）
    * 2。实现t:：print（）常量
    *
    *@
    **/



   /*
    *打印格式化字符串。其行为类似于c printf/
    *
    *@brief打印格式化字符串
    *@tparam arg-用于替换格式说明符的值的类型
    *@tparam args-用于替换格式说明符的值的类型
    *@param s-要打印的以空结尾的字符串（它可以包含格式说明符）
    *@param val-用于替换格式说明符的值
    *@param rest-用于替换格式说明符的值
    *
    *实例：
    *@代码
    *print_f（“苹果数%”，10）；
    *@终结码
    **/

   template <typename Arg, typename... Args>
   inline void print_f( const char* s, Arg val, Args... rest ) {
      while ( *s != '\0' ) {
         if ( *s == '%' ) {
            print( val );
            print_f( s+1, rest... );
            return;
         }
         prints_l( s, 1 );
         s++;
      }
   }

    /*
     *打印出值/值列表
     *@brief打印输出值/值列表
     *@param a-要打印的值
     *@param args-要打印的其他值
     *
     *实例：
*
     *@代码
     *const char*s=“你好，世界！”；
     *uint64_t unsigned_64_bit_int=1e+18；
     *uint128_t unsigned_128_bit_int（87654323456）；
     *uint64_t string_as_unsigned_64_bit=n（abcde）；
     *打印（s，unsigned_64_bit_int，unsigned_128_bit_int，string_as_unsigned_64_bit）；
     *//输出：你好，世界！10000000000000008765433456abcde
     *@终结码
     **/

   template<typename Arg, typename... Args>
   void print( Arg&& a, Args&&... args ) {
      print(std::forward<Arg>(a));
      print(std::forward<Args>(args)...);
   }

   /*
    *模拟C++风格的流
    **/

   class iostream {};

   /*
    *过载C++流
    *@简约过载C++流
    *@param out-输出应力
    *@param v-要打印的值
    *@返回iostream&-对输入输出流的引用
    *
    *实例：
*
    *@代码
    *const char*s=“你好，世界！”；
    *uint64_t unsigned_64_bit_int=1e+18；
    *uint128_t unsigned_128_bit_int（87654323456）；
    *uint64_t string_as_unsigned_64_bit=n（abcde）；
    *std：：out<<s<“”<<unsigned_64_bit_int<“”<<unsigned_128_bit_int<“”<<string_as_unsigned_64_bit；
    *//输出：你好，世界！10000000000000087654323456 abcde
    *@终结码
    **/

   template<typename T>
   inline iostream& operator<<( iostream& out, const T& v ) {
      print( v );
      return out;
   }

   static iostream cout;

///@控制台CPPAPI


}
