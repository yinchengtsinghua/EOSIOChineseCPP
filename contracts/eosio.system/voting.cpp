
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

#include "eosio.system.hpp"

#include <eosiolib/eosio.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/print.hpp>
#include <eosiolib/datastream.hpp>
#include <eosiolib/serialize.hpp>
#include <eosiolib/multi_index.hpp>
#include <eosiolib/privileged.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/transaction.hpp>
#include <eosio.token/eosio.token.hpp>

#include <algorithm>
#include <cmath>

namespace eosiosystem {
   using eosio::indexed_by;
   using eosio::const_mem_fun;
   using eosio::bytes;
   using eosio::print;
   using eosio::singleton;
   using eosio::transaction;

   /*
    *此方法将为“producer”创建一个producer_config和producer_info对象。
    *
    *@pre-producer尚未注册
    *@pre-producer要注册的是一个帐户
    *@生产商注册的预先授权
    *
    **/

   void system_contract::regproducer( const account_name producer, const eosio::public_key& producer_key, const std::string& url, uint16_t location ) {
      eosio_assert( url.size() < 512, "url too long" );
      eosio_assert( producer_key != eosio::public_key(), "public key should not be the default value" );
      require_auth( producer );

      auto prod = _producers.find( producer );

      if ( prod != _producers.end() ) {
         _producers.modify( prod, producer, [&]( producer_info& info ){
               info.producer_key = producer_key;
               info.is_active    = true;
               info.url          = url;
               info.location     = location;
            });
      } else {
         _producers.emplace( producer, [&]( producer_info& info ){
               info.owner         = producer;
               info.total_votes   = 0;
               info.producer_key  = producer_key;
               info.is_active     = true;
               info.url           = url;
               info.location      = location;
         });
      }
   }

   void system_contract::unregprod( const account_name producer ) {
      require_auth( producer );

      const auto& prod = _producers.get( producer, "producer not found" );

      _producers.modify( prod, 0, [&]( producer_info& info ){
            info.deactivate();
      });
   }

   void system_contract::update_elected_producers( block_timestamp block_time ) {
      _gstate.last_producer_schedule_update = block_time;

      auto idx = _producers.get_index<N(prototalvote)>();

      std::vector< std::pair<eosio::producer_key,uint16_t> > top_producers;
      top_producers.reserve(21);

      for ( auto it = idx.cbegin(); it != idx.cend() && top_producers.size() < 21 && 0 < it->total_votes && it->active(); ++it ) {
         top_producers.emplace_back( std::pair<eosio::producer_key,uint16_t>({{it->owner, it->producer_key}, it->location}) );
      }

      if ( top_producers.size() < _gstate.last_producer_schedule_size ) {
         return;
      }

///按生产商名称排序
      std::sort( top_producers.begin(), top_producers.end() );

      std::vector<eosio::producer_key> producers;

      producers.reserve(top_producers.size());
      for( const auto& item : top_producers )
         producers.push_back(item.first);

      bytes packed_schedule = pack(producers);

      if( set_proposed_producers( packed_schedule.data(),  packed_schedule.size() ) >= 0 ) {
         _gstate.last_producer_schedule_size = static_cast<decltype(_gstate.last_producer_schedule_size)>( top_producers.size() );
      }
   }

   double stake2vote( int64_t staked ) {
///todo subtract 2080使大数更接近这十年。
      double weight = int64_t( (now() - (block_timestamp::block_timestamp_epoch / 1000)) / (seconds_per_day * 7) )  / double( 52 );
      return double(staked) * std::pow( 2, weight );
   }
   /*
    *@pre-producer必须从最低到最高排序，并且必须注册并激活
    *@pre如果设置了代理，则不能为任何生产商投票。
    *@pre如果设置了代理，则代理帐户必须存在并注册为代理
    *@pre所有列出的生产商或代理必须事先注册。
    *@pre-voter必须授权此操作
    *Pre-Voter之前一定已经为投票设定了一些EOS。
    *@pre-voter->staked必须是最新的
    *
    *Post之前投票的每一个制片人都会因之前的投票权而减少投票。
    *Post每一位新投票的制作人都会增加投票量。
    *@post previous proxy将由先前的投票权减掉的投票权代理
    *@post new proxy将代理“投票权”增加新的投票权
    *
    *如果对代理投票，在代理更新自己的投票之前，制作人的投票不会改变。
    **/

   void system_contract::voteproducer( const account_name voter_name, const account_name proxy, const std::vector<account_name>& producers ) {
      require_auth( voter_name );
      update_votes( voter_name, proxy, producers, true );
   }

   void system_contract::update_votes( const account_name voter_name, const account_name proxy, const std::vector<account_name>& producers, bool voting ) {
//验证输入
      if ( proxy ) {
         eosio_assert( producers.size() == 0, "cannot vote for producers and proxy at same time" );
         eosio_assert( voter_name != proxy, "cannot proxy to self" );
         require_recipient( proxy );
      } else {
         eosio_assert( producers.size() <= 30, "attempt to vote for too many producers" );
         for( size_t i = 1; i < producers.size(); ++i ) {
            eosio_assert( producers[i-1] < producers[i], "producer votes must be unique and sorted" );
         }
      }

      auto voter = _voters.find(voter_name);
eosio_assert( voter != _voters.end(), "user must stake before they can vote" ); ///staking创建投票对象
      eosio_assert( !proxy || !voter->is_proxy, "account registered as a proxy is not allowed to use a proxy" );

      /*
       *第一次有人投票时，我们计算并设置最后一个投票权重，因为他们在
       *在激活的总权益达到阈值后，我们可以使用最后的投票权来确定这是
       *他们的第一次投票，应该考虑他们的股份被激活。
       **/

      if( voter->last_vote_weight <= 0.0 ) {
         _gstate.total_activated_stake += voter->staked;
         if( _gstate.total_activated_stake >= min_activated_stake && _gstate.thresh_activated_stake_time == 0 ) {
            _gstate.thresh_activated_stake_time = current_time();
         }
      }

      auto new_vote_weight = stake2vote( voter->staked );
      if( voter->is_proxy ) {
         new_vote_weight += voter->proxied_vote_weight;
      }

      /*st:：container:：flat_map<account_name，pair<double，bool/*new*/>>producer_deltas；
      如果（投票人->最后一票权重>0）
         如果（投票人->代理人）
            auto old_proxy=_voters.find（投票人->代理）；
            eosio断言（旧的代理服务器！=\u voters.end（），“找不到旧代理”）；//数据损坏
            _voters.modify（旧_proxy，0，[&]（auto&vp）
                  副总裁：代理投票权=投票人->最后投票权；
               （}）；
            传播权重更改（*old_proxy）；
         }否则{
            对于（const auto&p:voter->producers）
               auto&d=生产商_deltas[p]；
               d.第一个-=投票人->最后一个投票权；
               d.秒=假；
            }
         }
      }

      如果（代理）{
         auto new_proxy=_voters.find（代理）；
         eosio断言（新的\代理！=\u voters.end（），“指定的代理无效”；//if（！投票）数据损坏其他错误投票
         EOSIOO断言（！）投票新_proxy->is_proxy，“proxy not found”）；
         如果（新投票权重>=0）
            _投票者。修改（新_代理，0，[&]（自动&vp）
                  副总裁：代理投票权=新投票权；
               （}）；
            传播权重更改（*new_proxy）；
         }
      }否则{
         如果（新投票权重>=0）
            对于（const auto&p:生产者）
               auto&d=生产商_deltas[p]；
               d.first+=新的投票权；
               d.秒=真；
            }
         }
      }

      对于（const auto&pd：生产商_deltas）
         auto pitr=_producers.find（pd.first）；
         如果（皮特！=_producers.end（））
            EOSIOO断言（！）表决pitr->active（）！pd.second.second/*不来自新集合*/, "producer is not currently registered" );

            _producers.modify( pitr, 0, [&]( auto& p ) {
               p.total_votes += pd.second.first;
if ( p.total_votes < 0 ) { //浮点运算可以给出较小的负数。
                  p.total_votes = 0;
               }
               _gstate.total_producer_vote_weight += pd.second.first;
//eosio_assert（p.total_投票数>=0，“发生了一些不好的事情”）；
            });
         } else {
            /*IOYAsReSt（！）pd.second.second/*不来自新集合*/，“生产者未注册”）；//数据损坏
         }
      }

      _投票者。修改（投票者，0，[&]（自动和AV）
         av.last-vote-weight=新的投票权；
         av.producers=制作人；
         av.proxy=代理；
      （}）；
   }

   /**
    *标记为代理人的账户可与其他账户的权重投票。
    *已选择它作为代理。其他帐户必须将其voteProducer刷新为
    *更新代理的权重。
    *
    *@param isproxy-如果代理希望代表其他人投票，则为true，否则为false
    *@pre-proxy必须有一些赌注（选民表中的现有行）
    *@pre new state必须不同于当前状态
    **/

   void system_contract::regproxy( const account_name proxy, bool isproxy ) {
      require_auth( proxy );

      auto pitr = _voters.find(proxy);
      if ( pitr != _voters.end() ) {
         eosio_assert( isproxy != pitr->is_proxy, "action has no effect" );
         eosio_assert( !isproxy || !pitr->proxy, "account that uses a proxy is not allowed to become a proxy" );
         _voters.modify( pitr, 0, [&]( auto& p ) {
               p.is_proxy = isproxy;
            });
         propagate_weight_change( *pitr );
      } else {
         _voters.emplace( proxy, [&]( auto& p ) {
               p.owner  = proxy;
               p.is_proxy = isproxy;
            });
      }
   }

   void system_contract::propagate_weight_change( const voter_info& voter ) {
      eosio_assert( voter.proxy == 0 || !voter.is_proxy, "account registered as a proxy is not allowed to use a proxy" );
      double new_weight = stake2vote( voter.staked );
      if ( voter.is_proxy ) {
         new_weight += voter.proxied_vote_weight;
      }

///不要传播小的变化（1~=epsilon）
      if ( fabs( new_weight - voter.last_vote_weight ) > 1 )  {
         if ( voter.proxy ) {
auto& proxy = _voters.get( voter.proxy, "proxy not found" ); //数据损坏
            _voters.modify( proxy, 0, [&]( auto& p ) {
                  p.proxied_vote_weight += new_weight - voter.last_vote_weight;
               }
            );
            propagate_weight_change( proxy );
         } else {
            auto delta = new_weight - voter.last_vote_weight;
            for ( auto acnt : voter.producers ) {
auto& pitr = _producers.get( acnt, "producer not found" ); //数据损坏
               _producers.modify( pitr, 0, [&]( auto& p ) {
                     p.total_votes += delta;
                     _gstate.total_producer_vote_weight += delta;
               });
            }
         }
      }
      _voters.modify( voter, 0, [&]( auto& v ) {
            v.last_vote_weight = new_weight;
         }
      );
   }

} ///namespace eosios系统
