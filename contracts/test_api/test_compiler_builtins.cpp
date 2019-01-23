
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
/*
 *@文件测试编译器builtins.cpp
 *@eos/license中定义的版权
 **/


#include <eosiolib/eosio.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/compiler_builtins.h>

#include "test_api.hpp"

unsigned __int128 operator "" _ULLL( const char* lit ) {
      __int128 ret = 0;
      size_t   i = 0;
      bool     sign = false;

      if (lit[i] == '-') {
         ++i;
         sign = true;
      }

      if (lit[i] == '+')
         ++i;

      for (; lit[i] != '\0' ; ++i) {
         const char c = lit[i];
         ret *= 10;
         ret += c - '0';
      }

      if (sign)
         ret *= -1;

      return (unsigned __int128)ret;
}

__int128 operator "" _LLL( const char* lit ) {
      __int128 ret = 0;
      size_t   i = 0;
      bool     sign = false;

      if (lit[i] == '-') {
         ++i;
         sign = true;
      }

      if (lit[i] == '+')
         ++i;

      for (; lit[i] != '\0' ; ++i) {
         const char c = lit[i];
         ret *= 10;
         ret += c - '0';
      }

      if (sign)
         ret *= -1;

      return ret;
}

void test_compiler_builtins::test_multi3() {
   /*
    *负值测试
    **/

   __int128 res   = 0;
   __int128 lhs_a = -30;
   __int128 rhs_a = 100;
   __int128 lhs_b = 100;
   __int128 rhs_b = -30;

   __multi3( res, uint64_t(lhs_a), uint64_t( lhs_a >> 64 ), uint64_t(rhs_a), uint64_t( rhs_a >> 64 ) );
   eosio_assert( res == -3000, "__multi3 result should be -3000" ); 

   __multi3( res, uint64_t(lhs_b), uint64_t( lhs_b >> 64 ), uint64_t(rhs_b), uint64_t( rhs_b >> 64 ) );
   eosio_assert( res == -3000, "__multi3 result should be -3000" ); 

   __multi3( res, uint64_t(lhs_a), uint64_t( lhs_a >> 64 ), uint64_t(rhs_b), uint64_t( rhs_b >> 64 ) );
   eosio_assert( res == 900, "__multi3 result should be 900" ); 

   /*
    *阳性值测试
    **/

   __multi3( res, uint64_t(lhs_b), uint64_t( lhs_b >> 64 ), uint64_t(rhs_a), uint64_t( rhs_a >> 64 ) );
   eosio_assert( res == 10000, "__multi3 result should be 10000" ); 

   /*
    *测试标识
    **/

   __multi3( res, 1, 0, uint64_t(rhs_a), uint64_t(rhs_a >> 64) );
   eosio_assert( res == 100, "__multi3 result should be 100" );

   __multi3( res, 1, 0, uint64_t(rhs_b), uint64_t(rhs_b >> 64) );
   eosio_assert( res == -30, "__multi3 result should be -30" );
} 

void test_compiler_builtins::test_divti3() {
   /*
    *负值测试
    **/

   __int128 res   = 0;
   __int128 lhs_a = -30;
   __int128 rhs_a = 100;
   __int128 lhs_b = 100;
   __int128 rhs_b = -30;

   __divti3( res, uint64_t(lhs_a), uint64_t( lhs_a >> 64 ), uint64_t(rhs_a), uint64_t( rhs_a >> 64 ) );
   eosio_assert( res == 0, "__divti3 result should be 0" ); 

   __divti3( res, uint64_t(lhs_b), uint64_t( lhs_b >> 64 ), uint64_t(rhs_b), uint64_t( rhs_b >> 64 ) );
   eosio_assert( res == -3, "__divti3 result should be -3" ); 

   __divti3( res, uint64_t(lhs_a), uint64_t( lhs_a >> 64 ), uint64_t(rhs_b), uint64_t( rhs_b >> 64 ) );
   eosio_assert( res == 1, "__divti3 result should be 1" ); 

   /*
    *阳性值测试
    **/

   __int128 lhs_c = 3333;
   __int128 rhs_c = 3333;

   __divti3( res, uint64_t(lhs_b), uint64_t( lhs_b >> 64 ), uint64_t(rhs_a), uint64_t( rhs_a >> 64 ) );
   eosio_assert( res == 1, "__divti3 result should be 1" ); 

   __divti3( res, uint64_t(lhs_b), uint64_t( lhs_b >> 64 ), uint64_t(rhs_c), uint64_t( rhs_c >> 64 ) );
   eosio_assert( res == 0, "__divti3 result should be 0" ); 

   __divti3( res, uint64_t(lhs_c), uint64_t( lhs_c >> 64 ), uint64_t(rhs_a), uint64_t( rhs_a >> 64 ) );
   eosio_assert( res == 33, "__divti3 result should be 33" ); 

   /*
    *测试标识
    **/

   __divti3( res, uint64_t(lhs_b), uint64_t( lhs_b >> 64 ), 1, 0 );
   eosio_assert( res == 100, "__divti3 result should be 100" ); 

   __divti3( res, uint64_t(lhs_a), uint64_t( lhs_a >> 64 ), 1, 0 );
   eosio_assert( res == -30, "__divti3 result should be -30" ); 
}

void test_compiler_builtins::test_divti3_by_0() {
   __int128 res = 0;

   __divti3( res, 100, 0, 0, 0 );
   eosio_assert( false, "Should have eosio_asserted" );
}

void test_compiler_builtins::test_udivti3() {
   /*
    *负值测试
    **/

   unsigned __int128 res   = 0;
   unsigned __int128 lhs_a = (unsigned __int128)-30;
   unsigned __int128 rhs_a = 100;
   unsigned __int128 lhs_b = 100;
   unsigned __int128 rhs_b = (unsigned __int128)-30;

   __udivti3( res, uint64_t(lhs_a), uint64_t( lhs_a >> 64 ), uint64_t(rhs_a), uint64_t( rhs_a >> 64 ) );
   eosio_assert( res == 3402823669209384634633746074317682114_ULLL, "__udivti3 result should be 0" ); 

   __udivti3( res, uint64_t(lhs_b), uint64_t( lhs_b >> 64 ), uint64_t(rhs_b), uint64_t( rhs_b >> 64 ) );
   eosio_assert( res == 0, "__udivti3 result should be 0" ); 

   __udivti3( res, uint64_t(lhs_a), uint64_t( lhs_a >> 64 ), uint64_t(rhs_b), uint64_t( rhs_b >> 64 ) );
   eosio_assert( res == 1, "__udivti3 result should be 1" ); 

   /*
    *阳性值测试
    **/

   __int128 lhs_c = 3333;
   __int128 rhs_c = 3333;

   __udivti3( res, uint64_t(lhs_b), uint64_t( lhs_b >> 64 ), uint64_t(rhs_a), uint64_t( rhs_a >> 64 ) );
   eosio_assert( res == 1, "__divti3 result should be 1" ); 

   __udivti3( res, uint64_t(lhs_b), uint64_t( lhs_b >> 64 ), uint64_t(rhs_c), uint64_t( rhs_c >> 64 ) );
   eosio_assert( res == 0, "__divti3 result should be 0" ); 

   __udivti3( res, uint64_t(lhs_c), uint64_t( lhs_c >> 64 ), uint64_t(rhs_a), uint64_t( rhs_a >> 64 ) );
   eosio_assert( res == 33, "__divti3 result should be 33" ); 

   /*
    *测试标识
    **/

   __udivti3( res, uint64_t(lhs_b), uint64_t( lhs_b >> 64 ), 1, 0 );
   eosio_assert( res == 100, "__divti3 result should be 100" ); 

   __udivti3( res, uint64_t(lhs_a), uint64_t( lhs_a >> 64 ), 1, 0 );
   eosio_assert( res == (unsigned __int128)-30, "__divti3 result should be -30" ); 
}

void test_compiler_builtins::test_udivti3_by_0() {
   unsigned __int128 res = 0;

   __udivti3( res, 100, 0, 0, 0 );
   eosio_assert( false, "Should have eosio_asserted" );
}


void test_compiler_builtins::test_lshlti3() {
   __int128 res      = 0;
   __int128 val      = 1;
   __int128 test_res = 0;

   test_res =   0x8000000000000000;
   test_res <<= 1;


   __lshlti3( res, uint64_t(val), uint64_t(val >> 64), 0 );
   eosio_assert( res == 1, "__lshlti3 result should be 1" );


   __lshlti3( res, uint64_t(val), uint64_t(val >> 64), 1 );
   eosio_assert( res == ( 1 << 1 ), "__lshlti3 result should be 2" );

   __lshlti3( res, uint64_t(val), uint64_t( val >> 64 ), 31 );
   eosio_assert( (unsigned __int128)res == 2147483648_ULLL, "__lshlti3 result should be 2^31" );
   
   __lshlti3( res, uint64_t(val), uint64_t( val >> 64 ), 63 );
   eosio_assert( (unsigned __int128)res == 9223372036854775808_ULLL, "__lshlti3 result should be 2^63" );

   __lshlti3( res, uint64_t(val), uint64_t( val >> 64 ), 64 );
   eosio_assert( res == test_res, "__lshlti3 result should be 2^64" );

   __lshlti3( res, uint64_t(val), uint64_t( val >> 64 ), 127 );
   test_res <<= 63;
   eosio_assert( res == test_res, "__lshlti3 result should be 2^127" );

   __lshlti3( res, uint64_t(val), uint64_t( val >> 64 ), 128 );
   test_res <<= 1;
//应翻滚
   eosio_assert( res == test_res, "__lshlti3 result should be 2^128" );
}

void test_compiler_builtins::test_ashlti3() {
   __int128 res      = 0;
   __int128 val      = 1;
   __int128 test_res = 0;

   test_res =   0x8000000000000000;
   test_res <<= 1;

   __ashlti3( res, uint64_t(val), uint64_t(val >> 64), 0 );
   eosio_assert( res == 1, "__ashlti3 result should be 1" );


   __ashlti3( res, uint64_t(val), uint64_t(val >> 64), 1 );
   eosio_assert( res == (1 << 1), "__ashlti3 result should be 2" );

   __ashlti3( res, uint64_t(val), uint64_t(val >> 64), 31 );
   eosio_assert( res == (__int128)2147483648_ULLL, "__ashlti3 result should be 2^31" );
   
   __ashlti3( res, uint64_t(val), uint64_t(val >> 64), 63 );
   eosio_assert( res == (__int128)9223372036854775808_ULLL, "__ashlti3 result should be 2^63" );

   __ashlti3( res, uint64_t(val), uint64_t(val >> 64), 64 );
   eosio_assert( res == test_res, "__ashlti3 result should be 2^64" );

   __ashlti3( res, uint64_t(val), uint64_t(val >> 64), 127 );
   test_res <<= 63;
   eosio_assert( res == test_res, "__ashlti3 result should be 2^127" );

   __ashlti3( res, uint64_t(val), uint64_t(val >> 64), 128 );
   test_res <<= 1;
//应翻滚
   eosio_assert( res == test_res, "__ashlti3 result should be 2^128" );
}


void test_compiler_builtins::test_lshrti3() {
   __int128 res      = 0;
   __int128 val      = 0x8000000000000000;
   __int128 test_res = 0x8000000000000000;

   val      <<= 64;
   test_res <<= 64;
   
   __lshrti3( res, uint64_t(val), uint64_t(val >> 64), 0 );
   eosio_assert( res == test_res, "__lshrti3 result should be 2^127" );

   __lshrti3( res, uint64_t(val), uint64_t(val >> 64), 1 );
   eosio_assert( res == (__int128)85070591730234615865843651857942052864_ULLL, "__lshrti3 result should be 2^126" );

   __lshrti3( res, uint64_t(val), uint64_t(val >> 64), 63 );
   eosio_assert( res == (__int128)18446744073709551616_ULLL, "__lshrti3 result should be 2^64" );

   __lshrti3( res, uint64_t(val), uint64_t(val >> 64), 64 );
   eosio_assert( res == (__int128)9223372036854775808_ULLL, "__lshrti3 result should be 2^63" );

   __lshrti3( res, uint64_t(val), uint64_t(val >> 64), 96 );
   eosio_assert( res == (__int128)2147483648_ULLL, "__lshrti3 result should be 2^31" );

   __lshrti3( res, uint64_t(val), uint64_t(val >> 64), 127 );
   eosio_assert( res == 0x1, "__lshrti3 result should be 2^0" );
}

void test_compiler_builtins::test_ashrti3() {
   __int128 res      = 0;
   __int128 test     = 1;
   __int128 val      = -170141183460469231731687303715884105728_LLL;

   test <<= 127; 

   __ashrti3( res, uint64_t(val), uint64_t(val >> 64), 0 );
   eosio_assert( res == -170141183460469231731687303715884105728_LLL, "__ashrti3 result should be -2^127" );

   __ashrti3(res, uint64_t(val), uint64_t(val >> 64), 1 );
   eosio_assert( res == -85070591730234615865843651857942052864_LLL, "__ashrti3 result should be -2^126" );

   __ashrti3(res, uint64_t(val), uint64_t(val >> 64), 2 );
   eosio_assert( res == test >> 2, "__ashrti3 result should be -2^125" );

   __ashrti3( res, uint64_t(val), uint64_t(val >> 64), 64 );
   eosio_assert( res == test >> 64, "__ashrti3 result should be -2^63" );

   __ashrti3( res, uint64_t(val), uint64_t(val >> 64), 95 );
   eosio_assert( res == test >> 95, "__ashrti3 result should be -2^31" );

   __ashrti3( res, uint64_t(val), uint64_t(val >> 64), 127 );
   eosio_assert( res == test >> 127, "__ashrti3 result should be -2^0" );
}


void test_compiler_builtins::test_modti3() {
   __int128 res    = 0;
   __int128 lhs_a  = -30;
   __int128 rhs_a  = 100;
   __int128 lhs_b  = 30;
   __int128 rhs_b  = -100;
   
   __modti3( res, uint64_t(lhs_a), uint64_t(lhs_a >> 64), uint64_t(rhs_a), uint64_t(rhs_a >> 64) );
   eosio_assert( res ==  -30, "__modti3 result should be -30" );

   __modti3( res, uint64_t(lhs_b), uint64_t(lhs_b >> 64), uint64_t(rhs_b), uint64_t(rhs_b >> 64) );
   eosio_assert( res ==  30, "__modti3 result should be 30" );
   
   __modti3( res, uint64_t(lhs_a), uint64_t(lhs_a >> 64), uint64_t(rhs_b), uint64_t(rhs_b >> 64) );
   eosio_assert( res ==  -30, "__modti3 result should be -30" );

   __modti3( res, uint64_t(rhs_a), uint64_t(rhs_a >> 64), uint64_t(lhs_b), uint64_t(lhs_b >> 64) );
   eosio_assert( res ==  10, "__modti3 result should be 10" );

   __modti3( res, uint64_t(rhs_a), uint64_t(rhs_a >> 64), uint64_t(rhs_b), uint64_t(rhs_b >> 64) );
   eosio_assert( res ==  0, "__modti3 result should be 0" );

   __modti3( res, uint64_t(rhs_a), uint64_t(rhs_a >> 64), uint64_t(rhs_a), uint64_t(rhs_a >> 64) );
   eosio_assert( res ==  0, "__modti3 result should be 0" );

   __modti3( res, 0, 0, uint64_t(rhs_a), uint64_t(rhs_a >> 64) );
   eosio_assert( res ==  0, "__modti3 result should be 0" );
}

void test_compiler_builtins::test_modti3_by_0() {
   __int128 res = 0;
   __int128 lhs = 100;

   __modti3( res, uint64_t(lhs), uint64_t(lhs >> 64), 0, 0 );
   eosio_assert( false, "should have thrown an error" );
}

void test_compiler_builtins::test_umodti3() {
   unsigned __int128 res    = 0;
   unsigned __int128 lhs_a  = (unsigned __int128)-30;
   unsigned __int128 rhs_a  = 100;
   unsigned __int128 lhs_b  = 30;
   unsigned __int128 rhs_b  = (unsigned __int128)-100;
   
   __umodti3( res, uint64_t(lhs_a), uint64_t(lhs_a >> 64), uint64_t(rhs_a), uint64_t(rhs_a >> 64) );
   eosio_assert( res ==  (unsigned __int128)-30, "__modti3 result should be -30" );

   __umodti3( res, uint64_t(lhs_b), uint64_t(lhs_b >> 64), uint64_t(rhs_b), uint64_t(rhs_b >> 64) );
   eosio_assert( res ==  30, "__modti3 result should be 30" );
   
   __umodti3( res, uint64_t(lhs_a), uint64_t(lhs_a >> 64), uint64_t(rhs_b), uint64_t(rhs_b >> 64) );
   eosio_assert( res ==  (unsigned __int128)-30, "__modti3 result should be -30" );

   __umodti3( res, uint64_t(rhs_a), uint64_t(rhs_a >> 64), uint64_t(lhs_b), uint64_t(lhs_b >> 64) );
   eosio_assert( res ==  10, "__modti3 result should be 10" );

   __umodti3( res, uint64_t(rhs_a), uint64_t(rhs_a >> 64), uint64_t(rhs_b), uint64_t(rhs_b >> 64) );
   eosio_assert( res ==  0, "__modti3 result should be 0" );

   __umodti3( res, uint64_t(rhs_a), uint64_t(rhs_a >> 64), uint64_t(rhs_a), uint64_t(rhs_a >> 64) );
   eosio_assert( res ==  0, "__modti3 result should be 0" );

   __umodti3( res, 0, 0, uint64_t(rhs_a), uint64_t(rhs_a >> 64) );
   eosio_assert( res ==  0, "__modti3 result should be 0" );
}

void test_compiler_builtins::test_umodti3_by_0() {
   unsigned __int128 res = 0;
   unsigned __int128 lhs = 100;

   __umodti3( res, uint64_t(lhs), uint64_t(lhs >> 64), 0, 0 );
   eosio_assert( false, "should have thrown an error" );
}
