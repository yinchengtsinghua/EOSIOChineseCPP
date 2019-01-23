
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include "tester.hpp"
#include <boost/signals2/signal.hpp>
#include <boost/signals2/shared_connection_block.hpp>

namespace eosio { namespace testing {
   using namespace boost::signals2;

   /*
    *@brief测试人员的网络类提供了一个简单的虚拟P2P网络，将测试区块链连接在一起。
    *
    *测试仪可以在任何给定时间连接到零个或多个测试仪网络。当一个新的
    *测试人员加入网络，它将与网络中已有的所有其他区块链（区块）同步。
    *只知道新的链将被推送到以前的网络成员，反之亦然，忽略不在上的块。
    *主叉）。此后，每当网络中的任何区块链获得新的区块时，该区块将被推送到所有区块。
    *网络中的其他区块链。
    **/

   class tester_network {
   public:
      /*
       *@brief向网络添加新的区块链
       *@param new_区块链要添加的区块链
       **/

      void connect_blockchain(base_tester &new_blockchain);

      /*
       *@brief从网络中删除区块链
       *@param离开区块链移除区块链
       **/

      void disconnect_blockchain(base_tester &leaving_blockchain);

      /*
       *@brief断开所有区块链与网络的连接
       **/

      void disconnect_all();

      /*
       *@brief向网络中的所有区块链发送一个区块
       *@param阻止要发送的块
       **/

      void propagate_block(const signed_block &block, const base_tester &skip_db);

   protected:
      std::map<base_tester *, scoped_connection> blockchains;
   };

} } ///eosio：：测试
