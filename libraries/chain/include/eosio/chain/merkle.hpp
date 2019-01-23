
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once
#include <eosio/chain/types.hpp>

namespace eosio { namespace chain {

   digest_type make_canonical_left(const digest_type& val);
   digest_type make_canonical_right(const digest_type& val);

   bool is_canonical_left(const digest_type& val);
   bool is_canonical_right(const digest_type& val);


   inline auto make_canonical_pair(const digest_type& l, const digest_type& r) {
      return make_pair(make_canonical_left(l), make_canonical_right(r));
   };

   /*
    *计算一组摘要的merkle根，如果id是奇数，它将复制最后一个id。
    **/

   digest_type merkle( vector<digest_type> ids );

} } ///EOSIO：链
