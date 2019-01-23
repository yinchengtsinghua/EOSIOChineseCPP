
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <boost/test/unit_test.hpp>
#include <eosio/testing/tester.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <boost/range/algorithm.hpp>

#ifdef NON_VALIDATING_TEST
#define TESTER tester
#else
#define TESTER validating_tester
#endif

using namespace eosio::testing;
using namespace eosio::chain;
using mvo = fc::mutable_variant_object;

BOOST_AUTO_TEST_SUITE(producer_schedule_tests)

//根据时间表和槽号计算预期生产商
   account_name get_expected_producer(const vector<producer_key>& schedule, const uint64_t slot) {
      const auto& index = (slot % (schedule.size() * config::producer_repetitions)) / config::producer_repetitions;
      return schedule.at(index).producer_name;
   };

//检查两个计划是否相等
   bool is_schedule_equal(const vector<producer_key>& first, const vector<producer_key>& second) {
      bool is_equal = first.size() == second.size();
      for (uint32_t i = 0; i < first.size(); i++) {
         is_equal = is_equal && first.at(i) == second.at(i);
      }
      return is_equal;
   };

//计算下一轮第一个区块的区块数
//新的生产商时间表将在下一轮第一轮计划中生效。
//但是，在有效块数被认为是不可逆的之前，它不会被应用。
   uint64_t calc_block_num_of_next_round_first_block(const controller& control){
      auto res = control.head_block_num() + 1;
      const auto blocks_per_round = control.head_block_state()->active_schedule.producers.size() * config::producer_repetitions;
      while((res % blocks_per_round) != 0) {
         res++;
      }
      return res;
   };
#if 0
   BOOST_FIXTURE_TEST_CASE( verify_producer_schedule, TESTER ) try {

//确保生产商计划按预期工作的实用功能
      const auto& confirm_schedule_correctness = [&](const vector<producer_key>& new_prod_schd, const uint64_t eff_new_prod_schd_block_num)  {
const uint32_t check_duration = 1000; //块数
         for (uint32_t i = 0; i < check_duration; ++i) {
            const auto current_schedule = control->head_block_state()->active_schedule.producers;
            const auto& current_absolute_slot = control->get_global_properties().proposed_schedule_block_num;
//确定预期生产商
            const auto& expected_producer = get_expected_producer(current_schedule, *current_absolute_slot + 1);

//只有当有效数据块编号被视为不可逆时，新计划才会应用。
            const bool is_new_schedule_applied = control->last_irreversible_block_num() > eff_new_prod_schd_block_num;

//确保我们在正确的时间有正确的时间表
            if (is_new_schedule_applied) {
               BOOST_TEST(is_schedule_equal(new_prod_schd, current_schedule));
            } else {
               BOOST_TEST(!is_schedule_equal(new_prod_schd, current_schedule));
            }

//生产块
            produce_block();

//检查生产商是否与我们期望的相同
            BOOST_TEST(control->head_block_producer() == expected_producer);
         }
      };

//创建生产商帐户
      vector<account_name> producers = {
              "inita", "initb", "initc", "initd", "inite", "initf", "initg",
              "inith", "initi", "initj", "initk", "initl", "initm", "initn",
              "inito", "initp", "initq", "initr", "inits", "initt", "initu"
      };
      create_accounts(producers);

//----测试第一组生产商----
//发送“设置产品”操作并确认计划正确性
      set_producers(producers);
      const auto first_prod_schd = get_producer_keys(producers);
      const auto eff_first_prod_schd_block_num = calc_block_num_of_next_round_first_block(*control);
      confirm_schedule_correctness(first_prod_schd, eff_first_prod_schd_block_num);

//----测试第二组生产商----
      vector<account_name> second_set_of_producer = {
              producers[3], producers[6], producers[9], producers[12], producers[15], producers[18], producers[20]
      };
//发送“设置产品”操作并确认计划正确性
      set_producers(second_set_of_producer);
      const auto second_prod_schd = get_producer_keys(second_set_of_producer);
      const auto& eff_second_prod_schd_block_num = calc_block_num_of_next_round_first_block(*control);
      confirm_schedule_correctness(second_prod_schd, eff_second_prod_schd_block_num);

//----故意漏掉一些块----
      const int64_t num_of_missed_blocks = 5000;
      produce_block(fc::microseconds(500 * 1000 * num_of_missed_blocks));
//确保时间表仍然正确
      confirm_schedule_correctness(second_prod_schd, eff_second_prod_schd_block_num);
      produce_block();

//——测试第三套生产商----
      vector<account_name> third_set_of_producer = {
              producers[2], producers[5], producers[8], producers[11], producers[14], producers[17], producers[20],
              producers[0], producers[3], producers[6], producers[9], producers[12], producers[15], producers[18],
              producers[1], producers[4], producers[7], producers[10], producers[13], producers[16], producers[19]
      };
//发送“设置产品”操作并确认计划正确性
      set_producers(third_set_of_producer);
      const auto third_prod_schd = get_producer_keys(third_set_of_producer);
      const auto& eff_third_prod_schd_block_num = calc_block_num_of_next_round_first_block(*control);
      confirm_schedule_correctness(third_prod_schd, eff_third_prod_schd_block_num);

   } FC_LOG_AND_RETHROW()


   BOOST_FIXTURE_TEST_CASE( verify_producers, TESTER ) try {

      vector<account_name> valid_producers = {
         "inita", "initb", "initc", "initd", "inite", "initf", "initg",
         "inith", "initi", "initj", "initk", "initl", "initm", "initn",
         "inito", "initp", "initq", "initr", "inits", "initt", "initu"
      };
      create_accounts(valid_producers);
      set_producers(valid_producers);

//帐户initz不存在
      vector<account_name> nonexisting_producer = { "initz" };
      BOOST_CHECK_THROW(set_producers(nonexisting_producer), wasm_execution_error);

//用inita替换initg，inita现在重复
      vector<account_name> invalid_producers = {
         "inita", "initb", "initc", "initd", "inite", "initf", "inita",
         "inith", "initi", "initj", "initk", "initl", "initm", "initn",
         "inito", "initp", "initq", "initr", "inits", "initt", "initu"
      };

      BOOST_CHECK_THROW(set_producers(invalid_producers), wasm_execution_error);

   } FC_LOG_AND_RETHROW()

   BOOST_FIXTURE_TEST_CASE( verify_header_schedule_version, TESTER ) try {

//确保标题中的生产者计划版本正确的实用程序函数
      const auto& confirm_header_schd_ver_correctness = [&](const uint64_t expected_version, const uint64_t eff_new_prod_schd_block_num)  {
const uint32_t check_duration = 1000; //块数
         for (uint32_t i = 0; i < check_duration; ++i) {
//只有当有效数据块编号被视为不可逆时，新计划才会应用。
            const bool is_new_schedule_applied = control->last_irreversible_block_num() > eff_new_prod_schd_block_num;

//生产块
            produce_block();

//确保在正确的时间更新头块头段
            const auto current_schd_ver = control->head_block_header().schedule_version;
            if (is_new_schedule_applied) {
               BOOST_TEST(current_schd_ver == expected_version);
            } else {
               BOOST_TEST(current_schd_ver != expected_version);
            }
         }
      };

//创建生产商帐户
      vector<account_name> producers = {
              "inita", "initb", "initc", "initd", "inite", "initf", "initg",
              "inith", "initi", "initj", "initk", "initl", "initm", "initn",
              "inito", "initp", "initq", "initr", "inits", "initt", "initu"
      };
      create_accounts(producers);

//发送“设置产品”操作并确认计划正确性
      set_producers(producers, 1);
      const auto& eff_first_prod_schd_block_num = calc_block_num_of_next_round_first_block(*control);
//确认版本正确
      confirm_header_schd_ver_correctness(1, eff_first_prod_schd_block_num);

//洗牌生产者和设置新的一个较小的版本
      boost::range::random_shuffle(producers);
      set_producers(producers, 0);
      const auto& eff_second_prod_schd_block_num = calc_block_num_of_next_round_first_block(*control);
//即使我们用较小的版本号设置它，版本也应该增加。
      confirm_header_schd_ver_correctness(2, eff_second_prod_schd_block_num);

//洗牌生产者和设置新的一个更大的版本号
      boost::range::random_shuffle(producers);
      set_producers(producers, 1000);
      const auto& eff_third_prod_schd_block_num = calc_block_num_of_next_round_first_block(*control);
//即使我们用很大的版本号设置它，版本也应该增加。
      confirm_header_schd_ver_correctness(3, eff_third_prod_schd_block_num);

   } FC_LOG_AND_RETHROW()
#endif


BOOST_FIXTURE_TEST_CASE( producer_schedule_promotion_test, TESTER ) try {
   create_accounts( {N(alice),N(bob),N(carol)} );
   produce_block();

   auto compare_schedules = [&]( const vector<producer_key>& a, const producer_schedule_type& b ) {
      return std::equal( a.begin(), a.end(), b.producers.begin(), b.producers.end() );
   };

   auto res = set_producers( {N(alice),N(bob)} );
   vector<producer_key> sch1 = {
                                 {N(alice), get_public_key(N(alice), "active")},
                                 {N(bob),   get_public_key(N(bob),   "active")}
                               };
//wdump（（fc:：json:：to_pretty_string（res）））；
   wlog("set producer schedule to [alice,bob]");
   BOOST_REQUIRE_EQUAL( true, control->proposed_producers().valid() );
   BOOST_CHECK_EQUAL( true, compare_schedules( sch1, *control->proposed_producers() ) );
   BOOST_CHECK_EQUAL( control->pending_producers().version, 0 );
produce_block(); //启动新块，将建议的计划提升为挂起
   BOOST_CHECK_EQUAL( control->pending_producers().version, 1 );
   BOOST_CHECK_EQUAL( true, compare_schedules( sch1, control->pending_producers() ) );
   BOOST_CHECK_EQUAL( control->active_producers().version, 0 );
   produce_block();
produce_block(); //启动将挂起计划提升为活动的新块
   BOOST_CHECK_EQUAL( control->active_producers().version, 1 );
   BOOST_CHECK_EQUAL( true, compare_schedules( sch1, control->active_producers() ) );
   produce_blocks(7);

   res = set_producers( {N(alice),N(bob),N(carol)} );
   vector<producer_key> sch2 = {
                                 {N(alice), get_public_key(N(alice), "active")},
                                 {N(bob),   get_public_key(N(bob),   "active")},
                                 {N(carol), get_public_key(N(carol), "active")}
                               };
   wlog("set producer schedule to [alice,bob,carol]");
   BOOST_REQUIRE_EQUAL( true, control->proposed_producers().valid() );
   BOOST_CHECK_EQUAL( true, compare_schedules( sch2, *control->proposed_producers() ) );

   produce_block();
produce_blocks(23); //爱丽丝制作了她第一轮的最后一块。
//Bob的第一个块（将lib前进到Alice的最后一个块）已开始，但尚未完成。
   BOOST_REQUIRE_EQUAL( control->head_block_producer(), N(alice) );
   BOOST_REQUIRE_EQUAL( control->pending_block_state()->header.producer, N(bob) );
   BOOST_CHECK_EQUAL( control->pending_producers().version, 2 );

produce_blocks(12); //鲍勃制作了他的前11个街区
   BOOST_CHECK_EQUAL( control->active_producers().version, 1 );
produce_blocks(12); //鲍勃生产他的第12个街区。
//爱丽丝在第二轮的第一轮比赛中已经开始，但还没有结束（这将把自由党推进到鲍勃的最后一轮）。
   BOOST_REQUIRE_EQUAL( control->head_block_producer(), N(alice) );
   BOOST_REQUIRE_EQUAL( control->pending_block_state()->header.producer, N(bob) );
   BOOST_CHECK_EQUAL( control->active_producers().version, 2 );
   BOOST_CHECK_EQUAL( true, compare_schedules( sch2, control->active_producers() ) );

produce_block(); //爱丽丝制作了她第二轮的第一块，这改变了现行的时间表。

//下一块将根据新的时间表生产。
   produce_block();
BOOST_CHECK_EQUAL( control->head_block_producer(), N(carol) ); //下一块刚好是卡罗尔制作的。

   BOOST_REQUIRE_EQUAL( validate(), true );
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( producer_schedule_reduction, tester ) try {
   create_accounts( {N(alice),N(bob),N(carol)} );
   produce_block();

   auto compare_schedules = [&]( const vector<producer_key>& a, const producer_schedule_type& b ) {
      return std::equal( a.begin(), a.end(), b.producers.begin(), b.producers.end() );
   };

   auto res = set_producers( {N(alice),N(bob),N(carol)} );
   vector<producer_key> sch1 = {
                                 {N(alice), get_public_key(N(alice), "active")},
                                 {N(bob),   get_public_key(N(bob),   "active")},
                                 {N(carol),   get_public_key(N(carol),   "active")}
                               };
   wlog("set producer schedule to [alice,bob,carol]");
   BOOST_REQUIRE_EQUAL( true, control->proposed_producers().valid() );
   BOOST_CHECK_EQUAL( true, compare_schedules( sch1, *control->proposed_producers() ) );
   BOOST_CHECK_EQUAL( control->pending_producers().version, 0 );
produce_block(); //启动新块，将建议的计划提升为挂起
   BOOST_CHECK_EQUAL( control->pending_producers().version, 1 );
   BOOST_CHECK_EQUAL( true, compare_schedules( sch1, control->pending_producers() ) );
   BOOST_CHECK_EQUAL( control->active_producers().version, 0 );
   produce_block();
produce_block(); //启动将挂起计划提升为活动的新块
   BOOST_CHECK_EQUAL( control->active_producers().version, 1 );
   BOOST_CHECK_EQUAL( true, compare_schedules( sch1, control->active_producers() ) );
   produce_blocks(7);

   res = set_producers( {N(alice),N(bob)} );
   vector<producer_key> sch2 = {
                                 {N(alice), get_public_key(N(alice), "active")},
                                 {N(bob),   get_public_key(N(bob),   "active")}
                               };
   wlog("set producer schedule to [alice,bob]");
   BOOST_REQUIRE_EQUAL( true, control->proposed_producers().valid() );
   BOOST_CHECK_EQUAL( true, compare_schedules( sch2, *control->proposed_producers() ) );

   produce_blocks(48);
   BOOST_REQUIRE_EQUAL( control->head_block_producer(), N(bob) );
   BOOST_REQUIRE_EQUAL( control->pending_block_state()->header.producer, N(carol) );
   BOOST_CHECK_EQUAL( control->pending_producers().version, 2 );

   produce_blocks(47);
   BOOST_CHECK_EQUAL( control->active_producers().version, 1 );
   produce_blocks(1);

   BOOST_REQUIRE_EQUAL( control->head_block_producer(), N(carol) );
   BOOST_REQUIRE_EQUAL( control->pending_block_state()->header.producer, N(alice) );
   BOOST_CHECK_EQUAL( control->active_producers().version, 2 );
   BOOST_CHECK_EQUAL( true, compare_schedules( sch2, control->active_producers() ) );

   produce_blocks(2);
   BOOST_CHECK_EQUAL( control->head_block_producer(), N(bob) );

   BOOST_REQUIRE_EQUAL( validate(), true );
} FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE( empty_producer_schedule_has_no_effect, tester ) try {
   create_accounts( {N(alice),N(bob),N(carol)} );
   produce_block();

   auto compare_schedules = [&]( const vector<producer_key>& a, const producer_schedule_type& b ) {
      return std::equal( a.begin(), a.end(), b.producers.begin(), b.producers.end() );
   };

   auto res = set_producers( {N(alice),N(bob)} );
   vector<producer_key> sch1 = {
                                 {N(alice), get_public_key(N(alice), "active")},
                                 {N(bob),   get_public_key(N(bob),   "active")}
                               };
   wlog("set producer schedule to [alice,bob]");
   BOOST_REQUIRE_EQUAL( true, control->proposed_producers().valid() );
   BOOST_CHECK_EQUAL( true, compare_schedules( sch1, *control->proposed_producers() ) );
   BOOST_CHECK_EQUAL( control->pending_producers().producers.size(), 0 );

//启动一个新块，将建议的计划提升为挂起
   produce_block();
   BOOST_CHECK_EQUAL( control->pending_producers().version, 1 );
   BOOST_CHECK_EQUAL( true, compare_schedules( sch1, control->pending_producers() ) );
   BOOST_CHECK_EQUAL( control->active_producers().version, 0 );

//启动一个新块，将挂起的计划提升为活动
   produce_block();
   BOOST_CHECK_EQUAL( control->active_producers().version, 1 );
   BOOST_CHECK_EQUAL( true, compare_schedules( sch1, control->active_producers() ) );
   produce_blocks(7);

   res = set_producers( {} );
   wlog("set producer schedule to []");
   BOOST_REQUIRE_EQUAL( true, control->proposed_producers().valid() );
   BOOST_CHECK_EQUAL( control->proposed_producers()->producers.size(), 0 );
   BOOST_CHECK_EQUAL( control->proposed_producers()->version, 2 );

   produce_blocks(12);
   BOOST_CHECK_EQUAL( control->pending_producers().version, 1 );

//空的生产商计划未从“建议”提升为“挂起”
   produce_block();
   BOOST_CHECK_EQUAL( control->pending_producers().version, 2 );
   BOOST_CHECK_EQUAL( false, control->proposed_producers().valid() );

//但不应将其从挂起提升为活动
   produce_blocks(24);
   BOOST_CHECK_EQUAL( control->active_producers().version, 1 );
   BOOST_CHECK_EQUAL( control->pending_producers().version, 2 );

//设置新的生产商计划仍应使用版本2
   res = set_producers( {N(alice),N(bob),N(carol)} );
   vector<producer_key> sch2 = {
                                 {N(alice), get_public_key(N(alice), "active")},
                                 {N(bob),   get_public_key(N(bob),   "active")},
                                 {N(carol), get_public_key(N(carol), "active")}
                               };
   wlog("set producer schedule to [alice,bob,carol]");
   BOOST_REQUIRE_EQUAL( true, control->proposed_producers().valid() );
   BOOST_CHECK_EQUAL( true, compare_schedules( sch2, *control->proposed_producers() ) );
   BOOST_CHECK_EQUAL( control->proposed_producers()->version, 2 );

//生成足够的块以将建议的计划提升为挂起，因为现有挂起的生产商为零，所以可以执行此操作。
   produce_blocks(24);
   BOOST_CHECK_EQUAL( control->active_producers().version, 1 );
   BOOST_CHECK_EQUAL( control->pending_producers().version, 2 );
   BOOST_CHECK_EQUAL( true, compare_schedules( sch2, control->pending_producers() ) );

//生成足够的块以将挂起的计划提升为活动的
   produce_blocks(24);
   BOOST_CHECK_EQUAL( control->active_producers().version, 2 );
   BOOST_CHECK_EQUAL( true, compare_schedules( sch2, control->active_producers() ) );

   BOOST_REQUIRE_EQUAL( validate(), true );
} FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()
