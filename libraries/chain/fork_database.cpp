
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <eosio/chain/fork_database.hpp>
#include <eosio/chain/exceptions.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <fc/io/fstream.hpp>
#include <fstream>

namespace eosio { namespace chain {
   using boost::multi_index_container;
   using namespace boost::multi_index;


   struct by_block_id;
   struct by_block_num;
   struct by_lib_block_num;
   struct by_prev;
   typedef multi_index_container<
      block_state_ptr,
      indexed_by<
         hashed_unique< tag<by_block_id>, member<block_header_state, block_id_type, &block_header_state::id>, std::hash<block_id_type>>,
         ordered_non_unique< tag<by_prev>, const_mem_fun<block_header_state, const block_id_type&, &block_header_state::prev> >,
         ordered_non_unique< tag<by_block_num>,
            composite_key< block_state,
               member<block_header_state,uint32_t,&block_header_state::block_num>,
               member<block_state,bool,&block_state::in_current_chain>
            >,
            composite_key_compare< std::less<uint32_t>, std::greater<bool> >
         >,
         ordered_non_unique< tag<by_lib_block_num>,
            composite_key< block_header_state,
                member<block_header_state,uint32_t,&block_header_state::dpos_irreversible_blocknum>,
                member<block_header_state,uint32_t,&block_header_state::bft_irreversible_blocknum>,
                member<block_header_state,uint32_t,&block_header_state::block_num>
            >,
            composite_key_compare< std::greater<uint32_t>, std::greater<uint32_t>, std::greater<uint32_t> >
         >
      >
   > fork_multi_index_type;


   struct fork_database_impl {
      fork_multi_index_type index;
      block_state_ptr       head;
      fc::path              datadir;
   };


   fork_database::fork_database( const fc::path& data_dir ):my( new fork_database_impl() ) {
      my->datadir = data_dir;

      if (!fc::is_directory(my->datadir))
         fc::create_directories(my->datadir);

      auto fork_db_dat = my->datadir / config::forkdb_filename;
      if( fc::exists( fork_db_dat ) ) {
         string content;
         fc::read_file_contents( fork_db_dat, content );

         fc::datastream<const char*> ds( content.data(), content.size() );
         unsigned_int size; fc::raw::unpack( ds, size );
         for( uint32_t i = 0, n = size.value; i < n; ++i ) {
            block_state s;
            fc::raw::unpack( ds, s );
            set( std::make_shared<block_state>( move( s ) ) );
         }
         block_id_type head_id;
         fc::raw::unpack( ds, head_id );

         my->head = get_block( head_id );

         fc::remove( fork_db_dat );
      }
   }

   void fork_database::close() {
      if( my->index.size() == 0 ) return;

      auto fork_db_dat = my->datadir / config::forkdb_filename;
      std::ofstream out( fork_db_dat.generic_string().c_str(), std::ios::out | std::ios::binary | std::ofstream::trunc );
      uint32_t num_blocks_in_fork_db = my->index.size();
      fc::raw::pack( out, unsigned_int{num_blocks_in_fork_db} );
      for( const auto& s : my->index ) {
         fc::raw::pack( out, *s );
      }
      if( my->head )
         fc::raw::pack( out, my->head->id );
      else
         fc::raw::pack( out, block_id_type() );

///我们通常不会将头块指示为不可逆。
///如果lib是头块，则通常无法对其进行修剪，因为
///下一个块需要构建头块。我们正在退出
///现在，我们可以在退出前将此块修剪为不可逆。
      auto lib    = my->head->dpos_irreversible_blocknum;
      auto oldest = *my->index.get<by_block_num>().begin();
      if( oldest->block_num <= lib ) {
         prune( oldest );
      }

      my->index.clear();
   }

   fork_database::~fork_database() {
      close();
   }

   void fork_database::set( block_state_ptr s ) {
      auto result = my->index.insert( s );
      EOS_ASSERT( s->id == s->header.id(), fork_database_exception, 
                  "block state id (${id}) is different from block state header id (${hid})", ("id", string(s->id))("hid", string(s->header.id())) );

//fc_断言（s->block_num==s->header.block_num（））；

      EOS_ASSERT( result.second, fork_database_exception, "unable to insert block state, duplicate state detected" );
      if( !my->head ) {
         my->head =  s;
      } else if( my->head->block_num < s->block_num ) {
         my->head =  s;
      }
   }

   block_state_ptr fork_database::add( const block_state_ptr& n, bool skip_validate_previous ) {
      EOS_ASSERT( n, fork_database_exception, "attempt to add null block state" );
      EOS_ASSERT( my->head, fork_db_block_not_found, "no head block set" );

      if( !skip_validate_previous ) {
         auto prior = my->index.find( n->block->previous );
         EOS_ASSERT( prior != my->index.end(), unlinkable_block_exception,
                     "unlinkable block", ("id", n->block->id())("previous", n->block->previous) );
      }

      auto inserted = my->index.insert(n);
      EOS_ASSERT( inserted.second, fork_database_exception, "duplicate block added?" );

      my->head = *my->index.get<by_lib_block_num>().begin();

      auto lib    = my->head->dpos_irreversible_blocknum;
      auto oldest = *my->index.get<by_block_num>().begin();

      if( oldest->block_num < lib ) {
         prune( oldest );
      }

      return n;
   }

   block_state_ptr fork_database::add( signed_block_ptr b, bool skip_validate_signee ) {
      EOS_ASSERT( b, fork_database_exception, "attempt to add null block" );
      EOS_ASSERT( my->head, fork_db_block_not_found, "no head block set" );

      const auto& by_id_idx = my->index.get<by_block_id>();
      auto existing = by_id_idx.find( b->id() );
      EOS_ASSERT( existing == by_id_idx.end(), fork_database_exception, "we already know about this block" );

      auto prior = by_id_idx.find( b->previous );
      EOS_ASSERT( prior != by_id_idx.end(), unlinkable_block_exception, "unlinkable block", ("id", string(b->id()))("previous", string(b->previous)) );

      auto result = std::make_shared<block_state>( **prior, move(b), skip_validate_signee );
      EOS_ASSERT( result, fork_database_exception , "fail to add new block state" );
      return add(result, true);
   }

   const block_state_ptr& fork_database::head()const { return my->head; }

   /*
    *给定两个头块，返回叉图的两个分支
    *以一个共同的祖先结束（相同的先前块）
    **/

   pair< branch_type, branch_type >  fork_database::fetch_branch_from( const block_id_type& first,
                                                                       const block_id_type& second )const {
      pair<branch_type,branch_type> result;
      auto first_branch = get_block(first);
      auto second_branch = get_block(second);

      while( first_branch->block_num > second_branch->block_num )
      {
         result.first.push_back(first_branch);
         first_branch = get_block( first_branch->header.previous );
         EOS_ASSERT( first_branch, fork_db_block_not_found, "block ${id} does not exist", ("id", string(first_branch->header.previous)) );
      }

      while( second_branch->block_num > first_branch->block_num )
      {
         result.second.push_back( second_branch );
         second_branch = get_block( second_branch->header.previous );
         EOS_ASSERT( second_branch, fork_db_block_not_found, "block ${id} does not exist", ("id", string(second_branch->header.previous)) );
      }

      while( first_branch->header.previous != second_branch->header.previous )
      {
         result.first.push_back(first_branch);
         result.second.push_back(second_branch);
         first_branch = get_block( first_branch->header.previous );
         second_branch = get_block( second_branch->header.previous );
         EOS_ASSERT( first_branch && second_branch, fork_db_block_not_found, 
                     "either block ${fid} or ${sid} does not exist", 
                     ("fid", string(first_branch->header.previous))("sid", string(second_branch->header.previous)) );
      }

      if( first_branch && second_branch )
      {
         result.first.push_back(first_branch);
         result.second.push_back(second_branch);
      }
      return result;
} ///从中提取分支

///remove用此ID生成的所有无效分叉，包括此ID
   void fork_database::remove( const block_id_type& id ) {
      vector<block_id_type> remove_queue{id};

      for( uint32_t i = 0; i < remove_queue.size(); ++i ) {
         auto itr = my->index.find( remove_queue[i] );
         if( itr != my->index.end() )
            my->index.erase(itr);

         auto& previdx = my->index.get<by_prev>();
         auto  previtr = previdx.lower_bound(remove_queue[i]);
         while( previtr != previdx.end() && (*previtr)->header.previous == remove_queue[i] ) {
            remove_queue.push_back( (*previtr)->id );
            ++previtr;
         }
      }
//wdump（（my->index.size（））；
      my->head = *my->index.get<by_lib_block_num>().begin();
   }

   void fork_database::set_validity( const block_state_ptr& h, bool valid ) {
      if( !valid ) {
         remove( h->id );
      } else {
///remove早于不可逆转并将块标记为有效
         h->validated = true;
      }
   }

   void fork_database::mark_in_current_chain( const block_state_ptr& h, bool in_current_chain ) {
      if( h->in_current_chain == in_current_chain )
         return;

      auto& by_id_idx = my->index.get<by_block_id>();
      auto itr = by_id_idx.find( h->id );
      EOS_ASSERT( itr != by_id_idx.end(), fork_db_block_not_found, "could not find block in fork database" );

by_id_idx.modify( itr, [&]( auto& bsp ) { //需要以这种方式进行修改，而不是直接进行修改，以便BoostMultiIndex可以重新排序
         bsp->in_current_chain = in_current_chain;
      });
   }

   void fork_database::prune( const block_state_ptr& h ) {
      auto num = h->block_num;

      auto& by_bn = my->index.get<by_block_num>();
      auto bni = by_bn.begin();
      while( bni != by_bn.end() && (*bni)->block_num < num ) {
         prune( *bni );
         bni = by_bn.begin();
      }

      auto itr = my->index.find( h->id );
      if( itr != my->index.end() ) {
         irreversible(*itr);
         my->index.erase(itr);
      }

      auto& numidx = my->index.get<by_block_num>();
      auto nitr = numidx.lower_bound( num );
      while( nitr != numidx.end() && (*nitr)->block_num == num ) {
         auto itr_to_remove = nitr;
         ++nitr;
         auto id = (*itr_to_remove)->id;
         remove( id );
      }
   }

   block_state_ptr   fork_database::get_block(const block_id_type& id)const {
      auto itr = my->index.find( id );
      if( itr != my->index.end() )
         return *itr;
      return block_state_ptr();
   }

   block_state_ptr   fork_database::get_block_in_current_chain_by_num( uint32_t n )const {
      const auto& numidx = my->index.get<by_block_num>();
      auto nitr = numidx.lower_bound( n );
//删除以下断言以便返回空值
//FC断言（硝酸盐！=numidx.end（）&&（*nitro）->block_num==n，
//“在fork数据库中找不到块号为$block_num“，”（block_num，n））；
//fc_断言（（*nitro）->In_current_chain==true，
//“在fork数据库中找到的块（块号为$block_num）不在当前链中，”“（block_num，n））；
      if( nitr == numidx.end() || (*nitr)->block_num != n || (*nitr)->in_current_chain != true )
         return block_state_ptr();
      return *nitr;
   }

   void fork_database::add( const header_confirmation& c ) {
      auto b = get_block( c.block_id );
      EOS_ASSERT( b, fork_db_block_not_found, "unable to find block id ${id}", ("id",c.block_id));
      b->add_confirmation( c );

      if( b->bft_irreversible_blocknum < b->block_num &&
         b->confirmations.size() >= ((b->active_schedule.producers.size() * 2) / 3 + 1) ) {
         set_bft_irreversible( c.block_id );
      }
   }

   /*
    *此方法将此块设置为bft不可逆，并将更新
    *所有构建在其基础上的块，如果存在相同的bft-irb
    *bft irb小于此块编号。
    *
    *这需要对所有分叉进行搜索。
    **/

   void fork_database::set_bft_irreversible( block_id_type id ) {
      auto& idx = my->index.get<by_block_id>();
      auto itr = idx.find(id);
      uint32_t block_num = (*itr)->block_num;
      idx.modify( itr, [&]( auto& bsp ) {
           bsp->bft_irreversible_blocknum = bsp->block_num;
      });

      /*为了防止堆栈溢出，我们对
       *fork数据库。在每一个阶段，我们都要迭代前一阶段的叶子。
       *并查找链接其上一个节点的所有节点。如果我们更新bft lib，那么我们
       *将其添加到下一层的队列中。这个lambda接受一个层并返回
       *需要为下一层迭代的所有块ID。
       **/

      auto update = [&]( const vector<block_id_type>& in ) {
         vector<block_id_type> updated;

         for( const auto& i : in ) {
            auto& pidx = my->index.get<by_prev>();
            auto pitr  = pidx.lower_bound( i );
            auto epitr = pidx.upper_bound( i );
            while( pitr != epitr ) {
               pidx.modify( pitr, [&]( auto& bsp ) {
                 if( bsp->bft_irreversible_blocknum < block_num ) {
                    bsp->bft_irreversible_blocknum = block_num;
                    updated.push_back( bsp->id );
                 }
               });
               ++pitr;
            }
         }
         return updated;
      };

      vector<block_id_type> queue{id};
      while( queue.size() ) {
         queue = update( queue );
      }
   }

} } ///EOSIO：链
