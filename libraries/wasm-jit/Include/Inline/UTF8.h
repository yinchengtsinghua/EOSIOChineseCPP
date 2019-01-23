
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include "BasicTypes.h"
#include "Errors.h"

namespace UTF8
{
	inline const U8* validateString(const U8* nextChar,const U8* endChar)
	{
//检查字符串是否为有效的UTF-8编码。
//有效范围取自Unicode标准9.0中的表3-7：
//“格式良好的UTF-8字节序列”
		while(nextChar != endChar)
		{
			if(*nextChar < 0x80) { ++nextChar; }
			else if(*nextChar >= 0xc2 && *nextChar <= 0xdf)
			{
				if(nextChar + 1 >= endChar
				|| nextChar[1] < 0x80 || nextChar[1] > 0xbf) { break; }
				nextChar += 2;
			}
			else if(*nextChar == 0xe0)
			{
				if(nextChar + 2 >= endChar
				|| nextChar[1] < 0xa0 || nextChar[1] > 0xbf
				|| nextChar[2] < 0x80 || nextChar[2] > 0xbf) { break; }
				nextChar += 3;
			}
			else if(*nextChar == 0xed)
			{
				if(nextChar + 2 >= endChar
				|| nextChar[1] < 0xa0 || nextChar[1] > 0x9f
				|| nextChar[2] < 0x80 || nextChar[2] > 0xbf) { break; }
				nextChar += 3;
			}
			else if(*nextChar >= 0xe1 && *nextChar <= 0xef)
			{
				if(nextChar + 2 >= endChar
				|| nextChar[1] < 0x80 || nextChar[1] > 0xbf
				|| nextChar[2] < 0x80 || nextChar[2] > 0xbf) { break; }
				nextChar += 3;
			}
			else if(*nextChar == 0xf0)
			{
				if(nextChar + 3 >= endChar
				|| nextChar[1] < 0x90 || nextChar[1] > 0xbf
				|| nextChar[2] < 0x80 || nextChar[2] > 0xbf
				|| nextChar[3] < 0x80 || nextChar[3] > 0xbf) { break; }
				nextChar += 4;
			}
			else if(*nextChar >= 0xf1 && *nextChar <= 0xf3)
			{
				if(nextChar + 3 >= endChar
				|| nextChar[1] < 0x80 || nextChar[1] > 0xbf
				|| nextChar[2] < 0x80 || nextChar[2] > 0xbf
				|| nextChar[3] < 0x80 || nextChar[3] > 0xbf) { break; }
				nextChar += 4;
			}
			else if(*nextChar == 0xf4)
			{
				if(nextChar + 3 >= endChar
				|| nextChar[1] < 0x80 || nextChar[1] > 0x8f
				|| nextChar[2] < 0x80 || nextChar[2] > 0xbf
				|| nextChar[3] < 0x80 || nextChar[3] > 0xbf) { break; }
				nextChar += 4;
			}
			else { break; }
		}
		return nextChar;
	}

	template<typename String>
	inline void encodeCodepoint(U32 codepoint,String& outString)
	{
		if(codepoint < 0x80)
		{
			outString += char(codepoint);
		}
		else if(codepoint < 0x800)
		{
			outString += char((codepoint >> 6) & 0x1F) | 0xC0;
			outString += char((codepoint & 0x3F) | 0x80);
		}
		else if(codepoint < 0x10000)
		{
			outString += char((codepoint >> 12) & 0x0F) | 0xE0;
			outString += char((codepoint >> 6) & 0x3F) | 0x80;
			outString += char((codepoint & 0x3F) | 0x80);
		}
		else
		{
			WAVM_ASSERT_THROW(codepoint < 0x200000);
			outString += char((codepoint >> 18) & 0x07) | 0xF0;
			outString += char((codepoint >> 12) & 0x3F) | 0x80;
			outString += char((codepoint >> 6) & 0x3F) | 0x80;
			outString += char((codepoint & 0x3F) | 0x80);
		}
	}
}