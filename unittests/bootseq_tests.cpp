
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

#include <eosio.system/eosio.system.wast.hpp>
#include <eosio.system/eosio.system.abi.hpp>
//这些合同仍在开发阶段。
#include <eosio.token/eosio.token.wast.hpp>
#include <eosio.token/eosio.token.abi.hpp>
#include <eosio.msig/eosio.msig.wast.hpp>
#include <eosio.msig/eosio.msig.abi.hpp>

#include <Runtime/Runtime.h>

#include <fc/variant_object.hpp>

#ifdef NON_VALIDATING_TEST
#define TESTER tester
#else
#define TESTER validating_tester
#endif


using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;

using mvo = fc::mutable_variant_object;

struct genesis_account {
   account_name aname;
   uint64_t     initial_balance;
};

std::vector<genesis_account> test_genesis( {
  {N(b1),    100'000'000'0000ll},
  {N(whale4), 40'000'000'0000ll},
  {N(whale3), 30'000'000'0000ll},
  {N(whale2), 20'000'000'0000ll},
  {N(proda),      1'000'000'0000ll},
  {N(prodb),      1'000'000'0000ll},
  {N(prodc),      1'000'000'0000ll},
  {N(prodd),      1'000'000'0000ll},
  {N(prode),      1'000'000'0000ll},
  {N(prodf),      1'000'000'0000ll},
  {N(prodg),      1'000'000'0000ll},
  {N(prodh),      1'000'000'0000ll},
  {N(prodi),      1'000'000'0000ll},
  {N(prodj),      1'000'000'0000ll},
  {N(prodk),      1'000'000'0000ll},
  {N(prodl),      1'000'000'0000ll},
  {N(prodm),      1'000'000'0000ll},
  {N(prodn),      1'000'000'0000ll},
  {N(prodo),      1'000'000'0000ll},
  {N(prodp),      1'000'000'0000ll},
  {N(prodq),      1'000'000'0000ll},
  {N(prodr),      1'000'000'0000ll},
  {N(prods),      1'000'000'0000ll},
  {N(prodt),      1'000'000'0000ll},
  {N(produ),      1'000'000'0000ll},
  {N(runnerup1),1'000'000'0000ll},
  {N(runnerup2),1'000'000'0000ll},
  {N(runnerup3),1'000'000'0000ll},
  {N(minow1),        100'0000ll},
  {N(minow2),          1'0000ll},
  {N(minow3),          1'0000ll},
  {N(masses),800'000'000'0000ll}
});

class bootseq_tester : public TESTER {
public:

   fc::variant get_global_state() {
      vector<char> data = get_row_by_account( config::system_account_name, config::system_account_name, N(global), N(global) );
      if (data.empty()) std::cout << "\nData is empty\n" << std::endl;
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant( "eosio_global_state", data, abi_serializer_max_time );

   }

    auto buyram( name payer, name receiver, asset ram ) {
       auto r = base_tester::push_action(config::system_account_name, N(buyram), payer, mvo()
                    ("payer", payer)
                    ("receiver", receiver)
                    ("quant", ram)
                    );
       produce_block();
       return r;
    }

    auto delegate_bandwidth( name from, name receiver, asset net, asset cpu, uint8_t transfer = 1) {
       auto r = base_tester::push_action(config::system_account_name, N(delegatebw), from, mvo()
                    ("from", from )
                    ("receiver", receiver)
                    ("stake_net_quantity", net)
                    ("stake_cpu_quantity", cpu)
                    ("transfer", transfer)
                    );
       produce_block();
       return r;
    }

    void create_currency( name contract, name manager, asset maxsupply, const private_key_type* signer = nullptr ) {
        auto act =  mutable_variant_object()
                ("issuer",       manager )
                ("maximum_supply", maxsupply );

        base_tester::push_action(contract, N(create), contract, act );
    }

    auto issue( name contract, name manager, name to, asset amount ) {
       auto r = base_tester::push_action( contract, N(issue), manager, mutable_variant_object()
                ("to",      to )
                ("quantity", amount )
                ("memo", "")
        );
        produce_block();
        return r;
    }

    auto claim_rewards( name owner ) {
       auto r = base_tester::push_action( config::system_account_name, N(claimrewards), owner, mvo()("owner",  owner ));
       produce_block();
       return r;
    }

    auto set_privileged( name account ) {
       auto r = base_tester::push_action(config::system_account_name, N(setpriv), config::system_account_name,  mvo()("account", account)("is_priv", 1));
       produce_block();
       return r;
    }

    auto register_producer(name producer) {
       auto r = base_tester::push_action(config::system_account_name, N(regproducer), producer, mvo()
                       ("producer",  name(producer))
                       ("producer_key", get_public_key( producer, "active" ) )
                       ("url", "" )
                       ("location", 0 )
                    );
       produce_block();
       return r;
    }


    auto undelegate_bandwidth( name from, name receiver, asset net, asset cpu ) {
       auto r = base_tester::push_action(config::system_account_name, N(undelegatebw), from, mvo()
                    ("from", from )
                    ("receiver", receiver)
                    ("unstake_net_quantity", net)
                    ("unstake_cpu_quantity", cpu)
                    );
       produce_block();
       return r;
    }

    asset get_balance( const account_name& act ) {
         return get_currency_balance(N(eosio.token), symbol(CORE_SYMBOL), act);
    }

    void set_code_abi(const account_name& account, const char* wast, const char* abi, const private_key_type* signer = nullptr) {
       wdump((account));
        set_code(account, wast, signer);
        set_abi(account, abi, signer);
        if (account == config::system_account_name) {
           const auto& accnt = control->db().get<account_object,by_name>( account );
           abi_def abi_definition;
           BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi_definition), true);
           abi_ser.set_abi(abi_definition, abi_serializer_max_time);
        }
        produce_blocks();
    }


    abi_serializer abi_ser;
};

BOOST_AUTO_TEST_SUITE(bootseq_tests)

BOOST_FIXTURE_TEST_CASE( bootseq_test, bootseq_tester ) {
    try {

//创建eosio.msig和eosio.token
        create_accounts({N(eosio.msig), N(eosio.token), N(eosio.ram), N(eosio.ramfee), N(eosio.stake), N(eosio.vpay), N(eosio.bpay), N(eosio.saving) });

//设置以下帐户的代码：
//-eosio（代码：eosio.bios）（已由测试人员构造函数设置）
//-eosio.msig（代码：eosio.msig）
//-eosio.token（代码：eosio.token）
set_code_abi(N(eosio.msig), eosio_msig_wast, eosio_msig_abi);//，&eosio_active_pk）；
set_code_abi(N(eosio.token), eosio_token_wast, eosio_token_abi); //，&eosio_active_pk）；

//为eosio.msig和eosio.token设置特权
        set_privileged(N(eosio.msig));
        set_privileged(N(eosio.token));

//验证eosio.msig和eosio.token是否具有特权
        const auto& eosio_msig_acc = get<account_object, by_name>(N(eosio.msig));
        BOOST_TEST(eosio_msig_acc.privileged == true);
        const auto& eosio_token_acc = get<account_object, by_name>(N(eosio.token));
        BOOST_TEST(eosio_token_acc.privileged == true);


//在eosio.token中创建sys tokens，将其管理器设置为eosio
auto max_supply = core_from_string("10000000000.0000"); ///1X大于1b初始标记
auto initial_supply = core_from_string("1000000000.0000"); ///1X大于1b初始标记
        create_currency(N(eosio.token), config::system_account_name, max_supply);
//向eosio.system发行10亿sys代币的Genesis供应
        issue(N(eosio.token), config::system_account_name, config::system_account_name, initial_supply);

        auto actual = get_balance(config::system_account_name);
        BOOST_REQUIRE_EQUAL(initial_supply, actual);

//创建Genesis帐户
        for( const auto& a : test_genesis ) {
           create_account( a.aname, config::system_account_name );
        }

//将eosio.system设置为eosio
        set_code_abi(config::system_account_name, eosio_system_wast, eosio_system_abi);

//为每个Genesis帐户购买RAM并购买CPU和net
        for( const auto& a : test_genesis ) {
           auto ib = a.initial_balance;
           auto ram = 1000;
           auto net = (ib - ram) / 2;
           auto cpu = ib - net - ram;

           auto r = buyram(config::system_account_name, a.aname, asset(ram));
           BOOST_REQUIRE( !r->except_ptr );

           r = delegate_bandwidth(N(eosio.stake), a.aname, asset(net), asset(cpu));
           BOOST_REQUIRE( !r->except_ptr );
        }

        auto producer_candidates = {
                N(proda), N(prodb), N(prodc), N(prodd), N(prode), N(prodf), N(prodg),
                N(prodh), N(prodi), N(prodj), N(prodk), N(prodl), N(prodm), N(prodn),
                N(prodo), N(prodp), N(prodq), N(prodr), N(prods), N(prodt), N(produ),
                N(runnerup1), N(runnerup2), N(runnerup3)
        };

//注册生产者
        for( auto pro : producer_candidates ) {
           register_producer(pro);
        }

//投票给制片人
        auto votepro = [&]( account_name voter, vector<account_name> producers ) {
          std::sort( producers.begin(), producers.end() );
          base_tester::push_action(config::system_account_name, N(voteproducer), voter, mvo()
                                ("voter",  name(voter))
                                ("proxy", name(0) )
                                ("producers", producers)
                     );
        };
        votepro( N(b1), { N(proda), N(prodb), N(prodc), N(prodd), N(prode), N(prodf), N(prodg),
                           N(prodh), N(prodi), N(prodj), N(prodk), N(prodl), N(prodm), N(prodn),
                           N(prodo), N(prodp), N(prodq), N(prodr), N(prods), N(prodt), N(produ)} );
        votepro( N(whale2), {N(runnerup1), N(runnerup2), N(runnerup3)} );
        votepro( N(whale3), {N(proda), N(prodb), N(prodc), N(prodd), N(prode)} );

//总桩号=b1+Whale2+Whale3桩号=（100000000-1000）+（20000000-1000）+（30000000-1000）
        BOOST_TEST(get_global_state()["total_activated_stake"].as<int64_t>() == 1499999997000);

//由于总激活股份少于150000000，因此不会设置生产商。
produce_blocks_for_n_rounds(2); //2轮，因为新生产商计划是在下一轮的第一个区块不可逆时设置的。
        auto active_schedule = control->head_block_state()->active_schedule;
        BOOST_TEST(active_schedule.producers.size() == 1);
        BOOST_TEST(active_schedule.producers.front().producer_name == "eosio");

//花点时间让生产者的工资池由通货膨胀率填满。
produce_min_num_of_blocks_to_spend_time_wo_inactive_prod(fc::seconds(30 * 24 * 3600)); //30天
//由于激活的股份总数少于150000000，因此不应该要求奖励。
        BOOST_REQUIRE_THROW(claim_rewards(N(runnerup1)), eosio_assert_message_exception);

//这将使投票总数增加（40000000-1000）
        votepro( N(whale4), {N(prodq), N(prodr), N(prods), N(prodt), N(produ)} );
        BOOST_TEST(get_global_state()["total_activated_stake"].as<int64_t>() == 1899999996000);

//由于总投票权超过150000000股，将设置新的生产商设置。
produce_blocks_for_n_rounds(2); //2轮，因为新生产商计划是在下一轮的第一个区块不可逆时设置的。
        active_schedule = control->head_block_state()->active_schedule;
        BOOST_REQUIRE(active_schedule.producers.size() == 21);
        BOOST_TEST(active_schedule.producers.at(0).producer_name == "proda");
        BOOST_TEST(active_schedule.producers.at(1).producer_name == "prodb");
        BOOST_TEST(active_schedule.producers.at(2).producer_name == "prodc");
        BOOST_TEST(active_schedule.producers.at(3).producer_name == "prodd");
        BOOST_TEST(active_schedule.producers.at(4).producer_name == "prode");
        BOOST_TEST(active_schedule.producers.at(5).producer_name == "prodf");
        BOOST_TEST(active_schedule.producers.at(6).producer_name == "prodg");
        BOOST_TEST(active_schedule.producers.at(7).producer_name == "prodh");
        BOOST_TEST(active_schedule.producers.at(8).producer_name == "prodi");
        BOOST_TEST(active_schedule.producers.at(9).producer_name == "prodj");
        BOOST_TEST(active_schedule.producers.at(10).producer_name == "prodk");
        BOOST_TEST(active_schedule.producers.at(11).producer_name == "prodl");
        BOOST_TEST(active_schedule.producers.at(12).producer_name == "prodm");
        BOOST_TEST(active_schedule.producers.at(13).producer_name == "prodn");
        BOOST_TEST(active_schedule.producers.at(14).producer_name == "prodo");
        BOOST_TEST(active_schedule.producers.at(15).producer_name == "prodp");
        BOOST_TEST(active_schedule.producers.at(16).producer_name == "prodq");
        BOOST_TEST(active_schedule.producers.at(17).producer_name == "prodr");
        BOOST_TEST(active_schedule.producers.at(18).producer_name == "prods");
        BOOST_TEST(active_schedule.producers.at(19).producer_name == "prodt");
        BOOST_TEST(active_schedule.producers.at(20).producer_name == "produ");

//花点时间让生产者的工资池由通货膨胀率填满。
produce_min_num_of_blocks_to_spend_time_wo_inactive_prod(fc::seconds(30 * 24 * 3600)); //30天
//由于总激活股数大于150000000，应补足池奖励大于零。
        claim_rewards(N(runnerup1));
        BOOST_TEST(get_balance(N(runnerup1)).get_amount() > 0);

const auto first_june_2018 = fc::seconds(1527811200); //2018～06－01
const auto first_june_2028 = fc::seconds(1843430400); //2023-06－01
//确保2018-06-01之后10年内
        BOOST_REQUIRE(control->head_block_time().time_since_epoch() < first_june_2028);

//这应该是一个错误，因为布洛克一号只能在10年后把所有的股份都拆下来。

        BOOST_REQUIRE_THROW(undelegate_bandwidth(N(b1), N(b1), core_from_string("49999500.0000"), core_from_string("49999500.0000")), eosio_assert_message_exception);

//跳过10年
        produce_block(first_june_2028 - control->head_block_time().time_since_epoch());

//第一街区现在应该可以把所有的木桩都拆下来了
        undelegate_bandwidth(N(b1), N(b1), core_from_string("49999500.0000"), core_from_string("49999500.0000"));

        return;
produce_blocks(7000); ///生成块，直到virutal带宽可以容纳一个小用户
        wlog("minow" );
        votepro( N(minow1), {N(p1), N(p2)} );


//TODO:完成此测试
    } FC_LOG_AND_RETHROW()
}

BOOST_AUTO_TEST_SUITE_END()
