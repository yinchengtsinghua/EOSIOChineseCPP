
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

#include <utility>
#include <vector>
#include <string>
#include <eosiolib/eosio.hpp>
#include <eosiolib/contract.hpp>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wshorten-64-to-32"
#pragma clang diagnostic ignored "-Wsign-compare"

class test_ram_limit : public eosio::contract {
   public:
      const uint32_t FIVE_MINUTES = 5*60;

      test_ram_limit(account_name self)
      :eosio::contract(self)
      {}

//ABI行动
      void setentry(account_name payer, uint64_t from, uint64_t to, uint64_t size) {
         const auto self = get_self();
         eosio::print("test_ram_limit::setentry ", eosio::name{self}, "\n");
         test_table table(self, self);
         for (int key = from; key <=to; ++key) {
            auto itr = table.find(key);
            if (itr != table.end()) {
               table.modify(itr, payer, [size](test& t) {
                  t.data.assign(size, (int8_t)size);
               });
            } else {
               table.emplace(payer, [key,size](test& t) {
                  t.key = key;
                  t.data.assign(size, (int8_t)size);
               });
            }
         }
      }

//ABI行动
      void rmentry(uint64_t from, uint64_t to) {
         const auto self = get_self();
         eosio::print("test_ram_limit::rmentry ", eosio::name{self}, "\n");
         test_table table(self, self);
         for (int key = from; key <=to; ++key) {
            auto itr = table.find(key);
            eosio_assert (itr != table.end(), "could not find test_table entry");
            table.erase(itr);
         }
      }

//ABI行动
      void printentry(uint64_t from, uint64_t to) {
         const auto self = get_self();
         eosio::print("test_ram_limit::printout ", eosio::name{self}, ":");
         test_table table(self, self);
         for (int key = from; key <=to; ++key) {
            auto itr = table.find(key);
            eosio::print("\nkey=", key);
            eosio_assert (itr != table.end(), "could not find test_table entry");
            eosio::print(" size=", itr->data.size());
         }
      }

   private:
      struct test {
         uint64_t            key;
         std::vector<int8_t> data;

         uint64_t primary_key()const { return key; }

         EOSLIB_SERIALIZE( test, (key)(data) )
      };
      typedef eosio::multi_index< N(test.table), test> test_table;
};

#pragma clang diagnostic pop

EOSIO_ABI( test_ram_limit, (setentry)(rmentry)(printentry) )
