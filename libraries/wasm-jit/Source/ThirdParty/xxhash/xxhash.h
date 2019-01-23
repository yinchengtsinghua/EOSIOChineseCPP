
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
/*
   xxhash-极快哈希算法
   头文件
   版权所有（c）2012-2016，Yann Collet。

   BSD 2条款许可证（http://www.opensource.org/licenses/bsd-license.php）

   以源和二进制形式重新分配和使用，有或无
   允许修改，前提是以下条件
   遇见：

       *源代码的再分配必须保留上述版权。
   注意，此条件列表和以下免责声明。
       *二进制形式的再分配必须复制上述内容
   版权声明、此条件列表和以下免责声明
   在提供的文件和/或其他材料中，
   分布。

   本软件由版权所有者和贡献者提供。
   “原样”和任何明示或暗示的保证，包括但不包括
   仅限于对适销性和适用性的暗示保证
   不承认特定目的。在任何情况下，版权
   所有人或出资人对任何直接、间接、附带的，
   特殊、惩戒性或后果性损害（包括但不包括
   仅限于采购替代货物或服务；使用损失，
   数据或利润；或业务中断），无论如何引起的
   责任理论，无论是合同责任、严格责任还是侵权责任。
   （包括疏忽或其他）因使用不当而引起的
   即使已告知此类损坏的可能性。

   您可以通过以下方式联系作者：
   -xxhash源存储库：https://github.com/cyan4973/xhash
**/


/*从XXhash主页提取的通知：

XXHASH是一种非常快速的哈希算法，以RAM速度限制运行。
它还成功地通过了smhasher套件中的所有测试。

比较（单线程，Windows 7个32位，在核心2 Duo@3GHz上使用smhasher）

姓名：speed q.score author
XXhash 5.4 GB/s 10
CRAPOW 3.2 GB/s 2安德鲁
Mumurhash 3A 2.7 GB/s 10奥斯汀Appleby
spokyhash 2.0 GB/s 10鲍勃·詹金斯
SBOX 1.4 GB/s 9布雷特·穆维
查找3 1.2 GB/s 9 Bob Jenkins
Superfasthash 1.2 GB/s 1 Paul Hsieh
CityHash64 1.05 GB/S 10 Pike和Alakuijala
FNv 0.55 GB/s 5福勒、诺尔、沃
CRC32 0.43 GB/s 9
MD5-32 0.33 GB/s 10罗纳德L.里维斯
sha1-32 0.28 GB/s 10个

q.得分是散列函数质量的度量。
这取决于能否成功通过smhasher测试集。
10是一个完美的分数。

64位版本，名为xxh64，从r35开始提供。
它提供了更好的速度，但仅适用于64位应用程序。
64位上的名称速度32位上的速度
xxh64 13.8 GB/s 1.9 GB/s
xxh32 6.8 GB/s 6.0 GB/s
**/


#ifndef XXHASH_H_5627135585666179
#define XXHASH_H_5627135585666179 1

#if defined (__cplusplus)
extern "C" {
#endif


/****************************
＊定义
****************************/

/*clude<stddef.h>/*大小\u t*/
typedef枚举XXH_OK=0，XXH_错误XXH_错误代码；


/****************************
＊API修改器
****************************/

/*黄连
*这对于在“static”模式下包含XXhash函数很有用。
*为了嵌入它们，并从公共列表中删除它们的符号。
*方法：
*定义xxh_private_api
*包括“xxash.h”
*`XXHASH.C`自动包括在内。
*将其编译并链接为单独的模块是没有用的。
**/

#ifdef XXH_PRIVATE_API
#  ifndef XXH_STATIC_LINKING_ONLY
#    define XXH_STATIC_LINKING_ONLY
#  endif
#  if defined(__GNUC__)
#    define XXH_PUBLIC_API static __inline __attribute__((unused))
/*elif defined（uu cplusplus）（defined（u stdc_u version_uuuu）&&（u stdc_u version_uuuuu>=199901l）/*c99*/）
定义xxh_public_api static inline
elif定义（35u msc35u ver）
定义xxh_public_api static_uuu inline
否则
define xxh_public_api static/*此版本可能会为未使用的静态函数生成警告；禁用相关警告*/

#  endif
#else
/*define xxh_public_api/*什么都不做*/
endif/*xxh_私有_API*/


/*xxh_名称空间，又称名称空间仿真：

如果您想在自己的库中包含并公开_xhash函数，
但也要避免与其他库（也可能包括XXHASH）发生符号冲突，

您可以使用xxh_名称空间，自动为xxhash库中的任何公共符号加前缀。
具有xxh_名称空间的值（因此，请避免使用空值和数字值）。

请注意，只要调用程序包含“xxash.h”，就不需要对其进行任何更改：
常规符号名称将由此标题自动转换。
**/

#ifdef XXH_NAMESPACE
#  define XXH_CAT(A,B) A##B
#  define XXH_NAME2(A,B) XXH_CAT(A,B)
#  define XXH_versionNumber XXH_NAME2(XXH_NAMESPACE, XXH_versionNumber)
#  define XXH32 XXH_NAME2(XXH_NAMESPACE, XXH32)
#  define XXH32_createState XXH_NAME2(XXH_NAMESPACE, XXH32_createState)
#  define XXH32_freeState XXH_NAME2(XXH_NAMESPACE, XXH32_freeState)
#  define XXH32_reset XXH_NAME2(XXH_NAMESPACE, XXH32_reset)
#  define XXH32_update XXH_NAME2(XXH_NAMESPACE, XXH32_update)
#  define XXH32_digest XXH_NAME2(XXH_NAMESPACE, XXH32_digest)
#  define XXH32_copyState XXH_NAME2(XXH_NAMESPACE, XXH32_copyState)
#  define XXH32_canonicalFromHash XXH_NAME2(XXH_NAMESPACE, XXH32_canonicalFromHash)
#  define XXH32_hashFromCanonical XXH_NAME2(XXH_NAMESPACE, XXH32_hashFromCanonical)
#  define XXH64 XXH_NAME2(XXH_NAMESPACE, XXH64)
#  define XXH64_createState XXH_NAME2(XXH_NAMESPACE, XXH64_createState)
#  define XXH64_freeState XXH_NAME2(XXH_NAMESPACE, XXH64_freeState)
#  define XXH64_reset XXH_NAME2(XXH_NAMESPACE, XXH64_reset)
#  define XXH64_update XXH_NAME2(XXH_NAMESPACE, XXH64_update)
#  define XXH64_digest XXH_NAME2(XXH_NAMESPACE, XXH64_digest)
#  define XXH64_copyState XXH_NAME2(XXH_NAMESPACE, XXH64_copyState)
#  define XXH64_canonicalFromHash XXH_NAME2(XXH_NAMESPACE, XXH64_canonicalFromHash)
#  define XXH64_hashFromCanonical XXH_NAME2(XXH_NAMESPACE, XXH64_hashFromCanonical)
#endif


/*********************************
*版本
*********************************/

#define XXH_VERSION_MAJOR    0
#define XXH_VERSION_MINOR    6
#define XXH_VERSION_RELEASE  2
#define XXH_VERSION_NUMBER  (XXH_VERSION_MAJOR *100*100 + XXH_VERSION_MINOR *100 + XXH_VERSION_RELEASE)
XXH_PUBLIC_API unsigned XXH_versionNumber (void);


/***************************************************************
* 32位散列
***************************************************************/

typedef unsigned int XXH32_hash_t;

/*XXH32（）：
    计算存储在内存地址“input”的序列“length”字节的32位散列值。
    输入和输入+长度之间的内存必须有效（已分配并可读取）。
    “seed”可用于按可预见的方式更改结果。
    核心2双核速度@3 GHz（单线程，smhasher基准）：5.4 GB/s*/

XXH_PUBLIC_API XXH32_hash_t XXH32 (const void* input, size_t length, unsigned int seed);

/*=====流式处理===*/
/*edef struct XXH32_state_s XXH32_state_t；/*类型不完整*/
xxh_public_api xxh32_state_t*xh32_createstate（无效）；
xxh_public_api xh_errorcode xxh32_freestate（xxh32_state_t*stateptr）；
xxh_public_api void xxh32_copystate（xxh32_state_t*dst_state，const xh32_state_t*src_state）；

xxh_public_api xh_errorcode xxh32_reset（xxh32_state_t*stateptr，unsigned int seed）；
xxh_public_api xh_errorcode xxh32_update（xxh32_state_t*stateptr，const void*input，size_t length）；
xxh_public_api xxh32_hash_t xh32_digest（const xh32_state_t*stateptr）；

/*
这些函数生成在多个段中提供的输入的XXhash。
注意，对于小输入，由于状态管理，它们比单个调用函数慢。
对于小输入，首选'XXH32（）'和'XXH64（）'。

必须首先使用xxh*CreateState（）分配xxh状态。

使用xxh*Reset（）通过使用种子初始化状态来启动新哈希。

然后，根据需要多次调用xxh*update（）来提供散列状态。
显然，输入必须被分配和读取。
函数返回一个错误代码，0表示OK，任何其他值表示有错误。

最后，可以通过使用xxh*_digest（）随时生成哈希值。
此函数以int或long long的形式返回nn位散列。

仍然可以在摘要之后继续将输入插入哈希状态，
稍后通过再次调用xxh*_digest（）生成一些新的哈希。

完成后，如果动态分配，则释放XXH状态空间。
**/


/*=====规范表示法===*/

typedef struct { unsigned char digest[4]; } XXH32_canonical_t;
XXH_PUBLIC_API void XXH32_canonicalFromHash(XXH32_canonical_t* dst, XXH32_hash_t hash);
XXH_PUBLIC_API XXH32_hash_t XXH32_hashFromCanonical(const XXH32_canonical_t* src);

/*XXH函数的默认结果类型是无符号32位和64位原语。
*规范表示使用人类可读的写入约定，即big endian（大数字优先）。
*这些函数允许将哈希结果转换为其规范格式。
*这样，散列值可以写入文件/内存，并在不同的系统和程序上保持可比较性。
**/



#ifndef XXH_NO_LONG_LONG
/***************************************************************
* 64位散列
***************************************************************/

typedef unsigned long long XXH64_hash_t;

/*XXH64（）：
    计算存储在内存地址“input”的长度为“len”的序列的64位散列值。
    “seed”可用于按可预见的方式更改结果。
    此函数在64位系统上运行得更快，但在32位系统上运行得较慢（请参见基准测试）。
**/

XXH_PUBLIC_API XXH64_hash_t XXH64 (const void* input, size_t length, unsigned long long seed);

/*=====流式处理===*/
/*edef struct xxh64_state_s xh64_state_t；/*类型不完整*/
xxh_public_api xxh64_state_t*xh64_createstate（无效）；
xxh_public_api xh_errorcode xxh64_freestate（xh64_state_t*stateptr）；
xxh_public_api void xxh64_copystate（xh64_state_t*dst_state，const xh64_state_t*src_state）；

xxh_public_api xh_errorcode xxh64_reset（xxh64_state_t*stateptr，无符号长种子）；
xxh_public_api xh_errorcode xxh64_update（xxh64_state_t*stateptr，const void*input，size_t length）；
xxh_public_api xxh64_hash_t xh64_digest（const xh64_state_t*stateptr）；

/*=====规范表示法===*/

typedef struct { unsigned char digest[8]; } XXH64_canonical_t;
XXH_PUBLIC_API void XXH64_canonicalFromHash(XXH64_canonical_t* dst, XXH64_hash_t hash);
XXH_PUBLIC_API XXH64_hash_t XXH64_hashFromCanonical(const XXH64_canonical_t* src);
/*DIF/*XXHU不长\U长*/


仅限ifdef xxh_静态链接

/*=============================================================================
   本节包含不保证保持稳定的定义。
   它们可能在未来的版本中发生变化，与库的不同版本不兼容。
   它们只能与静态链接一起使用。
   不要将这些定义与动态链接结合使用！
========================================================================*/


/*这些定义只是为了使
   例如，在堆栈或结构中静态分配XXH状态。
   不要直接使用成员。*/


struct XXH32_state_s {
   unsigned total_len_32;
   unsigned large_len;
   unsigned v1;
   unsigned v2;
   unsigned v3;
   unsigned v4;
   /*igned mem32[4]；/*缓冲区定义为用于对齐的U32*/
   无符号内存大小；
   unsigned reserved；/*从不读也不写，将在将来的版本中删除*/

/*/*typedef'd to xxh32_state_t*/

ifndef xxh_no诳long诳long/*删除64位支持*/

struct XXH64_state_s {
   unsigned long long total_len;
   unsigned long long v1;
   unsigned long long v2;
   unsigned long long v3;
   unsigned long long v4;
   /*igned long long mem64[4]；/*缓冲区定义为用于对齐的U64*/
   无符号内存大小；
   unsigned reserved[2]；在将来的版本中将删除/*从不读也不写*/

/*/*typedef'd to xxh64_state_t*/
第二节

ifdef xxh_private_api
include“xxash.c”/*include xxash函数体作为“static”用于内联*/

#endif

/*DIF/*XXHU静态链接\U仅限*/


如果定义了（35u cplusplus）
}
第二节

endif/*xxash_h_5627135585666179*/

