
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

#pragma once
#include <eosio/chain/block.hpp>
#include <eosio/chain/types.hpp>
#include <chrono>

namespace eosio {
   using namespace chain;
   using namespace fc;

   static_assert(sizeof(std::chrono::system_clock::duration::rep) >= 8, "system_clock is expected to be at least 64 bits");
   typedef std::chrono::system_clock::duration::rep tstamp;

   struct chain_size_message {
      uint32_t                   last_irreversible_block_num = 0;
      block_id_type              last_irreversible_block_id;
      uint32_t                   head_num = 0;
      block_id_type              head_id;
   };

   struct handshake_message {
uint16_t                   network_version = 0; ///<计算基数以上的增量值
chain_id_type              chain_id; ///<用于识别链
fc::sha256                 node_id; ///<用于识别对等点和防止自连接
chain::public_key_type     key; ///<身份验证密钥；可以是生产者或对等密钥，也可以是空的
      tstamp                     time;
fc::sha256                 token; ///<时间摘要以证明我们拥有上述密钥的私钥
chain::signature_type      sig; ///<摘要签名
      string                     p2p_address;
      uint32_t                   last_irreversible_block_num = 0;
      block_id_type              last_irreversible_block_id;
      uint32_t                   head_num = 0;
      block_id_type              head_id;
      string                     os;
      string                     agent;
      int16_t                    generation;
   };


  enum go_away_reason {
no_reason, ///<没有理由离开
self, ///<连接本身
duplicate, ///<连接是冗余的
wrong_chain, ///<对等方的链ID不匹配
wrong_version, ///<对等网络版本不匹配
forked, ///<对等方的不可逆块不同
unlinkable, ///<对等发送了一个无法使用的块
bad_transaction, ///<对等方发送了一个验证失败的事务
validation, ///<对等端发送了一个验证失败的块
benign_other, ///<超时等原因。不是致命的，但需要重新设置
fatal_other, ///<a catch all for errors we don't discriminate
authentication ///<对等端验证失败
  };

  constexpr auto reason_str( go_away_reason rsn ) {
    switch (rsn ) {
    case no_reason : return "no reason";
    case self : return "self connect";
    case duplicate : return "duplicate";
    case wrong_chain : return "wrong chain";
    case wrong_version : return "wrong version";
    case forked : return "chain is forked";
    case unlinkable : return "unlinkable block received";
    case bad_transaction : return "bad transaction";
    case validation : return "invalid block";
    case authentication : return "authentication failure";
    case fatal_other : return "some other failure";
    case benign_other : return "some other non-fatal condition";
    default : return "some crazy reason";
    }
  }

  struct go_away_message {
    go_away_message (go_away_reason r = no_reason) : reason(r), node_id() {}
    go_away_reason reason;
fc::sha256 node_id; ///<重复通知
  };

  struct time_message {
tstamp  org;       //！<原始时间戳
tstamp  rec;       //！<接收时间戳
tstamp  xmt;       //！<传输时间戳
mutable tstamp  dst;       //！<目标时间戳
  };

  enum id_list_modes {
    none,
    catch_up,
    last_irr_catch_up,
    normal
  };

  constexpr auto modes_str( id_list_modes m ) {
    switch( m ) {
    case none : return "none";
    case catch_up : return "catch up";
    case last_irr_catch_up : return "last irreversible";
    case normal : return "normal";
    default: return "undefined mode";
    }
  }

  template<typename T>
  struct select_ids {
    select_ids () : mode(none),pending(0),ids() {}
    id_list_modes  mode;
    uint32_t       pending;
    vector<T>      ids;
    bool           empty () const { return (mode == none || ids.empty()); }
  };

  using ordered_txn_ids = select_ids<transaction_id_type>;
  using ordered_blk_ids = select_ids<block_id_type>;

  struct notice_message {
    notice_message () : known_trx(), known_blocks() {}
    ordered_txn_ids known_trx;
    ordered_blk_ids known_blocks;
  };

  struct request_message {
    request_message () : req_trx(), req_blocks() {}
    ordered_txn_ids req_trx;
    ordered_blk_ids req_blocks;
  };

   struct sync_request_message {
      uint32_t start_block;
      uint32_t end_block;
   };

   using net_message = static_variant<handshake_message,
                                      chain_size_message,
                                      go_away_message,
                                      time_message,
                                      notice_message,
                                      request_message,
                                      sync_request_message,
signed_block,         //哪一个＝7
packed_transaction>;  //哪一个＝8

} //命名空间EOSIO

FC_REFLECT( eosio::select_ids<fc::sha256>, (mode)(pending)(ids) )
FC_REFLECT( eosio::chain_size_message,
            (last_irreversible_block_num)(last_irreversible_block_id)
            (head_num)(head_id))
FC_REFLECT( eosio::handshake_message,
            (network_version)(chain_id)(node_id)(key)
            (time)(token)(sig)(p2p_address)
            (last_irreversible_block_num)(last_irreversible_block_id)
            (head_num)(head_id)
            (os)(agent)(generation) )
FC_REFLECT( eosio::go_away_message, (reason)(node_id) )
FC_REFLECT( eosio::time_message, (org)(rec)(xmt)(dst) )
FC_REFLECT( eosio::notice_message, (known_trx)(known_blocks) )
FC_REFLECT( eosio::request_message, (req_trx)(req_blocks) )
FC_REFLECT( eosio::sync_request_message, (start_block)(end_block) )

/*
 *
网络代码的目标
1。低延迟以最小化丢失的块并可能减少块间隔
2。最小化块和事务之间的冗余数据。
三。启用新节点的快速同步
4。更新到新的boost/fc



状态：
   所有节点都知道它们拥有哪些块和事务
   所有节点都知道其对等节点具有哪些块和事务
   节点知道它请求了哪些块和事务。
   所有节点都知道何时知道事务

   发送Hello消息
   写入循环（真）
      如果Peer知道最后一个不可逆块
         如果对等端不知道您知道一个块或事务
            发送你知道的ID（这样他们就不会发送给你）
            产量继续
         如果对等端不知道某个块
            在块对等中发送事务不知道，然后发送块摘要
            产量继续
         如果对等端不知道您已验证的新公共终结点
            将新终结点中继到对等点
            产量继续
         如果对等端不知道事务
            发送远程对等方不知道的最旧事务
            产量继续
         等待来自网络光纤的新验证块、事务或对等信号
      }否则{
         我们假设对等机处于同步模式，在这种情况下，它在
         请求/响应依据

         等待来自读取循环的同步通知
      }


    读环
      如果你好消息
         验证对等端最后一个IR块是否处于我们的状态或断开连接，它们是否位于fork上
         验证对等网络协议

      如果通知消息更新远程对等方已知事务的列表
      如果是TRX消息，则作为未验证插入到全局状态
      如果BLK摘要消息，则插入到全局状态*如果*我们知道所有相关事务
         else关闭连接


    如果我的头块<同龄人的库和我的头块年龄>块间隔*圆形\大小/2，则
    进入同步模式…
        将需要获取的块编号在对等机之间进行划分，并发送获取请求
        如果对等方没有及时响应请求，则向另一个对等方发出请求
        确保飞行中有一个连续的请求队列，并且每次请求都被填满
        发送另一个请求。

     一旦你赶上了所有同龄人，通知所有同龄人你的头像，让他们知道你
     了解lib并开始向您发送实时事务

并行回迁，分组请求


只有在我们还不知道的情况下才将事务转发给对等方。

如果txn大于3mtu，则发送通知而不是事务。





**/

