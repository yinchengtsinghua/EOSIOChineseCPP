
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include "Inline/BasicTypes.h"
#include "CLI.h"
#include "WAST/WAST.h"
#include "WASM/WASM.h"

int commandMain(int argc,char** argv)
{
	if(argc != 3)
	{
		std::cerr <<  "Usage: Disassemble in.wasm out.wast" << std::endl;
		return EXIT_FAILURE;
	}
	const char* inputFilename = argv[1];
	const char* outputFilename = argv[2];
	
//加载WASM文件。
	IR::Module module;
	if(!loadBinaryModule(inputFilename,module)) { return EXIT_FAILURE; }

//将模块打印到WAST。
	const std::string wastString = WAST::print(module);
	
//将序列化数据写入输出文件。
	std::ofstream outputStream(outputFilename);
	outputStream.write(wastString.data(),wastString.size());
	outputStream.close();

	return EXIT_SUCCESS;
}
