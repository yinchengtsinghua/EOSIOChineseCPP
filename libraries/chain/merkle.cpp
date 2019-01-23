
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <eosio/chain/merkle.hpp>
#include <fc/io/raw.hpp>

namespace eosio { namespace chain {

/*
 *为了保持校对简洁，在散列之前，我们设置了第一位
 *前一个哈希值的0或1表示它所在的边
 *
 *这免除了我们必须指出左右接触的证据。
 *因为节点值意味着
 **/


digest_type make_canonical_left(const digest_type& val) {
   digest_type canonical_l = val;
   canonical_l._hash[0] &= 0xFFFFFFFFFFFFFF7FULL;
   return canonical_l;
}

digest_type make_canonical_right(const digest_type& val) {
   digest_type canonical_r = val;
   canonical_r._hash[0] |= 0x0000000000000080ULL;
   return canonical_r;
}

bool is_canonical_left(const digest_type& val) {
   return (val._hash[0] & 0x0000000000000080ULL) == 0;
}

bool is_canonical_right(const digest_type& val) {
   return (val._hash[0] & 0x0000000000000080ULL) != 0;
}


digest_type merkle(vector<digest_type> ids) {
   if( 0 == ids.size() ) { return digest_type(); }

   while( ids.size() > 1 ) {
      if( ids.size() % 2 )
         ids.push_back(ids.back());

      for (int i = 0; i < ids.size() / 2; i++) {
         ids[i] = digest_type::hash(make_canonical_pair(ids[2 * i], ids[(2 * i) + 1]));
      }

      ids.resize(ids.size() / 2);
   }

   return ids.front();
}

} } //EOSIO：链
