
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once
#include <eosiolib/types.h>
#include <eosiolib/print.hpp>

namespace eosio
{
    /*
    *@defgroup固定点固定点
    *@InGroup Mathcppapi公司
    *@brief 32,64128256位固定点变量版本
    *
    *浮点操作是不确定的，因此在智能合约中被阻止。
    *智能合约开发人员应使用适当的固定点模板类
    *通过传递以整数格式表示的数字和小数位数
    *要求。
    *这些模板类还支持算术运算和基本比较运算符
    *@
    **/


//一些正向声明
    template <uint8_t Q> struct fixed_point32;
    template <uint8_t Q> struct fixed_point64;
    template <uint8_t Q> struct fixed_point128;

//将支持下一版本中的固定点256
#if 0
    template <uint8_t Q> struct fixed_point256;
    /*
    固定点256位表示的*@defgroup模板类
    *@ingroup contractdev公司
    *@brief template param q是q因子，即小数位数。
    *
    **/

    template <uint8_t Q>
    struct fixed_point256
    {
        int128_t val;
        fixed_point256(int256_t v=0) : val(v) {}
        template <uint8_t QR> fixed_point256(const fixed_point256<QR> &r);
        template <uint8_t QR> fixed_point256(const fixed_point128<QR> &r);
        template <uint8_t QR> fixed_point256(const fixed_point64<QR> &r);
        template <uint8_t QR> fixed_point256(const fixed_point32<QR> &r);
        /*
        *获取64位固定数字的整数部分
        *@brief获取固定数字的整数部分
        *@return返回固定数字的整数部分
        *
        *实例：
        *@代码
        *固定64<18>A（1234.455667）
        *std：：cout<<a.int_part（）；//输出：1234
        *@终结码
        **/

        int128_t int_part() const {
            return val >> q;
        }

        /*
        *获取64位固定数字的小数部分
        *@brief获取固定数字的小数部分
        *@return返回固定数字的小数部分
        *
        *实例：
        *@代码
        *固定64<18>A（1234.455667）
        *std:：cout<<a.decimal_part（）；//输出：45567
        *@终结码
        **/

        uint128_t frac_part() const {
            if(!Q) return 0;
            return val << (32-Q);
        }



        template <uint8_t QR> fixed_point256 &operator=(const fixed_point32<QR> &r);
        template <uint8_t QR> fixed_point256 &operator=(const fixed_point64<QR> &r);
        template <uint8_t QR> fixed_point256 &operator=(const fixed_point128<QR> &r);
        template <uint8_t QR> fixed_point256 &operator=(const fixed_point256<QR> &r);
//比较函数
        template <uint8_t QR> bool operator==(const fixed_point256<QR> &r) { return (val == r.val);}
        template <uint8_t QR> bool operator>(const fixed_point256<QR> &r) { return (val > r.val);}
        template <uint8_t QR> bool operator<(const fixed_point256<QR> &r) { return (val < r.val);}
    };
#endif

    /*
    *模板参数q表示q因子，即小数位数。
    *
    固定点类的*@brief 128位表示。
    *
    *实例：
    *@代码
    *固定点128<6>A（123232.455667233）
    *固定点128<0>A（123424）
    *固定点128<18>C=A*B；
    *固定点128<24>d=a+b+c；
    *固定点128<24>e=b/a；
    *@终结码
    **/

    template <uint8_t Q>
    struct fixed_point128
    {
        static_assert(Q < 128, "Maximum number of decimals supported in fixed_point128 is 128 decimals");

        /*
         *@以int128表示的固定点的简短值
         *
         *以Int128表示的定点值
         **/

        int128_t val;

        /*
        *固定点128的各种构造函数。无法从int128、fixed、point128、64、32实例创建固定点128实例
        *
        *@为固定点128简要介绍各种构造函数
        *
        *实例：
        *@代码
        *固定点64<18>A（1234.455667）；
        *固定点128<3>b（a）；
        *固定点32<6>B（13324.32323）；
        *固定点128<5>C（A）；
        *@终结码
        **/

        
        /*
         *从int128构造一个新的固定点128对象
         *
         *@brief构造一个新的固定点128对象
         *@param v-int128表示定点值
         **/

        fixed_point128(int128_t v=0) : val(v) {}

         /*
         *从另一个固定点128构造一个新的固定点128对象
         *
         *@brief从另一个固定点128构造一个新的固定点128对象
         *@param r-另一个固定点128作为源
         **/

        template <uint8_t qr> fixed_point128(const fixed_point128<qr> &r);

        /*
         *从另一个固定点64构造一个新的固定点128对象
         *
         *@brief从另一个固定点64构造一个新的固定点128对象
         *@param r-固定点64作为源
         **/

        template <uint8_t qr> fixed_point128(const fixed_point64<qr> &r);

        /*
         *从另一个固定点32构造一个新的固定点128对象
         *
         *@brief从另一个固定点32构造一个新的固定点128对象
         *@param r-固定点32作为源
         **/

        template <uint8_t qr> fixed_point128(const fixed_point32<qr> &r);

        /*
        *获取64位固定数字的整数部分
        *
        *@brief获取固定数字的整数部分
        *@return返回固定数字的整数部分
        *
        *实例：
        *@代码
        *固定点64<5>A（1234.455667）
        *std：：cout<<a.int_part（）；//输出：1234
        *@终结码
        **/

        int128_t int_part() const {
            return val >> Q;
        }

        /*
        *获取64位固定数字的小数部分
        *
        *@brief获取固定数字的小数部分
        *@return返回固定数字的小数部分
        *
        *实例：
        *@代码
        *固定点128<3>A（1234.455667）
        *std:：cout<<a.decimal_part（）；//输出：455
        *@终结码
        **/

        uint128_t frac_part() const {
            if(!Q) return 0;
            return uint128_t(val << (32-Q));
        }

        /*
         *打印定点值
         *
         *@brief打印定点值
         **/

        void print() const {
           uint128_t ip((uint128_t)int_part());
           uint128_t fp(frac_part());
           printui128(&ip);
           prints(".");
           printui128(&fp);
        }

//各种分配运算符
        /*
         *分配运算符。将固定点32分配给固定点128
         *
         *@brief赋值运算符
         *@tparam qr-震源精度
         *@param r-源
         *@返回对该对象的固定点128&-引用
         **/

        template <uint8_t qr> fixed_point128 &operator=(const fixed_point32<qr> &r);
         /*
         *分配运算符。将固定点32分配给固定点64
         *
         *@brief赋值运算符
         *@tparam qr-震源精度
         *@param r-源
         *@返回对该对象的固定点128&-引用
         **/

        template <uint8_t qr> fixed_point128 &operator=(const fixed_point64<qr> &r);
         /*
         *分配运算符。将固定点32分配给固定点32
         *
         *@brief赋值运算符
         *@tparam qr-震源精度
         *@param r-源
         *@返回对该对象的固定点128&-引用
         **/

        template <uint8_t qr> fixed_point128 &operator=(const fixed_point128<qr> &r);

//比较函数
        /*
         *相等运算符
         *
         *@brief相等运算符
         *@tparam qr-震源精度
         *@param r-源
         *@返回真-如果等于
         *@返回false-否则
         **/

        template <uint8_t qr> bool operator==(const fixed_point128<qr> &r) { return (val == r.val);}

         /*
         *大于运算符
         *
         *@brief大于operator
         *@tparam qr-震源精度
         *@param r-源
         *@返回真-如果等于
         *@返回false-否则
         **/

        template <uint8_t qr> bool operator>(const fixed_point128<qr> &r) { return (val > r.val);}

         /*
         *小于运算符
         *
         *@brief小于operator
         *@tparam qr-震源精度
         *@param r-源
         *@返回真-如果等于
         *@返回false-否则
         **/

        template <uint8_t qr> bool operator<(const fixed_point128<qr> &r) { return (val < r.val);}
    };


    /*
    *@brief固定点类的64位表示。
    *
    *实例：
    *@代码
    *固定点64<6>A（123232.455667233）
    *固定点64<0>A（123424）
    *固定点64<18>C=A*B；
    *固定点64<24>d=a+b+c；
    *固定点64<24>e=b/a；
    *@终结码
    **/

    template <uint8_t Q>
    struct fixed_point64
    {
        static_assert(Q < 128, "Maximum number of decimals supported in fixed_point64 is 128 decimals");

        /*
         *@以Int64表示的固定点的简短值
         *
         *以Int64表示的定点值
         **/

        int64_t val;

        /*
         *从Int64构造一个新的固定点64对象
         *
         *@brief构造一个新的固定点64对象
         *@param v-int64表示定点值
         **/

        fixed_point64(int64_t v=0) : val(v) {}

        /*
         *从另一个固定点64构造一个新的固定点64对象
         *
         *@brief从另一个固定点64构造一个新的固定点64对象
         *@param r-另一个固定点64作为源
         **/

        template <uint8_t QR> fixed_point64(const fixed_point64<QR> &r);

        /*
         *从另一个固定点32构造一个新的固定点64对象
         *
         *@brief从另一个固定点32构造一个新的固定点64对象
         *@param r-固定点64作为源
         **/

        template <uint8_t QR> fixed_point64(const fixed_point32<QR> &r);

        /*
        *获取64位固定数字的整数部分
        *@brief获取固定数字的整数部分
        *@return返回固定数字的整数部分
        *
        *实例：
        *@代码
        *固定点64<18>A（1234.455667）
        *std：：cout<<a.int_part（）；//输出：1234
        *@终结码
        **/

        int64_t int_part() const {
            return val >> Q;
        }

        /*
        *获取64位固定数字的小数部分
        *@brief获取固定数字的小数部分
        *@return返回固定数字的小数部分
        *
        *实例：
        *@代码
        *固定64<3>A（1234.455667）
        *std:：cout<<a.decimal_part（）；//输出：455
        *@终结码
        **/

        uint64_t frac_part() const {
            if(!Q) return 0;
            return uint64_t(val << (32-Q));
        }

        /*
         *打印定点值
         *
         *@brief打印定点值
         **/

        void print() const {
           printi(int_part());
           prints(".");
           printi128(frac_part());
        }

//各种分配运算符
        /*
         *分配运算符。将固定点32分配给固定点64
         *
         *@brief赋值运算符
         *@tparam qr-震源精度
         *@param r-源
         *@返回对该对象的固定点64&-引用
         **/

        template <uint8_t QR> fixed_point64 &operator=(const fixed_point32<QR> &r);

        /*
         *分配运算符。将固定点64分配给固定点64
         *
         *@brief赋值运算符
         *@tparam qr-震源精度
         *@param r-源
         *@返回对该对象的固定点64&-引用
         **/

        template <uint8_t QR> fixed_point64 &operator=(const fixed_point64<QR> &r);

//算术运算
        /*
         *加法运算符
         *
         *@brief加法运算符
         *@tparam qr-第二个加数的精度
         *@参数r-第二个加数
         *@return-加法的结果
         **/

        template <uint8_t QR> fixed_point64< (Q>QR)?Q:QR > operator+(const fixed_point64<QR> &r) const;

        /*
         *减法运算符
         *
         *@brief减法运算符
         *@tparam qr-分钟精度
         *@param r-分钟
         *@return-减法的结果
         **/

        template <uint8_t QR> fixed_point64< (Q>QR)?Q:QR > operator-(const fixed_point64<QR> &r) const;

//两个固定点64实例的乘积和除法将固定点128
//小数的总数将是最大的
        /*
         *乘法运算符
         *
         *@brief乘法运算符
         *@tparam qr-乘数精度
         *@param r-乘数
         *@return-乘法的结果
         **/

        template <uint8_t QR> fixed_point128<Q+QR> operator*(const fixed_point64<QR> &r) const;

        /*
         *部门操作员
         *
         *@brief除法运算符
         *@tparam qr-除数精度
         *@param r-除数
         *@返回-除法结果
         **/

        template <uint8_t QR> fixed_point128<Q+64-QR> operator/(const fixed_point64<QR> &r) const;

//比较函数
        /*
         *相等运算符
         *
         *@brief相等运算符
         *@tparam qr-震源精度
         *@param r-源
         *@返回真-如果等于
         *@返回false-否则
         **/

        template <uint8_t QR> bool operator==(const fixed_point64<QR> &r) { return (val == r.val);}

        /*
         *大于运算符
         *
         *@brief大于operator
         *@tparam qr-震源精度
         *@param r-源
         *@返回真-如果等于
         *@返回false-否则
         **/

        template <uint8_t QR> bool operator>(const fixed_point64<QR> &r) { return (val > r.val);}

        /*
         *小于运算符
         *
         *@brief小于operator
         *@tparam qr-震源精度
         *@param r-源
         *@返回真-如果等于
         *@返回false-否则
         **/

        template <uint8_t QR> bool operator<(const fixed_point64<QR> &r) { return (val < r.val);}
    };

    /*
     固定点类的*@brief 32位表示。
     *
     *该类用于替换浮点变量。
     *可解决浮点非中介相关问题
     *
     *实例：
     *@代码
     *
     *固定点32<17>b（9.654）；
     *固定点32<18>C=A*B；
     *固定点32<24>d=a+b+c；
     *固定点32<24>e=b/a；
     *@终结码
     *
     **/

//固定点32位版本。模板参数“q”是比例因子
    template <uint8_t Q>
    struct fixed_point32
    {
        static_assert(Q < 128, "Maximum number of decimals supported in fixed_point32 is 128 decimals");
        
        /*
         *@以Int32表示的固定点的简短值
         *
         *以Int32表示的定点值
         **/

        int32_t val;

        /*
         *从另一个固定点32构造一个新的固定点32对象
         *
         *@brief从另一个固定点32构造一个新的固定点32对象
         *@param r-另一个固定点32作为源
         **/    

        template <uint8_t QR> fixed_point32(const fixed_point32<QR> &r);

        /*
         
         *
         *@brief从另一个固定点64构造一个新的固定点32对象
         *@param r-另一个固定点32作为源
         **/

        template <uint8_t QR> fixed_point32(const fixed_point64<QR> &r);
       

        /*
         *从Int32构造一个新的固定点32对象
         *
         *@brief构造一个新的固定点32对象
         *@param param-int32表示定点值
         **/

        fixed_point32(int32_t param=0) : val(param) {}

//根据比例因子将给定的双变量转换为Int32
//固定点32（双d=0）：val（d*（1<<q））
        /*
        double到_double（）常量
            返回double（val）/（1<<q）；
        }
        **/


        /*
        *获取64位固定数字的整数部分
        *@brief获取固定数字的整数部分
        *@return返回固定数字的整数部分
        *
        *实例：
        *@代码
        *固定点32<18>A（1234.455667）
        *std：：cout<<a.int_part（）；//输出：1234
        *@终结码
        **/

        int32_t int_part() const {
            return val >> Q;
        }
        uint32_t frac_part() const {
            if(!Q) return 0;
            return uint32_t(val << (32-Q));
        }

        /*
         *打印定点值
         *
         *@brief打印定点值
         **/

        void print() const {
           printi(int_part());
           prints(".");
           printi128(frac_part());
        }

//各种分配运算符
        /*
         *分配运算符。将固定点32分配给固定点32
         *
         *@brief赋值运算符
         *@tparam qr-震源精度
         *@param r-源
         *@返回对该对象的固定点32&-引用
         **/

        template <uint8_t QR> fixed_point32 &operator=(const fixed_point32<QR> &r);

        /*
         *分配运算符。将固定点64分配给固定点32
         *
         *@brief赋值运算符
         *@tparam qr-震源精度
         *@param r-源
         *@返回对该对象的固定点32&-引用
         **/

        template <uint8_t QR> fixed_point32 &operator=(const fixed_point64<QR> &r);

        /*
         *加法运算符
         *
         *@brief加法运算符
         *@tparam qr-第二个加数的精度
         *@参数r-第二个加数
         *@return-加法的结果
         **/

        template <uint8_t QR> fixed_point32< (Q>QR)?Q:QR > operator+(const fixed_point32<QR> &r) const;

        /*
         *减法运算符
         *
         *@brief减法运算符
         *@tparam qr-分钟精度
         *@param r-分钟
         *@return-减法的结果
         **/

        template <uint8_t QR> fixed_point32< (Q>QR)?Q:QR > operator-(const fixed_point32<QR> &r) const;

//到固定点32的productd实例将固定点64
        /*
         *乘法运算符
         *
         *@brief乘法运算符
         *@tparam qr-乘数精度
         *@param r-乘数
         *@return-乘法的结果
         **/

        template <uint8_t QR> fixed_point64<Q+QR> operator*(const fixed_point32<QR> &r) const;

        /*
         *部门操作员
         *
         *@brief除法运算符
         *@tparam qr-除数精度
         *@param r-除数
         *@返回-除法结果
         **/

        template <uint8_t QR> fixed_point64<Q+32-QR> operator/(const fixed_point32<QR> &r) const;

//比较函数
        /*
         *相等运算符
         *
         *@brief相等运算符
         *@tparam qr-震源精度
         *@param r-源
         *@返回真-如果等于
         *@返回false-否则
         **/

        template <uint8_t QR> bool operator==(const fixed_point32<QR> &r) { return (val == r.val);}

        /*
         *大于运算符
         *
         *@brief大于operator
         *@tparam qr-震源精度
         *@param r-源
         *@返回真-如果等于
         *@返回false-否则
         **/

        template <uint8_t QR> bool operator>(const fixed_point32<QR> &r) { return (val > r.val);}

        /*
         *小于运算符
         *
         *@brief小于operator
         *@tparam qr-震源精度
         *@param r-源
         *@返回真-如果等于
         *@返回false-否则
         **/

        template <uint8_t QR> bool operator<(const fixed_point32<QR> &r) { return (val < r.val);}
    };


///}固定点

//帮助程序函数
    template<typename T>
    T assignHelper(T rhs_val, uint8_t q, uint8_t qr)
    {
        T result = (q > qr) ? rhs_val << (q-qr) : rhs_val >> (qr-q);
        return result;
    }


#if 0
//固定点256方法
    template<uint32_t q> template<uint32_t qr>
    fixed_point256<q>::fixed_point256(const fixed_point256<qr> &r) {
        val = assignHelper<int256_t>(r.val, q, qr);
    }

    template<uint32_t q> template<uint32_t qr>
    fixed_point256<q>::fixed_point256(const fixed_point128<qr> &r) {
        val = assignHelper<int256_t>(r.val, q, qr);
    }

    template<uint32_t q> template<uint32_t qr>
    fixed_point256<q>::fixed_point256(const fixed_point64<qr> &r) {
        val = assignHelper<int256_t>(r.val, q, qr);
    }

    template<uint32_t q> template <uint32_t qr>
    fixed_point256<q>::fixed_point256(const fixed_point32<qr> &r) {
        val = assignHelper<int256_t>(r.val, q, qr);
    }
#endif

//固定点128方法
    template<uint8_t Q> template<uint8_t QR>
    fixed_point128<Q>::fixed_point128(const fixed_point128<QR> &r) {
        val = assignHelper<int128_t>(r.val, Q, QR);
    }

    template<uint8_t Q> template<uint8_t QR>
    fixed_point128<Q>::fixed_point128(const fixed_point64<QR> &r) {
        val = assignHelper<int128_t>(r.val, Q, QR);
    }

    template<uint8_t Q> template <uint8_t QR>
    fixed_point128<Q>::fixed_point128(const fixed_point32<QR> &r) {
        val = assignHelper<int128_t>(r.val, Q, QR);
    }


//固定点64方法
    template<uint8_t Q> template<uint8_t QR>
    fixed_point64<Q>::fixed_point64(const fixed_point64<QR> &r) {
        val = assignHelper<int64_t>(r.val, Q, QR);
    }

    template<uint8_t Q> template <uint8_t QR>
    fixed_point64<Q>::fixed_point64(const fixed_point32<QR> &r) {
        val = assignHelper<int32_t>(r.val, Q, QR);
    }

    /*
    *@brief addition between two fixed\u point64 variables and the result goes to fixed\u point64
    *
    *两个固定点64变量之间的加法
    *结果的小数位数将为左、右小数的最大小数位数。
    **/

    template<uint8_t Q> template<uint8_t QR>
    fixed_point64< (Q>QR)?Q:QR > fixed_point64<Q>::operator+(const fixed_point64<QR> &rhs) const
    {
//如果两者的比例因子相同，则无需制作除结果之外的任何中间对象
        if(Q == QR)
        {
            return fixed_point64<Q>(val + rhs.val);
        }
        return fixed_point64<(Q>QR)?Q:QR>(
            fixed_point64<(Q>QR)?Q:QR>( *this ).val +
            fixed_point64<(Q>QR)?Q:QR>( rhs ).val
        );
    }

    /*
    *@两个固定点64变量之间的简短减法，结果转到固定点64
    *
    *两个固定点64变量之间的减法
    *结果的小数位数将为左、右小数的最大小数位数。
    **/

    template<uint8_t Q> template<uint8_t QR>
    fixed_point64< (Q>QR)?Q:QR > fixed_point64<Q>::operator-(const fixed_point64<QR> &rhs) const
    {
//如果两者的比例因子相同，则无需制作除结果之外的任何中间对象
        if(Q == QR)
        {
            return fixed_point64<Q>(val - rhs.val);
        }
        return fixed_point64<(Q>QR)?Q:QR>(
            fixed_point64<(Q>QR)?Q:QR>( *this ).val -
            fixed_point64<(Q>QR)?Q:QR>( rhs ).val
        );
    }

    /*
    固定点64的*@brief乘法运算符。结果转到固定点64
    *
    *固定点64的乘法运算符。结果转到固定点128
    *结果的小数位数为左、右小数位数之和。
    *
    *实例：
    *@代码
    *固定点128<33>结果=固定点64<0>（131313）/固定点64<0>（2323）
    *@终结码
    **/

    template<uint8_t Q> template <uint8_t QR>
    fixed_point128<Q+QR> fixed_point64<Q>::operator*(const fixed_point64<QR> &r) const {
        return fixed_point128<Q+QR>(int128_t(val)*r.val);
    }

    /*
    *@两个固定点的简短划分64结果将存储在固定点128中
    *
    *固定点的除法运算符64
    *
    *实例：
    *@代码
    *固定点128<33>结果=固定点64<0>（131313）/固定点64<0>（2323）
    *@终结码
    **/

    template <uint8_t Q> template <uint8_t QR>
    fixed_point128<Q+64-QR> fixed_point64<Q>::operator/(const fixed_point64<QR> &r) const {
//std:：cout<“在”<<val<“上执行除法，用”<<q<“precision/”<<r.val<“，用”<<q r<“precision。结果精度“<”（（q>qr）？q:qr）<<std:：endl；
//通过另外移动64位将val转换为128位，并将结果转换为128位
//q（x+64-y）=q（x+64）/q（y）
       eosio_assert( !(r.int_part() == 0 && r.frac_part() == 0), "divide by zero" );
        return fixed_point128<Q+64-QR>((int128_t(val)<<64)/r.val);
    }

//固定点32方法
    template<uint8_t Q> template <uint8_t QR>
    fixed_point32<Q>::fixed_point32(const fixed_point32<QR> &r) {
        val = assignHelper<int32_t>(r.val, Q, QR);
    }

    template<uint8_t Q> template <uint8_t QR>
    fixed_point32<Q>::fixed_point32(const fixed_point64<QR> &r) {
        val = assignHelper<int32_t>(r.val, Q, QR);
    }

    template<uint8_t Q> template <uint8_t QR>
    fixed_point32<Q> &fixed_point32<Q>::operator=(const fixed_point32<QR> &r) {
        val = assignHelper<int32_t>(r.val, Q, QR);
    }

    template<uint8_t Q> template <uint8_t QR>
    fixed_point32<Q> &fixed_point32<Q>::operator=(const fixed_point64<QR> &r) {
        val = assignHelper<int32_t>(r.val, Q, QR);
    }

    /*
    *@brief addition between two fixedou point32 variables and the result goes to fixedou point32
    *
    *两个固定点32变量之间的加法
    *结果的小数位数将为左、右小数的最大小数位数。
    *
    **/

    template<uint8_t Q> template<uint8_t QR>
    fixed_point32< (Q>QR)?Q:QR > fixed_point32<Q>::operator+(const fixed_point32<QR> &rhs) const
    {
//如果两者的比例因子相同，则无需制作除结果之外的任何中间对象
        if(Q == QR)
        {
            return fixed_point32<Q>(val + rhs.val);
        }
        return fixed_point32<(Q>QR)?Q:QR>(
            fixed_point32<(Q>QR)?Q:QR>( *this ).val +
            fixed_point32<(Q>QR)?Q:QR>( rhs ).val
        );
    }

    /*
    *@两个固定点32变量之间的简短减法，结果转到固定点32
    *
    *两个固定点32变量之间的减法
    *结果的小数位数将为左、右小数的最大小数位数。
    *
    **/

    template<uint8_t Q> template<uint8_t QR>
    fixed_point32< (Q>QR)?Q:QR > fixed_point32<Q>::operator-(const fixed_point32<QR> &rhs) const
    {
//如果两者的比例因子相同，则无需制作除结果之外的任何中间对象
        if(Q == QR)
        {
            return fixed_point32<Q>(val - rhs.val);
        }
        return fixed_point32<(Q>QR)?Q:QR>(
            fixed_point32<(Q>QR)?Q:QR>( *this ).val -
            fixed_point32<(Q>QR)?Q:QR>( rhs ).val
        );
    }

    /*
    固定点32的*@brief乘法运算符。结果转到固定点64
    *
    *固定点32的乘法运算符。结果转到固定点64
    *结果的小数位数为左、右小数位数之和。
    *
    *实例：
    *@代码
    *固定点64<33>结果=固定点32<0>（131313）/固定点32<0>（2323）
    *@终结码
    **/

    template<uint8_t Q> template <uint8_t QR>
    fixed_point64<Q+QR> fixed_point32<Q>::operator*(const fixed_point32<QR> &r) const {
        return fixed_point64<Q+QR>(int64_t(val)*r.val);
    }

    /*
    *@两个固定点32结果的简短划分将存储在固定点64中
    *
    *固定点的除法运算符32
    *
    *实例：
    *@代码
    *固定点64<33>结果=固定点32<0>（131313）/固定点32<0>（2323）
    *@终结码
    **/

    template <uint8_t Q> template <uint8_t QR>
    fixed_point64<Q+32-QR> fixed_point32<Q>::operator/(const fixed_point32<QR> &r) const {
//将val转换为64位并执行除法
//q（x+32-y）=q（x+32）/q（y）
        eosio_assert( !(r.int_part() == 0 && r.frac_part() == 0), "divide by zero" );
        return fixed_point64<Q+32-QR>((int64_t(val)<<32)/r.val);
    }

    /*
    *@brief包装函数，用于划分两个unit64变量并将结果存储在固定的点64中
    *
    *用于划分两个Unit32变量并将结果存储在固定的_Point64中的包装函数
    *
    *实例：
    *@代码
    *Fixed_Point64<33>结果=Fixed_Divide（131313，2323）
    *@终结码
    **/

    template <uint8_t Q>
    fixed_point64<Q> fixed_divide(uint32_t lhs, uint32_t rhs)
    {

        eosio_assert( rhs != 0, "divide by zero" );
        fixed_point64<Q> result = fixed_point32<0>((int32_t)lhs) / fixed_point32<0>((int32_t)rhs);
        return result;
    }

    /*
    *@brief包装函数，用于划分两个unit64变量并将结果存储在固定的点128中
    *用于划分两个Unit64变量并将结果存储在固定的_Point128中的包装函数
    *
    *实例：
    *@代码
    *Fixed_Point128<33>结果=Fixed_Divide（131313，2323）
    *@终结码
    **/


    template <uint8_t Q>
    fixed_point128<Q> fixed_divide(uint64_t lhs, uint64_t rhs)
    {

        eosio_assert( rhs != 0, "divide by zero" );
        fixed_point128<Q> result = fixed_point64<0>((int32_t)lhs) / fixed_point64<0>((int32_t)rhs);
        return fixed_point128<Q>(result);
    }


};
