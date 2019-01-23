
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once
#include <cstdint>
#include <softfloat.hpp>

extern "C" {
   __int128 ___fixdfti(uint64_t);
   __int128 ___fixsfti(uint32_t);
   __int128 ___fixtfti( float128_t);
   unsigned __int128 ___fixunsdfti(uint64_t);
   unsigned __int128 ___fixunssfti(uint32_t);
   unsigned __int128 ___fixunstfti(float128_t);
   double ___floattidf(__int128);
   double ___floatuntidf(unsigned __int128);
}
