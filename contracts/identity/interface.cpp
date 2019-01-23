
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include "interface.hpp"

namespace identity {

identity_name interface::get_claimed_identity( account_name acnt ) {
   return accounts_table( _self, acnt ).get_or_default(0);
}

account_name interface::get_owner_for_identity( uint64_t receiver, identity_name ident ) {
//对于每个受信任的所有者证书
//检查证书是否仍然可信
//查看帐户是否已认领
   certs_table certs( _self, ident );
   auto idx = certs.template get_index<N(bytuple)>();
   auto itr = idx.lower_bound(certrow::key(N(owner), 1, 0));
   account_name owner = 0;
   while (itr != idx.end() && itr->property == N(owner) && itr->trusted) {
      if (sizeof(account_name) == itr->data.size()) {
         account_name account = *reinterpret_cast<const account_name*>(itr->data.data());
         if (ident == get_claimed_identity(account)) {
            if (is_trusted(itr->certifier) ) {
//证明者仍然可信
               if (!owner || owner == account) {
                  owner = account;
               } else {
//发现的矛盾：不同的业主为同一身份认证
                  return 0;
               }
            } else if (_self == receiver){
//证明者不再受信任，需要取消设置标志
               idx.modify(itr, 0, [&](certrow& r) {
                     r.trusted = 0;
                  });
            } else {
//证书颁发者不再受信任，但代码以只读模式运行
            }
         }
      } else {
//坏行-跳过它
      }
      ++itr;
   }
   if (owner) {
//找到所有者，标记为可信的证书之间没有矛盾
      return owner;
   }
//找不到可信证书
//让我们看看是否有任何不可信的认证成为可信的
   itr = idx.lower_bound(certrow::key(N(owner), 0, 0));
   while (itr != idx.end() && itr->property == N(owner) && !itr->trusted) {
      if (sizeof(account_name) == itr->data.size()) {
         account_name account = *reinterpret_cast<const account_name*>(itr->data.data());
         if (ident == get_claimed_identity(account) && is_trusted(itr->certifier)) {
            if (_self == receiver) {
//证书颁发者变得可信，我们有更新标志的权限
               idx.modify(itr, 0, [&](certrow& r) {
                     r.trusted = 1;
                  });
            }
            if (!owner || owner == account) {
               owner = account;
            } else {
//发现的矛盾：不同的业主为同一身份认证
               return 0;
            }
         }
      } else {
//坏行-跳过它
      }
      ++itr;
   }
   return owner;
}

identity_name interface::get_identity_for_account( uint64_t receiver, account_name acnt ) {
//检查帐户拥有自我认证所有者的身份
//验证受信任的证书颁发者已确认所有者
   auto identity = get_claimed_identity(acnt);
   return (identity != 0 && acnt == get_owner_for_identity(receiver, identity)) ? identity : 0;
}

} //命名空间标识
