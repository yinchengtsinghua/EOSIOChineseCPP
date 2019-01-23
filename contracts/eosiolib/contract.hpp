
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

namespace eosio {

/*
 *@defgroup contract type合同类型
 *@ingroup类型
 *@brief定义的合同类型是每个eosio合同的%基类
 *
 *@
 *
 **/


/*
 *@brief%EOSIO合同的基类。
 *@details%EOSIO合同的基类。%一个新合同应该从这个类派生，这样它就可以使用eosio-abi宏。
 **/

class contract {
   public:
      /*
       *根据合同名称建立新合同
       *
       *@brief构造一个新的契约对象。
       *@param n-此合同的名称
       **/

      contract( account_name n ):_self(n){}
      
      /*
       *
       *获取此合同名称
       *
       *@brief获取此合同名称。
       *@return account\u name-本合同名称
       **/

      inline account_name get_self()const { return _self; }

   protected:
      /*
       *本合同名称
       *
       *@简要说明本合同的名称。
       **/

      account_name _self;
};

///@合同类型
} ///命名空间eosio
