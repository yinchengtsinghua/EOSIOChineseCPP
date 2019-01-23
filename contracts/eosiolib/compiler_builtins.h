
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

#pragma once

#include <eosiolib/system.h>

extern "C" {
  /*
   *@defgroup compiler builtins api编译器内置API
   *@InGroup Mathapi公司
   *@brief声明由工具链生成的int128助手内置项。
   *
   *@
   **/


 /*
  *将拆分为两个64位无符号整数的两个128位整数相乘，并将值赋给第一个参数。
  *@brief将两个128位无符号整数相乘（表示为两个64位无符号整数）。
  *@param res它将被结果产品替换。
  *@param la low前128位因子的64位。
  *@param ha前128位因子的高64位。
  *@param lb low第二个128位因子的64位。
  *@param hb第二个128位因子的高64位。
  *@post`res`替换为乘法结果
  *实例：
  *@代码
  *_uuu int128 res=0；
  *在128 A=100；
  *输入128 b=100；
  *多功能3（Res，A，（A>>64），B，（B>>64））；
  *printi128（&res）；//输出：10000
  *@终结码
  **/

  void __multi3(__int128& res, uint64_t la, uint64_t ha, uint64_t lb, uint64_t hb);

 /*
  *将两个128位整数分割为两个64位无符号整数，并将值赋给第一个参数。
  *@brief将两个128位整数（表示为两个64位无符号整数）
  *@param res它将被结果产品替换。
  *@param la low前128位因子的64位。
  *@param ha前128位因子的高64位。
  *@param lb low第二个128位因子的64位。
  *@param hb第二个128位因子的高64位。
  *@post`res`替换为除法结果
  *实例：
  *@代码
  *_uuu int128 res=0；
  *在128 A=100；
  *输入128 b=100；
  *第3部分（资源，A，（A>>64），B，（B>>64））；
  *printi128（&res）；//输出：1
  *@终结码
  **/

  void __divti3(__int128& res, uint64_t la, uint64_t ha, uint64_t lb, uint64_t hb);

 /*
  *将两个128位无符号整数拆分为两个64位无符号整数，并将值赋给第一个参数。
  *@brief将两个128位无符号整数（表示为两个64位无符号整数）
  *@param res它将被结果产品替换。
  *@param la low前128位因子的64位。
  *@param ha前128位因子的高64位。
  *@param lb low第二个128位因子的64位。
  *@param hb第二个128位因子的高64位。
  *实例：
  *@代码
  *无符号uuu int128 res=0；
  *无符号uuu int128 A=100；
  *无符号uuu int128 b=100；
  *音频I3（Res，A，（A>>64），B，（B>>64））；
  *printi128（&res）；//输出：1
  *@终结码
  **/

  void __udivti3(unsigned __int128& res, uint64_t la, uint64_t ha, uint64_t lb, uint64_t hb);

 /*
  *对拆分为两个64位无符号整数的两个128位整数执行模块化运算，并将值赋给第一个参数。
  *@brief对两个128位整数（表示为两个64位无符号整数）执行模块化算法
  *@param res它将被结果产品替换。
  *@param la low前128位因子的64位。
  *@param ha前128位因子的高64位。
  *@param lb low第二个128位因子的64位。
  *@param hb第二个128位因子的高64位。
  *@post`res`替换为模数的结果
  *实例：
  *@代码
  *_uuu int128 res=0；
  *在128 A=100；
  *输入128 b=3；
  *uu modti3（分辨率，A，（A>>64），B，（B>>64））；
  *printi128（&res）；//输出：1
  *@终结码
  **/

  void __modti3(__int128& res, uint64_t la, uint64_t ha, uint64_t lb, uint64_t hb);

 /*
  *对拆分为两个64位无符号整数的两个128位无符号整数执行模块化运算，并将值赋给第一个参数。
  *@brief对两个128位无符号整数（表示为两个64位无符号整数）执行模块化算法
  *@param res它将被结果产品替换。
  *@param la low前128位因子的64位。
  *@param ha前128位因子的高64位。
  *@param lb low第二个128位因子的64位。
  *@param hb第二个128位因子的高64位。
  *@post`res`替换为模数的结果
  *实例：
  *@代码
  *无符号uuu int128 res=0；
  *无符号uuu int128 A=100；
  *无符号uuu int128 b=3；
  *Umoudti3（资源，A，（A>>64），B，（B>>64））；
  *printi128（&res）；//输出：1
  *@终结码
  **/

  void __umodti3(unsigned __int128& res, uint64_t la, uint64_t ha, uint64_t lb, uint64_t hb);

/*
  *对128位整数执行逻辑左移，拆分为两个64位无符号整数，并将值赋给第一个参数。
  *@brief对128位整数（表示为两个64位无符号整数）执行逻辑左移。
  *@param res它将被结果产品替换。
  *@param lo low 128位因子的64位。
  *@param hi 128位因子的64位高位。
  *@param shift要移位的位数。
  *@post`res`替换为操作结果
  *实例：
  *@代码
  *_uuu int128 res=0；
  *在128 A=8；
  *uu lshlti3（资源，A，（A>>64），1）；
  *printi128（&res）；//输出：16
  *@终结码
  **/

  void __lshlti3(__int128& res, uint64_t lo, uint64_t hi, uint32_t shift);

 /*
  *对128位整数执行逻辑右移，拆分为两个64位无符号整数，并将值赋给第一个参数。
  *@brief对128位整数（表示为两个64位无符号整数）执行逻辑右移。
  *@param res它将被结果产品替换。
  *@param lo low 128位因子的64位。
  *@param hi 128位因子的64位高位。
  *@param shift要移位的位数。
  *@post`res`替换为操作结果
  *实例：
  *@代码
  *_uuu int128 res=0；
  *在128 A=8；
  *Lshrti3（Res，A，（A>>64），1）；
  *printi128（&res）；//输出：4
  *@终结码
  **/

  void __lshrti3(__int128& res, uint64_t lo, uint64_t hi, uint32_t shift);

/*
  *对一个128位整数执行算术左移，拆分为两个64位无符号整数，并将值赋给第一个参数。
  *@brief对128位整数（表示为两个64位无符号整数）执行左移位运算
  *@param res它将被结果产品替换。
  *@param lo low 128位因子的64位。
  *@param hi 128位因子的64位高位。
  *@param shift要移位的位数。
  *@post`res`替换为操作结果
  *实例：
  *@代码
  *_uuu int128 res=0；
  *在128 A=8；
  *uu ashti3（Res，A，（A>>64），1）；
  *printi128（&res）；//输出：16
  *@终结码
  **/

  void __ashlti3(__int128& res, uint64_t lo, uint64_t hi, uint32_t shift);

 /*
  *对128位整数执行算术右移，拆分为两个64位无符号整数，并将值赋给第一个参数。
  *@brief对128位整数（表示为两个64位无符号整数）执行算术右移。
  *@param res它将被结果产品替换。
  *@param lo low 128位因子的64位。
  *@param hi 128位因子的64位高位。
  *@param shift要移位的位数。
  *@post`res`替换为操作结果
  *实例：
  *@代码
  *_uuu int128 res=0；
  *输入128 A=-8；
  *Uuu Asharti3（Res，A，（A>>64），1）；
  *printi128（&res）；//输出：-4
  *@终结码
  **/

  void __ashrti3(__int128& res, uint64_t lo, uint64_t hi, uint32_t shift);

 /*
  *将两个长双精度拆分为两个64位无符号整数，并将值赋给第一个参数。
  *@brief添加两个长双精度数（表示为两个64位无符号整数）
  *@param ret它将被结果产品替换。
  *@param la low前128位因子的64位。
  *@param ha前128位因子的高64位。
  *@param lb low第二个128位因子的64位。
  *@param hb第二个128位因子的高64位。
  *@post`ret`已替换为操作结果
  **/

  void __addtf3( long double& ret, uint64_t la, uint64_t ha, uint64_t lb, uint64_t hb );

 /*
  *减去两个长双精度拆分为两个64位无符号整数，并将值赋给第一个参数。
  *@brief减去两个长双精度数（表示为两个64位无符号整数）
  *@param ret它将被结果产品替换。
  *@param la low前128位因子的64位。
  *@param ha前128位因子的高64位。
  *@param lb low第二个128位因子的64位。
  *@param hb第二个128位因子的高64位。
  *@post`ret`已替换为操作结果
  **/

  void __subtf3( long double& ret, uint64_t la, uint64_t ha, uint64_t lb, uint64_t hb ); 

 /*
  *将两个长双精度拆分为两个64位无符号整数，并将值赋给第一个参数。
  *@brief乘两个长双精度数（表示为两个64位无符号整数）
  *@param ret它将被结果产品替换。
  *@param la low前128位因子的64位。
  *@param ha前128位因子的高64位。
  *@param lb low第二个128位因子的64位。
  *@param hb第二个128位因子的高64位。
  *@post`ret`已替换为操作结果
  **/

  void __multf3( long double& ret, uint64_t la, uint64_t ha, uint64_t lb, uint64_t hb ); 

 /*
  *将两个长双精度拆分为两个64位无符号整数，并将值赋给第一个参数。
  *@brief除以两个长双精度数（表示为两个64位无符号整数）
  *@param ret它将被结果产品替换。
  *@param la low前128位因子的64位。
  *@param ha前128位因子的高64位。
  *@param lb low第二个128位因子的64位。
  *@param hb第二个128位因子的高64位。
  *@post`ret`已替换为操作结果
  **/

  void __divtf3( long double& ret, uint64_t la, uint64_t ha, uint64_t lb, uint64_t hb ); 

 /*
  *检查两个双精度拆分为两个64位无符号整数之间的相等性
  *@brief检查两个双精度数（表示为两个64位无符号整数）之间的相等性
  *@param la low前128位因子的64位。
  *@param ha前128位因子的高64位。
  *@param lb low第二个128位因子的64位。
  *@param hb第二个128位因子的高64位。
  *@如果a大于b返回1
  *@如果a等于b返回0
  *@返回-1如果a小于b
  *@如果a或b为NaN，返回1
  **/

  int __eqtf2( uint64_t la, uint64_t ha, uint64_t lb, uint64_t hb ); 

 /*
  *检查两个双精度拆分为两个64位无符号整数之间的不等式
  *@brief check两个双精度数之间的不等式（表示为两个64位无符号整数）
  *@param la low前128位因子的64位。
  *@param ha前128位因子的高64位。
  *@param lb low第二个128位因子的64位。
  *@param hb第二个128位因子的高64位。
  *@如果a大于b返回1
  *@如果a等于b返回0
  *@返回-1如果a小于b
  *@return1如果a或b是NaN
  **/

  int __netf2( uint64_t la, uint64_t ha, uint64_t lb, uint64_t hb ); 


 /*
  *检查第一个双精度数是否大于或等于第二个双精度数，将双精度数拆分为两个64位无符号整数。
  *@brief检查第一个双精度数是否大于或等于第二个双精度数（双精度数表示为两个64位无符号整数）
  *@param la low前128位因子的64位。
  *@param ha前128位因子的高64位。
  *@param lb low第二个128位因子的64位。
  *@param hb第二个128位因子的高64位。
  *@如果a大于b返回1
  *@如果a等于b返回0
  *@返回-1如果a小于b
  *@返回-1，如果a或b为NaN
  **/

  int __getf2( uint64_t la, uint64_t ha, uint64_t lb, uint64_t hb ); 

 /*
  *检查第一个双精度数是否大于第二个双精度数，将双精度数拆分为两个64位无符号整数。
  *@brief检查第一个双精度数是否大于第二个双精度数（双精度数表示为两个64位无符号整数）
  *@param la low前128位因子的64位。
  *@param ha前128位因子的高64位。
  *@param lb low第二个128位因子的64位。
  *@param hb第二个128位因子的高64位。
  *@如果a大于b返回1
  *@如果a等于b返回0
  *@返回-1如果a小于b
  *@如果a或b为NaN，则返回0
  **/

  int __gttf2( uint64_t la, uint64_t ha, uint64_t lb, uint64_t hb ); 

 /*
  *检查第一个双精度数是否小于或等于第二个双精度数，将双精度数拆分为两个64位无符号整数。
  *@brief检查第一个双精度数是否小于或等于第二个双精度数（双精度数表示为两个64位无符号整数）
  *@param la low前128位因子的64位。
  *@param ha前128位因子的高64位。
  *@param lb low第二个128位因子的64位。
  *@param hb第二个128位因子的高64位。
  *@如果a大于b返回1
  *@如果a等于b返回0
  *@返回-1如果a小于b
  *@如果a或b为NaN，返回1
  **/

  int __letf2( uint64_t la, uint64_t ha, uint64_t lb, uint64_t hb ); 

 /*
  *检查第一个双精度数是否小于第二个双精度数，将双精度数拆分为两个64位无符号整数。
  *@brief检查第一个双精度数是否小于第二个双精度数（双精度数表示为两个64位无符号整数）
  *@param la low前128位因子的64位。
  *@param ha前128位因子的高64位。
  *@param lb low第二个128位因子的64位。
  *@param hb第二个128位因子的高64位。
  *@如果a大于b返回1
  *@如果a等于b返回0
  *@返回-1如果a小于b
  *@如果a或b为NaN，则返回0
  **/

  int __lttf2( uint64_t la, uint64_t ha, uint64_t lb, uint64_t hb ); 

 /*
  *比较两个拆分为两个64位无符号整数的双精度数。
  *@brief比较两个双精度数（双精度数表示为两个64位无符号整数）
  *@param la low前128位因子的64位。
  *@param ha前128位因子的高64位。
  *@param lb low第二个128位因子的64位。
  *@param hb第二个128位因子的高64位。
  *@如果a大于b返回1
  *@如果a等于b返回0
  *@返回-1如果a小于b
  *@如果a或b为NaN，返回1
  **/

  int __cmptf2( uint64_t la, uint64_t ha, uint64_t lb, uint64_t hb ); 

 /*
  *检查其中一个双精度数是否为NaN，则将双精度数拆分为两个64位无符号整数。
  *@brief检查其中一个双精度数是否为NaN，（双精度数表示为两个64位无符号整数）
  *@param la low前128位因子的64位。
  *@param ha前128位因子的高64位。
  *@param lb low第二个128位因子的64位。
  *@param hb第二个128位因子的高64位。
  *@如果a或b为NaN，返回1
  *@如果a或b不是NaN，返回0
  **/

  int __unordtf2( uint64_t la, uint64_t ha, uint64_t lb, uint64_t hb ); 

 /*
  *将float扩展到long double
  *@brief将float扩展到long double
  *@param ret它将被结果产品替换。
  *@param f要扩展的输入浮点
  *@post`ret`替换为扩展浮点
  **/

  void __extendsftf2( long double& ret, float f ); 

 /*
  *将double扩展到long double
  *@brief将float扩展到long double
  *@param ret它将被结果产品替换。
  *@param f要扩展的输入浮点
  *@post`ret`替换为扩展浮点
  **/

  void __extenddftf2( long double& ret, double f ); 

 /*
  *将long double（拆分为两个64位无符号整数）转换为64位整数
  *@brief将long double（拆分为两个64位无符号整数）转换为64位整数
  *@param l前128位因子的64位低位。
  *@param h前128位因子的高64位。
  *@返回转换后的64位整数。
  **/

  int64_t __fixtfdi( uint64_t l, uint64_t h ); 

 /*
  *将long double（拆分为两个64位无符号整数）转换为32位整数
  *@brief将long double（拆分为两个64位无符号整数）转换为32位整数
  *@param l前128位因子的64位低位。
  *@param h前128位因子的高64位。
  *@返回转换后的32位整数。
  **/

  int32_t __fixtfsi( uint64_t l, uint64_t h ); 

 /*
  *将long double（拆分为两个64位无符号整数）转换为64位无符号整数。
  *@brief将long double（拆分为两个64位无符号整数）转换为64位无符号整数
  *@param l前128位因子的64位低位。
  *@param h前128位因子的高64位。
  *@返回转换后的64位无符号整数。
  **/

  uint64_t __fixunstfdi( uint64_t l, uint64_t h ); 

 /*
  *将长双精度（拆分为两个64位无符号整数）转换为32位无符号整数
  *@brief将long double（拆分为两个64位无符号整数）转换为32位无符号整数
  *@param l前128位因子的64位低位。
  *@param h前128位因子的高64位。
  *@返回转换后的32位无符号整数。
  **/

  uint32_t __fixunstfsi( uint64_t l, uint64_t h ); 

 /*
  *将long double（拆分为两个64位无符号整数）截断为double
  *@brief将long double（拆分为两个64位无符号整数）转换为double
  *@param l前128位因子的64位低位。
  *@param h前128位因子的高64位。
  *@返回转换后的双精度值
  **/

  double __trunctfdf2( uint64_t l, uint64_t h ); 

 /*
  *将long double（拆分为两个64位无符号整数）截断为float
  *@brief将long double（拆分为两个64位无符号整数）转换为float
  *@param l前128位因子的64位低位。
  *@param h前128位因子的高64位。
  *@返回转换后的浮点值
  **/

  float __trunctfsf2( uint64_t l, uint64_t h ); 

  void __break_point();

///@
} //外部“C”
