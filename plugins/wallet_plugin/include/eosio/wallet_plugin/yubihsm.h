
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
/*
 *版权所有（c）2016 Yubico AB
 *保留所有权利。
 *
 *以源和二进制形式重新分配和使用，有或无
 *允许修改，前提是以下条件
 *相遇：
 *
 **重新发布源代码必须保留上述版权。
 *注意，此条件列表和以下免责声明。
 *
 **二进制形式的再分配必须复制上述内容
 *版权声明、本条件清单及以下内容
 *文件和/或提供的其他材料中的免责声明
 *与分布有关。
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
 **/


/*
 @主页

 @章节介绍

 libyubihsm是一个用于与yubihsm设备通信的库。

 分段使用

 要使用库include<yubihsm.h>并将-lyubihsm标志传递给
 链接器。
 调试输出由函数yh_set_verbosity（）控制。

 使用yubihsm的第一步是使用yh_init（），init a初始化库
 用yh_init_connector（）连接，然后用yh_connect_best（）连接。
 在此之后，必须使用yh_create_session_derived（）建立会话，并且
 yh_authenticate_session（）。
 当建立会话时，可以通过它交换命令，函数
 在名称空间中，yh-util是执行
 设备的特定任务。

 @部分API API参考

 尤比希姆
 所有公共职能和定义

 @部分示例代码示例
 下面是一个小例子，建立一个与Yubihsm的会话并获取
 关闭会话前的一些随机事件。
 \代码{c}
 利息主（空）
   yh_connector*connector=空；
   yh_session*session=空；
   uint8_t context[yh_context_len]=0_
   uint8_t data[128]=0_
   size_t data_len=sizeof（数据）；

   断言（yh_init（）==yhr_success）；
   断言（yh_init_connector（“http://localhost:12345”，&connector）=
 成功；
   断言（yh_connect_best（&connector，1，null）=yhr_success）；
   断言（yh_create_session_derived（connector，1，yh_default_password，
 strlen（yh_default_password），false，context，sizeof（context），&session）=
 成功；
   断言（yh_authenticate_session（session，context，sizeof（context））=
 成功；
   断言（yh-util-get-random（session，sizeof（data），data，&data-len）=
 成功；
   断言（data_len==sizeof（data））；
   断言（yh-util-close-session（session）=yhr-success）；
   断言（yh_destroy_session（&session）=yhr_success）；
   断言（yh_disconnect（connector）=yhr_success）；
 }
 终止码

 **/


/*@文件YuBiHSM
 *
 *你需要的一切。
 **/


#ifndef YUBIHSM_H
#define YUBIHSM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

///用于身份验证的上下文数组的长度
#define YH_CONTEXT_LEN 16
///身份验证的主机质询长度
#define YH_HOST_CHAL_LEN 8
///消息缓冲区的最大长度
#define YH_MSG_BUF_SIZE 2048
///身份验证密钥的长度
#define YH_KEY_LEN 16
///设备供应商ID
#define YH_VID 0x1050
///设备产品ID
#define YH_PID 0x0030
///response命令标志
#define YH_CMD_RESP_FLAG 0x80
///max设备可以容纳的项目数
#define YH_MAX_ITEMS_COUNT                                                     \
256 //TODO:这真的应该在API中定义吗？
///max设备可容纳的会话数
#define YH_MAX_SESSIONS 16 //托多：这里也是，真的是API的一部分吗？
///默认加密密钥
#define YH_DEFAULT_ENC_KEY                                                     \
  "\x09\x0b\x47\xdb\xed\x59\x56\x54\x90\x1d\xee\x1c\xc6\x55\xe4\x20"
///默认MAC密钥
#define YH_DEFAULT_MAC_KEY                                                     \
  "\x59\x2f\xd4\x83\xf7\x59\xe2\x99\x09\xa0\x4c\x45\x05\xd2\xce\x0a"
///默认身份验证密钥密码
#define YH_DEFAULT_PASSWORD "password"
///salt用于pbkdf2密钥派生
#define YH_DEFAULT_SALT "Yubico"
///pbkdf2密钥派生的迭代次数
#define YH_DEFAULT_ITERS 10000
///能力阵列长度
#define YH_CAPABILITIES_LEN 8
///max设备可以保存的日志条目
#define YH_MAX_LOG_ENTRIES 64 //托多：真的是API的一部分吗？
///对象标签长度
#define YH_OBJ_LABEL_LEN 40
///max域数
#define YH_MAX_DOMAINS 16

//调试级别
//没有消息
#define YH_VERB_QUIET 0x00
///中间结果
#define YH_VERB_INTERMEDIATE 0x01
///加密结果
#define YH_VERB_CRYPTO 0x02
//原始消息
#define YH_VERB_RAW 0x04
//一般信息
#define YH_VERB_INFO 0x08
///错误消息
#define YH_VERB_ERR 0x10
///已启用所有以前的选项
#define YH_VERB_ALL 0xff

///This is the overhead when do aes ccm wrapping，1 byte identifier，13
///bytes nonce和16 bytes mac
#define YH_CCM_WRAP_OVERHEAD (1 + 13 + 16)

#ifdef __cplusplus
extern "C" {
#endif

///对连接器的引用
typedef struct yh_connector yh_connector;

///对会话的引用
typedef struct yh_session yh_session;

///capabilities表示
typedef struct {
///capabilities表示为一个8字节的uint8_t数组。
  uint8_t capabilities[YH_CAPABILITIES_LEN];
} yh_capabilities;

/*
 *返回代码。
 */

typedef enum {
//成功
  YHR_SUCCESS = 0,
//内存错误
  YHR_MEMORY = -1,
//初始化错误
  YHR_INIT_ERROR = -2,
//网络误差
  YHR_NET_ERROR = -3,
///找不到连接器
  YHR_CONNECTOR_NOT_FOUND = -4,
///参数无效
  YHR_INVALID_PARAMS = -5,
//错误的长度
  YHR_WRONG_LENGTH = -6,
///缓冲区太小
  YHR_BUFFER_TOO_SMALL = -7,
///密码错误
  YHR_CRYPTOGRAM_MISMATCH = -8,
///authenticate会话错误
  YHR_AUTH_SESSION_ERROR = -9,
///mac不匹配
  YHR_MAC_MISMATCH = -10,
///设备成功
  YHR_DEVICE_OK = -11,
///无效命令
  YHR_DEVICE_INV_COMMAND = -12,
///命令格式错误/数据无效
  YHR_DEVICE_INV_DATA = -13,
///无效会话
  YHR_DEVICE_INV_SESSION = -14,
///message加密/验证失败
  YHR_DEVICE_AUTH_FAIL = -15,
///分配了所有会话
  YHR_DEVICE_SESSIONS_FULL = -16,
///会话创建失败
  YHR_DEVICE_SESSION_FAILED = -17,
///存储失败
  YHR_DEVICE_STORAGE_FAILED = -18,
//错误的长度
  YHR_DEVICE_WRONG_LENGTH = -19,
///操作权限错误
  YHR_DEVICE_INV_PERMISSION = -20,
///log缓冲区已满，并设置了强制审核
  YHR_DEVICE_LOG_FULL = -21,
///找不到对象
  YHR_DEVICE_OBJ_NOT_FOUND = -22,
///id使用非法
  YHR_DEVICE_ID_ILLEGAL = -23,
///otp提交无效
  YHR_DEVICE_INVALID_OTP = -24,
///设备处于演示模式，必须进行电源循环
  YHR_DEVICE_DEMO_MODE = -25,
///命令执行未终止
  YHR_DEVICE_CMD_UNEXECUTED = -26,
//未知误差
  YHR_GENERIC_ERROR = -27,
///具有该ID的对象已存在
  YHR_DEVICE_OBJECT_EXISTS = -28,
///connector操作失败
  YHR_CONNECTOR_ERROR = -29
} yh_rc;

///macro定义命令和响应命令
#define ADD_COMMAND(c, v) c = v, c##_R = v | YH_CMD_RESP_FLAG

/*
 *命令定义
 **/

typedef enum {
///回声
  ADD_COMMAND(YHC_ECHO, 0x01),
///创建会话
  ADD_COMMAND(YHC_CREATE_SES, 0x03),
///authenticate会话
  ADD_COMMAND(YHC_AUTH_SES, 0x04),
///session消息
  ADD_COMMAND(YHC_SES_MSG, 0x05),
///get设备信息
  ADD_COMMAND(YHC_GET_DEVICE_INFO, 0x06),
//BSL
  ADD_COMMAND(YHC_BSL, 0x07),
//重置
  ADD_COMMAND(YHC_RESET, 0x08),
//关闭会话
  ADD_COMMAND(YHC_CLOSE_SES, 0x40),
///存储统计信息
  ADD_COMMAND(YHC_STATS, 0x041),
//不透明
  ADD_COMMAND(YHC_PUT_OPAQUE, 0x42),
//不透明
  ADD_COMMAND(YHC_GET_OPAQUE, 0x43),
///Put身份验证密钥
  ADD_COMMAND(YHC_PUT_AUTHKEY, 0x44),
///Put非对称密钥
  ADD_COMMAND(YHC_PUT_ASYMMETRIC_KEY, 0x45),
///生成非对称密钥
  ADD_COMMAND(YHC_GEN_ASYMMETRIC_KEY, 0x46),
///用pkcs1签名数据
  ADD_COMMAND(YHC_SIGN_DATA_PKCS1, 0x47),
//列表对象
  ADD_COMMAND(YHC_LIST, 0x48),
///用pkcs1解密数据
  ADD_COMMAND(YHC_DECRYPT_PKCS1, 0x49),
///导出包装的对象
  ADD_COMMAND(YHC_EXPORT_WRAPPED, 0x4a),
///导入包装的对象
  ADD_COMMAND(YHC_IMPORT_WRAPPED, 0x4b),
//放包钥匙
  ADD_COMMAND(YHC_PUT_WRAP_KEY, 0x4c),
///获取审核日志
  ADD_COMMAND(YHC_GET_LOGS, 0x4d),
///get对象信息
  ADD_COMMAND(YHC_GET_OBJECT_INFO, 0x4e),
///Put全局选项
  ADD_COMMAND(YHC_PUT_OPTION, 0x4f),
///获取全局选项
  ADD_COMMAND(YHC_GET_OPTION, 0x50),
///get伪随机数据
  ADD_COMMAND(YHC_GET_PSEUDO_RANDOM, 0x51),
//把HMAC密钥
  ADD_COMMAND(YHC_PUT_HMAC_KEY, 0x52),
//HMAC数据
  ADD_COMMAND(YHC_HMAC_DATA, 0x53),
///获取公钥
  ADD_COMMAND(YHC_GET_PUBKEY, 0x54),
///用PSS签署数据
  ADD_COMMAND(YHC_SIGN_DATA_PSS, 0x55),
///用ECDSA签名数据
  ADD_COMMAND(YHC_SIGN_DATA_ECDSA, 0x56),
///进行ECDH交换
  ADD_COMMAND(YHC_DECRYPT_ECDH, 0x57),
///删除对象
  ADD_COMMAND(YHC_DELETE_OBJECT, 0x58),
///使用OAEP解密数据
  ADD_COMMAND(YHC_DECRYPT_OAEP, 0x59),
///生成HMAC密钥
  ADD_COMMAND(YHC_GENERATE_HMAC_KEY, 0x5a),
///生成换行键
  ADD_COMMAND(YHC_GENERATE_WRAP_KEY, 0x5b),
///验证HMAC数据
  ADD_COMMAND(YHC_VERIFY_HMAC, 0x5c),
//SSH认证
  ADD_COMMAND(YHC_SSH_CERTIFY, 0x5d),
//放置模板
  ADD_COMMAND(YHC_PUT_TEMPLATE, 0x5e),
//获取模板
  ADD_COMMAND(YHC_GET_TEMPLATE, 0x5f),
//解密OTP
  ADD_COMMAND(YHC_OTP_DECRYPT, 0x60),
///创建OTP AEAD
  ADD_COMMAND(YHC_OTP_AEAD_CREATE, 0x61),
///从随机创建OTP AEAD
  ADD_COMMAND(YHC_OTP_AEAD_RANDOM, 0x62),
///重新包装OTP AEAD
  ADD_COMMAND(YHC_OTP_AEAD_REWRAP, 0x63),
///证明非对称密钥
  ADD_COMMAND(YHC_ATTEST_ASYMMETRIC, 0x64),
///PUT OTP AEAD密钥
  ADD_COMMAND(YHC_PUT_OTP_AEAD_KEY, 0x65),
///生成OTP AEAD密钥
  ADD_COMMAND(YHC_GENERATE_OTP_AEAD_KEY, 0x66),
//设置日志索引
  ADD_COMMAND(YHC_SET_LOG_INDEX, 0x67),
//数据包
  ADD_COMMAND(YHC_WRAP_DATA, 0x68),
//展开数据
  ADD_COMMAND(YHC_UNWRAP_DATA, 0x69),
///与EDDSA签署数据
  ADD_COMMAND(YHC_SIGN_DATA_EDDSA, 0x6a),
///闪烁设备
  ADD_COMMAND(YHC_BLINK, 0x6b),
///误差
  YHC_ERROR = 0x7f,
} yh_cmd;

#undef ADD_COMMAND

/*
 *对象类型
 **/

typedef enum {
//不透明物体
  YH_OPAQUE = 0x01,
///身份验证密钥
  YH_AUTHKEY = 0x02,
///非对称密钥
  YH_ASYMMETRIC = 0x03,
//包绕键
  YH_WRAPKEY = 0x04,
//HMAC密钥
  YH_HMACKEY = 0x05,
//模板
  YH_TEMPLATE = 0x06,
///OTP AEAD键
  YH_OTP_AEAD_KEY = 0x07,
///public key（虚拟..）
  YH_PUBLIC = 0x83,
} yh_object_type;

///此处定义的最大算法数
#define YH_MAX_ALGORITHM_COUNT 0xff
/*
 ＊算法
 **/

typedef enum {
  YH_ALGO_RSA_PKCS1_SHA1 = 1,
  YH_ALGO_RSA_PKCS1_SHA256 = 2,
  YH_ALGO_RSA_PKCS1_SHA384 = 3,
  YH_ALGO_RSA_PKCS1_SHA512 = 4,
  YH_ALGO_RSA_PSS_SHA1 = 5,
  YH_ALGO_RSA_PSS_SHA256 = 6,
  YH_ALGO_RSA_PSS_SHA384 = 7,
  YH_ALGO_RSA_PSS_SHA512 = 8,
  YH_ALGO_RSA_2048 = 9,
  YH_ALGO_RSA_3072 = 10,
  YH_ALGO_RSA_4096 = 11,
  YH_ALGO_EC_P256 = 12,
  YH_ALGO_EC_P384 = 13,
  YH_ALGO_EC_P521 = 14,
  YH_ALGO_EC_K256 = 15,
  YH_ALGO_EC_BP256 = 16,
  YH_ALGO_EC_BP384 = 17,
  YH_ALGO_EC_BP512 = 18,
  YH_ALGO_HMAC_SHA1 = 19,
  YH_ALGO_HMAC_SHA256 = 20,
  YH_ALGO_HMAC_SHA384 = 21,
  YH_ALGO_HMAC_SHA512 = 22,
  YH_ALGO_EC_ECDSA_SHA1 = 23,
  YH_ALGO_EC_ECDH = 24,
  YH_ALGO_RSA_OAEP_SHA1 = 25,
  YH_ALGO_RSA_OAEP_SHA256 = 26,
  YH_ALGO_RSA_OAEP_SHA384 = 27,
  YH_ALGO_RSA_OAEP_SHA512 = 28,
  YH_ALGO_AES128_CCM_WRAP = 29,
  YH_ALGO_OPAQUE_DATA = 30,
  YH_ALGO_OPAQUE_X509_CERT = 31,
  YH_ALGO_MGF1_SHA1 = 32,
  YH_ALGO_MGF1_SHA256 = 33,
  YH_ALGO_MGF1_SHA384 = 34,
  YH_ALGO_MGF1_SHA512 = 35,
  YH_ALGO_TEMPL_SSH = 36,
  YH_ALGO_YUBICO_OTP_AES128 = 37,
  YH_ALGO_YUBICO_AES_AUTH = 38,
  YH_ALGO_YUBICO_OTP_AES192 = 39,
  YH_ALGO_YUBICO_OTP_AES256 = 40,
  YH_ALGO_AES192_CCM_WRAP = 41,
  YH_ALGO_AES256_CCM_WRAP = 42,
  YH_ALGO_EC_ECDSA_SHA256 = 43,
  YH_ALGO_EC_ECDSA_SHA384 = 44,
  YH_ALGO_EC_ECDSA_SHA512 = 45,
  YH_ALGO_EC_ED25519 = 46,
  YH_ALGO_EC_P224 = 47,
} yh_algorithm;

/*
 *全局选项
 **/

typedef enum {
///强制审核模式
  YH_OPTION_FORCE_AUDIT = 1,
///audit每个命令的日志记录
  YH_OPTION_COMMAND_AUDIT = 3,
} yh_option;

/*
 *连接器选项，用yh_set_connector_option（）设置
 **/

typedef enum {
///file with ca certificate to validate the connector with（const char*）not
///在Windows上实现
  YH_CONNECTOR_HTTPS_CA = 1,
///proxy server用于连接连接器（const char*）而不是
///在Windows上实现
  YH_CONNECTOR_PROXY_SERVER = 2,
} yh_connector_option;

///日志摘要截断为的大小
#define YH_LOG_DIGEST_SIZE 16
#pragma pack(push, 1)
/*
 *记录设备返回的结构
 **/

typedef struct {
///指数单调递增
  uint16_t number;
///what命令被执行@see yh_cmd
  uint8_t command;
///数据长度
  uint16_t length;
///id使用的身份验证密钥
  uint16_t session_key;
///id使用的第一个对象
  uint16_t target_key;
///id使用的第二个对象
  uint16_t second_key;
///command result@see yh_命令
  uint8_t result;
///s执行时单击
  uint32_t systick;
///truncted sha256这个最后一个摘要+这个条目的摘要
  uint8_t digest[YH_LOG_DIGEST_SIZE];
} yh_log_entry;

/*
 *对象描述符
 **/

typedef struct {
///object capabilities@see yh_capabilities/对象能力@see yh_能力
  yh_capabilities capabilities;
//对象标识
  uint16_t id;
//对象长度
  uint16_t len;
///对象域
  uint16_t domains;
//对象类型
  yh_object_type type;
///对象算法
  yh_algorithm algorithm;
///对象序列
  uint8_t sequence;
//对象起源
  uint8_t origin;
//对象标签
  char label[YH_OBJ_LABEL_LEN + 1];
///对象委派功能
  yh_capabilities delegated_capabilities;
} yh_object_descriptor;
#pragma pack(pop)

static const struct {
  const char *name;
  int bit;
} yh_capability[] = {
  {"asymmetric_decrypt_ecdh", 0x0b},
  {"asymmetric_decrypt_oaep", 0x0a},
  {"asymmetric_decrypt_pkcs", 0x09},
  {"asymmetric_gen", 0x04},
  {"asymmetric_sign_ecdsa", 0x07},
  {"asymmetric_sign_eddsa", 0x08},
  {"asymmetric_sign_pkcs", 0x05},
  {"asymmetric_sign_pss", 0x06},
  {"attest", 0x22},
  {"audit", 0x18},
  {"export_under_wrap", 0x10},
  {"export_wrapped", 0x0c},
  {"delete_asymmetric", 0x29},
  {"delete_authkey", 0x28},
  {"delete_hmackey", 0x2b},
  {"delete_opaque", 0x27},
  {"delete_otp_aead_key", 0x2d},
  {"delete_template", 0x2c},
  {"delete_wrapkey", 0x2a},
  {"generate_otp_aead_key", 0x24},
  {"generate_wrapkey", 0x0f},
  {"get_opaque", 0x00},
  {"get_option", 0x12},
  {"get_randomness", 0x13},
  {"get_template", 0x1a},
  {"hmackey_generate", 0x15},
  {"hmac_data", 0x16},
  {"hmac_verify", 0x17},
  {"import_wrapped", 0x0d},
  {"otp_aead_create", 0x1e},
  {"otp_aead_random", 0x1f},
  {"otp_aead_rewrap_from", 0x20},
  {"otp_aead_rewrap_to", 0x21},
  {"otp_decrypt", 0x1d},
  {"put_asymmetric", 0x03},
  {"put_authkey", 0x02},
  {"put_hmackey", 0x14},
  {"put_opaque", 0x01},
  {"put_option", 0x11},
  {"put_otp_aead_key", 0x23},
  {"put_template", 0x1b},
  {"put_wrapkey", 0x0e},
  {"reset", 0x1c},
  {"ssh_certify", 0x19},
  {"unwrap_data", 0x26},
  {"wrap_data", 0x25},
};

static const struct {
  const char *name;
  yh_algorithm algorithm;
} yh_algorithms[] = {
  {"aes128-ccm-wrap", YH_ALGO_AES128_CCM_WRAP},
  {"aes192-ccm-wrap", YH_ALGO_AES192_CCM_WRAP},
  {"aes256-ccm-wrap", YH_ALGO_AES256_CCM_WRAP},
  {"ecbp256", YH_ALGO_EC_BP256},
  {"ecbp384", YH_ALGO_EC_BP384},
  {"ecbp512", YH_ALGO_EC_BP512},
  {"ecdsa-sha1", YH_ALGO_EC_ECDSA_SHA1},
  {"ecdsa-sha256", YH_ALGO_EC_ECDSA_SHA256},
  {"ecdsa-sha384", YH_ALGO_EC_ECDSA_SHA384},
  {"ecdsa-sha512", YH_ALGO_EC_ECDSA_SHA512},
  {"ecdh", YH_ALGO_EC_ECDH},
  {"eck256", YH_ALGO_EC_K256},
  {"ecp224", YH_ALGO_EC_P224},
  {"ecp256", YH_ALGO_EC_P256},
  {"ecp384", YH_ALGO_EC_P384},
  {"ecp521", YH_ALGO_EC_P521},
  {"ed25519", YH_ALGO_EC_ED25519},
  {"hmac-sha1", YH_ALGO_HMAC_SHA1},
  {"hmac-sha256", YH_ALGO_HMAC_SHA256},
  {"hmac-sha384", YH_ALGO_HMAC_SHA384},
  {"hmac-sha512", YH_ALGO_HMAC_SHA512},
  {"mgf1-sha1", YH_ALGO_MGF1_SHA1},
  {"mgf1-sha256", YH_ALGO_MGF1_SHA256},
  {"mgf1-sha384", YH_ALGO_MGF1_SHA384},
  {"mgf1-sha512", YH_ALGO_MGF1_SHA512},
  {"opaque", YH_ALGO_OPAQUE_DATA},
  {"rsa2048", YH_ALGO_RSA_2048},
  {"rsa3072", YH_ALGO_RSA_3072},
  {"rsa4096", YH_ALGO_RSA_4096},
  {"rsa-pkcs1-sha1", YH_ALGO_RSA_PKCS1_SHA1},
  {"rsa-pkcs1-sha256", YH_ALGO_RSA_PKCS1_SHA256},
  {"rsa-pkcs1-sha384", YH_ALGO_RSA_PKCS1_SHA384},
  {"rsa-pkcs1-sha512", YH_ALGO_RSA_PKCS1_SHA512},
  {"rsa-pss-sha1", YH_ALGO_RSA_PSS_SHA1},
  {"rsa-pss-sha256", YH_ALGO_RSA_PSS_SHA256},
  {"rsa-pss-sha384", YH_ALGO_RSA_PSS_SHA384},
  {"rsa-pss-sha512", YH_ALGO_RSA_PSS_SHA512},
  {"rsa-oaep-sha1", YH_ALGO_RSA_OAEP_SHA1},
  {"rsa-oaep-sha256", YH_ALGO_RSA_OAEP_SHA256},
  {"rsa-oaep-sha384", YH_ALGO_RSA_OAEP_SHA384},
  {"rsa-oaep-sha512", YH_ALGO_RSA_OAEP_SHA512},
  {"template-ssh", YH_ALGO_TEMPL_SSH},
  {"x509-cert", YH_ALGO_OPAQUE_X509_CERT},
  {"yubico-aes-auth", YH_ALGO_YUBICO_AES_AUTH},
  {"yubico-otp-aes128", YH_ALGO_YUBICO_OTP_AES128},
  {"yubico-otp-aes192", YH_ALGO_YUBICO_OTP_AES192},
  {"yubico-otp-aes256", YH_ALGO_YUBICO_OTP_AES256},
};

static const struct {
  const char *name;
  yh_object_type type;
} yh_types[] = {
  {"authkey", YH_AUTHKEY},         {"asymmetric", YH_ASYMMETRIC},
  {"hmackey", YH_HMACKEY},         {"opaque", YH_OPAQUE},
  {"otpaeadkey", YH_OTP_AEAD_KEY}, {"template", YH_TEMPLATE},
  {"wrapkey", YH_WRAPKEY},
};

static const struct {
  const char *name;
  yh_option option;
} yh_options[] = {
  {"command_audit", YH_OPTION_COMMAND_AUDIT},
  {"force_audit", YH_OPTION_FORCE_AUDIT},
};

///生成原点
#define YH_ORIGIN_GENERATED 0x01
///来源已导入
#define YH_ORIGIN_IMPORTED 0x02
///origin被包装（注意：这与原始对象一起使用
//原产地）
#define YH_ORIGIN_IMPORTED_WRAPPED 0x10

/*
 *返回描述错误条件的字符串
 *
 *@param err yh_rc错误代码
 *
 *@返回带描述性错误的字符串
 */

const char *yh_strerror(yh_rc err);

/*
 *设置冗长
 *此函数可以在全局库初始化之前调用。
 *
 *@参数详细程度
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_set_verbosity(uint8_t verbosity);

/*
 *获得冗长
 *
 *@参数详细程度
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_get_verbosity(uint8_t *verbosity);

/*
 *设置调试输出文件
 *
 *@ PARAM输出
 *
 *@返回无效
 */

void yh_set_debug_output(FILE *output);

/*
 *全局库初始化
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_init(void);

/*
 *全局库清理
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_exit(void);

/*
 *实例化新连接器
 *
 *@param url要与此连接器关联的URL
 *@param connector对connector的引用
 *
 *@返回YH_rc错误代码
 **/

yh_rc yh_init_connector(const char *url, yh_connector **connector);

/*
 *设置连接器选项
 *
 *@param connector connector设置选项
 *@param opt选项设置@see yh_connector_选项
 *@param val要设置的值，类型特定于给定的选项
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_set_connector_option(yh_connector *connector, yh_connector_option opt,
                              const void *val);

/*
 *连接到所有指定的连接器
 *
 *@param connectors连接器数组指针
 *@param n_connectors数组中的连接器数（将设置为
 *返回成功的连接器）
 *@param超时（秒）
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_connect_all(yh_connector **connectors, size_t *n_connectors,
                     int timeout);

/*
 *连接到阵列中的一个连接器
 *
 *@param connectors连接器数组指针
 *@param n_connectors数组中的连接器数
 *@param idx已连接连接器的索引，可能为空
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_connect_best(yh_connector **connectors, size_t n_connectors, int *idx);

/*
 *从连接器上断开
 *
 *@param要断开的连接器
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_disconnect(yh_connector *connector);

/*
 *向连接器发送纯消息
 *
 *@param要发送到的连接器连接器
 *@param cmd要发送的命令
 *@param要发送的数据
 *@参数数据长度
 *@param response_cmd response命令
 *@param响应数据
 *@参数响应长度
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_send_plain_msg(yh_connector *connector, yh_cmd cmd,
                        const uint8_t *data, size_t data_len,
                        yh_cmd *response_cmd, uint8_t *response,
                        size_t *response_len);

/*
 *通过会话发送加密消息
 *
 *@param要发送的会话会话
 *@param cmd要发送的命令
 *@param要发送的数据
 *@参数数据长度
 *@param response_cmd response命令
 *@param响应数据
 *@参数响应长度
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_send_secure_msg(yh_session *session, yh_cmd cmd, const uint8_t *data,
                         size_t data_len, yh_cmd *response_cmd,
                         uint8_t *response, size_t *response_len);

/*
 *使用从密钥派生的frm密码创建会话
 *
 用于创建会话的@param connector连接器
 *@param auth_keyset_id要使用的身份验证密钥的ID
 *@param从中派生密钥的密码
 *@param password_len密码的长度（字节）
 *@param recreate_session会话将被重新创建。如果过期，将缓存
 *内存中的密码
 用于身份验证的@param上下文上下文数据
 *@param context\u len context length（参数上下文长度）
 *@param session创建的会话
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_create_session_derived(yh_connector *connector,
                                uint16_t auth_keyset_id,
                                const uint8_t *password, size_t password_len,
                                bool recreate_session, uint8_t *context,
                                size_t context_len, yh_session **session);

/*
 *创建会话
 *
 用于创建会话的@param connector连接器
 *@param auth_keyset_身份验证密钥的ID
 *@param key_enc加密密钥
 *@param key_enc_len加密密钥的长度
 *@param键\u mac键
 *@param key_mac_len mac key的长度
 *@param recreate_session会话将被重新创建。如果过期，将缓存
 *内存中的密码
 用于身份验证的@param上下文上下文数据
 *@param context\u len context length（参数上下文长度）
 *@param session创建的会话
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_create_session(yh_connector *connector, uint16_t auth_keyset_id,
                        const uint8_t *key_enc, size_t key_enc_len,
                        const uint8_t *key_mac, size_t key_mac_len,
                        bool recreate_session, uint8_t *context,
                        size_t context_len, yh_session **session);

/*
 *开始创建扩展会话
 *
 用于创建会话的@param connector连接器
 *@param auth_keyset_身份验证密钥的ID
 用于身份验证的@param上下文上下文数据
 *@param context\u len上下文数据的长度
 *@param card_cryptoram card cryptoram（参数卡密码卡）
 *@param card_cryptoram_len catd cryptoram length（参数卡密码长度）
 *@param session创建的会话
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_begin_create_session_ext(yh_connector *connector,
                                  uint16_t auth_keyset_id, uint8_t *context,
                                  size_t context_len, uint8_t *card_cryptogram,
                                  size_t card_cryptogram_len,
                                  yh_session **session);

/*
 *完成创建外部会话
 *
 用于创建会话的@param connector连接器
 *@param会话会话
 *@param key_senc会话加密密钥
 *@param key_senc_len session加密密钥长度
 *@param key_smac会话mac key
 *@param key_smac_len session mac key length（参数密钥长度）
 *@param key_srmac session返回mac key
 *@param key_srmac_len session返回mac key长度
 *@param上下文数据
 *@param context\u len context length（参数上下文长度）
 *@param card_cryptoram card cryptoram（参数卡密码卡）
 *@param card_cryptoram_len card cryptoram length/参数卡密码长度
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_finish_create_session_ext(yh_connector *connector, yh_session *session,
                                   const uint8_t *key_senc, size_t key_senc_len,
                                   const uint8_t *key_smac, size_t key_smac_len,
                                   const uint8_t *key_srmac,
                                   size_t key_srmac_len, uint8_t *context,
                                   size_t context_len, uint8_t *card_cryptogram,
                                   size_t card_cryptogram_len);

/*
 *与会话关联的可用数据
 *
 *@param要销毁的会话会话
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_destroy_session(yh_session **session);

/*
 *验证会话
 *
 *@param要验证的会话会话
 *@param上下文数据
 *@param context\u len context length（参数上下文长度）
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_authenticate_session(yh_session *session, uint8_t *context,
                              size_t context_len);

//实用和便利功能如下

/*
 *获取设备信息
 *
 *@param要发送的连接器
 *@param major版本major
 *@param minor版本minor
 *@param补丁版本路径
 *@参数序列号
 *@param log_日志条目总数
 *@param log使用的日志条目
 *@param算法数组
 *@param n_算法算法数量
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_get_device_info(yh_connector *connector, uint8_t *major,
                              uint8_t *minor, uint8_t *patch, uint32_t *serial,
                              uint8_t *log_total, uint8_t *log_used,
                              yh_algorithm *algorithms, size_t *n_algorithms);

/*
 *列表对象
 *
 *@param要使用的会话会话
 *@param id要筛选依据（0不筛选依据id）
 *@param type type to filter by（0 to not filter by type）@参见yh_object_type
 *@param domains要筛选的域（0不按域筛选）
 *@param过滤能力（0不过滤
 *能力）@参见YHU能力
 *@param算法要过滤的算法（0不按算法过滤）
 *@param要筛选的标签
 *@param objects返回的对象数组
 *@param n_objects对象的最大长度（将设置为在
 *返回）
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_list_objects(yh_session *session, uint16_t id,
                           yh_object_type type, uint16_t domains,
                           const yh_capabilities *capabilities,
                           yh_algorithm algorithm, const char *label,
                           yh_object_descriptor *objects, size_t *n_objects);

/*
 *获取对象信息
 *
 *@param要使用的会话会话
 *@param id对象id
 *@param type对象类型
 *@param对象对象信息
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_get_object_info(yh_session *session, uint16_t id,
                              yh_object_type type,
                              yh_object_descriptor *object);

/*
 *获取公钥
 *
 *@param要使用的会话会话
 *@param id对象id
 *@参数数据输出
 *@param datalen数据长度
 *@param对象算法
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_get_pubkey(yh_session *session, uint16_t id, uint8_t *data,
                         size_t *datalen, yh_algorithm *algorithm);

/*
 ＊关闭会话
 *
 *@param要关闭的会话会话
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_close_session(yh_session *session);

/*
 *使用pkcs1 v1.5签名数据
 *
 *<tt>in</tt>is either a raw hashed message（sha1，sha256，sha384 or sha512）
 *或者有正确的DigestInfo预挂。
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@param hashed如果只对数据进行hashed
 待签名数据中的*@param
 *@param英寸长度英寸
 *@param out签名数据
 *@param out\u len签名数据的长度
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_sign_pkcs1v1_5(yh_session *session, uint16_t key_id, bool hashed,
                             const uint8_t *in, size_t in_len, uint8_t *out,
                             size_t *out_len);

/*
 *使用PSS签署数据
 *
 *<tt>in</tt>is a raw hashed message（sha1，sha256，sha384 or sha512）.
 *
 *@param要使用的会话会话
 *@param key_id对象id
 待签名数据中的*@param
 *@param英寸长度英寸
 *@param out签名数据
 *@param out\u len签名数据的长度
 *@参数盐长度
 *@param mgf1用于mgf1的算法
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_sign_pss(yh_session *session, uint16_t key_id, const uint8_t *in,
                       size_t in_len, uint8_t *out, size_t *out_len,
                       size_t salt_len, yh_algorithm mgf1Algo);

/*
 *使用ECDSA对数据进行签名
 *
 *<tt>in</tt>是原始散列消息，截断散列到曲线长度或
 *填充哈希到曲线长度。
 *
 *@param要使用的会话会话
 *@param key_id对象id
 待签名数据中的*@param
 *@param英寸长度英寸
 *@param out签名数据
 *@param out\u len签名数据的长度
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_sign_ecdsa(yh_session *session, uint16_t key_id,
                         const uint8_t *in, size_t in_len, uint8_t *out,
                         size_t *out_len);

/*
 *使用EDDSA签署数据
 *
 *@param要使用的会话会话
 *@param key_id对象id
 待签名数据中的*@param
 *@param英寸长度英寸
 *@param out签名数据
 *@param out\u len签名数据的长度
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_sign_eddsa(yh_session *session, uint16_t key_id,
                         const uint8_t *in, size_t in_len, uint8_t *out,
                         size_t *out_len);

/*
 * HMAC数据
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@param输入HMAC的数据
 *@param英寸长度英寸
 *@参数输出HMAC
 *@param out_len HMAC的长度
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_hmac(yh_session *session, uint16_t key_id, const uint8_t *in,
                   size_t in_len, uint8_t *out, size_t *out_len);

/*
 *获取伪随机数据
 *
 *@param要使用的会话会话
 *@param len要获取的数据长度
 *@param out随机数据out
 *@param out_len随机数据长度
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_get_random(yh_session *session, size_t len, uint8_t *out,
                         size_t *out_len);

/*
 *导入RSA密钥
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@param标签
 *@param域域
 *@param能力
 *@param算法
 ＊PARAM P P
 *@帕拉姆Q Q
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_import_key_rsa(yh_session *session, uint16_t *key_id,
                             const char *label, uint16_t domains,
                             const yh_capabilities *capabilities,
                             yh_algorithm algorithm, const uint8_t *p,
                             const uint8_t *q);

/*
 *导入EC密钥
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@param标签
 *@param域域
 *@param能力
 *@param算法
 *@帕拉姆斯
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_import_key_ec(yh_session *session, uint16_t *key_id,
                            const char *label, uint16_t domains,
                            const yh_capabilities *capabilities,
                            yh_algorithm algorithm, const uint8_t *s);

/*
 ＊导入ED键
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@param标签
 *@param域域
 *@param能力
 *@param算法
 *@帕拉姆K K
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_import_key_ed(yh_session *session, uint16_t *key_id,
                            const char *label, uint16_t domains,
                            const yh_capabilities *capabilities,
                            yh_algorithm algorithm, const uint8_t *k);

/*
 *导入HMAC密钥
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@param标签
 *@param域域
 *@param能力
 *@param算法
 *@param键数据
 *@param key_len密钥长度
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_import_key_hmac(yh_session *session, uint16_t *key_id,
                              const char *label, uint16_t domains,
                              const yh_capabilities *capabilities,
                              yh_algorithm algorithm, const uint8_t *key,
                              size_t key_len);

/*
 *生成RSA密钥
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@param标签
 *@param域域
 *@param能力
 *@param算法
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_generate_key_rsa(yh_session *session, uint16_t *key_id,
                               const char *label, uint16_t domains,
                               const yh_capabilities *capabilities,
                               yh_algorithm algorithm);

/*
 *生成EC密钥
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@param标签
 *@param域域
 *@param能力
 *@param算法
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_generate_key_ec(yh_session *session, uint16_t *key_id,
                              const char *label, uint16_t domains,
                              const yh_capabilities *capabilities,
                              yh_algorithm algorithm);

/*
 *生成ED密钥
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@param标签
 *@param域域
 *@param能力
 *@param算法
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_generate_key_ed(yh_session *session, uint16_t *key_id,
                              const char *label, uint16_t domains,
                              const yh_capabilities *capabilities,
                              yh_algorithm algorithm);

/*
 *验证HMAC数据
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@param签名HMAC
 *@参数签名长度
 *@param要验证的数据
 *@参数数据长度
 *@param验证是否成功
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_hmac_verify(yh_session *session, uint16_t key_id,
                          const uint8_t *signature, size_t signature_len,
                          const uint8_t *data, size_t data_len, bool *verified);

/*
 *生成HMAC密钥
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@param标签
 *@param域域
 *@param能力
 *@param算法
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_generate_key_hmac(yh_session *session, uint16_t *key_id,
                                const char *label, uint16_t domains,
                                const yh_capabilities *capabilities,
                                yh_algorithm algorithm);

/*
 *解密PKCS1 v1.5数据
 *
 *@param要使用的会话会话
 *@param key_id对象id
 加密数据中的*@param
 以加密数据长度表示的*@param
 *@param出解密数据
 *@param out \u len解密数据的长度
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_decrypt_pkcs1v1_5(yh_session *session, uint16_t key_id,
                                const uint8_t *in, size_t in_len, uint8_t *out,
                                size_t *out_len);

/*
 *解密OAEP数据
 *
 *@param要使用的会话会话
 *@param key_id对象id
 加密数据中的*@param
 以加密数据长度表示的*@param
 *@param出解密数据
 *@param out \u len解密数据的长度
 *@param label oaep标签
 *@参数标签长度
 *@param mgf1algo mgf1算法
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_decrypt_oaep(yh_session *session, uint16_t key_id,
                           const uint8_t *in, size_t in_len, uint8_t *out,
                           size_t *out_len, const uint8_t *label,
                           size_t label_len, yh_algorithm mgf1Algo);

/*
 *进行ECDH密钥交换
 *
 *@param要使用的会话会话
 *@param key_id对象id
 公钥中的*@param
 公钥长度中的*@param
 *@param out约定密钥
 *@param out\u len商定密钥的长度
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_decrypt_ecdh(yh_session *session, uint16_t key_id,
                           const uint8_t *in, size_t in_len, uint8_t *out,
                           size_t *out_len);

/*
 *删除对象
 *
 *@param要使用的会话会话
 *@param id对象id
 *@param type对象类型
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_delete_object(yh_session *session, uint16_t id,
                            yh_object_type type);

/*
 *导出包裹下的对象
 *
 *@param要使用的会话会话
 *@param wrapping_key_id包装密钥的ID
 *@param target_对象类型
 *@param target_id对象的ID
 *@param输出包装数据
 *@param out_len包装数据的长度
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_export_wrapped(yh_session *session, uint16_t wrapping_key_id,
                             yh_object_type target_type, uint16_t target_id,
                             uint8_t *out, size_t *out_len);

/*
 *导入包装对象
 *
 *@param要使用的会话会话
 *@param wrapping_key_id包装密钥的ID
 打包数据中的*@param
 *@param，以包装数据的长度为单位
 *@param target_键入导入对象的类型
 *@param target_id导入对象的ID
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_import_wrapped(yh_session *session, uint16_t wrapping_key_id,
                             const uint8_t *in, size_t in_len,
                             yh_object_type *target_type, uint16_t *target_id);

/*
 *导入包装密钥
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@param标签
 *@param域域
 *@param能力
 *@param算法
 *@param委派的能力委派的能力
 *PARAM键
 *@param以长度键表示
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_import_key_wrap(yh_session *session, uint16_t *key_id,
                              const char *label, uint16_t domains,
                              const yh_capabilities *capabilities,
                              yh_algorithm algorithm,
                              const yh_capabilities *delegated_capabilities,
                              const uint8_t *in, size_t in_len);

/*
 *生成一个环绕键
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@param标签
 *@param域域
 *@param能力
 *@param算法
 *@param委派的能力委派的能力
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_generate_key_wrap(yh_session *session, uint16_t *key_id,
                                const char *label, uint16_t domains,
                                const yh_capabilities *capabilities,
                                yh_algorithm algorithm,
                                const yh_capabilities *delegated_capabilities);

/*
 *获取日志
 *
 *@param要使用的会话会话
 *@param unloaded_boot unloaded boot数
 *@param unloaded_auth unloaded authentications的数目
 *@param输出日志项数组
 *@param n_items进出项目数
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_get_logs(yh_session *session, uint16_t *unlogged_boot,
                       uint16_t *unlogged_auth, yh_log_entry *out,
                       size_t *n_items);

/*
 *设置日志索引
 *
 *@param要使用的会话会话
 *@param要设置的索引
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_set_log_index(yh_session *session, uint16_t index);

/*
 *获取不透明对象
 *
 *@param要使用的会话会话
 *@param object_id对象ID
 *@param out数据
 *@参数输出长度输出
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_get_opaque(yh_session *session, uint16_t object_id, uint8_t *out,
                         size_t *out_len);

/*
 *导入不透明对象
 *
 *@param要使用的会话会话
 *@param object_id对象ID
 *@param标签
 *@param域域
 *@param能力
 *@param算法
 对象数据中的*@param
 *@param英寸长度英寸
 *
 *@返回
 */

yh_rc yh_util_import_opaque(yh_session *session, uint16_t *object_id,
                            const char *label, uint16_t domains,
                            const yh_capabilities *capabilities,
                            yh_algorithm algorithm, const uint8_t *in,
                            size_t in_len);

/*
 ＊SSH认证
 *
 *@param要使用的会话会话
 *@param key_id密钥ID
 *@param template_id模板id
 *@param sig_algo签名算法
 证书请求中的*@param
 *@param英寸长度英寸
 *@param out签名
 *@参数输出长度输出
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_ssh_certify(yh_session *session, uint16_t key_id,
                          uint16_t template_id, yh_algorithm sig_algo,
                          const uint8_t *in, size_t in_len, uint8_t *out,
                          size_t *out_len);

/*
 *导入身份验证密钥
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@param标签
 *@param域域
 *@param能力
 *@param委派的能力委派的能力
 *@param要从中派生密钥的密码
 *@param password_len password以字节为单位的长度
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_import_authkey(yh_session *session, uint16_t *key_id,
                             const char *label, uint16_t domains,
                             const yh_capabilities *capabilities,
                             const yh_capabilities *delegated_capabilities,
                             const uint8_t *password, size_t password_len);

/*
 *获取模板
 *
 *@param要使用的会话会话
 *@param object_id对象ID
 *@param out数据
 *@参数输出长度输出
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_get_template(yh_session *session, uint16_t object_id,
                           uint8_t *out, size_t *out_len);

/*
 *导入模板
 *
 *@param要使用的会话会话
 *@param object_id对象ID
 *@param标签
 *@param域域
 *@param能力
 *@param算法
 数据中的*@param
 *@param英寸长度英寸
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_import_template(yh_session *session, uint16_t *object_id,
                              const char *label, uint16_t domains,
                              const yh_capabilities *capabilities,
                              yh_algorithm algorithm, const uint8_t *in,
                              size_t in_len);

/*
 *创建OTP AEAD
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@参数键OTP键
 *@param private_id OTP私人ID
 *@参数输出AEAD
 *@参数输出长度输出
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_otp_aead_create(yh_session *session, uint16_t key_id,
                              const uint8_t *key, const uint8_t *private_id,
                              uint8_t *out, size_t *out_len);

/*
 *随机创建OTP AEAD
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@参数输出AEAD
 *@参数输出长度输出
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_otp_aead_random(yh_session *session, uint16_t key_id,
                              uint8_t *out, size_t *out_len);

/*
 ＊解密OTP
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@参数AEAD AEAD
 *参数AEAD长度
 *@参数OTP OTP
 *@param usectr otp使用计数器
 *@param sessionctr otp会话计数器
 *@param tstph otp timestamp高
 *@param tstpl otp timestamp低
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_otp_decrypt(yh_session *session, uint16_t key_id,
                          const uint8_t *aead, size_t aead_len,
                          const uint8_t *otp, uint16_t *useCtr,
                          uint8_t *sessionCtr, uint8_t *tstph, uint16_t *tstpl);

/*
 *导入OTP AEAD密钥
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@param标签
 *@param域域
 *@param能力
 *@param nonce\u id nonce id（参数当前ID）
 *PARAM键
 *@param英寸长度英寸
 *
 *@返回
 */

yh_rc yh_util_put_otp_aead_key(yh_session *session, uint16_t *key_id,
                               const char *label, uint16_t domains,
                               const yh_capabilities *capabilities,
                               uint32_t nonce_id, const uint8_t *in,
                               size_t in_len);

/*
 *生成OTP AEAD密钥
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@param标签
 *@param域域
 *@param能力
 *@param算法
 *@param nonce\u id nonce id（参数当前ID）
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_generate_otp_aead_key(yh_session *session, uint16_t *key_id,
                                    const char *label, uint16_t domains,
                                    const yh_capabilities *capabilities,
                                    yh_algorithm algorithm, uint32_t nonce_id);

/*
 *证明非对称密钥
 *
 *@param要使用的会话会话
 *@param key_id对象id
 *@param认证密钥ID
 *@param out证书
 *@参数输出长度输出
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_attest_asymmetric(yh_session *session, uint16_t key_id,
                                uint16_t attest_id, uint8_t *out,
                                size_t *out_len);

/*
 *看跌期权
 *
 *@param要使用的会话会话
 *@param选项
 *@param len选项数据的长度
 *@param val选项数据
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_put_option(yh_session *session, yh_option option, size_t len,
                         uint8_t *val);

/*
 *获取全局选项
 *
 *@param要使用的会话会话
 *@param选项
 *@param out选项数据
 *@参数输出长度输出
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_get_option(yh_session *session, yh_option option, uint8_t *out,
                         size_t *out_len);

/*
 *获取存储统计信息
 *
 *@param要使用的会话会话
 *@param total_records可用的总记录数
 *@param free_记录可用记录数
 *@param total_pages可用总页数
 *@param free_pages可用页数
 *@param page_以字节为单位调整页面大小
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_get_storage_stats(yh_session *session, uint16_t *total_records,
                                uint16_t *free_records, uint16_t *total_pages,
                                uint16_t *free_pages, uint16_t *page_size);

/*
 *包装数据
 *
 *@param要使用的会话会话
 *@param key_id对象id
 要包装的数据中的*@param
 *@param英寸长度英寸
 *@param输出包装数据
 *@参数输出长度输出
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_wrap_data(yh_session *session, uint16_t key_id, const uint8_t *in,
                        size_t in_len, uint8_t *out, size_t *out_len);

/*
 *展开数据
 *
 *@param要使用的会话会话
 *@param key_id对象id
 打包数据中的*@param
 *@param英寸长度英寸
 *@param out展开的数据
 *@参数输出长度输出
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_unwrap_data(yh_session *session, uint16_t key_id,
                          const uint8_t *in, size_t in_len, uint8_t *out,
                          size_t *out_len);

/*
 *闪烁设备
 *
 *@param要使用的会话会话
 *@param秒闪烁
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_util_blink(yh_session *session, uint8_t seconds);

/*
 *重置设备
 *
 *@param要使用的会话会话
 *
 *@返回YH_rc错误代码。此功能通常会返回网络错误
 */

yh_rc yh_util_reset(yh_session *session);

/*
 *获取会话ID
 *
 *@param要使用的会话会话
 *@param sid会话ID
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_get_session_id(yh_session *session, uint8_t *sid);

/*
 *检查连接器是否连接了设备。
 *
 *@param连接器
 *
 *@返回真或假
 */

bool yh_connector_has_device(yh_connector *connector);

/*
 *获取连接器版本
 *
 *@param连接器
 *@param主版本
 *@参数次要版本
 *@param补丁版本
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_get_connector_version(yh_connector *connector, uint8_t *major,
                               uint8_t *minor, uint8_t *patch);

/*
 *获取连接器地址
 *
 *@param连接器
 *@param address指向字符串地址的指针
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_get_connector_address(yh_connector *connector, char **const address);

/*
 *将功能字符串转换为字节数组
 *
 *@param能力字符串能力
 *@param结果能力
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_capabilities_to_num(const char *capability, yh_capabilities *result);

/*
 *将功能字节数组转换为字符串
 *
 *@param num能力
 *@param字符串指针结果数组
 *@param n_result结果的元素数
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_num_to_capabilities(const yh_capabilities *num, const char *result[],
                             size_t *n_result);

/*
 *检查是否设置了功能
 *
 *@param能力
 *@param能力字符串
 *
 *@返回真或假
 */

bool yh_check_capability(const yh_capabilities *capabilities,
                         const char *capability);

/*
 *合并两组功能
 *
 *@param a一组功能
 *@param b一组功能
 *@param结果结果能力集
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_merge_capabilities(const yh_capabilities *a, const yh_capabilities *b,
                            yh_capabilities *result);

/*
 *用另一组功能筛选一组功能
 *
 *@param能力一组能力
 *@param要筛选的筛选功能
 *@param结果结果能力集
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_filter_capabilities(const yh_capabilities *capabilities,
                             const yh_capabilities *filter,
                             yh_capabilities *result);

/*
 *检查算法是否为RSA算法
 *
 *@param算法
 *
 *@返回真或假
 */

bool yh_is_rsa(yh_algorithm algorithm);

/*
 *检查算法是否为EC算法
 *
 *@param算法
 *
 *@返回真或假
 */

bool yh_is_ec(yh_algorithm algorithm);

/*
 *检查算法是否为ED算法
 *
 *@param算法
 *
 *@返回真或假
 */

bool yh_is_ed(yh_algorithm algorithm);

/*
 *检查算法是否为HMAC算法
 *
 *@param算法
 *
 *@返回真或假
 */

bool yh_is_hmac(yh_algorithm algorithm);

/*
 *获取算法位长度
 *
 *@param算法
 *@param result位长度
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_get_key_bitlength(yh_algorithm algorithm, size_t *result);

/*
 *将算法转换为字符串
 *
 *@param algo算法
 *@param结果字符串
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_algo_to_string(yh_algorithm algo, char const **result);

/*
 *将字符串转换为算法
 *
 *@param string算法作为字符串
 *@param algo算法
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_string_to_algo(const char *string, yh_algorithm *algo);

/*
 *将类型转换为字符串
 *
 *@param类型
 *@param结果字符串
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_type_to_string(yh_object_type type, char const **result);

/*
 *将字符串转换为类型
 *
 *@param string类型为string
 *@param类型
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_string_to_type(const char *string, yh_object_type *type);

/*
 *将字符串转换为选项
 *
 *@param string选项作为字符串
 *@param选项
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_string_to_option(const char *string, yh_option *option);

/*
 *验证一组日志条目
 *
 *@param logs指向一组日志项的指针
 *@param n_items项目数日志
 *@param last_previous_log指向第一个条目之前的条目的可选指针
 在日志中
 *
 *@返回真或假
 */

bool yh_verify_logs(yh_log_entry *logs, size_t n_items,
                    yh_log_entry *last_previous_log);

/*
 *将字符串解析为域参数。
 *
 *@param domains格式为1,2,3的字符串
 *@param result结果分析的域参数
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_parse_domains(const char *domains, uint16_t *result);

/*
 *将域写出字符串。
 *
 *@param域编码域
 *@param string保存结果的字符串
 *@param max_len字符串的最大长度
 *
 *@返回YH_rc错误代码
 */

yh_rc yh_domains_to_string(uint16_t domains, char *string, size_t max_len);
#ifdef __cplusplus
}
#endif

#endif
