
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once
#include <eosio/chain/block_state.hpp>
#include <boost/signals2/signal.hpp>

namespace eosio { namespace chain {

   using boost::signals2::signal;

   struct fork_database_impl;

   typedef vector<block_state_ptr> branch_type;

   /*
    *@class fork_数据库
    *@brief管理所有潜在未确认分叉的轻量状态
    *
    *当接收到新块时，它们将被推入fork数据库。叉子
    *数据库跟踪最长的链和最后一个不可逆的块号。所有
    *超过最后一个不可逆块的块在发出
    *不可逆信号。
    **/

   class fork_database {
      public:

         fork_database( const fc::path& data_dir );
         ~fork_database();

         void close();

         block_state_ptr  get_block(const block_id_type& id)const;
         block_state_ptr  get_block_in_current_chain_by_num( uint32_t n )const;
//vector<block_state_ptr>get_blocks_by_number（uint32_t n）const；

         /*
          *提供一个“有效”的块状态，其他分叉可以在此状态下生成。
          **/

         void            set( block_state_ptr s );

         /*此方法将尝试将块附加到现有的
          *块状态，并将返回指向新块状态的指针或
          *出错时抛出。
          **/

         block_state_ptr add( signed_block_ptr b, bool skip_validate_signee );
         block_state_ptr add( const block_state_ptr& next_block, bool skip_validate_previous );
         void            remove( const block_id_type& id );

         void            add( const header_confirmation& c );

         const block_state_ptr& head()const;

         /*
          *给定两个头块，返回叉图的两个分支
          *以一个共同的祖先结束（相同的先前块）
          **/

         pair< branch_type, branch_type >  fetch_branch_from( const block_id_type& first,
                                                              const block_id_type& second )const;


         /*
          *如果块无效，将被删除。如果有效，则阻止旧的
          *而lib在发出不可逆信号后被修剪。
          **/

         void set_validity( const block_state_ptr& h, bool valid );
         void mark_in_current_chain( const block_state_ptr& h, bool in_current_chain );
         void prune( const block_state_ptr& h );

         /*
          *此信号在块状态变为不可逆时发出，一旦变为不可逆时发出。
          *除非是头块，否则将其拆下。
          **/

         signal<void(block_state_ptr)> irreversible;

      private:
         void set_bft_irreversible( block_id_type id );
         unique_ptr<fork_database_impl> my;
   };

} } ///EOSIO：链
