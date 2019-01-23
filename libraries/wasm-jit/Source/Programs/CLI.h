
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include "Inline/BasicTypes.h"
#include "Inline/Floats.h"
#include "Inline/Timing.h"
#include "WAST/WAST.h"
#include "WASM/WASM.h"
#include "IR/Module.h"
#include "IR/Validate.h"
#include "Runtime/Runtime.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>

inline std::string loadFile(const char* filename)
{
	Timing::Timer timer;
	std::ifstream stream(filename,std::ios::binary | std::ios::ate);
	if(!stream.is_open())
	{
		std::cerr << "Failed to open " << filename << ": " << std::strerror(errno) << std::endl;
		return std::string();
	}
	std::string data;
	data.resize((unsigned int)stream.tellg());
	stream.seekg(0);
	stream.read(const_cast<char*>(data.data()),data.size());
	stream.close();
	Timing::logRatePerSecond("loaded file",timer,data.size() / 1024.0 / 1024.0,"MB");
	return data;
}

inline bool loadTextModule(const char* filename,const std::string& wastString,IR::Module& outModule)
{
	std::vector<WAST::Error> parseErrors;
	WAST::parseModule(wastString.c_str(),wastString.size(),outModule,parseErrors);
	if(!parseErrors.size()) { return true; }
	else
	{
//打印任何分析错误；
		std::cerr << "Error parsing WebAssembly text file:" << std::endl;
		for(auto& error : parseErrors)
		{
			std::cerr << filename << ":" << error.locus.describe() << ": " << error.message.c_str() << std::endl;
			std::cerr << error.locus.sourceLine << std::endl;
			std::cerr << std::setw(error.locus.column(8)) << "^" << std::endl;
		}
		return false;
	}
}

inline bool loadTextModule(const char* filename,IR::Module& outModule)
{
//将文件读入字符串。
	auto wastBytes = loadFile(filename);
	if(!wastBytes.size()) { return false; }
	const std::string wastString = std::move(wastBytes);

	return loadTextModule(filename,wastString,outModule);
}

inline bool loadBinaryModule(const std::string& wasmBytes,IR::Module& outModule)
{
	Timing::Timer loadTimer;

//从二进制Web程序集文件加载模块。
	try
	{
		Serialization::MemoryInputStream stream((const U8*)wasmBytes.data(),wasmBytes.size());
		WASM::serialize(stream,outModule);
	}
	catch(Serialization::FatalSerializationException exception)
	{
		std::cerr << "Error deserializing WebAssembly binary file:" << std::endl;
		std::cerr << exception.message << std::endl;
		return false;
	}
	catch(IR::ValidationException exception)
	{
		std::cerr << "Error validating WebAssembly binary file:" << std::endl;
		std::cerr << exception.message << std::endl;
		return false;
	}
	catch(std::bad_alloc)
	{
		std::cerr << "Memory allocation failed: input is likely malformed" << std::endl;
		return false;
	}

	Timing::logRatePerSecond("Loaded WASM",loadTimer,wasmBytes.size()/1024.0/1024.0,"MB");
	return true;
}

inline bool loadBinaryModule(const char* wasmFilename,IR::Module& outModule)
{
//读取packed.wasm文件字节。
	auto wasmBytes = loadFile(wasmFilename);
	if(!wasmBytes.size()) { return false; }

	return loadBinaryModule(wasmBytes,outModule);
}

inline bool loadModule(const char* filename,IR::Module& outModule)
{
//将指定的文件读取到数组中。
	auto fileBytes = loadFile(filename);
	if(!fileBytes.size()) { return false; }

//如果文件以WASM二进制幻数开头，则将其作为二进制模块加载。
	if(*(U32*)fileBytes.data() == 0x6d736100) { return loadBinaryModule(fileBytes,outModule); }
	else
	{
//否则，将其作为文本模块加载。
		auto wastString = std::move(fileBytes);
		return loadTextModule(filename,wastString,outModule);
	}
}

inline bool saveBinaryModule(const char* wasmFilename,const IR::Module& module)
{
	Timing::Timer saveTimer;

	std::vector<U8> wasmBytes;
	try
	{
//序列化WebAssembly模块。
		Serialization::ArrayOutputStream stream;
		WASM::serialize(stream,module);
		wasmBytes = stream.getBytes();
	}
	catch(Serialization::FatalSerializationException exception)
	{
		std::cerr << "Error serializing WebAssembly binary file:" << std::endl;
		std::cerr << exception.message << std::endl;
		return false;
	}
	
	Timing::logRatePerSecond("Saved WASM",saveTimer,wasmBytes.size()/1024.0/1024.0,"MB");

//将序列化数据写入输出文件。
	std::ofstream outputStream(wasmFilename,std::ios::binary);
	outputStream.write((char*)wasmBytes.data(),wasmBytes.size());
	outputStream.close();
	
	return true;
}

inline bool endsWith(const char *str, const char *suffix)
{
	if(!str || !suffix) { return false; }
	Uptr lenstr = strlen(str);
	Uptr lensuffix = strlen(suffix);
	if(lenstr < lensuffix) { return false; }
	return (strncmp(str+lenstr-lensuffix, suffix, lensuffix) == 0);
}

int commandMain(int argc,char** argv);

int main(int argc,char** argv)
{
	try
	{
		return commandMain(argc,argv);
	}
	catch(IR::ValidationException exception)
	{
		std::cerr << "Failed to validate module: " << exception.message << std::endl;
		return EXIT_FAILURE;
	}
	catch(Runtime::Exception exception)
	{
		std::cerr << "Runtime exception: " << describeExceptionCause(exception.cause) << std::endl;
		for(auto calledFunction : exception.callStack) { std::cerr << "  " << calledFunction << std::endl; }
		return EXIT_FAILURE;
	}
	catch(Serialization::FatalSerializationException exception)
	{
		std::cerr << "Fatal serialization exception: " << exception.message << std::endl;
		return EXIT_FAILURE;
	}
}