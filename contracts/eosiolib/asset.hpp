
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once
#include <eosiolib/serialize.hpp>
#include <eosiolib/print.hpp>
#include <eosiolib/system.h>
#include <eosiolib/symbol.hpp>
#include <tuple>
#include <limits>

namespace eosio {

  /*
   *@defgroup asset api资产API
   *@brief定义用于管理资产的API
   *@ingroup contractdev公司
   **/


  /*
   *@defgroup asset cpp api资产cpp api
   *@brief定义了用于管理资产的%cpp api
   *英格鲁阿塞塔皮
   *@
   **/


   /*
    *\struct存储资产所有者的信息
    *
    *@brief存储资产所有者的信息
    **/


   struct asset {
      /*
       *资产金额
       *
       *@简述资产金额
       **/

      int64_t      amount;

      /*
       *资产的符号名称
       *
       *@简述资产的符号名称
       **/

      symbol_type  symbol;

      /*
       *此资产可能的最大金额。上限为2^62-1
       *
       *@brief此资产可能的最大金额
       **/

      static constexpr int64_t max_amount    = (1LL << 62) - 1;

      /*
       *根据符号名称和金额构建新资产
       *
       *@brief构造新的资产对象
       *@param A-资产金额
       *@param s-符号的名称，默认为核心符号
       **/

      explicit asset( int64_t a = 0, symbol_type s = CORE_SYMBOL )
      :amount(a),symbol{s}
      {
         eosio_assert( is_amount_within_range(), "magnitude of asset amount must be less than 2^62" );
         eosio_assert( symbol.is_valid(),        "invalid symbol name" );
      }

      /*
       *检查金额是否不超过最大金额
       *
       *@简要检查金额是否不超过最大金额
       *@返回真-如果金额不超过最大金额
       *@返回false-否则
       **/

      bool is_amount_within_range()const { return -max_amount <= amount && amount <= max_amount; }

      /*
       *检查资产是否有效。%a有效资产的金额<=最大金额及其符号名称有效
       *
       *@简要检查资产是否有效
       *@返回真-如果资产有效
       *@返回false-否则
       **/

      bool is_valid()const               { return is_amount_within_range() && symbol.is_valid(); }

      /*
       *设置资产金额
       *
       *@brief设置资产金额
       *@param A-资产的新金额
       **/

      void set_amount( int64_t a ) {
         amount = a;
         eosio_assert( is_amount_within_range(), "magnitude of asset amount must be less than 2^62" );
      }

      /*
       *一元减号运算符
       *
       *@brief一元减号运算符
       *@返回资产-新资产及其金额为该资产的负金额
       **/

      asset operator-()const {
         asset r = *this;
         r.amount = -r.amount;
         return r;
      }

      /*
       *减法赋值运算符
       *
       *@brief减法赋值运算符
       *@param a-用另一个资产减去该资产
       *@返回资产并引用此资产
       *@post此资产的金额减去资产A的金额
       **/

      asset& operator-=( const asset& a ) {
         eosio_assert( a.symbol == symbol, "attempt to subtract asset with different symbol" );
         amount -= a.amount;
         eosio_assert( -max_amount <= amount, "subtraction underflow" );
         eosio_assert( amount <= max_amount,  "subtraction overflow" );
         return *this;
      }

      /*
       *添加分配运算符
       *
       *@brief加法赋值运算符
       *@param a-用另一个资产减去该资产
       *@返回资产并引用此资产
       *@post此资产的金额与资产A的金额相加。
       **/

      asset& operator+=( const asset& a ) {
         eosio_assert( a.symbol == symbol, "attempt to add asset with different symbol" );
         amount += a.amount;
         eosio_assert( -max_amount <= amount, "addition underflow" );
         eosio_assert( amount <= max_amount,  "addition overflow" );
         return *this;
      }

      /*
       *加法运算符
       *
       *@brief加法运算符
       *@param a-要添加的第一个资产
       *@param b-要添加的第二个资产
       *@返回资产-添加后的新资产
       **/

      inline friend asset operator+( const asset& a, const asset& b ) {
         asset result = a;
         result += b;
         return result;
      }

      /*
       *减法运算符
       *
       *@brief减法运算符
       *@param a-要减去的资产
       *@param b-用于减去的资产
       *@返回资产-减去A和B后的新资产
       **/

      inline friend asset operator-( const asset& a, const asset& b ) {
         asset result = a;
         result -= b;
         return result;
      }

      /*
       *乘法赋值运算符。将该资产的金额乘以一个数字，然后将该值赋给它自己。
       *
       *@brief乘法赋值运算符，带数字
       *@param a-资产金额的乘数
       *@返回资产-引用此资产
       *@post此资产的金额乘以
       **/

      asset& operator*=( int64_t a ) {
         int128_t tmp = (int128_t)amount * (int128_t)a;
         eosio_assert( tmp <= max_amount, "multiplication overflow" );
         eosio_assert( tmp >= -max_amount, "multiplication underflow" );
         amount = (int64_t)tmp;
         return *this;
      }

      /*
       *乘法运算符，数字继续
       *
       *@brief乘法运算符，数字继续
       *@param a-要乘以的资产
       *@param b-资产金额的乘数
       *@返回资产-倍增后的新资产
       **/

      friend asset operator*( const asset& a, int64_t b ) {
         asset result = a;
         result *= b;
         return result;
      }


      /*
       *乘法运算符，前面有一个数字
       *
       *@brief乘法运算符，前面有一个数字
       *@param a-资产金额的乘数
       *@param b-要乘以的资产
       *@返回资产-倍增后的新资产
       **/

      friend asset operator*( int64_t b, const asset& a ) {
         asset result = a;
         result *= b;
         return result;
      }

      /*
       *部门分配操作员。将该资产的金额除以一个数字，然后将该值分配给它自己。
       *
       *@brief division assignment operator，带数字
       *@param a-资产金额的除数
       *@返回资产-引用此资产
       *@post此资产的金额除以
       **/

      asset& operator/=( int64_t a ) {
         eosio_assert( a != 0, "divide by zero" );
         eosio_assert( !(amount == std::numeric_limits<int64_t>::min() && a == -1), "signed division overflow" );
         amount /= a;
         return *this;
      }

      /*
       *除法运算符，数字处理
       *
       *@brief division operator，进行数字运算
       *@param a-要分割的资产
       *@param b-资产金额的除数
       *@返还资产-因分割而产生的新资产
       **/

      friend asset operator/( const asset& a, int64_t b ) {
         asset result = a;
         result /= b;
         return result;
      }

      /*
       *部门运营商，使用其他资产
       *
       *@brief division operator，使用其他资产
       *@param A-金额作为股息的资产
       *@param b-金额作为除数的资产
       *@return int64_t-除法后的结果量
       *@pre两个资产必须具有相同的符号
       **/

      friend int64_t operator/( const asset& a, const asset& b ) {
         eosio_assert( a.symbol == b.symbol, "comparison of assets with different symbols is not allowed" );
         return a.amount / b.amount;
      }

      /*
       *相等运算符
       *
       *@brief相等运算符
       *@param a-要比较的第一个资产
       *@param b-要比较的第二个资产
       *@返回真-如果两个资产的金额相同
       *@返回false-否则
       *@pre两个资产必须具有相同的符号
       **/

      friend bool operator==( const asset& a, const asset& b ) {
         eosio_assert( a.symbol == b.symbol, "comparison of assets with different symbols is not allowed" );
         return a.amount == b.amount;
      }

      /*
       *不等式运算符
       *
       *@brief不等式运算符
       *@param a-要比较的第一个资产
       *@param b-要比较的第二个资产
       *@返回真-如果两个资产的金额不相同
       *@返回false-否则
       *@pre两个资产必须具有相同的符号
       **/

      friend bool operator!=( const asset& a, const asset& b ) {
         return !( a == b);
      }

      /*
       *小于运算符
       *
       *@brief小于operator
       *@param a-要比较的第一个资产
       *@param b-要比较的第二个资产
       *@返回真-如果第一个资产的金额小于第二个资产的金额
       *@返回false-否则
       *@pre两个资产必须具有相同的符号
       **/

      friend bool operator<( const asset& a, const asset& b ) {
         eosio_assert( a.symbol == b.symbol, "comparison of assets with different symbols is not allowed" );
         return a.amount < b.amount;
      }

      /*
       *小于或等于运算符
       *
       *@brief小于或等于operator
       *@param a-要比较的第一个资产
       *@param b-要比较的第二个资产
       *@返回真值-如果第一个资产的金额小于或等于第二个资产的金额
       *@返回false-否则
       *@pre两个资产必须具有相同的符号
       **/

      friend bool operator<=( const asset& a, const asset& b ) {
         eosio_assert( a.symbol == b.symbol, "comparison of assets with different symbols is not allowed" );
         return a.amount <= b.amount;
      }

      /*
       *大于运算符
       *
       *@brief大于operator
       *@param a-要比较的第一个资产
       *@param b-要比较的第二个资产
       *@返回真-如果第一个资产的金额大于第二个资产的金额
       *@返回false-否则
       *@pre两个资产必须具有相同的符号
       **/

      friend bool operator>( const asset& a, const asset& b ) {
         eosio_assert( a.symbol == b.symbol, "comparison of assets with different symbols is not allowed" );
         return a.amount > b.amount;
      }

      /*
       *大于或等于运算符
       *
       *@brief大于或等于operator
       *@param a-要比较的第一个资产
       *@param b-要比较的第二个资产
       *@返回真-如果第一个资产的金额大于或等于第二个资产的金额
       *@返回false-否则
       *@pre两个资产必须具有相同的符号
       **/

      friend bool operator>=( const asset& a, const asset& b ) {
         eosio_assert( a.symbol == b.symbol, "comparison of assets with different symbols is not allowed" );
         return a.amount >= b.amount;
      }

      /*
       *%打印资产
       *
       *@brief%打印资产
       **/

      void print()const {
         int64_t p = (int64_t)symbol.precision();
         int64_t p10 = 1;
         while( p > 0  ) {
            p10 *= 10; --p;
         }
         p = (int64_t)symbol.precision();

         char fraction[p+1];
         fraction[p] = '\0';
         auto change = amount % p10;

         for( int64_t i = p -1; i >= 0; --i ) {
            fraction[i] = (change % 10) + '0';
            change /= 10;
         }
         printi( amount / p10 );
         prints(".");
         prints_l( fraction, uint32_t(p) );
         prints(" ");
         symbol.print(false);
      }

      EOSLIB_SERIALIZE( asset, (amount)(symbol) )
   };

  /*
   *\struct存储资产所有者信息的扩展资产
   *
   *@brief扩展资产，用于存储资产所有者的信息
   **/

   struct extended_asset : public asset {
      /*
       *资产所有者
       *
       *@向资产所有者简要介绍
       **/

      account_name contract;

      /*
       *获取资产的扩展符号
       *
       *@brief获取资产的扩展符号
       *@返回扩展符号-资产的扩展符号
       **/

      extended_symbol get_extended_symbol()const { return extended_symbol( symbol, contract ); }

      /*
       *默认构造函数
       *
       *@brief构造新的扩展资产对象
       **/

      extended_asset() = default;

       /*
       *根据金额和扩展符号构造新的扩展资产
       *
       *@brief构造新的扩展资产对象
       **/

      extended_asset( int64_t v, extended_symbol s ):asset(v,s),contract(s.contract){}
      /*
       *根据资产和所有者名称构建新的扩展资产
       *
       *@brief构造新的扩展资产对象
       **/

      extended_asset( asset a, account_name c ):asset(a),contract(c){}

      /*
       *%打印扩展资产
       *
       *@brief%打印扩展资产
       **/

      void print()const {
         asset::print();
         prints("@");
         printn(contract);
      }

       /*
       *一元减号运算符
       *
       *@brief一元减号运算符
       *@返回扩展资产-新扩展资产及其金额为该扩展资产的负金额
       **/

      extended_asset operator-()const {
         asset r = this->asset::operator-();
         return {r, contract};
      }

      /*
       *减法运算符。这将减去扩展资产的金额。
       *
       *@brief减法运算符
       *@param a-要减去的扩展资产
       *@param b-用于减去的扩展资产
       *@返回扩展资产-减去后的新扩展资产
       *@pre两个扩展资产的所有者必须相同
       **/

      friend extended_asset operator - ( const extended_asset& a, const extended_asset& b ) {
         eosio_assert( a.contract == b.contract, "type mismatch" );
         asset r = static_cast<const asset&>(a) - static_cast<const asset&>(b);
         return {r, a.contract};
      }

      /*
       *加法运算符。这会增加扩展资产的金额。
       *
       *@brief加法运算符
       *@param a-要添加的扩展资产
       *@param b-要添加的扩展资产
       *@返回扩展资产-添加后的新扩展资产
       *@pre两个扩展资产的所有者必须相同
       **/

      friend extended_asset operator + ( const extended_asset& a, const extended_asset& b ) {
         eosio_assert( a.contract == b.contract, "type mismatch" );
         asset r = static_cast<const asset&>(a) + static_cast<const asset&>(b);
         return {r, a.contract};
      }

      EOSLIB_SERIALIZE( extended_asset, (amount)(symbol)(contract) )
   };

///}资产类型
} ///命名空间eosio
