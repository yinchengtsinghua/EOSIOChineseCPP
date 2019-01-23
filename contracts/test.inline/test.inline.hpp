
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <eosiolib/eosio.hpp>
#include <eosiolib/privileged.h>
#include <eosiolib/producer_schedule.hpp>

namespace eosio {

   class testinline : public contract {
      public:
         testinline( action_name self ):contract(self){}

         void reqauth( account_name from ) {
            require_auth( from );
         }

         void forward( action_name reqauth, account_name forward_code, account_name forward_auth ) {
            require_auth( reqauth );
            INLINE_ACTION_SENDER(testinline, reqauth)( forward_code, {forward_auth,N(active)}, {forward_auth} );
//发送内联操作（testinline（forward_code），reqauth，转发，n（active），转发）；
//eosio:：dispatch_inline<account_name>（n（forward_code），n（reqauth），forward_auth，n（active），forward_auth）；
         }
   };

} ///命名空间eosio
