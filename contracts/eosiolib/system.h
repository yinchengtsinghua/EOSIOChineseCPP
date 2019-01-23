
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

extern "C" {

   /*
    *@defgroup系统API系统API
    *@ingroup contractdev公司
    *@brief定义了与系统级内部函数交互的API
    *
    **/


   /*
    *@defgroup system c api系统c api
    *@ingroup系统API
    *@brief定义了与系统级内部函数交互的API
    *
    *@
    **/


   /*
    *中止此操作的处理，如果测试条件为真，则取消所有挂起的更改。
    *@brief中止此操作的处理，并解除所有挂起的更改
    *@param test-0中止，1忽略
    *
    *实例：
*
    *@代码
    *eosio_assert（1==2，“一不等于二”）；
    *eosio_assert（1==1，“一不等于一”）；
    *@终结码
    *
    *@param msg-一个以空结尾的字符串，解释失败的原因
    **/

   void  eosio_assert( uint32_t test, const char* msg );

   /*
    *中止此操作的处理，如果测试条件为真，则取消所有挂起的更改。
    *@brief中止此操作的处理，并解除所有挂起的更改
    *@param test-0中止，1忽略
    *@param msg-指向字符串开头的指针，解释失败的原因
    *@param msg_len-字符串长度
    **/

   void  eosio_assert_message( uint32_t test, const char* msg, uint32_t msg_len );

   /*
    *中止此操作的处理，如果测试条件为真，则取消所有挂起的更改。
    *@brief中止此操作的处理，并解除所有挂起的更改
    *@param test-0中止，1忽略
    *@param code-错误代码
    **/

   void  eosio_assert_code( uint32_t test, uint64_t code );

    /*
    *此方法将在不违反合同的情况下中止执行WASM。这用于绕过通常调用的所有清理/析构函数。
    *@brief在不违反合同的情况下中止执行WASM
    *@param code-退出代码
    *实例：
*
    *@代码
    *EOSIO出口（0）；
    *EOSIO出口（1）；
    *EOSIO出口（2）；
    *EOSIO出口（3）；
    *@终结码
    **/

   [[noreturn]] void  eosio_exit( int32_t code );


   /*
    *返回当前块的时间（以微秒为单位）
    *@brief获取当前块的时间（即包含此操作的块）
    *@当前块的返回时间（以微秒计）
    **/

   uint64_t  current_time();

   /*
    *返回包含此操作的块从1970年开始的时间（秒）
    *@brief获取当前块（即包含此操作的块）的时间（四舍五入到最接近的秒）
    *@从当前块的1970开始返回时间（秒）
    **/

   uint32_t  now() {
      return (uint32_t)( current_time() / 1000000 );
   }
///@系统CAPI


}
