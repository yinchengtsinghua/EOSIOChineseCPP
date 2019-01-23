
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
/*==--fixunssfti.c-实现uu fixunssfti-------------------------------
 *
 *LLVM编译器基础结构
 *
 *此文件是麻省理工学院和伊利诺伊大学公开赛的双重许可文件。
 *源许可证。有关详细信息，请参阅license.txt。
 *
 *==-------------------------------------------------------------------------------------==
 *
 *此文件实现编译器RT库的\uuu fixunssfti。
 *
 *==-------------------------------------------------------------------------------------==
 **/


#include "fp32.h"

typedef unsigned __int128 fixuint_t;

fixuint_t ___fixunssfti(uint32_t a) {
//将A分解为符号、指数、有效位
    const rep_t aRep = a;
    const rep_t aAbs = aRep & absMask;
    const int sign = aRep & signBit ? -1 : 1;
    const int exponent = (aAbs >> significandBits) - exponentBias;
    const rep_t significand = (aAbs & significandMask) | implicitBit;

//如果值或指数为负，则结果为零。
    if (sign == -1 || exponent < 0)
        return 0;

//如果该值对于整数类型太大，则饱和。
    if ((unsigned)exponent >= sizeof(fixuint_t) * CHAR_BIT)
        return ~(fixuint_t)0;

//如果0<=指数<有效位，则右移以获得结果。
//否则，向左移动。
    if (exponent < significandBits)
        return significand >> (significandBits - exponent);
    else
        return (fixuint_t)significand << (exponent - significandBits);
}
