
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
    *@defgroup action api操作api
    *@ingroup contractdev公司
    *@brief定义了用于查询操作和发送操作的API
    *
    **/


   /*
    *@defgroup action c api action c api
    *@ingroup操作api
    *@brief定义了查询动作和发送动作的API
    *
    *
    *eos.io操作具有以下抽象结构：
    *
    ＊` `
    *结构动作
    *scope_name scope；//为代码/类型定义要执行的主代码的合同
    *action_name；//要执行的操作
    *权限级别[]授权；//提供的帐户和权限级别
    *bytes data；//代码处理的不透明数据
    *}；
    ＊` `
    *
    *此API使您的合同能够检查当前操作的字段并采取相应的措施。
    *
    *实例：
    *@代码
    *//假设此操作用于以下示例：
    *//{
    *//“code”：“eos”，
    *//“type”：“传输”，
    *//“authorization”：[“account”：“inita”，“permission”：“active”]，
    *//“数据”：{
    *//“from”：“inita”，
    *//“to”：“initb”，
    *//“金额”：1000
    *//}
    *//}
    *
    *字符缓冲区[128]；
    *uint32_t total=read_action（buffer，5）；//buffer包含最多5个字节的操作内容
    *print（total）；//输出：5
    *
    *uint32_t msgsize=action_size（）；
    *print（msgsize）；//输出：上述动作数据字段的大小
    *
    *要求收件人（n（initc））；//此操作将通知initc帐户
    *
    *需要_auth（n（inita））；//由于inita存在于auth列表中，所以不执行任何操作
    *需要_auth（n（initb））；//引发异常
    *
    *print（current_time（））；//输出：当前块的时间戳（1970年以来以微秒为单位）
    *
    *@终结码
    *
    *
    *@
    **/


   /*
    *最多将@ref len字节的当前操作数据复制到指定位置
    *
    *@brief将当前操作数据复制到指定位置
    *@param msg-将复制当前操作数据的最多@ref len字节的指针
    *@param len-要复制的当前操作数据的len，0以报告所需大小
    *@返回复制到msg的字节数，或者如果len==0，可以复制的字节数。
    *@pre`msg`是指向至少'len'字节长的内存范围的有效指针
    *@post`msg`中填充了压缩的操作数据
    **/

   uint32_t read_action_data( void* msg, uint32_t len );

   /*
    *获取当前操作的数据字段的长度。此方法对于动态调整大小的操作很有用
    *
    *@brief获取当前动作数据字段的长度
    *@返回当前操作数据字段的长度
    **/

   uint32_t action_data_size();

   /*
    *将指定的帐户添加到要通知的帐户集
    *
    *@brief将指定的帐户添加到要通知的帐户集
    *@param name-要验证的帐户的名称
    **/

   void require_recipient( account_name name );

   /*
    *验证在操作上提供的auths集中是否存在@ref name。如果找不到就抛出。
    *
    *@brief验证所提供的身份验证集中存在指定的帐户
    *@param name-要验证的帐户的名称
    **/

   void require_auth( account_name name );

    /*
    *验证@ref name是否具有身份验证。
    *
    *@brief验证@ref name是否具有身份验证。
    *@param name-要验证的帐户的名称
    **/

   bool has_auth( account_name name );

   /*
    *验证在操作上提供的auths集中是否存在@ref name。如果找不到就抛出。
    *
    *@brief验证所提供的身份验证集中存在指定的帐户
    *@param name-要验证的帐户的名称
    *@param permission-要验证的权限级别
    **/

   void require_auth2( account_name name, permission_name permission );

   bool is_account( account_name name );

   /*
    *在此操作的父事务上下文中发送一个内联操作
    *
    *@param serialized_action-序列化操作
    *@param size-序列化操作的大小（字节）
    *@pre`serialized_action`是指向数组的有效指针，长度至少为'size`字节
    **/

   void send_inline(char *serialized_action, size_t size);

   /*
    *在此操作的父事务上下文中发送一个内联上下文无关操作
    *
    *@param serialized_action-序列化操作
    *@param size-序列化操作的大小（字节）
    *@pre`serialized_action`是指向数组的有效指针，长度至少为'size`字节
    **/

   void send_context_free_inline(char *serialized_action, size_t size);

   /*
    *验证对某个操作持有的一组写锁中是否存在@ref name。如果找不到则抛出
    *@brief验证持有的一组写锁中是否存在@ref name
    *@param name-要验证的帐户的名称
    **/

   void require_write_lock( account_name name );

   /*
    *验证对某个操作持有的读取锁集中是否存在@ref name。如果找不到则抛出
    *@brief验证持有的读取锁集中是否存在@ref name
    *@param name-要验证的帐户的名称
    **/

   void require_read_lock( account_name name );

   /*
    *以微秒为单位返回从1970年开始的发布时间
    *@brief获取发布时间
    *@以微秒为单位返回从1970年开始的发布时间
    **/

   uint64_t  publication_time();

   /*
    *获取动作的当前接收器
    *@brief获取动作的当前接收器
    *@返回指定操作当前接收者的帐户
    **/

   account_name current_receiver();
///AcOncCAPI
}
