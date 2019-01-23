
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <boost/test/unit_test.hpp>
#include <eosio/testing/tester.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/chain/fork_database.hpp>

#include <eosio.token/eosio.token.wast.hpp>
#include <eosio.token/eosio.token.abi.hpp>

#include <Runtime/Runtime.h>

#include <fc/variant_object.hpp>

using namespace eosio::chain;
using namespace eosio::testing;

private_key_type get_private_key( name keyname, string role ) {
   return private_key_type::regenerate<fc::ecc::private_key_shim>(fc::sha256::hash(string(keyname)+role));
}

public_key_type  get_public_key( name keyname, string role ){
   return get_private_key( keyname, role ).get_public_key();
}

void push_blocks( tester& from, tester& to ) {
   while( to.control->fork_db_head_block_num() < from.control->fork_db_head_block_num() ) {
      auto fb = from.control->fetch_block_by_number( to.control->fork_db_head_block_num()+1 );
      to.push_block( fb );
   }
}

BOOST_AUTO_TEST_SUITE(forked_tests)

BOOST_AUTO_TEST_CASE( irrblock ) try {
   tester c;
   c.produce_blocks(10);
   auto r = c.create_accounts( {N(dan),N(sam),N(pam),N(scott)} );
   auto res = c.set_producers( {N(dan),N(sam),N(pam),N(scott)} );
   vector<producer_key> sch = { {N(dan),get_public_key(N(dan), "active")},
                                {N(sam),get_public_key(N(sam), "active")},
                                {N(scott),get_public_key(N(scott), "active")},
                                {N(pam),get_public_key(N(pam), "active")}
                              };
   wlog("set producer schedule to [dan,sam,pam]");
   c.produce_blocks(50);

} FC_LOG_AND_RETHROW() 

struct fork_tracker {
   vector<signed_block_ptr> blocks;
   incremental_merkle       block_merkle;
};

BOOST_AUTO_TEST_CASE( fork_with_bad_block ) try {
   tester bios;
   bios.produce_block();
   bios.produce_block();
   bios.create_accounts( {N(a),N(b),N(c),N(d),N(e)} );

   bios.produce_block();
   auto res = bios.set_producers( {N(a),N(b),N(c),N(d),N(e)} );

//运行到生产商安装完毕并开始“A”轮
   while( bios.control->pending_block_state()->header.producer.to_string() != "a" || bios.control->head_block_state()->header.producer.to_string() != "e") {
      bios.produce_block();
   }

//同步远程节点
   tester remote;
   push_blocks(bios, remote);

//在BIOS上生成6个块
   for (int i = 0; i < 6; i ++) {
      bios.produce_block();
      BOOST_REQUIRE_EQUAL( bios.control->head_block_state()->header.producer.to_string(), "a" );
   }

   vector<fork_tracker> forks(7);
//足以跳过A的街区
   auto offset = fc::milliseconds(config::block_interval_ms * 13);

//在遥控器上跳过A的块
//创建7个数据块的7个数据块叉，以便在第i个数据块损坏的情况下，此数据块叉更长。
   for (size_t i = 0; i < 7; i ++) {
      auto b = remote.produce_block(offset);
      BOOST_REQUIRE_EQUAL( b->producer.to_string(), "b" );

      for (size_t j = 0; j < 7; j ++) {
         auto& fork = forks.at(j);

         if (j <= i) {
            auto copy_b = std::make_shared<signed_block>(b->clone());
            if (j == i) {
//损坏此块
               fork.block_merkle = remote.control->head_block_state()->blockroot_merkle;
               copy_b->action_mroot._hash[0] ^= 0x1ULL;
            } else if (j < i) {
//链接到损坏的链
               copy_b->previous = fork.blocks.back()->id();
            }

//重新签署区块
            auto header_bmroot = digest_type::hash( std::make_pair( copy_b->digest(), fork.block_merkle.get_root() ) );
            auto sig_digest = digest_type::hash( std::make_pair(header_bmroot, remote.control->head_block_state()->pending_schedule_hash) );
            copy_b->producer_signature = remote.get_private_key(N(b), "active").sign(sig_digest);

//将此新块添加到损坏的块merkle中
            fork.block_merkle.append(copy_b->id());
            fork.blocks.emplace_back(copy_b);
         } else {
            fork.blocks.emplace_back(b);
         }
      }

      offset = fc::milliseconds(config::block_interval_ms);
   }

//从最坏的分叉到最坏的分叉
   for (size_t i = 0; i < forks.size(); i++) {
      BOOST_TEST_CONTEXT("Testing Fork: " << i) {
         const auto& fork = forks.at(i);
//将叉推到原始节点
         for (int fidx = 0; fidx < fork.blocks.size() - 1; fidx++) {
            const auto& b = fork.blocks.at(fidx);
//仅当块尚不知道时才推块
            if (!bios.control->fetch_block_by_id(b->id())) {
               bios.push_block(b);
            }
         }

//推送应尝试损坏的分叉并失败的块
         BOOST_REQUIRE_THROW(bios.push_block(fork.blocks.back()), fc::exception);
      }
   }

//确保在不可逆性移动之前，我们仍然可以生成一个块。
   auto lib = bios.control->head_block_state()->dpos_irreversible_blocknum;
   size_t tries = 0;
   while (bios.control->head_block_state()->dpos_irreversible_blocknum == lib && ++tries < 10000) {
      bios.produce_block();
   }

} FC_LOG_AND_RETHROW();

BOOST_AUTO_TEST_CASE( forking ) try {
   tester c;
   c.produce_block();
   c.produce_block();
   auto r = c.create_accounts( {N(dan),N(sam),N(pam)} );
   wdump((fc::json::to_pretty_string(r)));
   c.produce_block();
   auto res = c.set_producers( {N(dan),N(sam),N(pam)} );
   vector<producer_key> sch = { {N(dan),get_public_key(N(dan), "active")},
                                {N(sam),get_public_key(N(sam), "active")},
                                {N(pam),get_public_key(N(pam), "active")}};
   wdump((fc::json::to_pretty_string(res)));
   wlog("set producer schedule to [dan,sam,pam]");
   c.produce_blocks(30);

   auto r2 = c.create_accounts( {N(eosio.token)} );
   wdump((fc::json::to_pretty_string(r2)));
   c.set_code( N(eosio.token), eosio_token_wast );
   c.set_abi( N(eosio.token), eosio_token_abi );
   c.produce_blocks(10);


   auto cr = c.push_action( N(eosio.token), N(create), N(eosio.token), mutable_variant_object()
              ("issuer",       "eosio" )
              ("maximum_supply", core_from_string("10000000.0000"))
      );

   wdump((fc::json::to_pretty_string(cr)));

   cr = c.push_action( N(eosio.token), N(issue), config::system_account_name, mutable_variant_object()
              ("to",       "dan" )
              ("quantity", core_from_string("100.0000"))
              ("memo", "")
      );

   wdump((fc::json::to_pretty_string(cr)));


   tester c2;
   wlog( "push c1 blocks to c2" );
   push_blocks(c, c2);
   wlog( "end push c1 blocks to c2" );

   wlog( "c1 blocks:" );
   c.produce_blocks(3);
   signed_block_ptr b;
   b = c.produce_block();
   account_name expected_producer = N(dan);
   BOOST_REQUIRE_EQUAL( b->producer.to_string(), expected_producer.to_string() );

   b = c.produce_block();
   expected_producer = N(sam);
   BOOST_REQUIRE_EQUAL( b->producer.to_string(), expected_producer.to_string() );
   c.produce_blocks(10);
   c.create_accounts( {N(cam)} );
   c.set_producers( {N(dan),N(sam),N(pam),N(cam)} );
   wlog("set producer schedule to [dan,sam,pam,cam]");
   c.produce_block();
//下一个块应该由PAM生成。

//同步第二链和第一链。
   wlog( "push c1 blocks to c2" );
   push_blocks(c, c2);
   wlog( "end push c1 blocks to c2" );

//现在萨姆和帕姆自食其力，丹自己动手制作积木。

   wlog( "sam and pam go off on their own fork on c2 while dan produces blocks by himself in c1" );
   auto fork_block_num = c.control->head_block_num();

   wlog( "c2 blocks:" );
c2.produce_blocks(12); //PAM生成12个块
b = c2.produce_block( fc::milliseconds(config::block_interval_ms * 13) ); //山姆跳过丹的街区
   expected_producer = N(sam);
   BOOST_REQUIRE_EQUAL( b->producer.to_string(), expected_producer.to_string() );
   c2.produce_blocks(11 + 12);


   wlog( "c1 blocks:" );
b = c.produce_block( fc::milliseconds(config::block_interval_ms * 13) ); //丹跳过帕姆的街区
   expected_producer = N(dan);
   BOOST_REQUIRE_EQUAL( b->producer.to_string(), expected_producer.to_string() );
   c.produce_blocks(11);

//链1上的dan现在从链2上得到所有块，这将导致拨叉开关。
   wlog( "push c2 blocks to c1" );
   for( uint32_t start = fork_block_num + 1, end = c2.control->head_block_num(); start <= end; ++start ) {
      wdump((start));
      auto fb = c2.control->fetch_block_by_number( start );
      c.push_block( fb );
   }
   wlog( "end push c2 blocks to c1" );

   wlog( "c1 blocks:" );
   c.produce_blocks(24);

b = c.produce_block(); //将活动计划切换到版本2将在此块中发生。
   expected_producer = N(pam);
   BOOST_REQUIRE_EQUAL( b->producer.to_string(), expected_producer.to_string() );

   b = c.produce_block();
   expected_producer = N(cam);
//boost_需要_equal（b->producer.to_string（），应为_producer.to_string（））；
   c.produce_blocks(10);

   wlog( "push c1 blocks to c2" );
   push_blocks(c, c2);
   wlog( "end push c1 blocks to c2" );

//现在，四个区块生产商活跃，两个相同的链（目前）
//我们可以测试在旧fork db代码中触发bug的情况：
   fork_block_num = c.control->head_block_num();
   wlog( "cam and dan go off on their own fork on c1 while sam and pam go off on their own fork on c2" );
   wlog( "c1 blocks:" );
c.produce_blocks(12); //丹生产12块
c.produce_block( fc::milliseconds(config::block_interval_ms * 25) ); //卡姆跳过萨姆和帕姆的街区。
c.produce_blocks(23); //Cam完成剩下的11个街区，然后Dan生成他的12个街区。
   wlog( "c2 blocks:" );
c2.produce_block( fc::milliseconds(config::block_interval_ms * 25) ); //帕姆跳过丹和萨姆的街区。
c2.produce_blocks(11); //帕姆完成剩下的11个街区
c2.produce_block( fc::milliseconds(config::block_interval_ms * 25) ); //萨姆跳过卡姆和丹的街区。
c2.produce_blocks(11); //萨姆完成剩下的11个街区

   wlog( "now cam and dan rejoin sam and pam on c2" );
c2.produce_block( fc::milliseconds(config::block_interval_ms * 13) ); //cam跳过pam的块（此块触发此分支上的块，使其不可逆）
c2.produce_blocks(11); //CAM生成剩余的11个块
b = c2.produce_block(); //丹制作一个方块

//链1上的一个节点现在从链2获取除最后一个块以外的所有块，这将导致分叉开关
   wlog( "push c2 blocks (except for the last block by dan) to c1" );
   for( uint32_t start = fork_block_num + 1, end = c2.control->head_block_num() - 1; start <= end; ++start ) {
      auto fb = c2.control->fetch_block_by_number( start );
      c.push_block( fb );
   }
   wlog( "end push c2 blocks to c1" );
   wlog( "now push dan's block to c1 but first corrupt it so it is a bad block" );
   signed_block bad_block = std::move(*b);
   bad_block.transaction_mroot = bad_block.previous;
   auto bad_block_bs = c.control->create_block_state_future( std::make_shared<signed_block>(std::move(bad_block)) );
   c.control->abort_block();
   BOOST_REQUIRE_EXCEPTION(c.control->push_block( bad_block_bs ), fc::exception,
      [] (const fc::exception &ex)->bool {
         return ex.to_detail_string().find("block not signed by expected key") != std::string::npos;
      });
} FC_LOG_AND_RETHROW()


/*
 *此测试验证fork choice规则是否支持
 *最后一个不可逆块的最大值超过一个较长的块。
 **/

BOOST_AUTO_TEST_CASE( prune_remove_branch ) try {
   tester c;
   c.produce_blocks(10);
   auto r = c.create_accounts( {N(dan),N(sam),N(pam),N(scott)} );
   auto res = c.set_producers( {N(dan),N(sam),N(pam),N(scott)} );
   wlog("set producer schedule to [dan,sam,pam,scott]");
   c.produce_blocks(50);

   tester c2;
   wlog( "push c1 blocks to c2" );
   push_blocks(c, c2);

//货叉发生在61号区块之后
   BOOST_REQUIRE_EQUAL(61, c.control->head_block_num());
   BOOST_REQUIRE_EQUAL(61, c2.control->head_block_num());

   int fork_num = c.control->head_block_num();

   auto nextproducer = [](tester &c, int skip_interval) ->account_name {
      auto head_time = c.control->head_block_time();
      auto next_time = head_time + fc::milliseconds(config::block_interval_ms * skip_interval);
      return c.control->head_block_state()->get_scheduled_producer(next_time).producer_name;   
   };

//福克C:2制片人：丹，山姆
//叉子C2:1制片人：斯科特
   int skip1 = 1, skip2 = 1;
   for (int i = 0; i < 50; ++i) {
      account_name next1 = nextproducer(c, skip1);
      if (next1 == N(dan) || next1 == N(sam)) {
         c.produce_block(fc::milliseconds(config::block_interval_ms * skip1)); skip1 = 1;
      } 
      else ++skip1;
      account_name next2 = nextproducer(c2, skip2);
      if (next2 == N(scott)) {
         c2.produce_block(fc::milliseconds(config::block_interval_ms * skip2)); skip2 = 1;
      } 
      else ++skip2;
   }

   BOOST_REQUIRE_EQUAL(87, c.control->head_block_num());
   BOOST_REQUIRE_EQUAL(73, c2.control->head_block_num());
   
//来自C2的推叉=>C
   int p = fork_num;
   while ( p < c2.control->head_block_num()) {
      auto fb = c2.control->fetch_block_by_number(++p);
      c.push_block(fb);
   }

   BOOST_REQUIRE_EQUAL(73, c.control->head_block_num());

} FC_LOG_AND_RETHROW() 


BOOST_AUTO_TEST_CASE( read_modes ) try {
   tester c;
   c.produce_block();
   c.produce_block();
   auto r = c.create_accounts( {N(dan),N(sam),N(pam)} );
   c.produce_block();
   auto res = c.set_producers( {N(dan),N(sam),N(pam)} );
   c.produce_blocks(200);
   auto head_block_num = c.control->head_block_num();

   tester head(true, db_read_mode::HEAD);
   push_blocks(c, head);
   BOOST_REQUIRE_EQUAL(head_block_num, head.control->fork_db_head_block_num());
   BOOST_REQUIRE_EQUAL(head_block_num, head.control->head_block_num());

   tester read_only(false, db_read_mode::READ_ONLY);
   push_blocks(c, read_only);
   BOOST_REQUIRE_EQUAL(head_block_num, read_only.control->fork_db_head_block_num());
   BOOST_REQUIRE_EQUAL(head_block_num, read_only.control->head_block_num());

   tester irreversible(true, db_read_mode::IRREVERSIBLE);
   push_blocks(c, irreversible);
   BOOST_REQUIRE_EQUAL(head_block_num, irreversible.control->fork_db_head_block_num());
   BOOST_REQUIRE_EQUAL(head_block_num - 49, irreversible.control->head_block_num());

} FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()
