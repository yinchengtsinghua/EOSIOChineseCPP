
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include <eosiolib/fixedpoint.hpp>
#include <eosiolib/eosio.hpp>

#include "test_api.hpp"

void test_fixedpoint::create_instances()
{
    {
//创建固定点的各种方法128
       eosio::fixed_point128<18> a(12345667);
       eosio::fixed_point128<18> b(12345667);
       eosio::fixed_point128<16> c(12345667);
       eosio_assert(b == a, "fixed_point128 instances comparison with same number of decimals");
       eosio_assert(c == a, "fixed_point128 instances with different number of decimals");
    }

    {
//创建固定点的各种方法64
       eosio::fixed_point64<5> a(12345667);
       eosio::fixed_point64<5> b(12345667);
       eosio::fixed_point64<5> c(12345667);
       eosio_assert(b == a, "fixed_point64 instances comparison with same number of decimals");
       eosio_assert(c == a, "fixed_point64 instances with different number of decimals");
    }

    {
//创建固定点的各种方法32
       eosio::fixed_point32<18> a(12345667);
       eosio::fixed_point32<18> b(12345667);
       eosio::fixed_point32<16> c(12345667);
       eosio_assert(b == a, "fixed_point32 instances comparison with same number of decimals");
       eosio_assert(c == a, "fixed_point32 instances with different number of decimals");
    }
}

void test_fixedpoint::test_addition()
{
    {
//创建固定点的各种方法32
       eosio::fixed_point32<0> a(100);
       eosio::fixed_point32<0> b(100);
       eosio::fixed_point32<0> c = a + b;
       eosio::fixed_point32<0> d = 200;
       eosio_assert(c == d, "fixed_point32 instances addition with zero decmimals");
    }
    {
//创建固定点的各种方法64
       eosio::fixed_point64<0> a(100);
       eosio::fixed_point64<0> b(100);
       eosio::fixed_point64<0> c = a + b;
       eosio::fixed_point64<0> d = 200;
       eosio_assert(c == d, "fixed_point64 instances addition with zero decmimals");
    }
};

void test_fixedpoint::test_subtraction()
{
    {
//创建固定点的各种方法64
       eosio::fixed_point64<0> a(100);
       eosio::fixed_point64<0> b(100);
       eosio::fixed_point64<0> c = a - b;
       eosio::fixed_point64<0> d = 0;
       eosio_assert(c == d, "fixed_point64 instances subtraction with zero decmimals");

       eosio::fixed_point64<0> a1(0);
       eosio::fixed_point64<0> c1 = a1 - b;
       eosio::fixed_point64<0> d1 = -100;
       eosio_assert(c1 == d1, "fixed_point64 instances subtraction with zero decmimals");
    }
    {
//创建固定点的各种方法32
       eosio::fixed_point32<0> a(100);
       eosio::fixed_point32<0> b(100);
       eosio::fixed_point32<0> c = a - b;
       eosio::fixed_point32<0> d = 0;
       eosio_assert(c == d, "fixed_point32 instances subtraction with zero decmimals");

//创建固定点的各种方法32
       eosio::fixed_point32<0> a1(0);
       eosio::fixed_point32<0> c1 = a1 - b;
       eosio::fixed_point32<0> d1 = -100;
       eosio_assert(c1 == d1, "fixed_point32 instances subtraction with zero decmimals");

    }
};

void test_fixedpoint::test_multiplication()
{
    {
//创建固定点的各种方法64
       eosio::fixed_point64<0> a(100);
       eosio::fixed_point64<0> b(200);
       eosio::fixed_point128<0> c = a * b;
       eosio::fixed_point128<0> d(200*100);
       eosio_assert(c == d, "fixed_point64 instances multiplication result in fixed_point128");
    }

    {
//创建固定点的各种方法32
       eosio::fixed_point32<0> a(100);
       eosio::fixed_point32<0> b(200);
       eosio::fixed_point64<0> c = a * b;
       eosio::fixed_point64<0> d(200*100);
       eosio_assert(c == d, "fixed_point32 instances multiplication result in fixed_point64");
    }
}

void test_fixedpoint::test_division()
{
    {
        uint64_t lhs = 10000000;
        uint64_t rhs = 333;

        eosio::fixed_point64<0> a((int64_t)lhs);
        eosio::fixed_point64<0> b((int64_t)rhs);
        eosio::fixed_point128<5> c = a / b;

        eosio::fixed_point128<5> e = eosio::fixed_divide<5>(lhs, rhs);
        print(e);
        eosio_assert(c == e, "fixed_point64 instances division result from operator and function and compare in fixed_point128");

    }

    {
        uint32_t lhs = 100000;
        uint32_t rhs = 33;

        eosio::fixed_point32<0> a((int32_t)lhs);
        eosio::fixed_point32<0> b((int32_t)rhs);
        eosio::fixed_point64<5> c = a / b;

        eosio::fixed_point64<5> e = eosio::fixed_divide<5>(lhs, rhs);
        eosio_assert(c == e, "fixed_point64 instances division result from operator and function and compare in fixed_point128");

    }
}

void test_fixedpoint::test_division_by_0()
{
    {
        uint64_t lhs = 10000000;
        uint64_t rhs = 0;

        eosio::fixed_point64<0> a((int64_t)lhs);
        eosio::fixed_point64<0> b((int64_t)rhs);

        eosio::fixed_point128<5> e = eosio::fixed_divide<5>(lhs, rhs);
//为了消除未使用的参数警告
        e = 0;
        eosio_assert(false, "should've thrown an error");

    }

 }
