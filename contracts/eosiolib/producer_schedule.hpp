
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once
#include <eosiolib/privileged.hpp>

#include <vector>

namespace eosio {

   /*
    *定义活动生产者集的顺序、帐户名和签名密钥。
    *
    *@brief定义了活动生产者集的顺序、帐户名和签名密钥。
    **/

   struct producer_schedule {
      /*
       *计划的版本号。它是按顺序递增的版本号
       *
       *@计划的简要版本号
       **/

      uint32_t                     version;

      /*
       *此计划的生产商列表，包括其签名密钥
       *
       *@此计划的生产者的简要列表，包括其签名密钥
       **/

      std::vector<producer_key>    producers;
   };

///@产品类型
} ///命名空间eosio
