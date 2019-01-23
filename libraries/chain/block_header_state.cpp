
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <eosio/chain/block_header_state.hpp>
#include <eosio/chain/exceptions.hpp>
#include <limits>

namespace eosio { namespace chain {


   bool block_header_state::is_active_producer( account_name n )const {
      return producer_to_last_produced.find(n) != producer_to_last_produced.end();
   }

   producer_key block_header_state::get_scheduled_producer( block_timestamp_type t )const {
      auto index = t.slot % (active_schedule.producers.size() * config::producer_repetitions);
      index /= config::producer_repetitions;
      return active_schedule.producers[index];
   }

   uint32_t block_header_state::calc_dpos_last_irreversible()const {
      vector<uint32_t> blocknums; blocknums.reserve( producer_to_last_implied_irb.size() );
      for( auto& i : producer_to_last_implied_irb ) {
         blocknums.push_back(i.second);
      }
///2/3必须更大，因此如果我将1/3放入从低到高排序的列表中，则2/3更大

      if( blocknums.size() == 0 ) return 0;
///TODO:更新到第n个元素
      std::sort( blocknums.begin(), blocknums.end() );
      return blocknums[ (blocknums.size()-1) / 3 ];
   }

  /*
   *为给定的块时间生成模板块头状态，它不会
   *包含事务mroot、操作mroot或新的_生产者作为这些组件
   *源自链状态。
   **/

  block_header_state block_header_state::generate_next( block_timestamp_type when )const {
    block_header_state result;

    if( when != block_timestamp_type() ) {
       EOS_ASSERT( when > header.timestamp, block_validate_exception, "next block must be in the future" );
    } else {
       (when = header.timestamp).slot++;
    }
    result.header.timestamp                                = when;
    result.header.previous                                 = id;
    result.header.schedule_version                         = active_schedule.version;
                                                           
    auto prokey                                            = get_scheduled_producer(when);
    result.block_signing_key                               = prokey.block_signing_key;
    result.header.producer                                 = prokey.producer_name;
                                                           
    result.pending_schedule_lib_num                        = pending_schedule_lib_num;
    result.pending_schedule_hash                           = pending_schedule_hash;
    result.block_num                                       = block_num + 1;
    result.producer_to_last_produced                       = producer_to_last_produced;
    result.producer_to_last_implied_irb                    = producer_to_last_implied_irb;
    result.producer_to_last_produced[prokey.producer_name] = result.block_num;
    result.blockroot_merkle = blockroot_merkle;
    result.blockroot_merkle.append( id );

    result.active_schedule                       = active_schedule;
    result.pending_schedule                      = pending_schedule;
    result.dpos_proposed_irreversible_blocknum   = dpos_proposed_irreversible_blocknum;
    result.bft_irreversible_blocknum             = bft_irreversible_blocknum;

    result.producer_to_last_implied_irb[prokey.producer_name] = result.dpos_proposed_irreversible_blocknum;
    result.dpos_irreversible_blocknum                         = result.calc_dpos_last_irreversible(); 

///增加确认的计数
    static_assert(std::numeric_limits<uint8_t>::max() >= (config::max_producers * 2 / 3) + 1, "8bit confirmations may not be able to hold all of the needed confirmations");

//这将使用上一个块活动的\计划，因为这是“计划”的标志，因此确认\此块
    auto num_active_producers = active_schedule.producers.size();
    uint32_t required_confs = (uint32_t)(num_active_producers * 2 / 3) + 1;

    if( confirm_count.size() < config::maximum_tracked_dpos_confirmations ) {
       result.confirm_count.reserve( confirm_count.size() + 1 );
       result.confirm_count  = confirm_count;
       result.confirm_count.resize( confirm_count.size() + 1 );
       result.confirm_count.back() = (uint8_t)required_confs;
    } else {
       result.confirm_count.resize( confirm_count.size() );
       memcpy( &result.confirm_count[0], &confirm_count[1], confirm_count.size() - 1 );
       result.confirm_count.back() = (uint8_t)required_confs;
    }

    return result;
} //下一代

   bool block_header_state::maybe_promote_pending() {
      if( pending_schedule.producers.size() &&
          dpos_irreversible_blocknum >= pending_schedule_lib_num )
      {
         active_schedule = move( pending_schedule );

         flat_map<account_name,uint32_t> new_producer_to_last_produced;
         for( const auto& pro : active_schedule.producers ) {
            auto existing = producer_to_last_produced.find( pro.producer_name );
            if( existing != producer_to_last_produced.end() ) {
               new_producer_to_last_produced[pro.producer_name] = existing->second;
            } else {
               new_producer_to_last_produced[pro.producer_name] = dpos_irreversible_blocknum;
            }
         }

         flat_map<account_name,uint32_t> new_producer_to_last_implied_irb;
         for( const auto& pro : active_schedule.producers ) {
            auto existing = producer_to_last_implied_irb.find( pro.producer_name );
            if( existing != producer_to_last_implied_irb.end() ) {
               new_producer_to_last_implied_irb[pro.producer_name] = existing->second;
            } else {
               new_producer_to_last_implied_irb[pro.producer_name] = dpos_irreversible_blocknum;
            }
         }

         producer_to_last_produced = move( new_producer_to_last_produced );
         producer_to_last_implied_irb = move( new_producer_to_last_implied_irb);
         producer_to_last_produced[header.producer] = block_num;

         return true;
      }
      return false;
   }

  void block_header_state::set_new_producers( producer_schedule_type pending ) {
      EOS_ASSERT( pending.version == active_schedule.version + 1, producer_schedule_exception, "wrong producer schedule version specified" );
      EOS_ASSERT( pending_schedule.producers.size() == 0, producer_schedule_exception,
                 "cannot set new pending producers until last pending is confirmed" );
      header.new_producers     = move(pending);
      pending_schedule_hash    = digest_type::hash( *header.new_producers );
      pending_schedule         = *header.new_producers;
      pending_schedule_lib_num = block_num;
  }


  /*
   *根据提供的带符号块头，将当前头状态转换为下一个头状态。
   *
   *给定有符号块头，根据头时间生成预期模板，
   *然后验证提供的头是否与模板匹配。
   *
   *如果标题指定了新的_生产者，则相应地应用它们。
   **/

  block_header_state block_header_state::next( const signed_block_header& h, bool skip_validate_signee )const {
    EOS_ASSERT( h.timestamp != block_timestamp_type(), block_validate_exception, "", ("h",h) );
    EOS_ASSERT( h.header_extensions.size() == 0, block_validate_exception, "no supported extensions" );

    EOS_ASSERT( h.timestamp > header.timestamp, block_validate_exception, "block must be later in time" );
    EOS_ASSERT( h.previous == id, unlinkable_block_exception, "block must link to current state" );
    auto result = generate_next( h.timestamp );
    EOS_ASSERT( result.header.producer == h.producer, wrong_producer, "wrong producer specified" );
    EOS_ASSERT( result.header.schedule_version == h.schedule_version, producer_schedule_exception, "schedule_version in signed block is corrupted" );

    auto itr = producer_to_last_produced.find(h.producer);
    if( itr != producer_to_last_produced.end() ) {
       EOS_ASSERT( itr->second < result.block_num - h.confirmed, producer_double_confirm, "producer ${prod} double-confirming known range", ("prod", h.producer) );
    }

//fc_断言（result.header.block_mroot==h.block_mroot，“mismatch block merkle root”）；

///在这一点下，状态更改不能单独用头进行验证，但决不能少于头，
///必须导致头状态更改

    result.set_confirmed( h.confirmed );

    auto was_pending_promoted = result.maybe_promote_pending();

    if( h.new_producers ) {
      EOS_ASSERT( !was_pending_promoted, producer_schedule_exception, "cannot set pending producer schedule in the same block in which pending was promoted to active" );
      result.set_new_producers( *h.new_producers );
    }

    result.header.action_mroot       = h.action_mroot;
    result.header.transaction_mroot  = h.transaction_mroot;
    result.header.producer_signature = h.producer_signature;
    result.id                        = result.header.id();

//来自控制器的假设_impl:：apply _block=所有不受信任的块将在此处预先验证其签名
    if( !skip_validate_signee ) {
       result.verify_signee( result.signee() );
    }

    return result;
} //下一个

  void block_header_state::set_confirmed( uint16_t num_prev_blocks ) {
     /*
     idump（（num_prev_blocks）（confirm_count.size（））；

     对于（uint32_t i=0；i<confirm_count.size（）；++i）
        std：：cerr<“confirm_count[”<<i<“]=”<<int（confirm_count[i]）<<\n“；
     }
     **/

     header.confirmed = num_prev_blocks;

     int32_t i = (int32_t)(confirm_count.size() - 1);
uint32_t blocks_to_confirm = num_prev_blocks + 1; ///也确认头块
     while( i >= 0 && blocks_to_confirm ) {
        --confirm_count[i];
//idump（（确认计数[i]）；
        if( confirm_count[i] == 0 )
        {
           uint32_t block_num_for_i = block_num - (uint32_t)(confirm_count.size() - 1 - i);
           dpos_proposed_irreversible_blocknum = block_num_for_i;
//idump（（dpos2_lib）（block_num）（dpos_un不可逆转的_block num））；

           if (i == confirm_count.size() - 1) {
              confirm_count.resize(0);
           } else {
              memmove( &confirm_count[0], &confirm_count[i + 1], confirm_count.size() - i  - 1);
              confirm_count.resize( confirm_count.size() - i - 1 );
           }

           return;
        }
        --i;
        --blocks_to_confirm;
     }
  }

  digest_type   block_header_state::sig_digest()const {
     auto header_bmroot = digest_type::hash( std::make_pair( header.digest(), blockroot_merkle.get_root() ) );
     return digest_type::hash( std::make_pair(header_bmroot, pending_schedule_hash) );
  }

  void block_header_state::sign( const std::function<signature_type(const digest_type&)>& signer ) {
     auto d = sig_digest();
     header.producer_signature = signer( d );
     EOS_ASSERT( block_signing_key == fc::crypto::public_key( header.producer_signature, d ), wrong_signing_key, "block is signed with unexpected key" );
  }

  public_key_type block_header_state::signee()const {
    return fc::crypto::public_key( header.producer_signature, sig_digest(), true );
  }

  void block_header_state::verify_signee( const public_key_type& signee )const {
     EOS_ASSERT( block_signing_key == signee, wrong_signing_key, "block not signed by expected key",
                 ("block_signing_key", block_signing_key)( "signee", signee ) );
  }

  void block_header_state::add_confirmation( const header_confirmation& conf ) {
     for( const auto& c : confirmations )
        EOS_ASSERT( c.producer != conf.producer, producer_double_confirm, "block already confirmed by this producer" );

     auto key = active_schedule.get_producer_key( conf.producer );
     EOS_ASSERT( key != public_key_type(), producer_not_in_schedule, "producer not in current schedule" );
     auto signer = fc::crypto::public_key( conf.producer_signature, sig_digest(), true );
     EOS_ASSERT( signer == key, wrong_signing_key, "confirmation not signed by expected key" );

     confirmations.emplace_back( conf );
  }


} } ///namespace eosio：：链
