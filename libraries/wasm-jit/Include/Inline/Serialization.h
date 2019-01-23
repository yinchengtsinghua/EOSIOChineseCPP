
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#pragma once

#include "Platform/Platform.h"

#include "../../../chain/include/eosio/chain/wasm_eosio_constraints.hpp"
#include <string>
#include <vector>
#include <string.h>
#include <algorithm>

namespace Serialization
{
//在序列化期间因各种错误引发的异常。
//任何使用序列化的代码都应该处理它！
	struct FatalSerializationException
	{
		std::string message;
		FatalSerializationException(std::string&& inMessage)
		: message(std::move(inMessage)) {}
	};

//抽象输出流。
	struct OutputStream
	{
		enum { isInput = false };

		OutputStream(): next(nullptr), end(nullptr) {}

		Uptr capacity() const { return SIZE_MAX; }
		
//将流光标前进numbytes，并返回指向上一个流光标的指针。
		inline U8* advance(Uptr numBytes)
		{
			if(next + numBytes > end) { extendBuffer(numBytes); }
			WAVM_ASSERT_THROW(next + numBytes <= end);

			U8* data = next;
			next += numBytes;
			return data;
		}

	protected:

		U8* next;
		U8* end;

//当缓冲区中没有足够的空间容纳对流的写入时调用。
//应该更新next和end-to-point到一个新的缓冲区，并确保
//缓冲区至少有numbytes。可能引发FatalSerializationException。
		virtual void extendBuffer(Uptr numBytes) = 0;
	};

//写入字节数组的输出流。
	struct ArrayOutputStream : public OutputStream
	{
//将输出数组从流移动到调用方。
		std::vector<U8>&& getBytes()
		{
			bytes.resize(next - bytes.data());
			next = nullptr;
			end = nullptr;
			return std::move(bytes);
		}
		
	private:

		std::vector<U8> bytes;

		virtual void extendBuffer(Uptr numBytes)
		{
			const Uptr nextIndex = next - bytes.data();

//以越来越大的增量增加阵列，因此所花费的时间
//缓冲器为O（1）。
			bytes.resize(std::max((Uptr)nextIndex+numBytes,(Uptr)bytes.size() * 7 / 5 + 32));

			next = bytes.data() + nextIndex;
			end = bytes.data() + bytes.size();
		}
		virtual bool canExtendBuffer(Uptr numBytes) const { return true; }
	};

//抽象输入流。
	struct InputStream
	{
		enum { isInput = true };

		InputStream(const U8* inNext,const U8* inEnd): next(inNext), end(inEnd) {}

		virtual Uptr capacity() const = 0;
		
//将流光标前进numbytes，并返回指向上一个流光标的指针。
		inline const U8* advance(Uptr numBytes)
		{
			if(next + numBytes > end) { getMoreData(numBytes); }
			const U8* data = next;
			next += numBytes;
			return data;
		}

//返回指向当前流光标的指针，确保后面至少有numbytes。
		inline const U8* peek(Uptr numBytes)
		{
			if(next + numBytes > end) { getMoreData(numBytes); }
			return next;
		}

	protected:

		const U8* next;
		const U8* end;
		
//当缓冲区中没有足够的空间来满足对流的读取时调用。
//应该更新next和end-to-point到一个新的缓冲区，并确保
//缓冲区至少有numbytes。可能引发FatalSerializationException。
		virtual void getMoreData(Uptr numBytes) = 0;
	};

//从一个连续的内存范围中读取的一种输入流。
	struct MemoryInputStream : InputStream
	{
		MemoryInputStream(const U8* begin,Uptr numBytes): InputStream(begin,begin+numBytes) {}
		virtual Uptr capacity() const { return end - next; }
	private:
		virtual void getMoreData(Uptr numBytes) { throw FatalSerializationException("expected data but found end of stream"); }
	};

//序列化原始字节序列。
	FORCEINLINE void serializeBytes(OutputStream& stream,const U8* bytes,Uptr numBytes)
	{ memcpy(stream.advance(numBytes),bytes,numBytes); }
	FORCEINLINE void serializeBytes(InputStream& stream,U8* bytes,Uptr numBytes)
	{ 
      if ( numBytes < eosio::chain::wasm_constraints::wasm_page_size )
         memcpy(bytes,stream.advance(numBytes),numBytes); 
      else
         throw FatalSerializationException(std::string("Trying to deserialize bytes of size : " + std::to_string((uint64_t)numBytes)));
   }
	
//序列化基本C++类型。
	template<typename Stream,typename Value>
	FORCEINLINE void serializeNativeValue(Stream& stream,Value& value) { serializeBytes(stream,(U8*)&value,sizeof(Value)); }

	template<typename Stream> void serialize(Stream& stream,U8& i) { serializeNativeValue(stream,i); }
	template<typename Stream> void serialize(Stream& stream,U32& i) { serializeNativeValue(stream,i); }
	template<typename Stream> void serialize(Stream& stream,U64& i) { serializeNativeValue(stream,i);  }
	template<typename Stream> void serialize(Stream& stream,I8& i) { serializeNativeValue(stream,i); }
	template<typename Stream> void serialize(Stream& stream,I32& i) { serializeNativeValue(stream,i); }
	template<typename Stream> void serialize(Stream& stream,I64& i) { serializeNativeValue(stream,i); }
	template<typename Stream> void serialize(Stream& stream,F32& f) { serializeNativeValue(stream,f); }
	template<typename Stream> void serialize(Stream& stream,F64& f) { serializeNativeValue(stream,f); }

//LEB128变长整数序列化。
	template<typename Value,Uptr maxBits>
	FORCEINLINE void serializeVarInt(OutputStream& stream,Value& inValue,Value minValue,Value maxValue)
	{
		Value value = inValue;

		if(value < minValue || value > maxValue)
		{
			throw FatalSerializationException(std::string("out-of-range value: ") + std::to_string(minValue) + "<=" + std::to_string(value) + "<=" + std::to_string(maxValue));
		}

		bool more = true;
		while(more)
		{
			U8 outputByte = (U8)(value&127);
			value >>= 7;
			more = std::is_signed<Value>::value
				? (value != 0 && value != Value(-1)) || (value >= 0 && (outputByte & 0x40)) || (value < 0 && !(outputByte & 0x40))
				: (value != 0);
			if(more) { outputByte |= 0x80; }
			*stream.advance(1) = outputByte;
		};
	}
	
	template<typename Value,Uptr maxBits>
	FORCEINLINE void serializeVarInt(InputStream& stream,Value& value,Value minValue,Value maxValue)
	{
//首先，将可变的输入字节数读取到固定大小的缓冲区中。
		enum { maxBytes = (maxBits + 6) / 7 };
		U8 bytes[maxBytes] = {0};
		Uptr numBytes = 0;
		I8 signExtendShift = (I8)sizeof(Value) * 8;
		while(numBytes < maxBytes)
		{
			U8 byte = *stream.advance(1);
			bytes[numBytes] = byte;
			++numBytes;
			signExtendShift -= 7;
			if(!(byte & 0x80)) { break; }
		};

//确保输入编码的数据不超过最大位。
		enum { numUsedBitsInLastByte = maxBits - (maxBytes-1) * 7 };
		enum { numUnusedBitsInLast = 8 - numUsedBitsInLastByte };
		enum { lastBitUsedMask = U8(1<<(numUsedBitsInLastByte-1)) };
		enum { lastByteUsedMask = U8(1<<numUsedBitsInLastByte)-U8(1) };
		enum { lastByteSignedMask = U8(~U8(lastByteUsedMask) & ~U8(0x80)) };
		const U8 lastByte = bytes[maxBytes-1];
		if(!std::is_signed<Value>::value)
		{
			if((lastByte & ~lastByteUsedMask) != 0)
			{
				throw FatalSerializationException("Invalid unsigned LEB encoding: unused bits in final byte must be 0");
			}
		}
		else
		{
			const I8 signBit = I8((lastByte & lastBitUsedMask) << numUnusedBitsInLast);
			const I8 signExtendedLastBit = signBit >> numUnusedBitsInLast;
			if((lastByte & ~lastByteUsedMask) != (signExtendedLastBit & lastByteSignedMask))
			{
				throw FatalSerializationException(
					"Invalid signed LEB encoding: unused bits in final byte must match the most-significant used bit");
			}
		}

//将缓冲区的字节解码为输出整数。
		value = 0;
		for(Uptr byteIndex = 0;byteIndex < maxBytes;++byteIndex)
		{ value |= Value(bytes[byteIndex] & ~0x80) << (byteIndex * 7); }
		
//符号将输出整数扩展到值的完整大小。
		if(std::is_signed<Value>::value && signExtendShift > 0)
		{ value = Value(value << signExtendShift) >> signExtendShift; }

//检查输出整数是否在预期范围内。
		if(value < minValue || value > maxValue)
		{ throw FatalSerializationException(std::string("out-of-range value: ") + std::to_string(minValue) + "<=" + std::to_string(value) + "<=" + std::to_string(maxValue)); }
	}

//各种常用LEB128参数的帮助程序。
	template<typename Stream,typename Value> void serializeVarUInt1(Stream& stream,Value& value) { serializeVarInt<Value,1>(stream,value,0,1); }
	template<typename Stream,typename Value> void serializeVarUInt7(Stream& stream,Value& value) { serializeVarInt<Value,7>(stream,value,0,127); }
	template<typename Stream,typename Value> void serializeVarUInt32(Stream& stream,Value& value) { serializeVarInt<Value,32>(stream,value,0,UINT32_MAX); }
	template<typename Stream,typename Value> void serializeVarUInt64(Stream& stream,Value& value) { serializeVarInt<Value,64>(stream,value,0,UINT64_MAX); }
	template<typename Stream,typename Value> void serializeVarInt7(Stream& stream,Value& value) { serializeVarInt<Value,7>(stream,value,-64,63); }
	template<typename Stream,typename Value> void serializeVarInt32(Stream& stream,Value& value) { serializeVarInt<Value,32>(stream,value,INT32_MIN,INT32_MAX); }
	template<typename Stream,typename Value> void serializeVarInt64(Stream& stream,Value& value) { serializeVarInt<Value,64>(stream,value,INT64_MIN,INT64_MAX); }

//序列化常量。如果反序列化，则在反序列化值与常量不匹配时引发FatalSerializationException。
	template<typename Constant>
	void serializeConstant(InputStream& stream,const char* constantMismatchMessage,Constant constant)
	{
		Constant savedConstant;
		serialize(stream,savedConstant);
		if(savedConstant != constant)
		{
			throw FatalSerializationException(std::string(constantMismatchMessage) + ": loaded " + std::to_string(savedConstant) + " but was expecting " + std::to_string(constant));
		}
	}
	template<typename Constant>
	void serializeConstant(OutputStream& stream,const char* constantMismatchMessage,Constant constant)
	{
		serialize(stream,constant);
	}

//序列化容器。
	template<typename Stream>
	void serialize(Stream& stream,std::string& string)
	{
      constexpr size_t max_size = eosio::chain::wasm_constraints::maximum_func_local_bytes;
		Uptr size = string.size();
		serializeVarUInt32(stream,size);
		if(Stream::isInput)
		{
//在调整字符串的大小之前推进流：
//在为格式错误的输入进行大量分配之前，请尝试获取序列化异常。
			const U8* inputBytes = stream.advance(size);
         if (size >= max_size)
            throw FatalSerializationException(std::string("Trying to deserialize string of size : " + std::to_string((uint64_t)size) + ", which is over by "+std::to_string(size - max_size )+" bytes"));
			string.resize(size);
			memcpy(const_cast<char*>(string.data()),inputBytes,size);
			string.shrink_to_fit();
		}
		else { serializeBytes(stream,(U8*)string.c_str(),size); }
	}

	template<typename Stream,typename Element,typename Allocator,typename SerializeElement>
	void serializeArray(Stream& stream,std::vector<Element,Allocator>& vector,SerializeElement serializeElement)
	{
      constexpr size_t max_size = eosio::chain::wasm_constraints::maximum_func_local_bytes;
		Uptr size = vector.size();
		serializeVarUInt32(stream,size);
		if(Stream::isInput)
		{
//将向量一次增大一个元素：
//在为格式错误的输入进行大量分配之前，请尝试获取序列化异常。
			vector.clear();
         if (size >= max_size)
            throw FatalSerializationException(std::string("Trying to deserialize array of size : " + std::to_string((uint64_t)size) + ", which is over by "+std::to_string(size - max_size )+" bytes"));
			for(Uptr index = 0;index < size;++index)
			{
				vector.push_back(Element());
				serializeElement(stream,vector.back());
			}
         vector.shrink_to_fit();
		}
		else
		{
			for(Uptr index = 0;index < vector.size();++index) { serializeElement(stream,vector[index]); }
		}
	}

	template<typename Stream,typename Element,typename Allocator>
	void serialize(Stream& stream,std::vector<Element,Allocator>& vector)
	{
		serializeArray(stream,vector,[](Stream& stream,Element& element){serialize(stream,element);});
	}
}
