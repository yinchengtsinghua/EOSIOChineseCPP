
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include "Inline/BasicTypes.h"
#include "Inline/Timing.h"
#include "WAST.h"
#include "Lexer.h"
#include "IR/Module.h"
#include "IR/Validate.h"
#include "Parse.h"

using namespace WAST;
using namespace IR;

static bool tryParseSizeConstraints(ParseState& state,U64 maxMax,SizeConstraints& outSizeConstraints)
{
	outSizeConstraints.min = 0;
	outSizeConstraints.max = UINT64_MAX;

//分析最小值。
	if(!tryParseI64(state,outSizeConstraints.min))
	{
		return false;
	}
	else
	{
//分析可选的最大值。
		if(!tryParseI64(state,outSizeConstraints.max)) { outSizeConstraints.max = UINT64_MAX; }
		else
		{
//验证最大大小是否在限制范围内，大小限制是否不分离。
			if(outSizeConstraints.max > maxMax)
			{
				parseErrorf(state,state.nextToken-1,"maximum size exceeds limit (%u>%u)",outSizeConstraints.max,maxMax);
				outSizeConstraints.max = maxMax;
			}
			else if(outSizeConstraints.max < outSizeConstraints.min)
			{
				parseErrorf(state,state.nextToken-1,"maximum size is less than minimum size (%u<%u)",outSizeConstraints.max,outSizeConstraints.min);
				outSizeConstraints.max = outSizeConstraints.min;
			}
		}

		return true;
	}
}

static SizeConstraints parseSizeConstraints(ParseState& state,U64 maxMax)
{
	SizeConstraints result;
	if(!tryParseSizeConstraints(state,maxMax,result))
	{
		parseErrorf(state,state.nextToken,"expected size constraints");
	}
	return result;
}

static GlobalType parseGlobalType(ParseState& state)
{
	GlobalType result;
	result.isMutable = tryParseParenthesizedTagged(state,t_mut,[&]
	{
		result.valueType = parseValueType(state);
	});
	if(!result.isMutable)
	{
		result.valueType = parseValueType(state);
	}
	return result;
}

static InitializerExpression parseInitializerExpression(ModuleParseState& state)
{
	InitializerExpression result;
	parseParenthesized(state,[&]
	{
		switch(state.nextToken->type)
		{
		case t_i32_const: { ++state.nextToken; result = (I32)parseI32(state); break; }
		case t_i64_const: { ++state.nextToken; result = (I64)parseI64(state); break; }
		case t_f32_const: { ++state.nextToken; result = parseF32(state); break; }
		case t_f64_const: { ++state.nextToken; result = parseF64(state); break; }
		case t_get_global:
		{
			++state.nextToken;
			result.type = InitializerExpression::Type::get_global;
			result.globalIndex = parseAndResolveNameOrIndexRef(
				state,
				state.globalNameToIndexMap,
				state.module.globals.size(),
				"global"
				);
			break;
		}
		default:
			parseErrorf(state,state.nextToken,"expected initializer expression");
			result.type = InitializerExpression::Type::error;
			throw RecoverParseException();
		};
	});

	return result;
}

static void errorIfFollowsDefinitions(ModuleParseState& state)
{
	if(state.module.functions.defs.size()
	|| state.module.tables.defs.size()
	|| state.module.memories.defs.size()
	|| state.module.globals.defs.size())
	{
		parseErrorf(state,state.nextToken,"import declarations must precede all definitions");
	}
}

template<typename Def,typename Type,typename DisassemblyName>
static Uptr createImport(
	ParseState& state,Name name,std::string&& moduleName,std::string&& exportName,
	NameToIndexMap& nameToIndexMap,IndexSpace<Def,Type>& indexSpace,std::vector<DisassemblyName>& disassemblyNameArray,
	Type type)
{
	const Uptr importIndex = indexSpace.imports.size();
	bindName(state,nameToIndexMap,name,indexSpace.size());
	disassemblyNameArray.push_back({name.getString()});
	indexSpace.imports.push_back({type,std::move(moduleName),std::move(exportName)});
	return importIndex;
}

static bool parseOptionalSharedDeclaration(ModuleParseState& state)
{
	if(ENABLE_THREADING_PROTOTYPE && state.nextToken->type == t_shared) { ++state.nextToken; return true; }
	else { return false; }
}

static void parseImport(ModuleParseState& state)
{
	errorIfFollowsDefinitions(state);

	require(state,t_import);

	std::string moduleName = parseUTF8String(state);
	std::string exportName = parseUTF8String(state);

	parseParenthesized(state,[&]
	{
//分析导入类型。
		const Token* importKindToken = state.nextToken;
		switch(importKindToken->type)
		{
		case t_func:
		case t_table:
		case t_memory:
		case t_global:
			++state.nextToken;
			break;
		default:
			parseErrorf(state,state.nextToken,"invalid import type");
			throw RecoverParseException();
		}
		
//分析导入的可选内部名称。
		Name name;
		tryParseName(state,name);

//分析导入类型并在适当的名称/索引空间中创建导入。
		switch(importKindToken->type)
		{
		case t_func:
		{
			NameToIndexMap localNameToIndexMap;
			std::vector<std::string> localDissassemblyNames;
			const UnresolvedFunctionType unresolvedFunctionType = parseFunctionTypeRefAndOrDecl(state,localNameToIndexMap,localDissassemblyNames);
			const Uptr importIndex = createImport(state,name,std::move(moduleName),std::move(exportName),
				state.functionNameToIndexMap,state.module.functions,state.disassemblyNames.functions,
				{UINT32_MAX});
			state.disassemblyNames.functions.back().locals = localDissassemblyNames;

//解析完所有类型声明后，解析函数导入类型。
			state.postTypeCallbacks.push_back([unresolvedFunctionType,importIndex](ModuleParseState& state)
			{
				state.module.functions.imports[importIndex].type = resolveFunctionType(state,unresolvedFunctionType);
			});
			break;
		}
		case t_table:
		{
			const SizeConstraints sizeConstraints = parseSizeConstraints(state,UINT32_MAX);
			const TableElementType elementType = TableElementType::anyfunc;
			require(state,t_anyfunc);
			const bool isShared = parseOptionalSharedDeclaration(state);
			createImport(state,name,std::move(moduleName),std::move(exportName),
				state.tableNameToIndexMap,state.module.tables,state.disassemblyNames.tables,
				{elementType,isShared,sizeConstraints});
			break;
		}
		case t_memory:
		{
			const SizeConstraints sizeConstraints = parseSizeConstraints(state,IR::maxMemoryPages);
			const bool isShared = parseOptionalSharedDeclaration(state);
			createImport(state,name,std::move(moduleName),std::move(exportName),
				state.memoryNameToIndexMap,state.module.memories,state.disassemblyNames.memories,
				MemoryType{isShared,sizeConstraints});
			break;
		}
		case t_global:
		{
			const GlobalType globalType = parseGlobalType(state);
			createImport(state,name,std::move(moduleName),std::move(exportName),
				state.globalNameToIndexMap,state.module.globals,state.disassemblyNames.globals,
				globalType);
			break;
		}
		default: Errors::unreachable();
		};
	});
}

static void parseExport(ModuleParseState& state)
{
	require(state,t_export);

	const std::string exportName = parseUTF8String(state);

	parseParenthesized(state,[&]
	{
		ObjectKind exportKind;
		switch(state.nextToken->type)
		{
		case t_func: exportKind = ObjectKind::function; break;
		case t_table: exportKind = ObjectKind::table; break;
		case t_memory: exportKind = ObjectKind::memory; break;
		case t_global: exportKind = ObjectKind::global; break;
		default:
			parseErrorf(state,state.nextToken,"invalid export kind");
			throw RecoverParseException();
		};
		++state.nextToken;
	
		Reference exportRef;
		if(!tryParseNameOrIndexRef(state,exportRef))
		{
			parseErrorf(state,state.nextToken,"expected name or index");
			throw RecoverParseException();
		}

		const Uptr exportIndex = state.module.exports.size();
		state.module.exports.push_back({std::move(exportName),exportKind,0});

		state.postDeclarationCallbacks.push_back([=](ModuleParseState& state)
		{
			Uptr& exportedObjectIndex = state.module.exports[exportIndex].index;
			switch(exportKind)
			{
			case ObjectKind::function: exportedObjectIndex = resolveRef(state,state.functionNameToIndexMap,state.module.functions.size(),exportRef); break;
			case ObjectKind::table: exportedObjectIndex = resolveRef(state,state.tableNameToIndexMap,state.module.tables.size(),exportRef); break;
			case ObjectKind::memory: exportedObjectIndex = resolveRef(state,state.memoryNameToIndexMap,state.module.memories.size(),exportRef); break;
			case ObjectKind::global: exportedObjectIndex = resolveRef(state,state.globalNameToIndexMap,state.module.globals.size(),exportRef); break;
			default:
				Errors::unreachable();
			}
		});
	});
}

static void parseType(ModuleParseState& state)
{
	require(state,t_type);

	Name name;
	tryParseName(state,name);

	parseParenthesized(state,[&]
	{
		require(state,t_func);
		
		NameToIndexMap parameterNameToIndexMap;
		std::vector<std::string> localDisassemblyNames;
		const FunctionType* functionType = parseFunctionType(state,parameterNameToIndexMap,localDisassemblyNames);

		Uptr functionTypeIndex = state.module.types.size();
		state.module.types.push_back(functionType);
		errorUnless(functionTypeIndex < UINT32_MAX);
		state.functionTypeToIndexMap[functionType] = (U32)functionTypeIndex;

		bindName(state,state.typeNameToIndexMap,name,functionTypeIndex);
		state.disassemblyNames.types.push_back(name.getString());
	});
}

static void parseData(ModuleParseState& state)
{
	const Token* firstToken = state.nextToken;
	require(state,t_data);

//分析可选的内存名称。
	Reference memoryRef;
	bool hasMemoryRef = tryParseNameOrIndexRef(state,memoryRef);

//分析数据基地址的初始值设定项表达式。
	const InitializerExpression baseAddress = parseInitializerExpression(state);

//分析包含段数据的字符串列表。
	std::string dataString;
	while(tryParseString(state,dataString)) {};
	
//创建数据段。
	std::vector<U8> dataVector((const U8*)dataString.data(),(const U8*)dataString.data() + dataString.size());
	const Uptr dataSegmentIndex = state.module.dataSegments.size();
	state.module.dataSegments.push_back({UINTPTR_MAX,baseAddress,std::move(dataVector)});

//对所有声明进行分析后调用的回调进行排队，以解析要放入数据段的内存。
	state.postDeclarationCallbacks.push_back([=](ModuleParseState& state)
	{
		if(!state.module.memories.size())
		{
			parseErrorf(state,firstToken,"data segments aren't allowed in modules without any memory declarations");
		}
		else
		{
			state.module.dataSegments[dataSegmentIndex].memoryIndex =
				hasMemoryRef ? resolveRef(state,state.memoryNameToIndexMap,state.module.memories.size(),memoryRef) : 0;
		}
	});
}

static Uptr parseElemSegmentBody(ModuleParseState& state,Reference tableRef,InitializerExpression baseIndex,const Token* elemToken)
{
//在堆上分配elementreferences数组，这样就不需要为post声明回调复制它。
	std::vector<Reference>* elementReferences = new std::vector<Reference>();
	
	Reference elementRef;
	while(tryParseNameOrIndexRef(state,elementRef))
	{
		elementReferences->push_back(elementRef);
	};
	
//创建表段。
	const Uptr tableSegmentIndex = state.module.tableSegments.size();
	state.module.tableSegments.push_back({UINTPTR_MAX,baseIndex,std::vector<Uptr>()});

//对所有声明进行分析以解析表元素的引用后调用的回调进行排队。
	state.postDeclarationCallbacks.push_back([tableRef,tableSegmentIndex,elementReferences,elemToken](ModuleParseState& state)
	{
		if(!state.module.tables.size())
		{
			parseErrorf(state,elemToken,"data segments aren't allowed in modules without any memory declarations");
		}
		else
		{
			TableSegment& tableSegment = state.module.tableSegments[tableSegmentIndex];
			tableSegment.tableIndex = tableRef ? resolveRef(state,state.tableNameToIndexMap,state.module.tables.size(),tableRef) : 0;

			tableSegment.indices.resize(elementReferences->size());
			for(Uptr elementIndex = 0;elementIndex < elementReferences->size();++elementIndex)
			{
				tableSegment.indices[elementIndex] = resolveRef(
					state,
					state.functionNameToIndexMap,
					state.module.functions.size(),
					(*elementReferences)[elementIndex]
					);
			}
		}
		
//释放在堆上分配的elementreferences数组。
		delete elementReferences;
	});

	return elementReferences->size();
}

static void parseElem(ModuleParseState& state)
{
	const Token* elemToken = state.nextToken;
	require(state,t_elem);

//分析可选表名。
	Reference tableRef;
	tryParseNameOrIndexRef(state,tableRef);

//分析元素的基索引的初始值设定项表达式。
	const InitializerExpression baseIndex = parseInitializerExpression(state);

	parseElemSegmentBody(state,tableRef,baseIndex,elemToken);
}

template<typename Def,typename Type,typename ParseImport,typename ParseDef,typename DisassemblyName>
static void parseObjectDefOrImport(
	ModuleParseState& state,
	NameToIndexMap& nameToIndexMap,
	IR::IndexSpace<Def,Type>& indexSpace,
	std::vector<DisassemblyName>& disassemblyNameArray,
	TokenType declarationTag,
	IR::ObjectKind kind,
	ParseImport parseImportFunc,
	ParseDef parseDefFunc)
{
	const Token* declarationTagToken = state.nextToken;
	require(state,declarationTag);

	Name name;
	tryParseName(state,name);
	
//处理内联导出声明。
	while(true)
	{
		const bool isExport = tryParseParenthesizedTagged(state,t_export,[&]
		{
			state.module.exports.push_back({parseUTF8String(state),kind,indexSpace.size()});
		});
		if(!isExport) { break; }
	};

//处理内联导入声明。
	std::string importModuleName;
	std::string exportName;
	const bool isImport = tryParseParenthesizedTagged(state,t_import,[&]
	{
		errorIfFollowsDefinitions(state);

		importModuleName = parseUTF8String(state);
		exportName = parseUTF8String(state);
	});
	if(isImport)
	{
		Type importType = parseImportFunc(state);
		createImport(state,name,std::move(importModuleName),std::move(exportName),
			nameToIndexMap,indexSpace,disassemblyNameArray,
			importType);
	}
	else
	{
		Def def = parseDefFunc(state,declarationTagToken);
		bindName(state,nameToIndexMap,name,indexSpace.size());
		indexSpace.defs.push_back(std::move(def));
		disassemblyNameArray.push_back({name.getString()});
	}
}

static void parseFunc(ModuleParseState& state)
{
	parseObjectDefOrImport(state,state.functionNameToIndexMap,state.module.functions,state.disassemblyNames.functions,t_func,ObjectKind::function,
		[&](ModuleParseState& state)
		{
//分析导入函数的类型。
			NameToIndexMap localNameToIndexMap;
			std::vector<std::string> localDisassemblyNames;
			const UnresolvedFunctionType unresolvedFunctionType = parseFunctionTypeRefAndOrDecl(state,localNameToIndexMap,localDisassemblyNames);

//解析完所有类型声明后，解析函数导入类型。
			const Uptr importIndex = state.module.functions.imports.size();
			state.postTypeCallbacks.push_back([unresolvedFunctionType,importIndex](ModuleParseState& state)
			{
				state.module.functions.imports[importIndex].type = resolveFunctionType(state,unresolvedFunctionType);
			});
			return IndexedFunctionType {UINT32_MAX};
		},
		parseFunctionDef);
}

static void parseTable(ModuleParseState& state)
{
	parseObjectDefOrImport(state,state.tableNameToIndexMap,state.module.tables,state.disassemblyNames.tables,t_table,ObjectKind::table,
//分析表导入。
		[](ModuleParseState& state)
		{
			const SizeConstraints sizeConstraints = parseSizeConstraints(state,UINT32_MAX);
			const TableElementType elementType = TableElementType::anyfunc;
			require(state,t_anyfunc);
			const bool isShared = parseOptionalSharedDeclaration(state);
			return TableType {elementType,isShared,sizeConstraints};
		},
//分析表定义。
		[](ModuleParseState& state,const Token*)
		{
//分析表类型。
			SizeConstraints sizeConstraints;
			const bool hasSizeConstraints = tryParseSizeConstraints(state,UINT32_MAX,sizeConstraints);
		
			const TableElementType elementType = TableElementType::anyfunc;
			require(state,t_anyfunc);

//如果无法分析显式大小约束，则表定义必须包含隐式定义大小的表段。
			if(!hasSizeConstraints)
			{
				parseParenthesized(state,[&]
				{
					require(state,t_elem);

					const Uptr tableIndex = state.module.tables.size();
					errorUnless(tableIndex < UINT32_MAX);
					const Uptr numElements = parseElemSegmentBody(state,Reference(U32(tableIndex)),InitializerExpression((I32)0),state.nextToken-1);
					sizeConstraints.min = sizeConstraints.max = numElements;
				});
			}
			
			const bool isShared = parseOptionalSharedDeclaration(state);
			return TableDef {TableType(elementType,isShared,sizeConstraints)};
		});
}

static void parseMemory(ModuleParseState& state)
{
	parseObjectDefOrImport(state,state.memoryNameToIndexMap,state.module.memories,state.disassemblyNames.memories,t_memory,ObjectKind::memory,
//分析内存导入。
		[](ModuleParseState& state)
		{
			const SizeConstraints sizeConstraints = parseSizeConstraints(state,IR::maxMemoryPages);
			const bool isShared = parseOptionalSharedDeclaration(state);
			return MemoryType {isShared,sizeConstraints};
		},
//解析内存定义
		[](ModuleParseState& state,const Token*)
		{
			SizeConstraints sizeConstraints;
			if(!tryParseSizeConstraints(state,IR::maxMemoryPages,sizeConstraints))
			{
				std::string dataString;

				parseParenthesized(state,[&]
				{
					require(state,t_data);
				
					while(tryParseString(state,dataString)) {};
				});

				std::vector<U8> dataVector((const U8*)dataString.data(),(const U8*)dataString.data() + dataString.size());
				sizeConstraints.min = sizeConstraints.max = (dataVector.size() + IR::numBytesPerPage - 1) / IR::numBytesPerPage;
				state.module.dataSegments.push_back({state.module.memories.size(),InitializerExpression(I32(0)),std::move(dataVector)});
			}

			const bool isShared = parseOptionalSharedDeclaration(state);
			return MemoryDef {MemoryType(isShared,sizeConstraints)};
		});
}

static void parseGlobal(ModuleParseState& state)
{
	parseObjectDefOrImport(state,state.globalNameToIndexMap,state.module.globals,state.disassemblyNames.globals,t_global,ObjectKind::global,
//分析全局导入。
		parseGlobalType,
//解析全局定义
		[](ModuleParseState& state,const Token*)
		{
			const GlobalType globalType = parseGlobalType(state);
			const InitializerExpression initializerExpression = parseInitializerExpression(state);
			return GlobalDef {globalType,initializerExpression};
		});
}

static void parseStart(ModuleParseState& state)
{
	require(state,t_start);

	Reference functionRef;
	if(!tryParseNameOrIndexRef(state,functionRef))
	{
		parseErrorf(state,state.nextToken,"expected function name or index");
	}

	state.postDeclarationCallbacks.push_back([functionRef](ModuleParseState& state)
	{
		state.module.startFunctionIndex = resolveRef(state,state.functionNameToIndexMap,state.module.functions.size(),functionRef);
	});
}

static void parseDeclaration(ModuleParseState& state)
{
	parseParenthesized(state,[&]
	{
		switch(state.nextToken->type)
		{
		case t_import: parseImport(state); return true;
		case t_export: parseExport(state); return true;
		case t_global: parseGlobal(state); return true;
		case t_memory: parseMemory(state); return true;
		case t_table: parseTable(state); return true;
		case t_type: parseType(state); return true;
		case t_data: parseData(state); return true;
		case t_elem: parseElem(state); return true;
		case t_func: parseFunc(state); return true;
		case t_start: parseStart(state); return true;
		default:
			parseErrorf(state,state.nextToken,"unrecognized definition in module");
			throw RecoverParseException();
		};
	});
}

namespace WAST
{
	void parseModuleBody(ModuleParseState& state)
	{
		const Token* firstToken = state.nextToken;

//分析模块的声明。
		while(state.nextToken->type != t_rightParenthesis)
		{
			parseDeclaration(state);
		};

//处理在分析完所有类型声明之后请求的回调。
		if(!state.errors.size())
		{
			for(const auto& callback : state.postTypeCallbacks)
			{
				callback(state);
			}
		}

//处理在分析完所有声明之后请求的回调。
		if(!state.errors.size())
		{
			for(const auto& callback : state.postDeclarationCallbacks)
			{
				callback(state);
			}
		}
		
//验证模块的定义（不包括函数代码，在分析时对其进行验证）。
		if(!state.errors.size())
		{
			try
			{
				IR::validateDefinitions(state.module);
			}
			catch(ValidationException validationException)
			{
				parseErrorf(state,firstToken,"validation exception: %s",validationException.message.c_str());
			}
		}

//设置模块的反汇编名称。
		WAVM_ASSERT_THROW(state.module.functions.size() == state.disassemblyNames.functions.size());
		WAVM_ASSERT_THROW(state.module.tables.size() == state.disassemblyNames.tables.size());
		WAVM_ASSERT_THROW(state.module.memories.size() == state.disassemblyNames.memories.size());
		WAVM_ASSERT_THROW(state.module.globals.size() == state.disassemblyNames.globals.size());
		IR::setDisassemblyNames(state.module,state.disassemblyNames);
	}

	bool parseModule(const char* string,Uptr stringLength,IR::Module& outModule,std::vector<Error>& outErrors)
	{
		Timing::Timer timer;
		
//勒克斯。
		LineInfo* lineInfo = nullptr;
		std::vector<UnresolvedError> unresolvedErrors;
		Token* tokens = lex(string,stringLength,lineInfo);
		ModuleParseState state(string,lineInfo,unresolvedErrors,tokens,outModule);

		try
		{
//分析（模块…）<eof>
			parseParenthesized(state,[&]
			{
				require(state,t_module);
				parseModuleBody(state);
			});
			require(state,t_eof);
		}
		catch(RecoverParseException) {}
		catch(FatalParseException) {}

//解决任何错误的行信息，并将其写入OutErrors。
		for(auto& unresolvedError : unresolvedErrors)
		{
			TextFileLocus locus = calcLocusFromOffset(state.string,lineInfo,unresolvedError.charOffset);
			outErrors.push_back({std::move(locus),std::move(unresolvedError.message)});
		}

//释放令牌和行信息。
		freeTokens(tokens);
		freeLineInfo(lineInfo);

		Timing::logRatePerSecond("lexed and parsed WAST",timer,stringLength / 1024.0 / 1024.0,"MB");

		return outErrors.size() == 0;
	}
}