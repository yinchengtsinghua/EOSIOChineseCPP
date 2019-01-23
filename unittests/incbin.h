
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
/*
 *来自https://github.com/graphitemaster/incbin，在公共领域获得许可
 **/

#ifndef INCBIN_HDR
#define INCBIN_HDR
#include <limits.h>
#if   defined(__AVX512BW__) || \
      defined(__AVX512CD__) || \
      defined(__AVX512DQ__) || \
      defined(__AVX512ER__) || \
      defined(__AVX512PF__) || \
      defined(__AVX512VL__) || \
      defined(__AVX512F__)
# define INCBIN_ALIGNMENT_INDEX 6
#elif defined(__AVX__)      || \
      defined(__AVX2__)
# define INCBIN_ALIGNMENT_INDEX 5
#elif defined(__SSE__)      || \
      defined(__SSE2__)     || \
      defined(__SSE3__)     || \
      defined(__SSSE3__)    || \
      defined(__SSE4_1__)   || \
      defined(__SSE4_2__)   || \
      defined(__neon__)
# define INCBIN_ALIGNMENT_INDEX 4
#elif ULONG_MAX != 0xffffffffu
# define INCBIN_ALIGNMENT_INDEX 3
# else
# define INCBIN_ALIGNMENT_INDEX 2
#endif

/*（1<<n）的查阅表格，其中“n”是“incbin”对齐“index”*/
#define INCBIN_ALIGN_SHIFT_0 1
#define INCBIN_ALIGN_SHIFT_1 2
#define INCBIN_ALIGN_SHIFT_2 4
#define INCBIN_ALIGN_SHIFT_3 8
#define INCBIN_ALIGN_SHIFT_4 16
#define INCBIN_ALIGN_SHIFT_5 32
#define INCBIN_ALIGN_SHIFT_6 64

/*实际对齐值*/
#define INCBIN_ALIGNMENT \
    INCBIN_CONCATENATE( \
        INCBIN_CONCATENATE(INCBIN_ALIGN_SHIFT, _), \
        INCBIN_ALIGNMENT_INDEX)

/*纵横交错*/
#define INCBIN_STR(X) \
    #X
#define INCBIN_STRINGIZE(X) \
    INCBIN_STR(X)
/*连接*/
#define INCBIN_CAT(X, Y) \
    X ## Y
#define INCBIN_CONCATENATE(X, Y) \
    INCBIN_CAT(X, Y)
/*延迟宏扩展*/
#define INCBIN_EVAL(X) \
    X
#define INCBIN_INVOKE(N, ...) \
    INCBIN_EVAL(N(__VA_ARGS__))

/*Green Hills使用不同的指令来包含二进制数据*/
#if defined(__ghs__)
#  define INCBIN_MACRO "\tINCBIN"
#else
#  define INCBIN_MACRO ".incbin"
#endif

#ifndef _MSC_VER
#  define INCBIN_ALIGN \
    __attribute__((aligned(INCBIN_ALIGNMENT)))
#else
#  define INCBIN_ALIGN __declspec(align(INCBIN_ALIGNMENT))
#endif

/*定义（uuu arm_uuu）/*GNU C和RealView
    定义（臂）/*Diab*/ \

    /*内定（_arm）/*成像工艺*/
定义incbin_arm
第二节

γ-干扰素
/*在支持的地方使用.balign*/

#  define INCBIN_ALIGN_HOST ".balign " INCBIN_STRINGIZE(INCBIN_ALIGNMENT) "\n"
#  define INCBIN_ALIGN_BYTE ".balign 1\n"
#elif defined(INCBIN_ARM)
/*
 *在ARM装配机上，对齐值计算为（1<<n），其中“n”是
 *轮班计数。这是传递给`.Align'的值
 **/

#  define INCBIN_ALIGN_HOST ".align" INCBIN_STRINGIZE(INCBIN_ALIGNMENT_INDEX) "\n"
#  define INCBIN_ALIGN_BYTE ".align 0\n"
#else
/*我们假设其他内联汇编程序将`.Align'视为`.Balign'*/
#  define INCBIN_ALIGN_HOST ".align" INCBIN_STRINGIZE(INCBIN_ALIGNMENT) "\n"
#  define INCBIN_ALIGN_BYTE ".align 1\n"
#endif

/*incbin常量由incbin.c生成的文件使用*/
#if defined(__cplusplus)
#  define INCBIN_EXTERNAL extern "C"
#  define INCBIN_CONST    extern const
#else
#  define INCBIN_EXTERNAL extern
#  define INCBIN_CONST    const
#endif

/*
 *@brief可以选择重写将数据发送到其中的链接器部分。
 *
 *@警告如果使用此工具，则必须处理特定于平台的链接器输出
 *自行命名分区
 *
 *覆盖默认链接器输出部分，例如，对于ESP8266/ARDUino：
 *@代码
 *定义incbin_output_节“.irom.text”
 *包括“incbin.h”
 *incbin（foo，“foo.txt”）；
 *//数据被发送到程序内存中，而程序内存永远不会被复制到RAM中
 *@终结码
 **/

#if !defined(INCBIN_OUTPUT_SECTION)
#  if defined(__APPLE__)
#    define INCBIN_OUTPUT_SECTION         ".const_data"
#  else
#    define INCBIN_OUTPUT_SECTION         ".rodata"
#  endif
#endif

#if defined(__APPLE__)
/*对于苹果品牌的编译器，指令是不同的。*/
#  define INCBIN_SECTION         INCBIN_OUTPUT_SECTION "\n"
#  define INCBIN_GLOBAL(NAME)    ".globl " INCBIN_STRINGIZE(INCBIN_PREFIX) #NAME "\n"
#  define INCBIN_INT             ".long "
#  define INCBIN_MANGLE          "_"
#  define INCBIN_BYTE            ".byte "
#  define INCBIN_TYPE(...)
#else
#  define INCBIN_SECTION         ".section " INCBIN_OUTPUT_SECTION "\n"
#  define INCBIN_GLOBAL(NAME)    ".global " INCBIN_STRINGIZE(INCBIN_PREFIX) #NAME "\n"
#  define INCBIN_INT             ".int "
#  if defined(__USER_LABEL_PREFIX__)
#    define INCBIN_MANGLE        INCBIN_STRINGIZE(__USER_LABEL_PREFIX__)
#  else
#    define INCBIN_MANGLE        ""
#  endif
#  if defined(INCBIN_ARM)
/*在ARM汇编程序上，“@”用作行注释标记*/
#    define INCBIN_TYPE(NAME)    ".type " INCBIN_STRINGIZE(INCBIN_PREFIX) #NAME ", %object\n"
#  elif defined(__MINGW32__) || defined(__MINGW64__)
/*Mingw也不支持这个指令*/
#    define INCBIN_TYPE(NAME)
#  else
/*在其他体系结构上使用“@”是安全的*/
#    define INCBIN_TYPE(NAME)    ".type " INCBIN_STRINGIZE(INCBIN_PREFIX) #NAME ", @object\n"
#  endif
#  define INCBIN_BYTE            ".byte "
#endif

/*用于符号名称的样式类型列表*/
#define INCBIN_STYLE_CAMEL 0
#define INCBIN_STYLE_SNAKE 1

/*
 *@brief指定用于符号名称的前缀。
 *
 *默认情况下，这是“g”，生成以下形式的符号：
 *@代码
 *包括“incbin.h”
 *incbin（foo，“foo.txt”）；
 *
 *//现在您有以下符号：
 *//const unsigned char gfoodata[]；
 *//const unsigned char*const gfooend；
 *//const unsigned int gfoose；
 *@终结码
 *
 *但是，如果在include之前指定前缀，例如：
 *@代码
 *定义incbin前缀incbin
 *包括“incbin.h”
 *incbin（foo，“foo.txt”）；
 *
 *//现在您有了以下符号：
 *//const unsigned char incbinfoodata[]；
 *//const unsigned char*const incbinfooend；
 *//const unsigned int incbinfoosize；
 *@终结码
 **/

#if !defined(INCBIN_PREFIX)
#  define INCBIN_PREFIX g
#endif

/*
 *@brief指定用于符号名称的样式。
 *
 *可能的选项有
 *—incbin_-style_-camel“骆驼壳”
 *—incbin“蛇”案
 *
 *默认选项是*incbin_-style_-camel*生成形式的符号：
 *@代码
 *包括“incbin.h”
 *incbin（foo，“foo.txt”）；
 *
 *//现在您有以下符号：
 *//const unsigned char<prefix>foodata[]；
 *//const unsigned char*const<prefix>fooend；
 *//const unsigned int<prefix>foosize；
 *@终结码
 *
 *但是，如果在包含之前指定了样式，例如：
 *@代码
 *定义incbin样式incbin样式
 *包括“incbin.h”
 *incbin（foo，“foo.txt”）；
 *
 *//现在您有以下符号：
 *//const unsigned char<prefix>foo_data[]；
 *//const unsigned char*const<prefix>foo_end；
 *//const unsigned int<prefix>foo_size；
 *@终结码
 **/

#if !defined(INCBIN_STYLE)
#  define INCBIN_STYLE INCBIN_STYLE_CAMEL
#endif

/*样式查找表*/
#define INCBIN_STYLE_0_DATA Data
#define INCBIN_STYLE_0_END End
#define INCBIN_STYLE_0_SIZE Size
#define INCBIN_STYLE_1_DATA _data
#define INCBIN_STYLE_1_END _end
#define INCBIN_STYLE_1_SIZE _size

/*样式查找：返回标识符*/
#define INCBIN_STYLE_IDENT(TYPE) \
    INCBIN_CONCATENATE( \
        INCBIN_STYLE_, \
        INCBIN_CONCATENATE( \
            INCBIN_EVAL(INCBIN_STYLE), \
            INCBIN_CONCATENATE(_, TYPE)))

/*样式查找：返回字符串文本*/
#define INCBIN_STYLE_STRING(TYPE) \
    INCBIN_STRINGIZE( \
        INCBIN_STYLE_IDENT(TYPE)) \

/*通过使用我们的样式间接调用宏来生成全局标签
 *键入名称并将其连接起来。*/

#define INCBIN_GLOBAL_LABELS(NAME, TYPE) \
    INCBIN_INVOKE( \
        INCBIN_GLOBAL, \
        INCBIN_CONCATENATE( \
            NAME, \
            INCBIN_INVOKE( \
                INCBIN_STYLE_IDENT, \
                TYPE))) \
    INCBIN_INVOKE( \
        INCBIN_TYPE, \
        INCBIN_CONCATENATE( \
            NAME, \
            INCBIN_INVOKE( \
                INCBIN_STYLE_IDENT, \
                TYPE)))

/*
 *@brief外部引用另一翻译单元中包含的二进制数据。
 *
 *生成三个引用包含在
 *另一个翻译单元。
 *
 *符号名称是“incbin_prefix”在*name*之前的串联，与
 *“数据”，以及“结束”和“大小”之后。下面提供了一个示例。
 *
 *@param name为二进制数据指定的名称
 *
 *@代码
 *INCBin-Outern（FOO）公司；
 *
 *//现在您有以下符号：
 *//extern const unsigned char<prefix>foodata[]；
 *//extern const unsigned char*const<prefix>fooend；
 *//extern const unsigned int<prefix>foosize；
 *@终结码
 **/

#define INCBIN_EXTERN(NAME) \
    INCBIN_EXTERNAL const INCBIN_ALIGN unsigned char \
        INCBIN_CONCATENATE( \
            INCBIN_CONCATENATE(INCBIN_PREFIX, NAME), \
            INCBIN_STYLE_IDENT(DATA))[]; \
    INCBIN_EXTERNAL const INCBIN_ALIGN unsigned char *const \
    INCBIN_CONCATENATE( \
        INCBIN_CONCATENATE(INCBIN_PREFIX, NAME), \
        INCBIN_STYLE_IDENT(END)); \
    INCBIN_EXTERNAL const unsigned int \
        INCBIN_CONCATENATE( \
            INCBIN_CONCATENATE(INCBIN_PREFIX, NAME), \
            INCBIN_STYLE_IDENT(SIZE))

/*
 *@brief在当前翻译单元中包含一个二进制文件。
 *
 *在当前翻译单元中包含一个二进制文件，生成三个符号。
 *分别对数据和大小进行编码的对象。
 *
 *符号名称是“incbin_prefix”在*name*之前的串联，与
 *“数据”，以及“结束”和“大小”之后。下面提供了一个示例。
 *
 *@param name要与此二进制数据关联的名称（作为标识符）。
 *@param filename要包含的文件（作为字符串文本）。
 *
 *@代码
 *incbin（icon，“icon.png”）；
 *
 *//现在您有以下符号：
 *//const unsigned char<prefix>iconda[]；
 *//const unsigned char*const<prefix>iconend；
 *//const unsigned int<prefix>iconsize；
 *@终结码
 *
 *@警告这必须在全局范围内使用
 *@警告如果incbin_样式不是默认的，标识符可能不同。
 *
 *在外部参考其他翻译单元中包含的数据。
 *请@参见incbin\u extern。
 **/

#ifdef _MSC_VER
#define INCBIN(NAME, FILENAME) \
    INCBIN_EXTERN(NAME)
#else
#define INCBIN(NAME, FILENAME) \
    __asm__(INCBIN_SECTION \
            INCBIN_GLOBAL_LABELS(NAME, DATA) \
            INCBIN_ALIGN_HOST \
            INCBIN_MANGLE INCBIN_STRINGIZE(INCBIN_PREFIX) #NAME INCBIN_STYLE_STRING(DATA) ":\n" \
            INCBIN_MACRO " \"" FILENAME "\"\n" \
            INCBIN_GLOBAL_LABELS(NAME, END) \
            INCBIN_ALIGN_BYTE \
            INCBIN_MANGLE INCBIN_STRINGIZE(INCBIN_PREFIX) #NAME INCBIN_STYLE_STRING(END) ":\n" \
                INCBIN_BYTE "1\n" \
            INCBIN_GLOBAL_LABELS(NAME, SIZE) \
            INCBIN_ALIGN_HOST \
            INCBIN_MANGLE INCBIN_STRINGIZE(INCBIN_PREFIX) #NAME INCBIN_STYLE_STRING(SIZE) ":\n" \
                INCBIN_INT INCBIN_MANGLE INCBIN_STRINGIZE(INCBIN_PREFIX) #NAME INCBIN_STYLE_STRING(END) " - " \
                           INCBIN_MANGLE INCBIN_STRINGIZE(INCBIN_PREFIX) #NAME INCBIN_STYLE_STRING(DATA) "\n" \
    ); \
    INCBIN_EXTERN(NAME)

#endif
#endif
