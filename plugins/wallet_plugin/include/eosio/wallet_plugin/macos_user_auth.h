
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include <CoreFoundation/CoreFoundation.h>

//请求用户身份验证，并在强制后使用true/false调用回调。**请注意，回调
//将在单独的线程中完成**
extern "C" void macos_user_auth(void(*cb)(int, void*), void* cb_userdata, CFStringRef message);