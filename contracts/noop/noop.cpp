
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


#include <eosiolib/eosio.hpp>

namespace eosio {

   class noop: public contract {
      public:
         noop( account_name self ): contract( self ) { }
         void anyaction( account_name from,
                         /*ST标准：：字符串和/*类型*/，
                         const std：：字符串&/*日期*/ )

         {
            require_auth( from );
         }
   };

   EOSIO_ABI( noop, ( anyaction ) )

} ///EOSIO
