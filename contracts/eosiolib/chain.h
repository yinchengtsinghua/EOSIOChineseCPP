
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

/*
 *@defgroup chaineapi链api
 *@brief定义用于查询内部链状态的API
 *@ingroup contractdev公司
 **/


/*
 *@defgroup chain c api chain c api定义组链
 *@brief定义了用于查询内部链状态的%c API
 *@ingroup链接API
 *@
 **/


extern "C" {
    /*
     *获取一组活动生产者。
     *@brief获取一组活动生产者。
     *
     *@param producers-指向帐户名缓冲区的指针
     *@param datalen-缓冲区的字节长度，当传递0时，将返回存储完整输出所需的大小。
     *
     *@return uint32_t-实际填充的字节数
     *@pre'producers'是指向至少'datalen'字节长的内存范围的指针。
     *@post传入的“producers”指针获取活动的producer数组。
     *
     *实例：
     *
     *@代码
     *帐户名生产者[21]；
     *uint32_t bytes_populated=get_active_producers（producers，sizeof（account_name）*21）；
     *@终结码
     **/


    uint32_t get_active_producers( account_name* producers, uint32_t datalen );

///@ ChanCAPI
}
