
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

#ifdef __cplusplus
extern "C" {
#endif
   /*
    *@defgroup console api控制台api
    *@brief定义用于记录/打印文本消息的API
    *@ingroup contractdev公司
    *
    **/


   /*
    *@defgroup控制台c api控制台C API
    *@brief defnes%c用于记录/打印文本消息的API
    *@ingroup控制台API
    *@
    **/


   /*
    *打印字符串
    *@brief打印字符串
    *@param cstr-以空结尾的字符串
    *
    *实例：
*
    *@代码
    *印刷品（“你好，世界！”）；//输出：你好，世界！
    *@终结码
    **/

   void prints( const char* cstr );

   /*
    *打印指定长度的字符串
    *@brief打印字符串
    *@param cstr-指向字符串的指针
    *@param len-要打印的字符串长度
    *
    *实例：
*
    *@代码
    *打印“你好，世界！”，5）；//输出：hello
    *@终结码
    **/

   void prints_l( const char* cstr, uint32_t len);

   /*
    *将值打印为64位带符号整数
    *@brief将值打印为64位带符号整数
    要打印的64位有符号整数的*@param值
    *
    *实例：
*
    *@代码
    *printi（-1e+18）；//输出：-1000000000000000
    *@终结码
    **/

   void printi( int64_t value );

   /*
    *将值打印为64位无符号整数
    *@brief将值打印为64位无符号整数
    *@要打印的64位无符号整数的参数值
    *
    *实例：
*
    *@代码
    *printui（1e+18）；//输出：1000000000000000
    *@终结码
    **/

   void printui( uint64_t value );

   /*
    *将值打印为128位带符号整数
    *@brief将值打印为128位带符号整数
    *@param value是指向要打印的128位有符号整数的指针。
    *
    *实例：
*
    *@代码
    *Int128_t Large_Int（-87654323456）；
    *printi128（&large_int）；//输出：-87654323456
    *@终结码
    **/

   void printi128( const int128_t* value );

   /*
    *将值打印为128位无符号整数
    *@brief将值打印为128位无符号整数
    *@param value是指向要打印的128位无符号整数的指针
    *
    *实例：
*
    *@代码
    *uint128_t large_int（87654323456）；
    *printui128（&large_int）；//输出：87654323456
    *@终结码
    **/

   void printui128( const uint128_t* value );

   /*
    *将值打印为单精度浮点数
    *@brief将值打印为单精度浮点数（即float）
    待打印浮点数的@参数值
    *
    *实例：
*
    *@代码
    *浮点数=5.0/10.0；
    *printsf（value）；//输出：0.5
    *@终结码
    **/

   void printsf(float value);

   /*
    *将值打印为双精度浮点数
    *@brief将值打印为双精度浮点数（即double）
    *@要打印的双精度参数值
    *
    *实例：
*
    *@代码
    *双值=5.0/10.0；
    *printdf（value）；//输出：0.5
    *@终结码
    **/

   void printdf(double value);

   /*
    *将值打印为四倍精度浮点数
    *@brief将值打印为四倍精度浮点数（即长双精度）
    *@param value是指向要打印的长双精度值的指针
    *
    *实例：
*
    *@代码
    *长双值=5.0/10.0；
    *printqf（value）；//输出：0.5
    *@终结码
    **/

   void printqf(const long double* value);

   /*
    *将64位名称打印为base32编码字符串
    *@brief将64位名称打印为base32编码字符串
    *@param name-要打印的64位名称
    *
    *实例：
    *@代码
    *printn（n（abcde））；//输出：abcde
    *@终结码
    **/

   void printn( uint64_t name );

   /*
    **/

   void printhex( const void* data, uint32_t datalen );

///@
#ifdef __cplusplus
}
#endif
