
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
#include <eosiolib/types.h>
#include <functional>
#include <tuple>
#include <string>

namespace eosio {

   typedef std::vector<std::tuple<uint16_t,std::vector<char>>> extensions_type;

   /*
    *将base32符号转换为其二进制表示形式，由字符串_使用到_name（）。
    *
    *@brief将base32符号转换为其二进制表示形式，由string_to_name（）使用
    *@param c-要转换的字符
    *@return constexpr char-转换字符
    *@ingroup类型
    **/

   static constexpr  char char_to_symbol( char c ) {
      if( c >= 'a' && c <= 'z' )
         return (c - 'a') + 6;
      if( c >= '1' && c <= '5' )
         return (c - '1') + 1;
      return 0;
   }


   /*
    *将base32字符串转换为uint64。这是一个constexpr，因此
    *此方法也可用于模板参数。
    *
    *@brief将base32字符串转换为uint64。
    *@param str-名称的字符串表示形式
    *@return constexpr uint64_t-名称的64位无符号整数表示形式
    *@ingroup类型
    **/

   static constexpr uint64_t string_to_name( const char* str ) {

      uint32_t len = 0;
      while( str[len] ) ++len;

      uint64_t value = 0;

      for( uint32_t i = 0; i <= 12; ++i ) {
         uint64_t c = 0;
         if( i < len && i <= 12 ) c = uint64_t(char_to_symbol( str[i] ));

         if( i < 12 ) {
            c &= 0x1f;
            c <<= 64-5*(i+1);
         }
         else {
            c &= 0x0f;
         }

         value |= c;
      }

      return value;
   }

   /*
    *用于从x的base32编码字符串解释生成编译时uint64_t
    *
    *@brief用于从x的base32编码字符串解释生成编译时uint64
    *@param x-名称的字符串表示形式
    *@return constexpr uint64_t-名称的64位无符号整数表示形式
    *@ingroup类型
    **/

   #define N(X) ::eosio::string_to_name(#X)


   static constexpr uint64_t name_suffix( uint64_t n ) {
      uint32_t remaining_bits_after_last_actual_dot = 0;
      uint32_t tmp = 0;
for( int32_t remaining_bits = 59; remaining_bits >= 4; remaining_bits -= 5 ) { //注意：剩余的位必须保持有符号整数
//按从左到右的顺序逐个获取名称中的字符（不包括第13个字符）
         auto c = (n >> remaining_bits) & 0x1Full;
if( !c ) { //如果这个字符是一个点
            tmp = static_cast<uint32_t>(remaining_bits);
} else { //如果这个字符不是点
            remaining_bits_after_last_actual_dot = tmp;
         }
      }

      uint64_t thirteenth_character = n & 0x0Full;
if( thirteenth_character ) { //如果第13个字符不是点
         remaining_bits_after_last_actual_dot = tmp;
      }

if( remaining_bits_after_last_actual_dot == 0 ) //除了潜在的前导点之外，名称中没有实际的点
         return n;

//在这一点上，最后一个点之后的剩余位必须在4到59的范围内（并且限制为5的增量）。

//除4个最低有效位（对应于第13个字符）外，与最后一个实际点之后的字符对应的剩余位的掩码。
      uint64_t mask = (1ull << remaining_bits_after_last_actual_dot) - 16;
      uint32_t shift = 64 - remaining_bits_after_last_actual_dot;

      return ( ((n & mask) << shift) + (thirteenth_character << (shift-1)) );
   }

   /*
    *包装uint64以确保它只传递给需要名称和
    *没有发生数学运算。它还使印刷专业化
    *以便打印为base32字符串。
    *
    *@brief包装了一个uint64，以确保它只传递给需要名称的方法
    *@ingroup类型
    **/

   struct name {
      /*
       *将名称转换为uint64的转换运算符
       *
       *@brief转换运算符
       *@返回uint64？转换结果
       **/

      operator uint64_t()const { return value; }

//与名称的eosio源代码定义中的name:：operator string（）保持同步
      std::string to_string() const {
         static const char* charmap = ".12345abcdefghijklmnopqrstuvwxyz";

         std::string str(13,'.');

         uint64_t tmp = value;
         for( uint32_t i = 0; i <= 12; ++i ) {
            char c = charmap[tmp & (i == 0 ? 0x0f : 0x1f)];
            str[12-i] = c;
            tmp >>= (i == 0 ? 4 : 5);
         }

         trim_right_dots( str );
         return str;
      }

      /*
       *名称的相等运算符
       *
       *@brief equality name运算符
       *@param A-要比较的第一个数据
       *@param b-要比较的第二个数据
       *@返回真-如果等于
       *@返回false-如果不相等
       **/

      friend bool operator==( const name& a, const name& b ) { return a.value == b.value; }

      /*
       *账户名称的内部表示
       *
       *@帐户名的简短内部表示
       **/

      account_name value = 0;

   private:
      static void trim_right_dots(std::string& str ) {
         const auto last = str.find_last_not_of('.');
         if (last != std::string::npos)
            str = str.substr(0, last + 1);
      }
   };

} //命名空间EOSIO

namespace std {
   /*
    *少提供校验和256
    *@brief少提供校验和256
    **/

   template<>
   struct less<checksum256> : binary_function<checksum256, checksum256, bool> {
      bool operator()( const checksum256& lhs, const checksum256& rhs ) const {
         return memcmp(&lhs, &rhs, sizeof(lhs)) < 0;
      }
   };

} //命名空间性病

/*
 *校验和256的相等运算符
 *
 *@brief equality operator for checksum256
 *@param lhs-要比较的第一个数据
 *@param rhs-要比较的第二个数据
 *@返回真-如果等于
 *@返回false-如果不相等
 **/

bool operator==(const checksum256& lhs, const checksum256& rhs) {
   return memcmp(&lhs, &rhs, sizeof(lhs)) == 0;
}

/*
 *校验和160的相等运算符
 *
 *@brief equality operator for checksum256
 *@param lhs-要比较的第一个数据
 *@param rhs-要比较的第二个数据
 *@返回真-如果等于
 *@返回false-如果不相等
 **/

bool operator==(const checksum160& lhs, const checksum160& rhs) {
   return memcmp(&lhs, &rhs, sizeof(lhs)) == 0;
}

/*
 *校验和160的相等运算符
 *
 *@brief equality operator for checksum256
 *@param lhs-要比较的第一个数据
 *@param rhs-要比较的第二个数据
 *@返回真-如果不相等
 *@返回false-如果等于
 **/

bool operator!=(const checksum160& lhs, const checksum160& rhs) {
   return memcmp(&lhs, &rhs, sizeof(lhs)) != 0;
}
