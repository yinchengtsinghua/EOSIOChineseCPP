
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
#include <eosiolib/types.h>
extern "C" {

/*
 *@defgroup cryptoapi链api
 *@brief定义用于计算和检查哈希的API
 *@ingroup contractdev公司
 **/


/*
 *@defgroup cryptocapi chain C api
 *@brief定义了用于计算和检查哈希的%c API
 *@ingroup链接API
 *@
 **/


/*
 *测试从数据生成的sha256哈希是否与提供的校验和匹配。
 *此方法在快速评估模式下优化为无操作。
 *@brief测试从数据生成的sha256哈希是否与提供的校验和匹配。
 *
 *@param data-要散列的数据
 *@param length-数据长度
 *@param hash-`checksum256*`要比较的哈希
 *
 “'data`的*@pre**assert256 hash**”等于提供的“hash”参数。
 *@post执行下一条语句。如果不是“真的”，则硬返回。
 *
 *实例：
*
 *@代码
 *校验和哈希；
 *字符数据；
 *uint32英尺长；
 *断言sha256（数据、长度、哈希）
 *//如果从数据生成的sha256哈希不等于提供的哈希，则不会触发下面的任何内容。
 *eosio:：print（“从数据生成的sha256哈希等于提供的哈希”）；
 *@终结码
 **/

void assert_sha256( const char* data, uint32_t length, const checksum256* hash );

/*
 *测试从数据生成的sha1哈希是否与提供的校验和匹配。
 *此方法在快速评估模式下优化为无操作。
 *@brief测试从数据生成的sha1哈希是否与提供的校验和匹配。
 *
 *@param data-要散列的数据
 *@param length-数据长度
 *@param hash-`checksum160*`要比较的哈希
 *
 “'data`的*@pre**sha1 hash**”等于提供的“hash”参数。
 *@post执行下一条语句。如果不是“真的”，则硬返回。
 *
 *实例：
*
 *@代码
 *校验和哈希；
 *字符数据；
 *uint32英尺长；
 *断言sha1（数据、长度、哈希）
 *//如果从数据生成的sha1哈希不等于提供的哈希，则不会触发下面的任何内容。
 *eosio:：print（“从数据生成的sha1哈希等于提供的哈希”）；
 *@终结码
 **/

void assert_sha1( const char* data, uint32_t length, const checksum160* hash );

/*
 *测试从数据生成的sha512哈希是否与提供的校验和匹配。
 *此方法在快速评估模式下优化为无操作。
 *@brief测试从数据生成的sha512哈希是否与提供的校验和匹配。
 *
 *@param data-要散列的数据
 *@param length-数据长度
 *@param hash-`checksum512*`要比较的哈希
 *
 “'data`的*@pre**assert512 hash**等于提供的'hash'参数。”
 *@post执行下一条语句。如果不是“真的”，则硬返回。
 *
 *实例：
*
 *@代码
 *校验和哈希；
 *字符数据；
 *uint32英尺长；
 *断言\u sha512（数据、长度、哈希）
 *//如果从数据生成的sha512哈希不等于提供的哈希，则不会触发下面的任何内容。
 *eosio:：print（“从数据生成的sha512哈希等于提供的哈希”）；
 *@终结码
 **/

void assert_sha512( const char* data, uint32_t length, const checksum512* hash );

/*
 *测试数据生成的ripemod160哈希是否与提供的校验和匹配。
 *@brief测试数据生成的ripemod160哈希是否与提供的校验和匹配。
 *
 *@param data-要散列的数据
 *@param length-数据长度
 *@param hash-`checksum160*`要比较的哈希
 *
 “'data`的*@pre**assert160 hash**等于提供的'hash'参数。”
 *@post执行下一条语句。如果不是“真的”，则硬返回。
 *
 *实例：
*
 *@代码
 *校验和哈希；
 *字符数据；
 *uint32英尺长；
 *断言ripemod160（数据、长度、哈希）
 *//如果从数据生成的ripemod160散列不等于提供的散列，则不会触发下面的任何内容。
 *eosio:：print（“从数据生成的ripemod160哈希等于提供的哈希”）；
 *@终结码
 **/

void assert_ripemd160( const char* data, uint32_t length, const checksum160* hash );

/*
 *使用“sha256”散列“data”，并将结果存储在散列指向的内存中。
 *@brief使用'sha256'散列'data'，并将结果存储在散列指向的内存中。
 *
 *@param data-要散列的数据
 *@param length-数据长度
 *@param hash-哈希指针
 *
 *实例：
*
 *@代码
 *校验和计算哈希；
 *sha256（数据、长度和计算哈希）；
 *eos_assert（calc_hash==hash，“无效hash”）；
 *@终结码
 **/

void sha256( const char* data, uint32_t length, checksum256* hash );

/*
 *使用“sha1”散列“data”，并将结果存储在哈希指向的内存中。
 *@brief使用“sha1”散列“data”，并将结果存储在散列指向的内存中。
 *
 *@param data-要散列的数据
 *@param length-数据长度
 *@param hash-哈希指针
 *
 *实例：
*
 *@代码
 *校验和计算哈希；
 *sha1（数据、长度和计算哈希）；
 *eos_assert（calc_hash==hash，“无效hash”）；
 *@终结码
 **/

void sha1( const char* data, uint32_t length, checksum160* hash );

/*
 *使用“sha512”散列“data”，并将结果存储在哈希指向的内存中。
 *@brief使用“sha512”散列“data”，并将结果存储在散列指向的内存中。
 *
 *@param data-要散列的数据
 *@param length-数据长度
 *@param hash-哈希指针
 *
 *实例：
*
 *@代码
 *校验和计算哈希；
 *sha512（数据、长度和计算哈希）；
 *eos_assert（calc_hash==hash，“无效hash”）；
 *@终结码
 **/

void sha512( const char* data, uint32_t length, checksum512* hash );

/*
 *使用“ripemod160”散列“data”，并将结果存储在散列指向的内存中。
 *@brief使用'ripemod160'散列'data'，并将结果存储在散列指向的内存中。
 *
 *@param data-要散列的数据
 *@param length-数据长度
 *@param hash-哈希指针
 *
 *实例：
*
 *@代码
 *校验和计算哈希；
 *ripemod160（数据、长度和计算哈希）；
 *eos_assert（calc_hash==hash，“无效hash”）；
 *@终结码
 **/

void ripemd160( const char* data, uint32_t length, checksum160* hash );

/*
 *计算用于给定签名和用于创建消息的哈希的公钥。
 *@brief计算用于给定签名和用于创建消息的哈希的公钥。
 *
 *@param digest-用于创建消息的哈希
 *@param sig-签名
 *@param siglen-签名长度
 *@param pub-公钥
 *@param publen-公钥长度
 *
 *实例：
*
 *@代码
 *@终结码
 **/

int recover_key( const checksum256* digest, const char* sig, size_t siglen, char* pub, size_t publen );

/*
 *使用摘要中生成的密钥和签名测试给定的公钥。
 *@brief使用摘要中生成的密钥和签名测试给定的公钥。
 *
 *@param digest-将从中生成密钥
 *@param sig-签名
 *@param siglen-签名长度
 *@param pub-公钥
 *@param publen-公钥长度
 *
 “'pub'的*@pre**assert recovery key**”等于从“digest”参数生成的密钥。
 *@post执行下一条语句。如果不是“真的”，则硬返回。
 *
 *实例：
*
 *@代码
 *校验和摘要；
 ＊查尔锡格；
 *尺寸符号；
 *Car酒吧；
 *尺寸
 *断言恢复密钥（摘要、sig、siglen、pub、publen）
 *//如果给定的公钥与摘要和签名中生成的密钥不匹配，则不会激发下面的任何内容。
 *eosio:：print（“pub key匹配摘要生成的pub key”）；
 *@终结码
 **/

void assert_recover_key( const checksum256* digest, const char* sig, size_t siglen, const char* pub, size_t publen );

///}@ CurtoPAPI

}
