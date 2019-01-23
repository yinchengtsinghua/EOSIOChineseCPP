
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
/*
*xxhash-快速哈希算法
*版权所有（c）2012-2016，Yann Collet
*
*BSD 2条款许可证（http://www.opensource.org/licenses/bsd-license.php）
*
*以源和二进制形式重新分配和使用，有或无
*允许修改，前提是以下条件
*相遇：
*
**重新发布源代码必须保留上述版权。
*注意，此条件列表和以下免责声明。
**二进制形式的再分配必须复制上述内容
*版权声明、此条件列表和以下免责声明
*在随附的文件和/或其他材料中，
*分布。
*
*本软件由版权所有者和贡献者提供。
*“原样”和任何明示或暗示的保证，包括但不包括
*仅限于对适销性和适用性的暗示保证
*不承认特定目的。在任何情况下，版权
*所有人或出资人对任何直接、间接、附带的，
*特殊、惩戒性或后果性损害（包括但不包括
*仅限于采购替代货物或服务；使用损失，
*数据或利润；或业务中断），无论是何种原因
*责任理论，无论是合同责任、严格责任还是侵权责任。
（包括疏忽或其他）因使用不当而引起的
*关于本软件，即使已告知此类损坏的可能性。
*
*您可以通过以下方式联系作者：
*-xxash主页：http://www.xxash.com
*-xxash源存储库：https://github.com/cyan4973/xxash
**/



/*********************************
*调谐参数
*********************************/

/*xxh-强制存储器访问：
 *默认情况下，对未对齐的内存的访问由“memcpy（）”控制，这是安全的和可移植的。
 *不幸的是，在某些目标/编译器组合中，生成的程序集是次优的。
 *下面的开关允许选择不同的访问方法以提高性能。
 *方法0（默认）：使用“memcpy（）”。安全便携。
 *方法1：“打包”语句。它取决于编译器扩展（即，不可移植）。
 *如果编译器支持此方法，则此方法是安全的，*通常*与“memcpy”一样快或更快。
 *方法2：直接访问。此方法不依赖于编译器，但违反了C标准。
 *它可以在不支持未对齐内存访问的目标上生成错误代码。
 *但在某些情况下，这是获得最佳性能的唯一已知方法（即GCC+ARMV6）
 *有关详细信息，请参阅http://stackoverflow.com/a/32095106/646947。
 *优先选择这些方法（0>1>2）
 **/

/*ndef xxh_force_memory_access/*可以在外部定义，例如在命令行上*/

定义XXH_强制_内存_访问2
elif defined（uuu intel_compiler）
  （定义（uu-gnuc_uuu）&（定义（u-arm_-arch_7_uuu）定义（u-arm_-arch_7a_uuuuuu）定义（u-arm_u-arch_7r_uuuuu）定义（u-arm_u-arch_7s_uuu）））
定义XXH_强制_内存_访问1
第二节
第二节

/*！xxh接受空输入指针：
 *如果输入指针为空指针，XXHASH默认行为是触发内存访问错误，因为它是一个错误的指针。
 *启用此选项时，空输入指针的XXHASH输出将与空长度输入相同。
 *默认情况下，此选项被禁用。要启用它，请取消下面的注释定义：
 **/

/*定义xxh_接受_空_输入_指针1*/

/*xxh_force_native_格式：
 *默认情况下，XXhash库根据little endian约定提供与endian无关的哈希值。
 *因此，小端和大端CPU的结果是相同的。
 *对于big-endian CPU来说，这是一个性能代价，因为需要进行一些交换来模拟小endian格式。
 *如果endian独立性对您的应用程序不重要，您可以将下面的定义设置为1，
 *提高big endian CPU的速度。
 *此选项对小型CPU没有影响。
 **/

/*ndef xxh_force_native_format/*可从外部定义*/
定义XXH_强制_本机_格式0
第二节

/*！xxh强制对齐检查：
 *这是一个小的性能技巧，只对很多非常小的键有用。
 *表示：检查输入是否对齐/未对齐。
 *检查每哈希花费一个初始分支；
 *当输入保证对齐时，将其设置为0，
 *或者当对齐对性能不重要时。
 **/

/*ndef xxh_force_align_check/*可从外部定义*/
如果定义（i386）定义（iX86）定义（x86）定义（x64定义（mX64）
定义xxh_力_对齐_检查0
否则
定义xxh_力_对齐_检查1
第二节
第二节


/*******************************************
*包括与内存相关的功能
*********************************/

/*如果要使用其他内存例程，请修改下面的本地函数
*对于malloc（），free（）。*/

#include <stdlib.h>
static void* XXH_malloc(size_t s) { return malloc(s); }
static void  XXH_free  (void* p)  { free(p); }
/*对于memcpy（）。*/
#include <string.h>
static void* XXH_memcpy(void* dest, const void* src, size_t size) { return memcpy(dest,src,size); }

#define XXH_STATIC_LINKING_ONLY
#include "xxhash.h"


/*********************************
*编译器特定选项
*********************************/

/*def _msc_ver/*Visual Studio*/
pragma warning（disable:4127）/*disable:c4127:条件表达式为常量*/

#  define FORCE_INLINE static __forceinline
#else
/*如果定义（uu cplusplus）定义（u stdc_version_uuuuu）&&u stdc_version_uuuuu>=199901l/*c99*/
ifdef_uu gnuc_uu
定义力u inline静态inline uu属性uuu（（始终inline））。
否则
定义力内联静态内联
第二节
否则
定义力_内联静态
endif/*uu stdc_版本*/

#endif


/*********************************
＊基本类型
*********************************/

#ifndef MEM_MODULE
/*F！defined（uuvms）&&（defined（uu cplusplus）（defined（u stdc_u version_uuuuuu）&（u stdc_u version_uuuuu>=199901l）/*c99*/）
include<stdint.h>
    typedef uint8_t字节；
    typedef uint16_t u16；
    typedef uint32_t u32；
否则
    typedef无符号字符字节；
    typedef无符号短型U16；
    typedef无符号int u32；
第二节
第二节

如果（定义为（xxh_force_memory_access）&&（xh_force_memory_access==2））

/*强制直接访问内存。仅在支持硬件中未对齐内存访问的CPU上工作*/

static U32 XXH_read32(const void* memPtr) { return *(const U32*) memPtr; }

#elif (defined(XXH_FORCE_MEMORY_ACCESS) && (XXH_FORCE_MEMORY_ACCESS==1))

/*_uuu pack指令更安全，但特定于编译器，因此对某些编译器来说可能存在问题。*/
/*目前仅为GCC和ICC定义*/
typedef union { U32 u32; } __attribute__((packed)) unalign;
static U32 XXH_read32(const void* ptr) { return ((const unalign*)ptr)->u32; }

#else

/*便携式安全解决方案。效率一般。
 *请参阅：http://stackoverflow.com/a/32095106/646947
 **/

static U32 XXH_read32(const void* memPtr)
{
    U32 val;
    memcpy(&val, memPtr, sizeof(val));
    return val;
}

/*dif/*xxh_force_direct_memory_access*/


/************************************************
*编译器特定的函数和宏
**************************************/

#define XXH_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)

/*注意：尽管mingw存在rotl（windows下的gcc），但性能似乎很差。*/
#if defined(_MSC_VER)
#  define XXH_rotl32(x,r) _rotl(x,r)
#  define XXH_rotl64(x,r) _rotl64(x,r)
#else
#  define XXH_rotl32(x,r) ((x << r) | (x >> (32 - r)))
#  define XXH_rotl64(x,r) ((x << r) | (x >> (64 - r)))
#endif

/*已定义（_msc_ver）/*Visual Studio*/
定义xxh_swap32_byteswap_ulong
elif xh_gcc_版本>=403
定义XXHUSWAP32UUUU内置USWAP32
否则
静态U32 XXHU SWAP32（U32 X）
{
    返回（（x<24）&0xff000000）
            （x<<8）&0x00ff0000）
            （（X>>8）&0x0000 ff00）
            （X>>24）&0x000000FF；
}
第二节


/*******************************************
*体系结构宏
*********************************/

typedef enum { XXH_bigEndian=0, XXH_littleEndian=1 } XXH_endianess;

/*xxh-cpu-little-endian可以在外部定义，例如在编译器命令行上。*/
#ifndef XXH_CPU_LITTLE_ENDIAN
    static const int g_one = 1;
#   define XXH_CPU_LITTLE_ENDIAN   (*(const char*)(&g_one))
#endif


/***********************
*内存读取
***********************/

typedef enum { XXH_aligned, XXH_unaligned } XXH_alignment;

FORCE_INLINE U32 XXH_readLE32_align(const void* ptr, XXH_endianess endian, XXH_alignment align)
{
    if (align==XXH_unaligned)
        return endian==XXH_littleEndian ? XXH_read32(ptr) : XXH_swap32(XXH_read32(ptr));
    else
        return endian==XXH_littleEndian ? *(const U32*)ptr : XXH_swap32(*(const U32*)ptr);
}

FORCE_INLINE U32 XXH_readLE32(const void* ptr, XXH_endianess endian)
{
    return XXH_readLE32_align(ptr, endian, XXH_unaligned);
}

static U32 XXH_readBE32(const void* ptr)
{
    return XXH_CPU_LITTLE_ENDIAN ? XXH_swap32(XXH_read32(ptr)) : XXH_read32(ptr);
}


/*********************************
*宏指令
*********************************/

/*罚款xxh_static_assert（c）枚举xh_static_assert=1/（int）（！！（c））/*仅在*变量声明后使用*号*/
xxh_public_api unsigned xh_version number（void）返回xh_version_number；


/***************************************************************
*32位哈希函数
***************************************************************/

static const U32 PRIME32_1 = 2654435761U;
static const U32 PRIME32_2 = 2246822519U;
static const U32 PRIME32_3 = 3266489917U;
static const U32 PRIME32_4 =  668265263U;
static const U32 PRIME32_5 =  374761393U;

static U32 XXH32_round(U32 seed, U32 input)
{
    seed += input * PRIME32_2;
    seed  = XXH_rotl32(seed, 13);
    seed *= PRIME32_1;
    return seed;
}

FORCE_INLINE U32 XXH32_endian_align(const void* input, size_t len, U32 seed, XXH_endianess endian, XXH_alignment align)
{
    const BYTE* p = (const BYTE*)input;
    const BYTE* bEnd = p + len;
    U32 h32;
#define XXH_get32bits(p) XXH_readLE32_align(p, endian, align)

#ifdef XXH_ACCEPT_NULL_INPUT_POINTER
    if (p==NULL) {
        len=0;
        bEnd=p=(const BYTE*)(size_t)16;
    }
#endif

    if (len>=16) {
        const BYTE* const limit = bEnd - 16;
        U32 v1 = seed + PRIME32_1 + PRIME32_2;
        U32 v2 = seed + PRIME32_2;
        U32 v3 = seed + 0;
        U32 v4 = seed - PRIME32_1;

        do {
            v1 = XXH32_round(v1, XXH_get32bits(p)); p+=4;
            v2 = XXH32_round(v2, XXH_get32bits(p)); p+=4;
            v3 = XXH32_round(v3, XXH_get32bits(p)); p+=4;
            v4 = XXH32_round(v4, XXH_get32bits(p)); p+=4;
        } while (p<=limit);

        h32 = XXH_rotl32(v1, 1) + XXH_rotl32(v2, 7) + XXH_rotl32(v3, 12) + XXH_rotl32(v4, 18);
    } else {
        h32  = seed + PRIME32_5;
    }

    h32 += (U32) len;

    while (p+4<=bEnd) {
        h32 += XXH_get32bits(p) * PRIME32_3;
        h32  = XXH_rotl32(h32, 17) * PRIME32_4 ;
        p+=4;
    }

    while (p<bEnd) {
        h32 += (*p) * PRIME32_5;
        h32 = XXH_rotl32(h32, 11) * PRIME32_1 ;
        p++;
    }

    h32 ^= h32 >> 15;
    h32 *= PRIME32_2;
    h32 ^= h32 >> 13;
    h32 *= PRIME32_3;
    h32 ^= h32 >> 16;

    return h32;
}


XXH_PUBLIC_API unsigned int XXH32 (const void* input, size_t len, unsigned int seed)
{
#if 0
    /*简单的版本，很好的代码维护，但不幸的是对于小的输入速度很慢*/
    XXH32_state_t state;
    XXH32_reset(&state, seed);
    XXH32_update(&state, input, len);
    return XXH32_digest(&state);
#else
    XXH_endianess endian_detected = (XXH_endianess)XXH_CPU_LITTLE_ENDIAN;

    if (XXH_FORCE_ALIGN_CHECK) {
        /*（（（大小）输入）&3）==0）/*输入为4字节对齐，利用速度优势*/
            if（（endian_detected==xxh_littleendian）xh_force_native_格式）
                返回xxh32撏endian_-align（input，len，seed，xxh撏littleendian，xxh_-aligned）；
            其他的
                返回xxh32撊endian撊align（input，len，seed，xxh撊bigendian，xxh撊aligned）；
    }

    if（（endian_detected==xxh_littleendian）xh_force_native_格式）
        返回xxh32撊endian_-align（input，len，seed，xxh撊littleendian，xxh_unaligned）；
    其他的
        返回xxh32撊endian撊align（input，len，seed，xxh撊bigendian，xxh撊unaligned）；
第二节
}



/*=====哈希流===*/


XXH_PUBLIC_API XXH32_state_t* XXH32_createState(void)
{
    return (XXH32_state_t*)XXH_malloc(sizeof(XXH32_state_t));
}
XXH_PUBLIC_API XXH_errorcode XXH32_freeState(XXH32_state_t* statePtr)
{
    XXH_free(statePtr);
    return XXH_OK;
}

XXH_PUBLIC_API void XXH32_copyState(XXH32_state_t* dstState, const XXH32_state_t* srcState)
{
    memcpy(dstState, srcState, sizeof(*dstState));
}

XXH_PUBLIC_API XXH_errorcode XXH32_reset(XXH32_state_t* statePtr, unsigned int seed)
{
    /*32_state_t state；/*使用本地状态to memcpy（），以避免出现严格的别名警告*/
    memset（&state，0，sizeof（state）-4）；/*不要写入reserved，以便将来删除*/

    state.v1 = seed + PRIME32_1 + PRIME32_2;
    state.v2 = seed + PRIME32_2;
    state.v3 = seed + 0;
    state.v4 = seed - PRIME32_1;
    memcpy(statePtr, &state, sizeof(state));
    return XXH_OK;
}


FORCE_INLINE XXH_errorcode XXH32_update_endian (XXH32_state_t* state, const void* input, size_t len, XXH_endianess endian)
{
    const BYTE* p = (const BYTE*)input;
    const BYTE* const bEnd = p + len;

#ifdef XXH_ACCEPT_NULL_INPUT_POINTER
    if (input==NULL) return XXH_ERROR;
#endif

    state->total_len_32 += (unsigned)len;
    state->large_len |= (len>=16) | (state->total_len_32>=16);

    /*（state->memsize+len<16）/*填充tmp缓冲区*/
        xxh_memcpy（（byte*）（state->mem32）+state->memsize，input，len）；
        state->memsize+=（无符号）len；
        返回；
    }

    if（state->memsize）/*上次更新留下的一些数据*/

        XXH_memcpy((BYTE*)(state->mem32) + state->memsize, input, 16-state->memsize);
        {   const U32* p32 = state->mem32;
            state->v1 = XXH32_round(state->v1, XXH_readLE32(p32, endian)); p32++;
            state->v2 = XXH32_round(state->v2, XXH_readLE32(p32, endian)); p32++;
            state->v3 = XXH32_round(state->v3, XXH_readLE32(p32, endian)); p32++;
            state->v4 = XXH32_round(state->v4, XXH_readLE32(p32, endian));
        }
        p += 16-state->memsize;
        state->memsize = 0;
    }

    if (p <= bEnd-16) {
        const BYTE* const limit = bEnd - 16;
        U32 v1 = state->v1;
        U32 v2 = state->v2;
        U32 v3 = state->v3;
        U32 v4 = state->v4;

        do {
            v1 = XXH32_round(v1, XXH_readLE32(p, endian)); p+=4;
            v2 = XXH32_round(v2, XXH_readLE32(p, endian)); p+=4;
            v3 = XXH32_round(v3, XXH_readLE32(p, endian)); p+=4;
            v4 = XXH32_round(v4, XXH_readLE32(p, endian)); p+=4;
        } while (p<=limit);

        state->v1 = v1;
        state->v2 = v2;
        state->v3 = v3;
        state->v4 = v4;
    }

    if (p < bEnd) {
        XXH_memcpy(state->mem32, p, (size_t)(bEnd-p));
        state->memsize = (unsigned)(bEnd-p);
    }

    return XXH_OK;
}

XXH_PUBLIC_API XXH_errorcode XXH32_update (XXH32_state_t* state_in, const void* input, size_t len)
{
    XXH_endianess endian_detected = (XXH_endianess)XXH_CPU_LITTLE_ENDIAN;

    if ((endian_detected==XXH_littleEndian) || XXH_FORCE_NATIVE_FORMAT)
        return XXH32_update_endian(state_in, input, len, XXH_littleEndian);
    else
        return XXH32_update_endian(state_in, input, len, XXH_bigEndian);
}



FORCE_INLINE U32 XXH32_digest_endian (const XXH32_state_t* state, XXH_endianess endian)
{
    const BYTE * p = (const BYTE*)state->mem32;
    const BYTE* const bEnd = (const BYTE*)(state->mem32) + state->memsize;
    U32 h32;

    if (state->large_len) {
        h32 = XXH_rotl32(state->v1, 1) + XXH_rotl32(state->v2, 7) + XXH_rotl32(state->v3, 12) + XXH_rotl32(state->v4, 18);
    } else {
        /*=state->v3/*==seed*/+prime32_5；
    }

    h32+=状态->总长度_32；

    同时（P+4<=弯曲）
        h32+=xxh_readle32（p，endian）*prime32_3；
        h32=xxh_rotl32（h32，17）*prime32_4；
        p+＝4；
    }

    同时（p<bend）
        H32+=（*P）*Prime32_5；
        h32=xxh_rotl32（h32，11）*prime32_1；
        P+；
    }

    H32^=H32>>15；
    h32*=prime32_2；
    H32^=H32>>13；
    h32*=prime32_3；
    H32^=H32>>16；

    返回H32；
}


xxh_public_api unsigned in t xxh32_digest（const xh32_state_t*state_in）
{
    检测到的xh端=（xh端端）xh CPU端；

    if（（endian_detected==xxh_littleendian）xh_force_native_格式）
        返回XXH32_Digest_Endian（State_In，XXH_Littleendian）；
    其他的
        返回xxh32_digestou endian（stateou in，xxh_bigendian）；
}


/*=====规范表示法===*/


/*默认的XXH结果类型是基本的无符号32和64位。
*规范表示遵循人类可读的写入约定，即大尾数法（大数字优先）。
*这些函数允许将哈希结果转换为其规范格式。
*这样，可以将哈希值写入文件或缓冲区，并在不同的系统和程序之间保持可比较性。
**/


XXH_PUBLIC_API void XXH32_canonicalFromHash(XXH32_canonical_t* dst, XXH32_hash_t hash)
{
    XXH_STATIC_ASSERT(sizeof(XXH32_canonical_t) == sizeof(XXH32_hash_t));
    if (XXH_CPU_LITTLE_ENDIAN) hash = XXH_swap32(hash);
    memcpy(dst, &hash, sizeof(*dst));
}

XXH_PUBLIC_API XXH32_hash_t XXH32_hashFromCanonical(const XXH32_canonical_t* src)
{
    return XXH_readBE32(src);
}


#ifndef XXH_NO_LONG_LONG

/***************************************************************
*64位哈希函数
***************************************************************/


/*=====内存访问===*/

#ifndef MEM_MODULE
# define MEM_MODULE
/*F！defined（uuvms）&&（defined（uu cplusplus）（defined（u stdc_u version_uuuuuu）&（u stdc_u version_uuuuu>=199901l）/*c99*/）
include<stdint.h>
    typedef uint64_t u64；
否则
    typedef unsigned long long u64；/*如果编译器不支持无符号long long，请在此处替换为另一个64位类型。请注意，xxash.h也需要更新。*/

# endif
#endif


#if (defined(XXH_FORCE_MEMORY_ACCESS) && (XXH_FORCE_MEMORY_ACCESS==2))

/*强制直接访问内存。仅在支持硬件中未对齐内存访问的CPU上工作*/
static U64 XXH_read64(const void* memPtr) { return *(const U64*) memPtr; }

#elif (defined(XXH_FORCE_MEMORY_ACCESS) && (XXH_FORCE_MEMORY_ACCESS==1))

/*_uuu pack指令更安全，但特定于编译器，因此对某些编译器来说可能存在问题。*/
/*目前仅为GCC和ICC定义*/
typedef union { U32 u32; U64 u64; } __attribute__((packed)) unalign64;
static U64 XXH_read64(const void* ptr) { return ((const unalign64*)ptr)->u64; }

#else

/*便携式安全解决方案。效率一般。
 *请参阅：http://stackoverflow.com/a/32095106/646947
 **/


static U64 XXH_read64(const void* memPtr)
{
    U64 val;
    memcpy(&val, memPtr, sizeof(val));
    return val;
}

/*dif/*xxh_force_direct_memory_access*/

如果定义了（35u msc35u ver）/*Visual Studio*/

#  define XXH_swap64 _byteswap_uint64
#elif XXH_GCC_VERSION >= 403
#  define XXH_swap64 __builtin_bswap64
#else
static U64 XXH_swap64 (U64 x)
{
    return  ((x << 56) & 0xff00000000000000ULL) |
            ((x << 40) & 0x00ff000000000000ULL) |
            ((x << 24) & 0x0000ff0000000000ULL) |
            ((x << 8)  & 0x000000ff00000000ULL) |
            ((x >> 8)  & 0x00000000ff000000ULL) |
            ((x >> 24) & 0x0000000000ff0000ULL) |
            ((x >> 40) & 0x000000000000ff00ULL) |
            ((x >> 56) & 0x00000000000000ffULL);
}
#endif

FORCE_INLINE U64 XXH_readLE64_align(const void* ptr, XXH_endianess endian, XXH_alignment align)
{
    if (align==XXH_unaligned)
        return endian==XXH_littleEndian ? XXH_read64(ptr) : XXH_swap64(XXH_read64(ptr));
    else
        return endian==XXH_littleEndian ? *(const U64*)ptr : XXH_swap64(*(const U64*)ptr);
}

FORCE_INLINE U64 XXH_readLE64(const void* ptr, XXH_endianess endian)
{
    return XXH_readLE64_align(ptr, endian, XXH_unaligned);
}

static U64 XXH_readBE64(const void* ptr)
{
    return XXH_CPU_LITTLE_ENDIAN ? XXH_swap64(XXH_read64(ptr)) : XXH_read64(ptr);
}


/*==XXH64==*/

static const U64 PRIME64_1 = 11400714785074694791ULL;
static const U64 PRIME64_2 = 14029467366897019727ULL;
static const U64 PRIME64_3 =  1609587929392839161ULL;
static const U64 PRIME64_4 =  9650029242287828579ULL;
static const U64 PRIME64_5 =  2870177450012600261ULL;

static U64 XXH64_round(U64 acc, U64 input)
{
    acc += input * PRIME64_2;
    acc  = XXH_rotl64(acc, 31);
    acc *= PRIME64_1;
    return acc;
}

static U64 XXH64_mergeRound(U64 acc, U64 val)
{
    val  = XXH64_round(0, val);
    acc ^= val;
    acc  = acc * PRIME64_1 + PRIME64_4;
    return acc;
}

FORCE_INLINE U64 XXH64_endian_align(const void* input, size_t len, U64 seed, XXH_endianess endian, XXH_alignment align)
{
    const BYTE* p = (const BYTE*)input;
    const BYTE* bEnd = p + len;
    U64 h64;
#define XXH_get64bits(p) XXH_readLE64_align(p, endian, align)

#ifdef XXH_ACCEPT_NULL_INPUT_POINTER
    if (p==NULL) {
        len=0;
        bEnd=p=(const BYTE*)(size_t)32;
    }
#endif

    if (len>=32) {
        const BYTE* const limit = bEnd - 32;
        U64 v1 = seed + PRIME64_1 + PRIME64_2;
        U64 v2 = seed + PRIME64_2;
        U64 v3 = seed + 0;
        U64 v4 = seed - PRIME64_1;

        do {
            v1 = XXH64_round(v1, XXH_get64bits(p)); p+=8;
            v2 = XXH64_round(v2, XXH_get64bits(p)); p+=8;
            v3 = XXH64_round(v3, XXH_get64bits(p)); p+=8;
            v4 = XXH64_round(v4, XXH_get64bits(p)); p+=8;
        } while (p<=limit);

        h64 = XXH_rotl64(v1, 1) + XXH_rotl64(v2, 7) + XXH_rotl64(v3, 12) + XXH_rotl64(v4, 18);
        h64 = XXH64_mergeRound(h64, v1);
        h64 = XXH64_mergeRound(h64, v2);
        h64 = XXH64_mergeRound(h64, v3);
        h64 = XXH64_mergeRound(h64, v4);

    } else {
        h64  = seed + PRIME64_5;
    }

    h64 += (U64) len;

    while (p+8<=bEnd) {
        U64 const k1 = XXH64_round(0, XXH_get64bits(p));
        h64 ^= k1;
        h64  = XXH_rotl64(h64,27) * PRIME64_1 + PRIME64_4;
        p+=8;
    }

    if (p+4<=bEnd) {
        h64 ^= (U64)(XXH_get32bits(p)) * PRIME64_1;
        h64 = XXH_rotl64(h64, 23) * PRIME64_2 + PRIME64_3;
        p+=4;
    }

    while (p<bEnd) {
        h64 ^= (*p) * PRIME64_5;
        h64 = XXH_rotl64(h64, 11) * PRIME64_1;
        p++;
    }

    h64 ^= h64 >> 33;
    h64 *= PRIME64_2;
    h64 ^= h64 >> 29;
    h64 *= PRIME64_3;
    h64 ^= h64 >> 32;

    return h64;
}


XXH_PUBLIC_API unsigned long long XXH64 (const void* input, size_t len, unsigned long long seed)
{
#if 0
    /*简单的版本，很好的代码维护，但不幸的是对于小的输入速度很慢*/
    XXH64_state_t state;
    XXH64_reset(&state, seed);
    XXH64_update(&state, input, len);
    return XXH64_digest(&state);
#else
    XXH_endianess endian_detected = (XXH_endianess)XXH_CPU_LITTLE_ENDIAN;

    if (XXH_FORCE_ALIGN_CHECK) {
        /*（（（大小T）输入）&7）==0）/*输入对齐，让我们利用速度优势*/
            if（（endian_detected==xxh_littleendian）xh_force_native_格式）
                返回xxh64-endian-unign（input，len，seed，xxh-littleendian，xxh-aligned）；
            其他的
                返回xxh64撊endian撊align（input，len，seed，xxh撊bigendian，xxh撊aligned）；
    }

    if（（endian_detected==xxh_littleendian）xh_force_native_格式）
        返回xxh64-endian-unign（input，len，seed，xxh-littleendian，xxh-unaligned）；
    其他的
        返回xxh64撊endian_-align（input，len，seed，xxh撊bigendian，xxh_unaligned）；
第二节
}

/*=====哈希流===*/


XXH_PUBLIC_API XXH64_state_t* XXH64_createState(void)
{
    return (XXH64_state_t*)XXH_malloc(sizeof(XXH64_state_t));
}
XXH_PUBLIC_API XXH_errorcode XXH64_freeState(XXH64_state_t* statePtr)
{
    XXH_free(statePtr);
    return XXH_OK;
}

XXH_PUBLIC_API void XXH64_copyState(XXH64_state_t* dstState, const XXH64_state_t* srcState)
{
    memcpy(dstState, srcState, sizeof(*dstState));
}

XXH_PUBLIC_API XXH_errorcode XXH64_reset(XXH64_state_t* statePtr, unsigned long long seed)
{
    /*64_state_t state；/*使用本地状态to memcpy（），以避免出现严格的别名警告*/
    memset（&state，0，sizeof（state）-8）；/*不要写入reserved，以便将来删除*/

    state.v1 = seed + PRIME64_1 + PRIME64_2;
    state.v2 = seed + PRIME64_2;
    state.v3 = seed + 0;
    state.v4 = seed - PRIME64_1;
    memcpy(statePtr, &state, sizeof(state));
    return XXH_OK;
}

FORCE_INLINE XXH_errorcode XXH64_update_endian (XXH64_state_t* state, const void* input, size_t len, XXH_endianess endian)
{
    const BYTE* p = (const BYTE*)input;
    const BYTE* const bEnd = p + len;

#ifdef XXH_ACCEPT_NULL_INPUT_POINTER
    if (input==NULL) return XXH_ERROR;
#endif

    state->total_len += len;

    /*（state->memsize+len<32）/*填充tmp缓冲区*/
        xxh_memcpy（（（byte*）state->mem64）+state->memsize，input，len）；
        状态->Memsize+=（U32）长度；
        返回；
    }

    如果（状态->内存大小）/*tmp缓冲区已满*/

        XXH_memcpy(((BYTE*)state->mem64) + state->memsize, input, 32-state->memsize);
        state->v1 = XXH64_round(state->v1, XXH_readLE64(state->mem64+0, endian));
        state->v2 = XXH64_round(state->v2, XXH_readLE64(state->mem64+1, endian));
        state->v3 = XXH64_round(state->v3, XXH_readLE64(state->mem64+2, endian));
        state->v4 = XXH64_round(state->v4, XXH_readLE64(state->mem64+3, endian));
        p += 32-state->memsize;
        state->memsize = 0;
    }

    if (p+32 <= bEnd) {
        const BYTE* const limit = bEnd - 32;
        U64 v1 = state->v1;
        U64 v2 = state->v2;
        U64 v3 = state->v3;
        U64 v4 = state->v4;

        do {
            v1 = XXH64_round(v1, XXH_readLE64(p, endian)); p+=8;
            v2 = XXH64_round(v2, XXH_readLE64(p, endian)); p+=8;
            v3 = XXH64_round(v3, XXH_readLE64(p, endian)); p+=8;
            v4 = XXH64_round(v4, XXH_readLE64(p, endian)); p+=8;
        } while (p<=limit);

        state->v1 = v1;
        state->v2 = v2;
        state->v3 = v3;
        state->v4 = v4;
    }

    if (p < bEnd) {
        XXH_memcpy(state->mem64, p, (size_t)(bEnd-p));
        state->memsize = (unsigned)(bEnd-p);
    }

    return XXH_OK;
}

XXH_PUBLIC_API XXH_errorcode XXH64_update (XXH64_state_t* state_in, const void* input, size_t len)
{
    XXH_endianess endian_detected = (XXH_endianess)XXH_CPU_LITTLE_ENDIAN;

    if ((endian_detected==XXH_littleEndian) || XXH_FORCE_NATIVE_FORMAT)
        return XXH64_update_endian(state_in, input, len, XXH_littleEndian);
    else
        return XXH64_update_endian(state_in, input, len, XXH_bigEndian);
}

FORCE_INLINE U64 XXH64_digest_endian (const XXH64_state_t* state, XXH_endianess endian)
{
    const BYTE * p = (const BYTE*)state->mem64;
    const BYTE* const bEnd = (const BYTE*)state->mem64 + state->memsize;
    U64 h64;

    if (state->total_len >= 32) {
        U64 const v1 = state->v1;
        U64 const v2 = state->v2;
        U64 const v3 = state->v3;
        U64 const v4 = state->v4;

        h64 = XXH_rotl64(v1, 1) + XXH_rotl64(v2, 7) + XXH_rotl64(v3, 12) + XXH_rotl64(v4, 18);
        h64 = XXH64_mergeRound(h64, v1);
        h64 = XXH64_mergeRound(h64, v2);
        h64 = XXH64_mergeRound(h64, v3);
        h64 = XXH64_mergeRound(h64, v4);
    } else {
        h64  = state->v3 + PRIME64_5;
    }

    h64 += (U64) state->total_len;

    while (p+8<=bEnd) {
        U64 const k1 = XXH64_round(0, XXH_readLE64(p, endian));
        h64 ^= k1;
        h64  = XXH_rotl64(h64,27) * PRIME64_1 + PRIME64_4;
        p+=8;
    }

    if (p+4<=bEnd) {
        h64 ^= (U64)(XXH_readLE32(p, endian)) * PRIME64_1;
        h64  = XXH_rotl64(h64, 23) * PRIME64_2 + PRIME64_3;
        p+=4;
    }

    while (p<bEnd) {
        h64 ^= (*p) * PRIME64_5;
        h64  = XXH_rotl64(h64, 11) * PRIME64_1;
        p++;
    }

    h64 ^= h64 >> 33;
    h64 *= PRIME64_2;
    h64 ^= h64 >> 29;
    h64 *= PRIME64_3;
    h64 ^= h64 >> 32;

    return h64;
}

XXH_PUBLIC_API unsigned long long XXH64_digest (const XXH64_state_t* state_in)
{
    XXH_endianess endian_detected = (XXH_endianess)XXH_CPU_LITTLE_ENDIAN;

    if ((endian_detected==XXH_littleEndian) || XXH_FORCE_NATIVE_FORMAT)
        return XXH64_digest_endian(state_in, XXH_littleEndian);
    else
        return XXH64_digest_endian(state_in, XXH_bigEndian);
}


/*=====规范表示法===*/

XXH_PUBLIC_API void XXH64_canonicalFromHash(XXH64_canonical_t* dst, XXH64_hash_t hash)
{
    XXH_STATIC_ASSERT(sizeof(XXH64_canonical_t) == sizeof(XXH64_hash_t));
    if (XXH_CPU_LITTLE_ENDIAN) hash = XXH_swap64(hash);
    memcpy(dst, &hash, sizeof(*dst));
}

XXH_PUBLIC_API XXH64_hash_t XXH64_hashFromCanonical(const XXH64_canonical_t* src)
{
    return XXH_readBE64(src);
}

/*DIF/*XXHU不长\U长*/
