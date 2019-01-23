
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include "common.hpp"

namespace identity {

   class interface : public identity_base {
      public:
         using identity_base::identity_base;

         identity_name get_claimed_identity( account_name acnt );

         account_name get_owner_for_identity( uint64_t receiver, identity_name ident );

         identity_name get_identity_for_account( uint64_t receiver, account_name acnt );
   };

}
