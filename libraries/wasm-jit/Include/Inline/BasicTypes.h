
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once
#include <cstdint>
#include <cstddef>

typedef uint8_t U8;
typedef int8_t I8;
typedef uint16_t U16;
typedef int16_t I16;
typedef uint32_t U32;
typedef int32_t I32;
typedef uint64_t U64;
typedef int64_t I64;
typedef float F32;
typedef double F64;

//osx libc将uintptr_t定义为一个long，其中u32/u64为int。这将导致uintptr_t/uint64被视为不同的类型，例如重载。
//通过定义我们自己的总是int类型的uptr/iptr来解决这个问题。
template<size_t pointerSize>
struct PointerIntHelper;
template<> struct PointerIntHelper<4> { typedef I32 IntType; typedef U32 UnsignedIntType; };
template<> struct PointerIntHelper<8> { typedef I64 IntType; typedef U64 UnsignedIntType; };
typedef PointerIntHelper<sizeof(size_t)>::UnsignedIntType Uptr;
typedef PointerIntHelper<sizeof(size_t)>::IntType Iptr;
