
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


#include <eosio/testing/tester.hpp>
#include <eosio/chain/global_property_object.hpp>
#include <fc/crypto/digest.hpp>

#include <boost/test/unit_test.hpp>

#ifdef NON_VALIDATING_TEST
#define TESTER tester
#else
#define TESTER validating_tester
#endif


using namespace eosio::chain;
using namespace eosio::testing;
namespace bfs = boost::filesystem;

BOOST_AUTO_TEST_SUITE(database_tests)

//撤消基础结构的简单测试
   BOOST_AUTO_TEST_CASE(undo_test) {
      try {
         TESTER test;

//对于这个单元测试，绕过对状态数据库访问的只读限制，这实际上需要改变数据库以正确地进行测试。
         eosio::chain::database& db = const_cast<eosio::chain::database&>( test.control->db() );

         auto ses = db.start_undo_session(true);

//创建帐户
         db.create<account_object>([](account_object &a) {
            a.name = "billy";
         });

//确保我们可以按名称检索该帐户
         auto ptr = db.find<account_object, by_name, std::string>("billy");
         BOOST_TEST(ptr != nullptr);

//撤消帐户创建
         ses.undo();

//确保我们再也找不到帐户
         ptr = db.find<account_object, by_name, std::string>("billy");
         BOOST_TEST(ptr == nullptr);
      } FC_LOG_AND_RETHROW()
   }

//在数据库上测试取块方法，用ID取块，用编号取块
   BOOST_AUTO_TEST_CASE(get_blocks) {
      try {
         TESTER test;
         vector<block_id_type> block_ids;

         const uint32_t num_of_blocks_to_prod = 200;
//生成200个块并检查它们的ID是否与上面的匹配
         test.produce_blocks(num_of_blocks_to_prod);
         for (uint32_t i = 0; i < num_of_blocks_to_prod; ++i) {
            block_ids.emplace_back(test.control->fetch_block_by_number(i + 1)->id());
            BOOST_TEST(block_header::num_from_id(block_ids.back()) == i + 1);
            BOOST_TEST(test.control->fetch_block_by_number(i + 1)->id() == block_ids.back());
         }

//用于检查预期不可逆块的实用函数
         auto calc_exp_last_irr_block_num = [&](uint32_t head_block_num) -> uint32_t {
            const auto producers_size = test.control->head_block_state()->active_schedule.producers.size();
            const auto max_reversible_rounds = EOS_PERCENT(producers_size, config::percent_100 - config::irreversible_threshold_percent);
            if( max_reversible_rounds == 0) {
               return head_block_num;
            } else {
               const auto current_round = head_block_num / config::producer_repetitions;
               const auto irreversible_round = current_round - max_reversible_rounds;
               return (irreversible_round + 1) * config::producer_repetitions - 1;
            }
         };

//检查最后一个不可逆块号是否设置正确
         const auto expected_last_irreversible_block_number = calc_exp_last_irr_block_num(num_of_blocks_to_prod);
         BOOST_TEST(test.control->head_block_state()->dpos_irreversible_blocknum == expected_last_irreversible_block_number);
//检查是否找不到块201（仅存在20个块）
         BOOST_TEST(test.control->fetch_block_by_number(num_of_blocks_to_prod + 1 + 1) == nullptr);

         const uint32_t next_num_of_blocks_to_prod = 100;
//生成100个块并检查它们的ID是否与上面的匹配
         test.produce_blocks(next_num_of_blocks_to_prod);

         const auto next_expected_last_irreversible_block_number = calc_exp_last_irr_block_num(
                 num_of_blocks_to_prod + next_num_of_blocks_to_prod);
//检查最后一个不可逆的块号是否正确更新
         BOOST_TEST(test.control->head_block_state()->dpos_irreversible_blocknum == next_expected_last_irreversible_block_number);
//检查201区是否现在可以找到
         BOOST_CHECK_NO_THROW(test.control->fetch_block_by_number(num_of_blocks_to_prod + 1));
//检查最新的头块匹配
         BOOST_TEST(test.control->fetch_block_by_number(num_of_blocks_to_prod + next_num_of_blocks_to_prod + 1)->id() ==
                    test.control->head_block_id());
      } FC_LOG_AND_RETHROW()
   }


BOOST_AUTO_TEST_SUITE_END()
