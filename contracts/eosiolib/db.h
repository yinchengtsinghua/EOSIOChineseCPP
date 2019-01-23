
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
/*
 *@文件dB
 *@eos/license中定义的版权
 *@brief定义了用于与区块链数据库接口的C API
 **/

#pragma once

#include <eosiolib/types.h>
extern "C" {
/*
 *@defgroup数据库API
 *@brief定义了在区块链上存储和检索数据的API
 *@ingroup contractdev公司
 *
 *DeGuffDeabaseCpp数据库C++API
 *@brief定义了到eosio数据库的接口
 *@ingroup数据库
 *
 *@细节
 *EOSIO按照以下广泛的结构组织数据：
 *-**代码**-具有写入权限的帐户名
 *-**范围**-存储数据的区域
 *-**表**-正在存储的表的名称
 *-**记录**-表中的一行
 **/


/*
 *@defgroup数据库C API
 *@brief定义了用于与数据库接口的%c API。
 *@ingroup数据库
 *
 *@details database c api提供到eosio数据库的低级接口。
 *
 *@section table types支持的表类型
 *以下是C API支持的表类型：
 * 1。主表
 *-64位整数键
 * 2。二级索引表
 *-64位整数键
 *-128位整数键
 *-256位整数键
 *-双键
 *长双键
 *@
 **/


/*
  *
  *将记录存储在主64位整数索引表中
  *
  *@brief将记录存储在主64位整数索引表中
  *@param scope-表所在的范围（暗示在当前接收器的代码中）
  *@param table-表名
  *@param payer-支付存储成本的帐户
  *@param id-条目的ID
  *@param data-要存储的记录
  *@param len-数据大小
  *@pre'data'是指向至少'len'字节长的内存范围的有效指针
  *@pre`*（（uint64_t*）data）`存储主键
  *@将迭代器返回到新创建的表行
  *@post表中创建了一个新条目
  **/

int32_t db_store_i64(account_name scope, table_name table, account_name payer, uint64_t id,  const void* data, uint32_t len);

/*
  *
  *更新主64位整数索引表中的记录
  *
  *@brief更新主64位整数索引表中的记录
  *@param iterator-要更新的记录所在的表行的迭代器
  *@param payer-支付存储成本的帐户（使用0继续使用当前付款方）
  *@param data-新更新的记录
  *@param len-数据大小
  *@pre'data'是指向至少'len'字节长的内存范围的有效指针
  *@pre`*（（uint64_t*）data）`存储主键
  *@pre`iterator`指向表中现有的表行
  *@post“迭代器”指向的表行中包含的记录将替换为新的更新记录
  **/

void db_update_i64(int32_t iterator, account_name payer, const void* data, uint32_t len);

/*
  *
  *从主64位整数索引表中删除记录
  *
  *@brief从主64位整数索引表中删除一条记录
  *@param iterator-要删除的表行的迭代器
  *@pre`iterator`指向表中现有的表行
  *@post删除“迭代器”指向的表行，并将相关的存储成本退还给付款人。
  *
  *实例：
  *
  *@代码
  *int32_t itr=db_find_i64（接收器，接收器，表1，n（alice））；
  *eosio_assert（itr>=0，“无法删除Alice，因为在表中找不到她”）；
  *db_删除_i64（itr）；
  *@终结码
  **/

void db_remove_i64(int32_t iterator);

/*
  *
  *获取主64位整数索引表中的记录
  *
  *@brief获取主64位整数索引表中的记录
  *@param iterator-包含要检索的记录的表行的迭代器
  *@param data-指向将用检索到的记录填充的缓冲区的指针
  *@param len-缓冲区的大小
  *@如果“len>0”，则返回复制到缓冲区的数据的大小；如果“len==0”，则返回检索到的记录的大小。
  *@pre`iterator`指向表中现有的表行
  *@pre'data'是指向至少'len'字节长的内存范围的有效指针
  *@post`data`将用检索到的记录填充（必要时截断为第一个'len'字节）
  *
  *实例：
  *
  *@代码
  *字符值[50]；
  *auto len=db_get_i64（itr，value，0）；
  *eosio_assert（len<=50，“buffer to small to store retrieved record”）；
  *db_get_i64（itr，value，len）；
  *@终结码
  **/

int32_t db_get_i64(int32_t iterator, const void* data, uint32_t len);

/*
  *
  *在主64位整数索引表中，查找引用表行后的表行。
  *
  *@brief在主64位整数索引表中查找引用表行后的表行
  *@param iterator-引用表行的迭代器
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为下一个表行的主键。
  *@将迭代器返回到被引用表行后面的表行（如果被引用表行是表中的最后一行，则返回表的结束迭代器）
  *@pre`iterator`指向表中现有的表行
  *@post`*primary`将替换为引用表行后的表行的主键（如果存在），否则`*primary`将保持不变
  *
  *实例：
  *
  *@代码
  *int32_t charlie_itr=db_find_i64（接收器，接收器，表1，n（charlie））；
  *//查理之后什么都不要
  *uint64初始值=0
  *int32_t end_itr=db_next_i64（charlie_itr，&prim）；
  *eosio_assert（end_itr<-1，“charlie不是表中的最后一个条目”）；
  *@终结码
  **/

int32_t db_next_i64(int32_t iterator, uint64_t* primary);

/*
  *
  *在主64位整数索引表中查找引用表行之前的表行。
  *
  *@brief在主64位整数索引表中查找引用表行之前的表行
  *@param iterator-引用表行的迭代器
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为上一表行的主键。
  *@return迭代器返回到被引用表行前面的表行（假设存在一个表行）（如果被引用表行是表中的第一个行，则返回-1）
  *@pre`iterator`指向表中现有的表行，或者它是表的结束iterator
  *@post`*primary`将替换为被引用表行前面的表行的主键（如果存在），否则`*primary`将保持不变。
  *
  *实例：
  *
  *@代码
  *uint64_t prim=0；
  *int32_t itr_prev=db_previous_i64（itr，&prim）；
  *@终结码
  **/

int32_t db_previous_i64(int32_t iterator, uint64_t* primary);

/*
  *
  *按主键在主64位整数索引表中查找表行
  *
  *@brief按主键在主64位整数索引表中查找表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param id-要查找的表行的主键
  *@如果找不到表行，返回主键等于'id'的表行迭代器或表的结束迭代器
  *
  *实例：
  *
  *@代码
  *int itr=db_find_i64（接收器，接收器，表1，n（charlie））；
  *@终结码
  **/

int32_t db_find_i64(account_name code, account_name scope, table_name table, uint64_t id);

/*
  *
  *在主64位整数索引表中查找与给定主键的低位条件匹配的表行。
  *与LowerBound条件匹配的表行是表中的第一行，其最低主键大于等于给定键。
  *
  *@brief在与给定主键的lowerbound条件匹配的主64位整数索引表中查找表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param id-用于确定下限的主键
  *@如果找不到表行，则将迭代器返回到找到的表行或表的结束迭代器
  **/

int32_t db_lowerbound_i64(account_name code, account_name scope, table_name table, uint64_t id);

/*
  *
  *在主64位整数索引表中查找与给定主键的上界条件匹配的表行。
  *与上界条件匹配的表行是表中第一个主键最低的表行，即>给定的键。
  *
  *@brief在与给定主键的上界条件匹配的主64位整数索引表中查找表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param id-用于确定上界的主键
  *@如果找不到表行，则将迭代器返回到找到的表行或表的结束迭代器
  **/

int32_t db_upperbound_i64(account_name code, account_name scope, table_name table, uint64_t id);

/*
  *
  *获取一个迭代器，它表示主64位整数索引表的最后一行的末尾。
  *
  *@brief获取一个迭代器，该迭代器表示主64位整数索引表的最后一行的末尾。
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@返回表的结束迭代器
  **/

int32_t db_end_i64(account_name code, account_name scope, table_name table);

/*
  *
  *在辅助64位整数索引表中存储64位整数辅助键与主键的关联
  *
  *@brief在辅助64位整数索引表中存储64位整数辅助键与主键的关联
  *@param scope-表所在的范围（暗示在当前接收器的代码中）
  *@param table-表名
  *@param payer-支付存储成本的帐户
  *@param id-要与辅助键关联的主键
  *@param secondary-指向辅助键的指针
  *@将迭代器返回到新创建的表行
  *@post在secondary 64位整数索引表中创建主键'id'和secondary key`*secondary'之间的新的secondary key关联
  **/

int32_t db_idx64_store(account_name scope, table_name table, account_name payer, uint64_t id, const uint64_t* secondary);

/*
  *
  *更新64位整数次键与辅助64位整数索引表中主键的关联
  *
  *@brief更新64位整数次键与辅助64位整数索引表中主键的关联
  *@param iterator-包含要更新的辅助键关联的表行的迭代器
  *@param payer-支付存储成本的帐户（使用0继续使用当前付款方）
  *@param secondary-指向将替换现有关联的**新**辅助键的指针
  *@pre`iterator`指向表中现有的表行
  *@post由“迭代器”指向的表行的次键替换为“*secondary”
  **/

void db_idx64_update(int32_t iterator, account_name payer, const uint64_t* secondary);

/*
  *
  *从辅助64位整数索引表中删除表行
  *
  *@brief从辅助的64位整数索引表中删除表行
  *@param iterator-要删除的表行的迭代器
  *@pre`iterator`指向表中现有的表行
  *@post删除“迭代器”指向的表行，并将相关的存储成本退还给付款人。
  **/

void db_idx64_remove(int32_t iterator);

/*
  *
  *在辅助的64位整数索引表中，查找引用表行后的表行。
  *
  *@brief在辅助的64位整数索引表中查找引用表行后的表行
  *@param iterator-引用表行的迭代器
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为下一个表行的主键。
  *@将迭代器返回到被引用表行后面的表行（如果被引用表行是表中的最后一行，则返回表的结束迭代器）
  *@pre`iterator`指向表中现有的表行
  *@post`*primary`将替换为引用表行后的表行的主键（如果存在），否则`*primary`将保持不变
  **/

int32_t db_idx64_next(int32_t iterator, uint64_t* primary);

/*
  *
  *在辅助的64位整数索引表中，查找引用表行之前的表行。
  *
  *@brief在辅助的64位整数索引表中查找引用表行之前的表行
  *@param iterator-引用表行的迭代器
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为上一表行的主键。
  *@return迭代器返回到被引用表行前面的表行（假设存在一个表行）（如果被引用表行是表中的第一个行，则返回-1）
  *@pre`iterator`指向表中现有的表行，或者它是表的结束iterator
  *@post`*primary`将替换为被引用表行前面的表行的主键（如果存在），否则`*primary`将保持不变。
  **/

int32_t db_idx64_previous(int32_t iterator, uint64_t* primary);

/*
  *
  *按主键在辅助的64位整数索引表中查找表行
  *
  *@brief按主键在辅助的64位整数索引表中查找表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param secondary-指向“uint64”变量的指针，该变量的值将设置为找到的表行的辅助键。
  *@param primary-要查找的表行的主键
  *@post如果且仅当找到表行时，`*secondary`将替换为找到的表行的secondary key
  *@如果找不到表行，返回主键等于'id'的表行迭代器或表的结束迭代器
  **/

int32_t db_idx64_find_primary(account_name code, account_name scope, table_name table, uint64_t* secondary, uint64_t primary);

/*
  *
  *按辅助键在辅助64位整数索引表中查找表行
  *
  *@brief按辅助键在辅助64位整数索引表中查找表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param secondary-指向用于查找表行的辅助键的指针
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为找到的表行的主键。
  *@post如果且仅当找到表行时，`*primary`将替换为找到的表行的主键
  *@如果找不到表行，则将迭代器返回到次键等于“*secondary”的第一个表行或表的结束迭代器
  **/

int32_t db_idx64_find_secondary(account_name code, account_name scope, table_name table, const uint64_t* secondary, uint64_t* primary);

/*
  *
  *在辅助的64位整数索引表中查找与给定辅助键的LowerBound条件匹配的表行。
  *与LowerBound条件匹配的表行是表中具有最低辅助键的第一个表行，该辅助键大于等于给定键。
  *
  *@brief在辅助64位整数索引表中查找与给定辅助键的低位条件匹配的表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param secondary-指向secondary key的指针，首先用于确定lowerbound，然后用找到的表行的secondary key替换。
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为找到的表行的主键。
  *@post如果且仅当找到表行时，`*secondary`将替换为找到的表行的secondary key
  *@post如果且仅当找到表行时，`*primary`将替换为找到的表行的主键
  *@如果找不到表行，则将迭代器返回到找到的表行或表的结束迭代器
  **/

int32_t db_idx64_lowerbound(account_name code, account_name scope, table_name table, uint64_t* secondary, uint64_t* primary);

/*
  *
  *在辅助64位整数索引表中查找与给定辅助键的上界条件匹配的表行。
  *与上界条件匹配的表行是表中具有最低次键的第一个表行，次键>给定键。
  *
  *@brief在辅助64位整数索引表中查找与给定辅助键的上界条件匹配的表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param secondary-指向secondary key的指针，首先用于确定上界，然后用找到的表行的secondary key替换。
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为找到的表行的主键。
  *@post如果且仅当找到表行时，`*secondary`将替换为找到的表行的secondary key
  *@post如果且仅当找到表行时，`*primary`将替换为找到的表行的主键
  *@如果找不到表行，则将迭代器返回到找到的表行或表的结束迭代器
  **/

int32_t db_idx64_upperbound(account_name code, account_name scope, table_name table, uint64_t* secondary, uint64_t* primary);

/*
  *
  *获取一个结束迭代器，该迭代器表示刚刚超过辅助64位整数索引表的最后一行的末尾。
  *
  *@brief获取一个结束迭代器，该迭代器只表示第二个64位整数索引表的最后一行的末尾。
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@返回表的结束迭代器
  **/

int32_t db_idx64_end(account_name code, account_name scope, table_name table);



/*
  *
  *在辅助128位整数索引表中存储128位整数辅助键与主键的关联
  *
  *@brief将128位整数次键与次128位整数索引表中的主键的关联存储起来。
  *@param scope-表所在的范围（暗示在当前接收器的代码中）
  *@param table-表名
  *@param payer-支付存储成本的帐户
  *@param id-要与辅助键关联的主键
  *@param secondary-指向辅助键的指针
  *@将迭代器返回到新创建的表行
  *@post在secondary 128位整数索引表中创建主键'id'和secondary key`*secondary'之间的新的secondary key关联
  **/

int32_t db_idx128_store(account_name scope, table_name table, account_name payer, uint64_t id, const uint128_t* secondary);

/*
  *
  *将128位整数次键与辅助128位整数索引表中的主键的关联更新为
  *
  *@brief更新128位整数次键与辅助128位整数索引表中主键的关联
  *@param iterator-包含要更新的辅助键关联的表行的迭代器
  *@param payer-支付存储成本的帐户（使用0继续使用当前付款方）
  *@param secondary-指向将替换现有关联的**新**辅助键的指针
  *@pre`iterator`指向表中现有的表行
  *@post由“迭代器”指向的表行的次键替换为“*secondary”
  **/

void db_idx128_update(int32_t iterator, account_name payer, const uint128_t* secondary);

/*
  *
  *从辅助128位整数索引表中删除表行
  *
  *@brief从第二个128位整数索引表中删除表行
  *@param iterator-要删除的表行的迭代器
  *@pre`iterator`指向表中现有的表行
  *@post删除“迭代器”指向的表行，并将相关的存储成本退还给付款人。
  **/

void db_idx128_remove(int32_t iterator);

/*
  *
  *在第二个128位整数索引表中，查找引用表行后的表行。
  *
  *@brief在第二个128位整数索引表中查找引用表行后的表行
  *@param iterator-引用表行的迭代器
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为下一个表行的主键。
  *@将迭代器返回到被引用表行后面的表行（如果被引用表行是表中的最后一行，则返回表的结束迭代器）
  *@pre`iterator`指向表中现有的表行
  *@post`*primary`将替换为引用表行后的表行的主键（如果存在），否则`*primary`将保持不变
  **/

int32_t db_idx128_next(int32_t iterator, uint64_t* primary);

/*
  *
  *在第二个128位整数索引表中查找引用表行之前的表行。
  *
  *@brief在第二个128位整数索引表中查找引用表行之前的表行
  *@param iterator-引用表行的迭代器
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为上一表行的主键。
  *@return迭代器返回到被引用表行前面的表行（假设存在一个表行）（如果被引用表行是表中的第一个行，则返回-1）
  *@pre`iterator`指向表中现有的表行，或者它是表的结束iterator
  *@post`*primary`将替换为被引用表行前面的表行的主键（如果存在），否则`*primary`将保持不变。
  **/

int32_t db_idx128_previous(int32_t iterator, uint64_t* primary);

/*
  *
  *按主键在第二个128位整数索引表中查找表行
  *
  *@brief按主键在第二个128位整数索引表中查找表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param secondary-指向“uint128”变量的指针，该变量的值将设置为找到的表行的辅助键。
  *@param primary-要查找的表行的主键
  *@post如果且仅当找到表行时，`*secondary`将替换为找到的表行的secondary key
  *@如果找不到表行，返回主键等于'id'的表行迭代器或表的结束迭代器
  **/

int32_t db_idx128_find_primary(account_name code, account_name scope, table_name table, uint128_t* secondary, uint64_t primary);

/*
  *
  *按辅助键在辅助128位整数索引表中查找表行
  *
  *@brief按辅助键在辅助128位整数索引表中查找表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param secondary-指向用于查找表行的辅助键的指针
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为找到的表行的主键。
  *@post如果且仅当找到表行时，`*primary`将替换为找到的表行的主键
  *@如果找不到表行，则将迭代器返回到次键等于“*secondary”的第一个表行或表的结束迭代器
  **/

int32_t db_idx128_find_secondary(account_name code, account_name scope, table_name table, const uint128_t* secondary, uint64_t* primary);

/*
  *
  *在辅助128位整数索引表中查找与给定辅助键的LowerBound条件匹配的表行。
  *与LowerBound条件匹配的表行是表中具有最低辅助键的第一个表行，该辅助键大于等于给定键。
  *
  *@brief在与给定次键的lowerbound条件匹配的辅助128位整数索引表中查找表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param secondary-指向secondary key的指针，首先用于确定lowerbound，然后用找到的表行的secondary key替换。
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为找到的表行的主键。
  *@post如果且仅当找到表行时，`*secondary`将替换为找到的表行的secondary key
  *@post如果且仅当找到表行时，`*primary`将替换为找到的表行的主键
  *@如果找不到表行，则将迭代器返回到找到的表行或表的结束迭代器
  **/

int32_t db_idx128_lowerbound(account_name code, account_name scope, table_name table, uint128_t* secondary, uint64_t* primary);

/*
  *
  *在辅助128位整数索引表中查找与给定辅助键的上界条件匹配的表行。
  *与上界条件匹配的表行是表中具有最低次键的第一个表行，次键>给定键。
  *
  *@brief在与给定的辅助键的上界条件匹配的辅助128位整数索引表中查找表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param secondary-指向secondary key的指针，首先用于确定上界，然后用找到的表行的secondary key替换。
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为找到的表行的主键。
  *@post如果且仅当找到表行时，`*secondary`将替换为找到的表行的secondary key
  *@post如果且仅当找到表行时，`*primary`将替换为找到的表行的主键
  *@如果找不到表行，则将迭代器返回到找到的表行或表的结束迭代器
  **/

int32_t db_idx128_upperbound(account_name code, account_name scope, table_name table, uint128_t* secondary, uint64_t* primary);

/*
  *
  *获取一个结束迭代器，该迭代器只表示第二个128位整数索引表的最后一行的末尾。
  *
  *@brief获取一个结束迭代器，该迭代器只表示第二个128位整数索引表的最后一行的末尾。
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@返回表的结束迭代器
  **/

int32_t db_idx128_end(account_name code, account_name scope, table_name table);

/*
  *
  *在次256位索引表中存储256位次键与主键的关联
  *
  *@brief在辅助256位索引表中存储256位辅助键与主键的关联
  *@param scope-表所在的范围（暗示在当前接收器的代码中）
  *@param table-表名
  *@param payer-支付存储成本的帐户
  *@param id-要与辅助键关联的主键
  *@param data-指向存储为2`uint128_t`整数数组的辅助键数据的指针
  *@param data_len-必须设置为2
  *@将迭代器返回到新创建的表行
  *@post主键'id'和指定的次键之间的新次键关联在次256位索引表中创建
  **/

int32_t db_idx256_store(account_name scope, table_name table, account_name payer, uint64_t id, const uint128_t* data, uint32_t data_len );

/*
  *
  *更新256位次键与256位次索引表中主键的关联
  *
  *@brief更新256位辅助键与辅助256位索引表中主键的关联
  *@param iterator-包含要更新的辅助键关联的表行的迭代器
  *@param payer-支付存储成本的帐户（使用0继续使用当前付款方）
  *@param data-指向将替换现有关联的**新**辅助键数据（存储为2个'uint128_t'整数数组）的指针。
  *@param data_len-必须设置为2
  *@pre`iterator`指向表中现有的表行
  *@post由“迭代器”指向的表行的次键替换为指定的次键
  **/

void db_idx256_update(int32_t iterator, account_name payer, const uint128_t* data, uint32_t data_len);

/*
  *
  *从辅助256位索引表中删除表行
  *
  *@brief从第二个256位索引表中删除表行
  *@param iterator-要删除的表行的迭代器
  *@pre`iterator`指向表中现有的表行
  *@post删除“迭代器”指向的表行，并将相关的存储成本退还给付款人。
  **/

void db_idx256_remove(int32_t iterator);

/*
  *
  *在第二个256位索引表中，查找引用表行后的表行。
  *
  *@brief在第二个256位索引表中查找引用表行后的表行
  *@param iterator-引用表行的迭代器
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为下一个表行的主键。
  *@将迭代器返回到被引用表行后面的表行（如果被引用表行是表中的最后一行，则返回表的结束迭代器）
  *@pre`iterator`指向表中现有的表行
  *@post`*primary`将替换为引用表行后的表行的主键（如果存在），否则`*primary`将保持不变
  **/

int32_t db_idx256_next(int32_t iterator, uint64_t* primary);

/*
  *
  *在第二个256位索引表中查找引用表行之前的表行。
  *
  *@brief在第二个256位索引表中查找引用表行之前的表行
  *@param iterator-引用表行的迭代器
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为上一表行的主键。
  *@return迭代器返回到被引用表行前面的表行（假设存在一个表行）（如果被引用表行是表中的第一个行，则返回-1）
  *@pre`iterator`指向表中现有的表行，或者它是表的结束iterator
  *@post`*primary`将替换为被引用表行前面的表行的主键（如果存在），否则`*primary`将保持不变。
  **/

int32_t db_idx256_previous(int32_t iterator, uint64_t* primary);

/*
  *
  *按主键在次256位索引表中查找表行
  *
  *@brief按主键在第二个128位整数索引表中查找表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param data-指向一个由2个'uint128_t'整数组成的数组的指针，该数组将用作缓冲区，以保存找到的表行的检索到的次键。
  *@param data_len-必须设置为2
  *@param primary-要查找的表行的主键
  *@post如果且仅当找到表行时，“data”指向的缓冲区将用找到的表行的辅助键填充。
  *@如果找不到表行，返回主键等于'id'的表行迭代器或表的结束迭代器
  **/

int32_t db_idx256_find_primary(account_name code, account_name scope, table_name table, uint128_t* data, uint32_t data_len, uint64_t primary);

/*
  *
  *按次键在次256位索引表中查找表行
  *
  *@brief按次键在次256位索引表中查找表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param data-指向用于查找表行的辅助键数据（存储为2`uint128_t`整数数组）的指针
  *@param data_len-必须设置为2
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为找到的表行的主键。
  *@post如果且仅当找到表行时，`*primary`将替换为找到的表行的主键
  *@如果找不到表行，则将迭代器返回到第1个表行，其中第二个键等于指定的第二个键或表的结束迭代器
  **/

int32_t db_idx256_find_secondary(account_name code, account_name scope, table_name table, const uint128_t* data, uint32_t data_len, uint64_t* primary);

/*
  *
  *在与给定次键的低位条件匹配的次256位索引表中查找表行。
  *与LowerBound条件匹配的表行是表中的第一个表行，其最低次键大于等于给定键（对256位键使用字典排序）
  *
  *@brief在与给定次键的低位条件匹配的次256位索引表中查找表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param data-指向辅助键数据的指针（存储为2个'uint128_t'整数的数组），首先用于确定低位，然后用找到的表行的辅助键替换。
  *@param data_len-必须设置为2
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为找到的表行的主键。
  *@post如果且仅当找到表行时，“data”指向的缓冲区将用找到的表行的辅助键填充。
  *@post如果且仅当找到表行时，`*primary`将替换为找到的表行的主键
  *@如果找不到表行，则将迭代器返回到找到的表行或表的结束迭代器
  **/

int32_t db_idx256_lowerbound(account_name code, account_name scope, table_name table, uint128_t* data, uint32_t data_len, uint64_t* primary);

/*
  *
  *在与给定次键的上界条件匹配的次256位索引表中查找表行。
  *与上界条件匹配的表行是表中具有最低次键的第一个表行，次键>给定键（对256位键使用字典顺序）
  *
  *@brief在与给定次键的上界条件匹配的次256位索引表中查找表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param data-指向次键数据的指针（存储为2个'uint128_t'整数数组），首先用于确定上界，然后用找到的表行的次键替换。
  *@param data_len-必须设置为2
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为找到的表行的主键。
  *@post如果且仅当找到表行时，“data”指向的缓冲区将用找到的表行的辅助键填充。
  *@post如果且仅当找到表行时，`*primary`将替换为找到的表行的主键
  *@如果找不到表行，则将迭代器返回到找到的表行或表的结束迭代器
  **/

int32_t db_idx256_upperbound(account_name code, account_name scope, table_name table, uint128_t* data, uint32_t data_len, uint64_t* primary);

/*
  *
  *获取一个结束迭代器，该迭代器表示刚刚超过辅助256位索引表的最后一行的末尾。
  *
  *@brief获取一个结束迭代器，该迭代器只表示第二个256位索引表的最后一行的末尾。
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@返回表的结束迭代器
  **/

int32_t db_idx256_end(account_name code, account_name scope, table_name table);

/*
  *
  *在二级双精度浮点索引表中存储双精度浮点辅助键与主键的关联
  *
  *@brief在二级双精度浮点索引表中存储一个双精度浮点二次键与一个主键的关联
  *@param scope-表所在的范围（暗示在当前接收器的代码中）
  *@param table-表名
  *@param payer-支付存储成本的帐户
  *@param id-要与辅助键关联的主键
  *@param secondary-指向辅助键的指针
  *@将迭代器返回到新创建的表行
  *@post主键'id'和辅助键“*secondary”之间的新辅助键关联在辅助双精度浮点索引表中创建
  **/

int32_t db_idx_double_store(account_name scope, table_name table, account_name payer, uint64_t id, const double* secondary);

/*
  *
  *在二级双精度浮点索引表中，将双精度浮点辅助键与主键的关联更新为主键。
  *
  *@brief更新双精度浮点索引表中双精度浮点次关键字与主关键字的关联
  *@param iterator-包含要更新的辅助键关联的表行的迭代器
  *@param payer-支付存储成本的帐户（使用0继续使用当前付款方）
  *@param secondary-指向将替换现有关联的**新**辅助键的指针
  *@pre`iterator`指向表中现有的表行
  *@post由“迭代器”指向的表行的次键替换为“*secondary”
  **/

void db_idx_double_update(int32_t iterator, account_name payer, const double* secondary);

/*
  *
  *从二级双精度浮点索引表中删除表行
  *
  *@brief从二级双精度浮点索引表中删除表行
  *@param iterator-要删除的表行的迭代器
  *@pre`iterator`指向表中现有的表行
  *@post删除“迭代器”指向的表行，并将相关的存储成本退还给付款人。
  **/

void db_idx_double_remove(int32_t iterator);

/*
  *
  *在二级双精度浮点索引表中查找引用表行后的表行。
  *
  *@brief在二级双精度浮点索引表中查找引用表行后的表行
  *@param iterator-引用表行的迭代器
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为下一个表行的主键。
  *@将迭代器返回到被引用表行后面的表行（如果被引用表行是表中的最后一行，则返回表的结束迭代器）
  *@pre`iterator`指向表中现有的表行
  *@post`*primary`将替换为引用表行后的表行的主键（如果存在），否则`*primary`将保持不变
  **/

int32_t db_idx_double_next(int32_t iterator, uint64_t* primary);

/*
  *
  *在二级双精度浮点索引表中查找引用表行之前的表行。
  *
  *@brief在二级双精度浮点索引表中查找引用表行之前的表行
  *@param iterator-引用表行的迭代器
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为上一表行的主键。
  *@return迭代器返回到被引用表行前面的表行（假设存在一个表行）（如果被引用表行是表中的第一个行，则返回-1）
  *@pre`iterator`指向表中现有的表行，或者它是表的结束iterator
  *@post`*primary`将替换为被引用表行前面的表行的主键（如果存在），否则`*primary`将保持不变。
  **/

int32_t db_idx_double_previous(int32_t iterator, uint64_t* primary);

/*
  *
  *按主键在二级双精度浮点索引表中查找表行
  *
  *@brief按主键在二级双精度浮点索引表中查找表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param secondary-指向“double”变量的指针，该变量的值将设置为找到的表行的辅助键。
  *@param primary-要查找的表行的主键
  *@post如果且仅当找到表行时，`*secondary`将替换为找到的表行的secondary key
  *@如果找不到表行，返回主键等于'id'的表行迭代器或表的结束迭代器
  **/

int32_t db_idx_double_find_primary(account_name code, account_name scope, table_name table, double* secondary, uint64_t primary);

/*
  *
  *使用辅助键在二级双精度浮点索引表中查找表行
  *
  *@brief按secondary键在secondary双精度浮点索引表中查找表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param secondary-指向用于查找表行的辅助键的指针
  *@param primary-指向“double”变量的指针，该变量的值将设置为找到的表行的主键。
  *@post如果且仅当找到表行时，`*primary`将替换为找到的表行的主键
  *@如果找不到表行，则将迭代器返回到次键等于“*secondary”的第一个表行或表的结束迭代器
  **/

int32_t db_idx_double_find_secondary(account_name code, account_name scope, table_name table, const double* secondary, uint64_t* primary);

/*
  *
  *在辅助双精度浮点索引表中查找表行，该表与给定辅助键的低位条件匹配。
  *与LowerBound条件匹配的表行是表中具有最低辅助键的第一个表行，该辅助键大于等于给定键。
  *
  *@brief在二级双精度浮点索引表中查找表行，该表与给定的二级键的低位条件匹配
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param secondary-指向secondary key的指针，首先用于确定lowerbound，然后用找到的表行的secondary key替换。
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为找到的表行的主键。
  *@post如果且仅当找到表行时，`*secondary`将替换为找到的表行的secondary key
  *@post如果且仅当找到表行时，`*primary`将替换为找到的表行的主键
  *@如果找不到表行，则将迭代器返回到找到的表行或表的结束迭代器
  **/

int32_t db_idx_double_lowerbound(account_name code, account_name scope, table_name table, double* secondary, uint64_t* primary);

/*
  *
  *在辅助双精度浮点索引表中查找与给定辅助键的上界条件匹配的表行。
  *与上界条件匹配的表行是表中具有最低次键的第一个表行，次键>给定键。
  *
  *@brief在二级双精度浮点索引表中查找与给定二级键的上界条件匹配的表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param secondary-指向secondary key的指针，首先用于确定上界，然后用找到的表行的secondary key替换。
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为找到的表行的主键。
  *@post如果且仅当找到表行时，`*secondary`将替换为找到的表行的secondary key
  *@post如果且仅当找到表行时，`*primary`将替换为找到的表行的主键
  *@如果找不到表行，则将迭代器返回到找到的表行或表的结束迭代器
  **/

int32_t db_idx_double_upperbound(account_name code, account_name scope, table_name table, double* secondary, uint64_t* primary);

/*
  *
  *获取一个结束迭代器，该迭代器表示刚刚超过辅助双精度浮点索引表的最后一行的末尾。
  *
  *@brief获取一个结束迭代器，该迭代器表示刚刚超过二级双精度浮点索引表的最后一行的末尾。
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@返回表的结束迭代器
  **/

int32_t db_idx_double_end(account_name code, account_name scope, table_name table);

/*
  *
  *在二级四精度浮点索引表中存储四精度浮点二次键与主键的关联
  *
  *@brief在二级四精度浮点索引表中存储四精度浮点二次键与主键的关联
  *@param scope-表所在的范围（暗示在当前接收器的代码中）
  *@param table-表名
  *@param payer-支付存储成本的帐户
  *@param id-要与辅助键关联的主键
  *@param secondary-指向辅助键的指针
  *@将迭代器返回到新创建的表行
  *@post主键'id'和辅助键“*secondary”之间的新辅助键关联在辅助四精度浮点索引表中创建
  **/

int32_t db_idx_long_double_store(account_name scope, table_name table, account_name payer, uint64_t id, const long double* secondary);

/*
  *
  *在二级四精度浮点索引表中，将四精度浮点辅助键与主键的关联更新为主键。
  *
  *@brief更新四精度浮点索引表中四精度浮点次键与主键的关联
  *@param iterator-包含要更新的辅助键关联的表行的迭代器
  *@param payer-支付存储成本的帐户（使用0继续使用当前付款方）
  *@param secondary-指向将替换现有关联的**新**辅助键的指针
  *@pre`iterator`指向表中现有的表行
  *@post由“迭代器”指向的表行的次键替换为“*secondary”
  **/

void db_idx_long_double_update(int32_t iterator, account_name payer, const long double* secondary);

/*
  *
  *从辅助四精度浮点索引表中删除表行
  *
  *@brief从二级四精度浮点索引表中删除表行
  *@param iterator-要删除的表行的迭代器
  *@pre`iterator`指向表中现有的表行
  *@post删除“迭代器”指向的表行，并将相关的存储成本退还给付款人。
  **/

void db_idx_long_double_remove(int32_t iterator);

/*
  *
  *在二级四倍精度浮点索引表中，查找引用表行后的表行。
  *
  *@brief在二级四精度浮点索引表中查找引用表行后的表行
  *@param iterator-引用表行的迭代器
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为下一个表行的主键。
  *@将迭代器返回到被引用表行后面的表行（如果被引用表行是表中的最后一行，则返回表的结束迭代器）
  *@pre`iterator`指向表中现有的表行
  *@post`*primary`将替换为引用表行后的表行的主键（如果存在），否则`*primary`将保持不变
  **/

int32_t db_idx_long_double_next(int32_t iterator, uint64_t* primary);

/*
  *
  *在二级四倍精度浮点索引表中查找引用表行之前的表行。
  *
  *@brief在二级四精度浮点索引表中查找引用表行之前的表行
  *@param iterator-引用表行的迭代器
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为上一表行的主键。
  *@return迭代器返回到被引用表行前面的表行（假设存在一个表行）（如果被引用表行是表中的第一个行，则返回-1）
  *@pre`iterator`指向表中现有的表行，或者它是表的结束iterator
  *@post`*primary`将替换为被引用表行前面的表行的主键（如果存在），否则`*primary`将保持不变。
  **/

int32_t db_idx_long_double_previous(int32_t iterator, uint64_t* primary);

/*
  *
  *按主键在二级四精度浮点索引表中查找表行
  *
  *@brief按主键在二级四精度浮点索引表中查找表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param secondary-指向“long double”变量的指针，该变量的值将设置为找到的表行的辅助键。
  *@param primary-要查找的表行的主键
  *@post如果且仅当找到表行时，`*secondary`将替换为找到的表行的secondary key
  *@如果找不到表行，返回主键等于'id'的表行迭代器或表的结束迭代器
  **/

int32_t db_idx_long_double_find_primary(account_name code, account_name scope, table_name table, long double* secondary, uint64_t primary);

/*
  *
  *按二次键在二次四精度浮点索引表中查找表行
  *
  *@brief在二级四精度浮点索引表中按二级键查找表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param secondary-指向用于查找表行的辅助键的指针
  *@param primary-指向“long double”变量的指针，该变量的值将设置为找到的表行的主键。
  *@post如果且仅当找到表行时，`*primary`将替换为找到的表行的主键
  *@如果找不到表行，则将迭代器返回到次键等于“*secondary”的第一个表行或表的结束迭代器
  **/

int32_t db_idx_long_double_find_secondary(account_name code, account_name scope, table_name table, const long double* secondary, uint64_t* primary);

/*
  *
  *在辅助四精度浮点索引表中查找与给定辅助键的低位条件匹配的表行。
  *与LowerBound条件匹配的表行是表中具有最低辅助键的第一个表行，该辅助键大于等于给定键。
  *
  *@brief在辅助四精度浮点索引表中查找表行，该表与给定辅助键的低位条件匹配。
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param secondary-指向secondary key的指针，首先用于确定lowerbound，然后用找到的表行的secondary key替换。
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为找到的表行的主键。
  *@post如果且仅当找到表行时，`*secondary`将替换为找到的表行的secondary key
  *@post如果且仅当找到表行时，`*primary`将替换为找到的表行的主键
  *@如果找不到表行，则将迭代器返回到找到的表行或表的结束迭代器
  **/

int32_t db_idx_long_double_lowerbound(account_name code, account_name scope, table_name table, long double* secondary, uint64_t* primary);

/*
  *
  *在辅助四精度浮点索引表中查找表行，该表与给定辅助键的上界条件匹配。
  *与上界条件匹配的表行是表中具有最低次键的第一个表行，次键>给定键。
  *
  *@brief在辅助四精度浮点索引表中查找与给定辅助键的上界条件匹配的表行
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@param secondary-指向secondary key的指针，首先用于确定上界，然后用找到的表行的secondary key替换。
  *@param primary-指向“uint64”变量的指针，该变量的值将设置为找到的表行的主键。
  *@post如果且仅当找到表行时，`*secondary`将替换为找到的表行的secondary key
  *@post如果且仅当找到表行时，`*primary`将替换为找到的表行的主键
  *@如果找不到表行，则将迭代器返回到找到的表行或表的结束迭代器
  **/

int32_t db_idx_long_double_upperbound(account_name code, account_name scope, table_name table, long double* secondary, uint64_t* primary);

/*
  *
  *获取一个结束迭代器，该迭代器表示刚好超过辅助四倍精度浮点索引表的最后一行的末尾。
  *
  *@brief获取一个结束迭代器，该迭代器只表示第二个四精度浮点索引表的最后一行的末尾。
  *@param code-表所有者的名称
  *@param scope-表所在的作用域
  *@param table-表名
  *@返回表的结束迭代器
  **/

int32_t db_idx_long_double_end(account_name code, account_name scope, table_name table);

///@数据库
}
