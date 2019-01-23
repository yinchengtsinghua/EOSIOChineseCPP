
//此源码被清华学神尹成大魔王专业翻译分析并修改
//尹成QQ77025077
//尹成微信18510341407
//尹成所在QQ群721929980
//尹成邮箱 yinc13@mails.tsinghua.edu.cn
//尹成毕业于清华大学,微软区块链领域全球最有价值专家
//https://mvp.microsoft.com/zh-cn/PublicProfile/4033620
#include "LLVMJIT.h"
#include "llvm/ADT/SmallVector.h"
#include "Inline/Timing.h"
#include "IR/Operators.h"
#include "IR/OperatorPrinter.h"
#include "Logging/Logging.h"
#include "llvm/Support/raw_ostream.h"

#define ENABLE_LOGGING 0
#define ENABLE_FUNCTION_ENTER_EXIT_HOOKS 0

using namespace IR;

namespace LLVMJIT
{
//模块的LLVM IR。
	struct EmitModuleContext
	{
		const Module& module;
		ModuleInstance* moduleInstance;

		llvm::Module* llvmModule;
		std::vector<llvm::Function*> functionDefs;
		std::vector<llvm::Constant*> importedFunctionPointers;
		std::vector<llvm::Constant*> globalPointers;
		llvm::Constant* defaultTablePointer;
		llvm::Constant* defaultTableMaxElementIndex;
		llvm::Constant* defaultMemoryBase;
		llvm::Constant* defaultMemoryEndOffset;
		
		llvm::DIBuilder diBuilder;
		llvm::DICompileUnit* diCompileUnit;
		llvm::DIFile* diModuleScope;

		llvm::DIType* diValueTypes[(Uptr)ValueType::num];

		llvm::MDNode* likelyFalseBranchWeights;
		llvm::MDNode* likelyTrueBranchWeights;

		EmitModuleContext(const Module& inModule,ModuleInstance* inModuleInstance)
		: module(inModule)
		, moduleInstance(inModuleInstance)
		, llvmModule(new llvm::Module("",context))
		, diBuilder(*llvmModule)
		{
			diModuleScope = diBuilder.createFile("unknown","unknown");
			diCompileUnit = diBuilder.createCompileUnit(0xffff,diModuleScope,"WAVM",true,"",0);

			diValueTypes[(Uptr)ValueType::any] = nullptr;
			diValueTypes[(Uptr)ValueType::i32] = diBuilder.createBasicType("i32",32,llvm::dwarf::DW_ATE_signed);
			diValueTypes[(Uptr)ValueType::i64] = diBuilder.createBasicType("i64",64,llvm::dwarf::DW_ATE_signed);
			diValueTypes[(Uptr)ValueType::f32] = diBuilder.createBasicType("f32",32,llvm::dwarf::DW_ATE_float);
			diValueTypes[(Uptr)ValueType::f64] = diBuilder.createBasicType("f64",64,llvm::dwarf::DW_ATE_float);
			#if ENABLE_SIMD_PROTOTYPE
			diValueTypes[(Uptr)ValueType::v128] = diBuilder.createBasicType("v128",128,llvm::dwarf::DW_ATE_signed);
			#endif
			
			auto zeroAsMetadata = llvm::ConstantAsMetadata::get(emitLiteral(I32(0)));
			auto i32MaxAsMetadata = llvm::ConstantAsMetadata::get(emitLiteral(I32(INT32_MAX)));
			likelyFalseBranchWeights = llvm::MDTuple::getDistinct(context,{llvm::MDString::get(context,"branch_weights"),zeroAsMetadata,i32MaxAsMetadata});
			likelyTrueBranchWeights = llvm::MDTuple::getDistinct(context,{llvm::MDString::get(context,"branch_weights"),i32MaxAsMetadata,zeroAsMetadata});

		}
		llvm::Module* emit();
	};

//用于对单个AST函数进行抖动的函数所使用的上下文。
	struct EmitFunctionContext
	{
		typedef void Result;

		EmitModuleContext& moduleContext;
		const Module& module;
		const FunctionDef& functionDef;
		const FunctionType* functionType;
		FunctionInstance* functionInstance;
		llvm::Function* llvmFunction;
		llvm::IRBuilder<> irBuilder;

		std::vector<llvm::Value*> localPointers;

		llvm::DISubprogram* diFunction;

//有关范围内控制结构的信息。
		struct ControlContext
		{
			enum class Type : U8
			{
				function,
				block,
				ifThen,
				ifElse,
				loop
			};

			Type type;
			llvm::BasicBlock* endBlock;
			llvm::PHINode* endPHI;
			llvm::BasicBlock* elseBlock;
			ResultType resultType;
			Uptr outerStackSize;
			Uptr outerBranchTargetStackSize;
			bool isReachable;
			bool isElseReachable;
		};

		struct BranchTarget
		{
			ResultType argumentType;
			llvm::BasicBlock* block;
			llvm::PHINode* phi;
		};

		std::vector<ControlContext> controlStack;
		std::vector<BranchTarget> branchTargetStack;
		std::vector<llvm::Value*> stack;

		EmitFunctionContext(EmitModuleContext& inEmitModuleContext,const Module& inModule,const FunctionDef& inFunctionDef,FunctionInstance* inFunctionInstance,llvm::Function* inLLVMFunction)
		: moduleContext(inEmitModuleContext)
		, module(inModule)
		, functionDef(inFunctionDef)
		, functionType(inModule.types[inFunctionDef.type.index])
		, functionInstance(inFunctionInstance)
		, llvmFunction(inLLVMFunction)
		, irBuilder(context)
		{}

		void emit();

//操作数堆栈操作
		llvm::Value* pop()
		{
			WAVM_ASSERT_THROW(stack.size() - (controlStack.size() ? controlStack.back().outerStackSize : 0) >= 1);
			llvm::Value* result = stack.back();
			stack.pop_back();
			return result;
		}

		void popMultiple(llvm::Value** outValues,Uptr num)
		{
			WAVM_ASSERT_THROW(stack.size() - (controlStack.size() ? controlStack.back().outerStackSize : 0) >= num);
			std::copy(stack.end() - num,stack.end(),outValues);
			stack.resize(stack.size() - num);
		}

		llvm::Value* getTopValue() const
		{
			return stack.back();
		}

		void push(llvm::Value* value)
		{
			stack.push_back(value);
		}

//为分支到基本块的参数创建phi节点。
		llvm::PHINode* createPHI(llvm::BasicBlock* basicBlock,ResultType type)
		{
			if(type == ResultType::none) { return nullptr; }
			else
			{
				auto originalBlock = irBuilder.GetInsertBlock();
				irBuilder.SetInsertPoint(basicBlock);
				auto phi = irBuilder.CreatePHI(asLLVMType(type),2);
				if(originalBlock) { irBuilder.SetInsertPoint(originalBlock); }
				return phi;
			}
		}

//调试日志记录。
		void logOperator(const std::string& operatorDescription)
		{
			if(ENABLE_LOGGING)
			{
				std::string controlStackString;
				for(Uptr stackIndex = 0;stackIndex < controlStack.size();++stackIndex)
				{
					if(!controlStack[stackIndex].isReachable) { controlStackString += "("; }
					switch(controlStack[stackIndex].type)
					{
					case ControlContext::Type::function: controlStackString += "F"; break;
					case ControlContext::Type::block: controlStackString += "B"; break;
					case ControlContext::Type::ifThen: controlStackString += "T"; break;
					case ControlContext::Type::ifElse: controlStackString += "E"; break;
					case ControlContext::Type::loop: controlStackString += "L"; break;
					default: Errors::unreachable();
					};
					if(!controlStack[stackIndex].isReachable) { controlStackString += ")"; }
				}

				std::string stackString;
				const Uptr stackBase = controlStack.size() == 0 ? 0 : controlStack.back().outerStackSize;
				for(Uptr stackIndex = 0;stackIndex < stack.size();++stackIndex)
				{
					if(stackIndex == stackBase) { stackString += "| "; }
					{
						llvm::raw_string_ostream stackTypeStream(stackString);
						stack[stackIndex]->getType()->print(stackTypeStream,true);
					}
					stackString += " ";
				}
				if(stack.size() == stackBase) { stackString += "|"; }

				Log::printf(Log::Category::debug,"%-50s %-50s %-50s\n",controlStackString.c_str(),operatorDescription.c_str(),stackString.c_str());
			}
		}
		
//将I32值强制为I1，反之亦然。
		llvm::Value* coerceI32ToBool(llvm::Value* i32Value)
		{
			return irBuilder.CreateICmpNE(i32Value,typedZeroConstants[(Uptr)ValueType::i32]);
		}
		llvm::Value* coerceBoolToI32(llvm::Value* boolValue)
		{
			return irBuilder.CreateZExt(boolValue,llvmI32Type);
		}
		
//边界检查并将内存操作I32地址操作数转换为LLVM指针。
		llvm::Value* coerceByteIndexToPointer(llvm::Value* byteIndex,U32 offset,llvm::Type* memoryType)
		{
			if(HAS_64BIT_ADDRESS_SPACE)
			{
//在64位运行时，如果地址是32位，则将其Zext为64位。
//这对安全至关重要，因为llvm将在下面的GEP中将其隐式签名扩展到64位，
//将其解释为带符号的偏移量，并允许访问沙盒内存范围之外的内存。
//在32位运行时中没有“far地址”。
				if(sizeof(Uptr) != 4) { byteIndex = irBuilder.CreateZExt(byteIndex,llvmI64Type); }

//将偏移量添加到字节索引。
				if(offset)
				{
					byteIndex = irBuilder.CreateAdd(byteIndex,irBuilder.CreateZExt(emitLiteral(offset),llvmI64Type));
				}

//如果有64bit地址空间，则内存有足够的虚拟地址空间分配给
//确保任何32位字节索引+32位偏移量都将落在虚拟地址沙盒中，
//因此不需要进行明确的边界检查。
			}
			else
			{
//使用LLVM内部函数将偏移量添加到字节索引，该函数在加法溢出时返回进位。
				llvm::Value* overflowed = emitLiteral(false);
				if(offset)
				{
					auto offsetByteIndexWithOverflow = irBuilder.CreateCall(
						getLLVMIntrinsic({llvmI32Type},llvm::Intrinsic::uadd_with_overflow),
						{byteIndex,emitLiteral(U32(offset))}
						);
					byteIndex = irBuilder.CreateExtractValue(offsetByteIndexWithOverflow,{0});
					overflowed = irBuilder.CreateExtractValue(offsetByteIndexWithOverflow,{1});
				}

//检查偏移量是否没有溢出，最后一个字节索引是否在虚拟地址空间内。
//分配给内存。
				emitConditionalTrapIntrinsic(
					irBuilder.CreateOr(
						overflowed,
						irBuilder.CreateICmpUGT(
							byteIndex,
							irBuilder.CreateSub(
								moduleContext.defaultMemoryEndOffset,
								emitLiteral(Uptr(memoryType->getPrimitiveSizeInBits() / 8) - 1)
								)
							)
						),
					"wavmIntrinsics.accessViolationTrap",FunctionType::get(),{});
			}

//将指针强制转换为适当的类型。
			auto bytePointer = irBuilder.CreateInBoundsGEP(moduleContext.defaultMemoryBase,byteIndex);
            
			return irBuilder.CreatePointerCast(bytePointer,memoryType->getPointerTo());
		}

//陷阱除以零
		void trapDivideByZero(ValueType type,llvm::Value* divisor)
		{
			emitConditionalTrapIntrinsic(
				irBuilder.CreateICmpEQ(divisor,typedZeroConstants[(Uptr)type]),
				"wavmIntrinsics.divideByZeroOrIntegerOverflowTrap",FunctionType::get(),{});
		}

//（x/0）或（int_min/-1）上的陷阱。
		void trapDivideByZeroOrIntegerOverflow(ValueType type,llvm::Value* left,llvm::Value* right)
		{
			emitConditionalTrapIntrinsic(
				irBuilder.CreateOr(
					irBuilder.CreateAnd(
						irBuilder.CreateICmpEQ(left,type == ValueType::i32 ? emitLiteral((U32)INT32_MIN) : emitLiteral((U64)INT64_MIN)),
						irBuilder.CreateICmpEQ(right,type == ValueType::i32 ? emitLiteral((U32)-1) : emitLiteral((U64)-1))
						),
					irBuilder.CreateICmpEQ(right,typedZeroConstants[(Uptr)type])
					),
				"wavmIntrinsics.divideByZeroOrIntegerOverflowTrap",FunctionType::get(),{});
		}

		llvm::Value* getLLVMIntrinsic(const std::initializer_list<llvm::Type*>& argTypes,llvm::Intrinsic::ID id)
		{
			return llvm::Intrinsic::getDeclaration(moduleContext.llvmModule,id,llvm::ArrayRef<llvm::Type*>(argTypes.begin(),argTypes.end()));
		}
		
//发出对wavm内部函数的调用。
		llvm::Value* emitRuntimeIntrinsic(const char* intrinsicName,const FunctionType* intrinsicType,const std::initializer_list<llvm::Value*>& args)
		{
			ObjectInstance* intrinsicObject = Intrinsics::find(intrinsicName,intrinsicType);
			WAVM_ASSERT_THROW(intrinsicObject);
			FunctionInstance* intrinsicFunction = asFunction(intrinsicObject);
			WAVM_ASSERT_THROW(intrinsicFunction->type == intrinsicType);
			auto intrinsicFunctionPointer = emitLiteralPointer(intrinsicFunction->nativeFunction,asLLVMType(intrinsicType)->getPointerTo());
			return irBuilder.CreateCall(intrinsicFunctionPointer,llvm::ArrayRef<llvm::Value*>(args.begin(),args.end()));
		}

//向非返回的内部函数发出条件调用的帮助函数。
		void emitConditionalTrapIntrinsic(llvm::Value* booleanCondition,const char* intrinsicName,const FunctionType* intrinsicType,const std::initializer_list<llvm::Value*>& args)
		{
			auto trueBlock = llvm::BasicBlock::Create(context,llvm::Twine(intrinsicName) + "Trap",llvmFunction);
			auto endBlock = llvm::BasicBlock::Create(context,llvm::Twine(intrinsicName) + "Skip",llvmFunction);

			irBuilder.CreateCondBr(booleanCondition,trueBlock,endBlock,moduleContext.likelyFalseBranchWeights);

			irBuilder.SetInsertPoint(trueBlock);
			emitRuntimeIntrinsic(intrinsicName,intrinsicType,args);
			irBuilder.CreateUnreachable();

			irBuilder.SetInsertPoint(endBlock);
		}

//
//MISC算子
//

		void nop(NoImm) {}
		void unknown(Opcode opcode) { Errors::unreachable(); }
		
//
//控制结构操作员
//
		
		void pushControlStack(
			ControlContext::Type type,
			ResultType resultType,
			llvm::BasicBlock* endBlock,
			llvm::PHINode* endPHI,
			llvm::BasicBlock* elseBlock = nullptr
			)
		{
//无法访问的运算符筛选应该过滤掉调用pushcontrolStack的任何操作码。
			if(controlStack.size()) { errorUnless(controlStack.back().isReachable); }

			controlStack.push_back({type,endBlock,endPHI,elseBlock,resultType,stack.size(),branchTargetStack.size(),true,true});
		}

		void pushBranchTarget(ResultType branchArgumentType,llvm::BasicBlock* branchTargetBlock,llvm::PHINode* branchTargetPHI)
		{
			branchTargetStack.push_back({branchArgumentType,branchTargetBlock,branchTargetPHI});
		}

		void block(ControlStructureImm imm)
		{
//为块结果创建一个结束块+phi。
			auto endBlock = llvm::BasicBlock::Create(context,"blockEnd",llvmFunction);
			auto endPHI = createPHI(endBlock,imm.resultType);

//推送以结束块/phi结尾的控件上下文。
			pushControlStack(ControlContext::Type::block,imm.resultType,endBlock,endPHI);
			
//为端块/phi推一个分支目标。
			pushBranchTarget(imm.resultType,endBlock,endPHI);
		}
		void loop(ControlStructureImm imm)
		{
//创建一个循环块，并为循环结果创建一个结束块+phi。
			auto loopBodyBlock = llvm::BasicBlock::Create(context,"loopBody",llvmFunction);
			auto endBlock = llvm::BasicBlock::Create(context,"loopEnd",llvmFunction);
			auto endPHI = createPHI(endBlock,imm.resultType);
			
//分支到循环体，然后切换红外生成器以在那里发射。
			irBuilder.CreateBr(loopBodyBlock);
			irBuilder.SetInsertPoint(loopBodyBlock);

//推送以结束块/phi结尾的控件上下文。
			pushControlStack(ControlContext::Type::loop,imm.resultType,endBlock,endPHI);
			
//推动循环体开始的分支目标。
			pushBranchTarget(ResultType::none,loopBodyBlock,nullptr);
		}
		void if_(ControlStructureImm imm)
		{
//为if创建一个then块和else块，并为if结果创建一个end block+phi。
			auto thenBlock = llvm::BasicBlock::Create(context,"ifThen",llvmFunction);
			auto elseBlock = llvm::BasicBlock::Create(context,"ifElse",llvmFunction);
			auto endBlock = llvm::BasicBlock::Create(context,"ifElseEnd",llvmFunction);
			auto endPHI = createPHI(endBlock,imm.resultType);

//从操作数堆栈中弹出if条件。
			auto condition = pop();
			irBuilder.CreateCondBr(coerceI32ToBool(condition),thenBlock,elseBlock);
			
//切换红外生成器以发射THEN块。
			irBuilder.SetInsertPoint(thenBlock);

//推送一个ifthen控制上下文，该上下文最终以end block/phi结尾，但可能
//由将控件上下文更改为else块的else运算符终止。
			pushControlStack(ControlContext::Type::ifThen,imm.resultType,endBlock,endPHI,elseBlock);
			
//为if端推一个分支目标。
			pushBranchTarget(imm.resultType,endBlock,endPHI);
			
		}
		void else_(NoImm imm)
		{
			WAVM_ASSERT_THROW(controlStack.size());
			ControlContext& currentContext = controlStack.back();

			if(currentContext.isReachable)
			{
//如果控件上下文需要一个结果，请从操作数堆栈中获取该结果并将其添加到
//控制上下文的结束phi。
				if(currentContext.resultType != ResultType::none)
				{
					llvm::Value* result = pop();
					currentContext.endPHI->addIncoming(result,irBuilder.GetInsertBlock());
				}

//分支到控件上下文的结尾。
				irBuilder.CreateBr(currentContext.endBlock);
			}
			WAVM_ASSERT_THROW(stack.size() == currentContext.outerStackSize);

//将红外发射器切换到ELSE块。
			WAVM_ASSERT_THROW(currentContext.elseBlock);
			WAVM_ASSERT_THROW(currentContext.type == ControlContext::Type::ifThen);
			currentContext.elseBlock->moveAfter(irBuilder.GetInsertBlock());
			irBuilder.SetInsertPoint(currentContext.elseBlock);

//将控件堆栈的顶部更改为else子句。
			currentContext.type = ControlContext::Type::ifElse;
			currentContext.isReachable = currentContext.isElseReachable;
			currentContext.elseBlock = nullptr;
		}
		void end(NoImm)
		{
			WAVM_ASSERT_THROW(controlStack.size());
			ControlContext& currentContext = controlStack.back();

			if(currentContext.isReachable)
			{
//如果控件上下文产生结果，则取操作数堆栈的顶部，然后
//将其添加到控件上下文的结束phi中。
				if(currentContext.resultType != ResultType::none)
				{
					llvm::Value* result = pop();
					currentContext.endPHI->addIncoming(result,irBuilder.GetInsertBlock());
				}

//分支到控件上下文的结尾。
				irBuilder.CreateBr(currentContext.endBlock);
			}
			WAVM_ASSERT_THROW(stack.size() == currentContext.outerStackSize);

			if(currentContext.elseBlock)
			{
//如果这是没有else子句的if的结尾，请创建一个伪else子句。
				currentContext.elseBlock->moveAfter(irBuilder.GetInsertBlock());
				irBuilder.SetInsertPoint(currentContext.elseBlock);
				irBuilder.CreateBr(currentContext.endBlock);
			}

//将红外发射器切换到端块。
			currentContext.endBlock->moveAfter(irBuilder.GetInsertBlock());
			irBuilder.SetInsertPoint(currentContext.endBlock);

			if(currentContext.endPHI)
			{
//如果控件上下文产生结果，则采用合并所有控件流的phi
//将其推到操作数堆栈的末尾。
				if(currentContext.endPHI->getNumIncomingValues()) { push(currentContext.endPHI); }
				else
				{
//如果没有任何输入值用于结束phi，请删除它并推一个虚拟值。
					currentContext.endPHI->eraseFromParent();
					WAVM_ASSERT_THROW(currentContext.resultType != ResultType::none);
					push(typedZeroConstants[(Uptr)asValueType(currentContext.resultType)]);
				}
			}

//此控件上下文引入的POP和分支目标。
			WAVM_ASSERT_THROW(currentContext.outerBranchTargetStackSize <= branchTargetStack.size());
			branchTargetStack.resize(currentContext.outerBranchTargetStackSize);

//弹出此控件上下文。
			controlStack.pop_back();
		}
		
//
//控制流操作员
//
		
		BranchTarget& getBranchTargetByDepth(Uptr depth)
		{
			WAVM_ASSERT_THROW(depth < branchTargetStack.size());
			return branchTargetStack[branchTargetStack.size() - depth - 1];
		}
		
//这是在无条件控制流之后调用的，以指示在弹出控制堆栈之前，跟随它的运算符是不可访问的。
		void enterUnreachable()
		{
//将操作数堆栈释放到外部控件上下文。
			WAVM_ASSERT_THROW(controlStack.back().outerStackSize <= stack.size());
			stack.resize(controlStack.back().outerStackSize);

//将当前控制上下文标记为不可访问：这将导致外部循环停止向我们分派操作员
//直到到达当前控件上下文的else/end。
			controlStack.back().isReachable = false;
		}
		
		void br_if(BranchImm imm)
		{
//从操作数堆栈中弹出条件。
			auto condition = pop();

			BranchTarget& target = getBranchTargetByDepth(imm.targetDepth);
			if(target.argumentType != ResultType::none)
			{
//使用stack top作为分支参数（不要弹出它），并将其添加到目标phi的输入值中。
				llvm::Value* argument = getTopValue();
				target.phi->addIncoming(argument,irBuilder.GetInsertBlock());
			}

//为不采用分支的情况创建新的基本块。
			auto falseBlock = llvm::BasicBlock::Create(context,"br_ifElse",llvmFunction);

//向假块或目标块发出条件分支。
			irBuilder.CreateCondBr(coerceI32ToBool(condition),target.block,falseBlock);

//继续发出错误块中的指令。
			irBuilder.SetInsertPoint(falseBlock);
		}
		
		void br(BranchImm imm)
		{
			BranchTarget& target = getBranchTargetByDepth(imm.targetDepth);
			if(target.argumentType != ResultType::none)
			{
//从堆栈中弹出分支参数，并将其添加到目标phi的传入值中。
				llvm::Value* argument = pop();
				target.phi->addIncoming(argument,irBuilder.GetInsertBlock());
			}

//分支到目标块。
			irBuilder.CreateBr(target.block);

			enterUnreachable();
		}
		void br_table(BranchTableImm imm)
		{
//从操作数堆栈中弹出表索引。
			auto index = pop();
			
//查找默认的分支目标，并假定其参数类型应用于所有目标。
//（这由验证器保证）
			BranchTarget& defaultTarget = getBranchTargetByDepth(imm.defaultTargetDepth);
			const ResultType argumentType = defaultTarget.argumentType;
			llvm::Value* argument = nullptr;
			if(argumentType != ResultType::none)
			{
//从堆栈中弹出分支参数，并将其添加到默认目标phi的传入值中。
				argument = pop();
				defaultTarget.phi->addIncoming(argument,irBuilder.GetInsertBlock());
			}

//创建LLVM开关指令。
			WAVM_ASSERT_THROW(imm.branchTableIndex < functionDef.branchTables.size());
			const std::vector<U32>& targetDepths = functionDef.branchTables[imm.branchTableIndex];
			auto llvmSwitch = irBuilder.CreateSwitch(index,defaultTarget.block,(unsigned int)targetDepths.size());

			for(Uptr targetIndex = 0;targetIndex < targetDepths.size();++targetIndex)
			{
				BranchTarget& target = getBranchTargetByDepth(targetDepths[targetIndex]);

//将此目标添加到switch指令。
				llvmSwitch->addCase(emitLiteral((U32)targetIndex),target.block);

				if(argumentType != ResultType::none)
				{
//如果这是该分支目标表中的第一个事例，请将branch参数添加到
//目标phi的输入值。
					target.phi->addIncoming(argument,irBuilder.GetInsertBlock());
				}
			}

			enterUnreachable();
		}
		void return_(NoImm)
		{
			if(functionType->ret != ResultType::none)
			{
//从堆栈中弹出返回值，并将其添加到返回phi的输入值中。
				llvm::Value* result = pop();
				controlStack[0].endPHI->addIncoming(result,irBuilder.GetInsertBlock());
			}

//分支到返回块。
			irBuilder.CreateBr(controlStack[0].endBlock);

			enterUnreachable();
		}

		void unreachable(NoImm)
		{
//调用导致陷阱的内部函数，并插入LLVM不可访问的终止符。
			emitRuntimeIntrinsic("wavmIntrinsics.unreachableTrap",FunctionType::get(),{});
			irBuilder.CreateUnreachable();

			enterUnreachable();
		}

//
//多态运算符
//

		void drop(NoImm) { stack.pop_back(); }

		void select(NoImm)
		{
			auto condition = pop();
			auto falseValue = pop();
			auto trueValue = pop();
			push(irBuilder.CreateSelect(coerceI32ToBool(condition),trueValue,falseValue));
		}

//
//调用操作符
//

		void call(CallImm imm)
		{
//将被调用函数索引映射到导入的函数指针或此模块中的函数。
			llvm::Value* callee;
			const FunctionType* calleeType;
			if(imm.functionIndex < moduleContext.importedFunctionPointers.size())
			{
				WAVM_ASSERT_THROW(imm.functionIndex < moduleContext.moduleInstance->functions.size());
				callee = moduleContext.importedFunctionPointers[imm.functionIndex];
				calleeType = moduleContext.moduleInstance->functions[imm.functionIndex]->type;
			}
			else
			{
				const Uptr calleeIndex = imm.functionIndex - moduleContext.importedFunctionPointers.size();
				WAVM_ASSERT_THROW(calleeIndex < moduleContext.functionDefs.size());
				callee = moduleContext.functionDefs[calleeIndex];
				calleeType = module.types[module.functions.defs[calleeIndex].type.index];
			}

//从操作数堆栈中弹出调用参数。
			auto llvmArgs = (llvm::Value**)alloca(sizeof(llvm::Value*) * calleeType->parameters.size());
			popMultiple(llvmArgs,calleeType->parameters.size());

//调用函数。
			auto result = irBuilder.CreateCall(callee,llvm::ArrayRef<llvm::Value*>(llvmArgs,calleeType->parameters.size()));

//将结果推送到操作数堆栈上。
			if(calleeType->ret != ResultType::none) { push(result); }
		}
		void call_indirect(CallIndirectImm imm)
		{
			WAVM_ASSERT_THROW(imm.type.index < module.types.size());
			
			auto calleeType = module.types[imm.type.index];
			auto functionPointerType = asLLVMType(calleeType)->getPointerTo()->getPointerTo();

//编译函数索引。
			auto tableElementIndex = pop();
			
//编译调用参数。
			auto llvmArgs = (llvm::Value**)alloca(sizeof(llvm::Value*) * calleeType->parameters.size());
			popMultiple(llvmArgs,calleeType->parameters.size());

//零将函数索引扩展到指针大小。
			auto functionIndexZExt = irBuilder.CreateZExt(tableElementIndex,sizeof(Uptr) == 4 ? llvmI32Type : llvmI64Type);
			
//如果函数索引大于函数表大小，则陷阱。
			emitConditionalTrapIntrinsic(
				irBuilder.CreateICmpUGE(functionIndexZExt,moduleContext.defaultTableMaxElementIndex),
				"wavmIntrinsics.indirectCallIndexOutOfBounds",FunctionType::get(),{});

//加载此表项的类型。
			auto functionTypePointerPointer = irBuilder.CreateInBoundsGEP(moduleContext.defaultTablePointer,{functionIndexZExt,emitLiteral((U32)0)});
			auto functionTypePointer = irBuilder.CreateLoad(functionTypePointerPointer);
			auto llvmCalleeType = emitLiteralPointer(calleeType,llvmI8PtrType);
			
//如果函数类型不匹配，则陷阱。
			emitConditionalTrapIntrinsic(
				irBuilder.CreateICmpNE(llvmCalleeType,functionTypePointer),
				"wavmIntrinsics.indirectCallSignatureMismatch",
				FunctionType::get(ResultType::none,{ValueType::i32,ValueType::i64,ValueType::i64}),
				{	tableElementIndex,
					irBuilder.CreatePtrToInt(llvmCalleeType,llvmI64Type),
					emitLiteral(reinterpret_cast<U64>(moduleContext.moduleInstance->defaultTable))	}
				);

//调用从表中加载的函数。
			auto functionPointerPointer = irBuilder.CreateInBoundsGEP(moduleContext.defaultTablePointer,{functionIndexZExt,emitLiteral((U32)1)});
			auto functionPointer = irBuilder.CreateLoad(irBuilder.CreatePointerCast(functionPointerPointer,functionPointerType));
			auto result = irBuilder.CreateCall(functionPointer,llvm::ArrayRef<llvm::Value*>(llvmArgs,calleeType->parameters.size()));

//将结果推送到操作数堆栈上。
			if(calleeType->ret != ResultType::none) { push(result); }
		}
		
//
//本地/全球运营商
//

		void get_local(GetOrSetVariableImm<false> imm)
		{
			WAVM_ASSERT_THROW(imm.variableIndex < localPointers.size());
			push(irBuilder.CreateLoad(localPointers[imm.variableIndex]));
		}
		void set_local(GetOrSetVariableImm<false> imm)
		{
			WAVM_ASSERT_THROW(imm.variableIndex < localPointers.size());
			auto value = irBuilder.CreateBitCast(pop(),localPointers[imm.variableIndex]->getType()->getPointerElementType());
			irBuilder.CreateStore(value,localPointers[imm.variableIndex]);
		}
		void tee_local(GetOrSetVariableImm<false> imm)
		{
			WAVM_ASSERT_THROW(imm.variableIndex < localPointers.size());
			auto value = irBuilder.CreateBitCast(getTopValue(),localPointers[imm.variableIndex]->getType()->getPointerElementType());
			irBuilder.CreateStore(value,localPointers[imm.variableIndex]);
		}
		
		void get_global(GetOrSetVariableImm<true> imm)
		{
			WAVM_ASSERT_THROW(imm.variableIndex < moduleContext.globalPointers.size());
			push(irBuilder.CreateLoad(moduleContext.globalPointers[imm.variableIndex]));
		}
		void set_global(GetOrSetVariableImm<true> imm)
		{
			WAVM_ASSERT_THROW(imm.variableIndex < moduleContext.globalPointers.size());
			auto value = irBuilder.CreateBitCast(pop(),moduleContext.globalPointers[imm.variableIndex]->getType()->getPointerElementType());
			irBuilder.CreateStore(value,moduleContext.globalPointers[imm.variableIndex]);
		}

//
//内存大小运算符
//这些函数只调用wavmintrinsics.growmemory/currentmemory，将指针传递给模块的默认内存。
//

		void grow_memory(MemoryImm)
		{
			auto deltaNumPages = pop();
			auto defaultMemoryObjectAsI64 = emitLiteral(reinterpret_cast<U64>(moduleContext.moduleInstance->defaultMemory));
			auto previousNumPages = emitRuntimeIntrinsic(
				"wavmIntrinsics.growMemory",
				FunctionType::get(ResultType::i32,{ValueType::i32,ValueType::i64}),
				{deltaNumPages,defaultMemoryObjectAsI64});
			push(previousNumPages);
		}
		void current_memory(MemoryImm)
		{
			auto defaultMemoryObjectAsI64 = emitLiteral(reinterpret_cast<U64>(moduleContext.moduleInstance->defaultMemory));
			auto currentNumPages = emitRuntimeIntrinsic(
				"wavmIntrinsics.currentMemory",
				FunctionType::get(ResultType::i32,{ValueType::i64}),
				{defaultMemoryObjectAsI64});
			push(currentNumPages);
		}

//
//常量运算符
//

		#define EMIT_CONST(typeId,nativeType) void typeId##_const(LiteralImm<nativeType> imm) { push(emitLiteral(imm.value)); }
		EMIT_CONST(i32,I32) EMIT_CONST(i64,I64)
		EMIT_CONST(f32,F32) EMIT_CONST(f64,F64)

//
//装载/储存操作员
//

		#define EMIT_LOAD_OP(valueTypeId,name,llvmMemoryType,naturalAlignmentLog2,conversionOp) \
			void valueTypeId##_##name(LoadOrStoreImm<naturalAlignmentLog2> imm) \
			{ \
				auto byteIndex = pop(); \
				auto pointer = coerceByteIndexToPointer(byteIndex,imm.offset,llvmMemoryType); \
				auto load = irBuilder.CreateLoad(pointer); \
				load->setAlignment(1<<imm.alignmentLog2); \
				load->setVolatile(true); \
				push(conversionOp(load,asLLVMType(ValueType::valueTypeId))); \
			}
		#define EMIT_STORE_OP(valueTypeId,name,llvmMemoryType,naturalAlignmentLog2,conversionOp) \
			void valueTypeId##_##name(LoadOrStoreImm<naturalAlignmentLog2> imm) \
			{ \
				auto value = pop(); \
				auto byteIndex = pop(); \
				auto pointer = coerceByteIndexToPointer(byteIndex,imm.offset,llvmMemoryType); \
				auto memoryValue = conversionOp(value,llvmMemoryType); \
				auto store = irBuilder.CreateStore(memoryValue,pointer); \
				store->setVolatile(true); \
				store->setAlignment(1<<imm.alignmentLog2); \
			}
			
		llvm::Value* identityConversion(llvm::Value* value,llvm::Type* type) { return value; }

		EMIT_LOAD_OP(i32,load8_s,llvmI8Type,0,irBuilder.CreateSExt)  EMIT_LOAD_OP(i32,load8_u,llvmI8Type,0,irBuilder.CreateZExt)
		EMIT_LOAD_OP(i32,load16_s,llvmI16Type,1,irBuilder.CreateSExt) EMIT_LOAD_OP(i32,load16_u,llvmI16Type,1,irBuilder.CreateZExt)
		EMIT_LOAD_OP(i64,load8_s,llvmI8Type,0,irBuilder.CreateSExt)  EMIT_LOAD_OP(i64,load8_u,llvmI8Type,0,irBuilder.CreateZExt)
		EMIT_LOAD_OP(i64,load16_s,llvmI16Type,1,irBuilder.CreateSExt)  EMIT_LOAD_OP(i64,load16_u,llvmI16Type,1,irBuilder.CreateZExt)
		EMIT_LOAD_OP(i64,load32_s,llvmI32Type,2,irBuilder.CreateSExt)  EMIT_LOAD_OP(i64,load32_u,llvmI32Type,2,irBuilder.CreateZExt)

		EMIT_LOAD_OP(i32,load,llvmI32Type,2,identityConversion) EMIT_LOAD_OP(i64,load,llvmI64Type,3,identityConversion)
		EMIT_LOAD_OP(f32,load,llvmF32Type,2,identityConversion) EMIT_LOAD_OP(f64,load,llvmF64Type,3,identityConversion)

		EMIT_STORE_OP(i32,store8,llvmI8Type,0,irBuilder.CreateTrunc) EMIT_STORE_OP(i64,store8,llvmI8Type,0,irBuilder.CreateTrunc)
		EMIT_STORE_OP(i32,store16,llvmI16Type,1,irBuilder.CreateTrunc) EMIT_STORE_OP(i64,store16,llvmI16Type,1,irBuilder.CreateTrunc)
		EMIT_STORE_OP(i32,store,llvmI32Type,2,irBuilder.CreateTrunc) EMIT_STORE_OP(i64,store32,llvmI32Type,2,irBuilder.CreateTrunc)
		EMIT_STORE_OP(i64,store,llvmI64Type,3,identityConversion)
		EMIT_STORE_OP(f32,store,llvmF32Type,2,identityConversion) EMIT_STORE_OP(f64,store,llvmF64Type,3,identityConversion)

//
//数字运算符宏
//

		#define EMIT_BINARY_OP(typeId,name,emitCode) void typeId##_##name(NoImm) \
			{ \
				const ValueType type = ValueType::typeId; SUPPRESS_UNUSED(type); \
				auto right = pop(); \
				auto left = pop(); \
				push(emitCode); \
			}
		#define EMIT_INT_BINARY_OP(name,emitCode) EMIT_BINARY_OP(i32,name,emitCode) EMIT_BINARY_OP(i64,name,emitCode)
		#define EMIT_FP_BINARY_OP(name,emitCode) EMIT_BINARY_OP(f32,name,emitCode) EMIT_BINARY_OP(f64,name,emitCode)

		#define EMIT_UNARY_OP(typeId,name,emitCode) void typeId##_##name(NoImm) \
			{ \
				const ValueType type = ValueType::typeId; SUPPRESS_UNUSED(type); \
				auto operand = pop(); \
				push(emitCode); \
			}
		#define EMIT_INT_UNARY_OP(name,emitCode) EMIT_UNARY_OP(i32,name,emitCode) EMIT_UNARY_OP(i64,name,emitCode)
		#define EMIT_FP_UNARY_OP(name,emitCode) EMIT_UNARY_OP(f32,name,emitCode) EMIT_UNARY_OP(f64,name,emitCode)

//
//int算子
//

		llvm::Value* emitSRem(ValueType type,llvm::Value* left,llvm::Value* right)
		{
//如果股息为零，则设陷阱。
			trapDivideByZero(type,right); 

//llvm的s rem具有未定义的行为，其中webassembly的rem定义如果相应的
//除法会溢出有符号整数。为了避免这种情况，我们只是围绕SREM分支，如果int_max%-1个案例
//检测到溢出。
			auto preOverflowBlock = irBuilder.GetInsertBlock();
			auto noOverflowBlock = llvm::BasicBlock::Create(context,"sremNoOverflow",llvmFunction);
			auto endBlock = llvm::BasicBlock::Create(context,"sremEnd",llvmFunction);
			auto noOverflow = irBuilder.CreateOr(
				irBuilder.CreateICmpNE(left,type == ValueType::i32 ? emitLiteral((U32)INT32_MIN) : emitLiteral((U64)INT64_MIN)),
				irBuilder.CreateICmpNE(right,type == ValueType::i32 ? emitLiteral((U32)-1) : emitLiteral((U64)-1))
				);
			irBuilder.CreateCondBr(noOverflow,noOverflowBlock,endBlock,moduleContext.likelyTrueBranchWeights);

			irBuilder.SetInsertPoint(noOverflowBlock);
			auto noOverflowValue = irBuilder.CreateSRem(left,right);
			irBuilder.CreateBr(endBlock);

			irBuilder.SetInsertPoint(endBlock);
			auto phi = irBuilder.CreatePHI(asLLVMType(type),2);
			phi->addIncoming(typedZeroConstants[(Uptr)type],preOverflowBlock);
			phi->addIncoming(noOverflowValue,noOverflowBlock);
			return phi;
		}
		
		llvm::Value* emitShiftCountMask(ValueType type,llvm::Value* shiftCount)
		{
//llvm的移位具有未定义的行为，Webassembly指定移位计数将换行数字。
//比操作数的位计数还要小。这与x86的本地移位指令匹配，但显式屏蔽
//无论如何，移位计数支持其他平台，并确保优化器不会利用UB。
			auto bitsMinusOne = irBuilder.CreateZExt(emitLiteral((U8)(getTypeBitWidth(type) - 1)),asLLVMType(type));
			return irBuilder.CreateAnd(shiftCount,bitsMinusOne);
		}

		llvm::Value* emitRotl(ValueType type,llvm::Value* left,llvm::Value* right)
		{
			auto bitWidthMinusRight = irBuilder.CreateSub(
				irBuilder.CreateZExt(emitLiteral(getTypeBitWidth(type)),asLLVMType(type)),
				right
				);
			return irBuilder.CreateOr(
				irBuilder.CreateShl(left,emitShiftCountMask(type,right)),
				irBuilder.CreateLShr(left,emitShiftCountMask(type,bitWidthMinusRight))
				);
		}
		
		llvm::Value* emitRotr(ValueType type,llvm::Value* left,llvm::Value* right)
		{
			auto bitWidthMinusRight = irBuilder.CreateSub(
				irBuilder.CreateZExt(emitLiteral(getTypeBitWidth(type)),asLLVMType(type)),
				right
				);
			return irBuilder.CreateOr(
				irBuilder.CreateShl(left,emitShiftCountMask(type,bitWidthMinusRight)),
				irBuilder.CreateLShr(left,emitShiftCountMask(type,right))
				);
		}

		EMIT_INT_BINARY_OP(add,irBuilder.CreateAdd(left,right))
		EMIT_INT_BINARY_OP(sub,irBuilder.CreateSub(left,right))
		EMIT_INT_BINARY_OP(mul,irBuilder.CreateMul(left,right))
		EMIT_INT_BINARY_OP(and,irBuilder.CreateAnd(left,right))
		EMIT_INT_BINARY_OP(or,irBuilder.CreateOr(left,right))
		EMIT_INT_BINARY_OP(xor,irBuilder.CreateXor(left,right))
		EMIT_INT_BINARY_OP(rotr,emitRotr(type,left,right))
		EMIT_INT_BINARY_OP(rotl,emitRotl(type,left,right))
			
//除法使用trapDivideByZero来避免llvm除法指令中未定义的行为。
		EMIT_INT_BINARY_OP(div_s, (trapDivideByZeroOrIntegerOverflow(type,left,right), irBuilder.CreateSDiv(left,right)) )
		EMIT_INT_BINARY_OP(rem_s, emitSRem(type,left,right) )
		EMIT_INT_BINARY_OP(div_u, (trapDivideByZero(type,right), irBuilder.CreateUDiv(left,right)) )
		EMIT_INT_BINARY_OP(rem_u, (trapDivideByZero(type,right), irBuilder.CreateURem(left,right)) )

//显式地将移位量操作数屏蔽为字大小，以避免llvm的未定义行为。
		EMIT_INT_BINARY_OP(shl,irBuilder.CreateShl(left,emitShiftCountMask(type,right)))
		EMIT_INT_BINARY_OP(shr_s,irBuilder.CreateAShr(left,emitShiftCountMask(type,right)))
		EMIT_INT_BINARY_OP(shr_u,irBuilder.CreateLShr(left,emitShiftCountMask(type,right)))
		
		EMIT_INT_BINARY_OP(eq,coerceBoolToI32(irBuilder.CreateICmpEQ(left,right)))
		EMIT_INT_BINARY_OP(ne,coerceBoolToI32(irBuilder.CreateICmpNE(left,right)))
		EMIT_INT_BINARY_OP(lt_s,coerceBoolToI32(irBuilder.CreateICmpSLT(left,right)))
		EMIT_INT_BINARY_OP(lt_u,coerceBoolToI32(irBuilder.CreateICmpULT(left,right)))
		EMIT_INT_BINARY_OP(le_s,coerceBoolToI32(irBuilder.CreateICmpSLE(left,right)))
		EMIT_INT_BINARY_OP(le_u,coerceBoolToI32(irBuilder.CreateICmpULE(left,right)))
		EMIT_INT_BINARY_OP(gt_s,coerceBoolToI32(irBuilder.CreateICmpSGT(left,right)))
		EMIT_INT_BINARY_OP(gt_u,coerceBoolToI32(irBuilder.CreateICmpUGT(left,right)))
		EMIT_INT_BINARY_OP(ge_s,coerceBoolToI32(irBuilder.CreateICmpSGE(left,right)))
		EMIT_INT_BINARY_OP(ge_u,coerceBoolToI32(irBuilder.CreateICmpUGE(left,right)))

		EMIT_INT_UNARY_OP(clz,irBuilder.CreateCall(getLLVMIntrinsic({operand->getType()},llvm::Intrinsic::ctlz),llvm::ArrayRef<llvm::Value*>({operand,emitLiteral(false)})))
		EMIT_INT_UNARY_OP(ctz,irBuilder.CreateCall(getLLVMIntrinsic({operand->getType()},llvm::Intrinsic::cttz),llvm::ArrayRef<llvm::Value*>({operand,emitLiteral(false)})))
		EMIT_INT_UNARY_OP(popcnt,irBuilder.CreateCall(getLLVMIntrinsic({operand->getType()},llvm::Intrinsic::ctpop),llvm::ArrayRef<llvm::Value*>({operand})))
		EMIT_INT_UNARY_OP(eqz,coerceBoolToI32(irBuilder.CreateICmpEQ(operand,typedZeroConstants[(Uptr)type])))

//
//FP算子
//

		EMIT_FP_BINARY_OP(add,irBuilder.CreateFAdd(left,right))
		EMIT_FP_BINARY_OP(sub,irBuilder.CreateFSub(left,right))
		EMIT_FP_BINARY_OP(mul,irBuilder.CreateFMul(left,right))
		EMIT_FP_BINARY_OP(div,irBuilder.CreateFDiv(left,right))
		EMIT_FP_BINARY_OP(copysign,irBuilder.CreateCall(getLLVMIntrinsic({left->getType()},llvm::Intrinsic::copysign),llvm::ArrayRef<llvm::Value*>({left,right})))

		EMIT_FP_UNARY_OP(neg,irBuilder.CreateFNeg(operand))
		EMIT_FP_UNARY_OP(abs,irBuilder.CreateCall(getLLVMIntrinsic({operand->getType()},llvm::Intrinsic::fabs),llvm::ArrayRef<llvm::Value*>({operand})))
		EMIT_FP_UNARY_OP(sqrt,irBuilder.CreateCall(getLLVMIntrinsic({operand->getType()},llvm::Intrinsic::sqrt),llvm::ArrayRef<llvm::Value*>({operand})))

		EMIT_FP_BINARY_OP(eq,coerceBoolToI32(irBuilder.CreateFCmpOEQ(left,right)))
		EMIT_FP_BINARY_OP(ne,coerceBoolToI32(irBuilder.CreateFCmpUNE(left,right)))
		EMIT_FP_BINARY_OP(lt,coerceBoolToI32(irBuilder.CreateFCmpOLT(left,right)))
		EMIT_FP_BINARY_OP(le,coerceBoolToI32(irBuilder.CreateFCmpOLE(left,right)))
		EMIT_FP_BINARY_OP(gt,coerceBoolToI32(irBuilder.CreateFCmpOGT(left,right)))
		EMIT_FP_BINARY_OP(ge,coerceBoolToI32(irBuilder.CreateFCmpOGE(left,right)))

		EMIT_UNARY_OP(i32,wrap_i64,irBuilder.CreateTrunc(operand,llvmI32Type))
		EMIT_UNARY_OP(i64,extend_s_i32,irBuilder.CreateSExt(operand,llvmI64Type))
		EMIT_UNARY_OP(i64,extend_u_i32,irBuilder.CreateZExt(operand,llvmI64Type))

		EMIT_FP_UNARY_OP(convert_s_i32,irBuilder.CreateSIToFP(operand,asLLVMType(type)))
		EMIT_FP_UNARY_OP(convert_s_i64,irBuilder.CreateSIToFP(operand,asLLVMType(type)))
		EMIT_FP_UNARY_OP(convert_u_i32,irBuilder.CreateUIToFP(operand,asLLVMType(type)))
		EMIT_FP_UNARY_OP(convert_u_i64,irBuilder.CreateUIToFP(operand,asLLVMType(type)))

		EMIT_UNARY_OP(f32,demote_f64,irBuilder.CreateFPTrunc(operand,llvmF32Type))
		EMIT_UNARY_OP(f64,promote_f32,irBuilder.CreateFPExt(operand,llvmF64Type))
		EMIT_UNARY_OP(f32,reinterpret_i32,irBuilder.CreateBitCast(operand,llvmF32Type))
		EMIT_UNARY_OP(f64,reinterpret_i64,irBuilder.CreateBitCast(operand,llvmF64Type))
		EMIT_UNARY_OP(i32,reinterpret_f32,irBuilder.CreateBitCast(operand,llvmI32Type))
		EMIT_UNARY_OP(i64,reinterpret_f64,irBuilder.CreateBitCast(operand,llvmI64Type))

//这些操作不完全匹配LLVM的语义，所以只需调用C++实现即可。
		EMIT_FP_BINARY_OP(min,emitRuntimeIntrinsic("wavmIntrinsics.floatMin",FunctionType::get(asResultType(type),{type,type}),{left,right}))
		EMIT_FP_BINARY_OP(max,emitRuntimeIntrinsic("wavmIntrinsics.floatMax",FunctionType::get(asResultType(type),{type,type}),{left,right}))
		EMIT_FP_UNARY_OP(ceil,emitRuntimeIntrinsic("wavmIntrinsics.floatCeil",FunctionType::get(asResultType(type),{type}),{operand}))
		EMIT_FP_UNARY_OP(floor,emitRuntimeIntrinsic("wavmIntrinsics.floatFloor",FunctionType::get(asResultType(type),{type}),{operand}))
		EMIT_FP_UNARY_OP(trunc,emitRuntimeIntrinsic("wavmIntrinsics.floatTrunc",FunctionType::get(asResultType(type),{type}),{operand}))
		EMIT_FP_UNARY_OP(nearest,emitRuntimeIntrinsic("wavmIntrinsics.floatNearest",FunctionType::get(asResultType(type),{type}),{operand}))
		EMIT_INT_UNARY_OP(trunc_s_f32,emitRuntimeIntrinsic("wavmIntrinsics.floatToSignedInt",FunctionType::get(asResultType(type),{ValueType::f32}),{operand}))
		EMIT_INT_UNARY_OP(trunc_s_f64,emitRuntimeIntrinsic("wavmIntrinsics.floatToSignedInt",FunctionType::get(asResultType(type),{ValueType::f64}),{operand}))
		EMIT_INT_UNARY_OP(trunc_u_f32,emitRuntimeIntrinsic("wavmIntrinsics.floatToUnsignedInt",FunctionType::get(asResultType(type),{ValueType::f32}),{operand}))
		EMIT_INT_UNARY_OP(trunc_u_f64,emitRuntimeIntrinsic("wavmIntrinsics.floatToUnsignedInt",FunctionType::get(asResultType(type),{ValueType::f64}),{operand}))

		#if ENABLE_SIMD_PROTOTYPE
		llvm::Value* emitAnyTrue(llvm::Value* boolVector)
		{
			const Uptr numLanes = boolVector->getType()->getVectorNumElements();
			llvm::Value* result = nullptr;
			for(Uptr laneIndex = 0;laneIndex < numLanes;++laneIndex)
			{
				llvm::Value* scalar = irBuilder.CreateExtractElement(boolVector,laneIndex);
				result = result ? irBuilder.CreateOr(result,scalar) : scalar;
			}
			return result;
		}
		llvm::Value* emitAllTrue(llvm::Value* boolVector)
		{
			const Uptr numLanes = boolVector->getType()->getVectorNumElements();
			llvm::Value* result = nullptr;
			for(Uptr laneIndex = 0;laneIndex < numLanes;++laneIndex)
			{
				llvm::Value* scalar = irBuilder.CreateExtractElement(boolVector,laneIndex);
				result = result ? irBuilder.CreateAnd(result,scalar) : scalar;
			}
			return result;
		}

		llvm::Value* unimplemented()
		{
			Errors::unreachable();
		}

		#define EMIT_SIMD_SPLAT(vectorType,coerceScalar,numLanes) \
			void vectorType##_splat(NoImm) \
			{ \
				auto scalar = pop(); \
				push(irBuilder.CreateVectorSplat(numLanes,coerceScalar)); \
			}
		EMIT_SIMD_SPLAT(i8x16,irBuilder.CreateTrunc(scalar,llvmI8Type),16)
		EMIT_SIMD_SPLAT(i16x8,irBuilder.CreateTrunc(scalar,llvmI16Type),8)
		EMIT_SIMD_SPLAT(i32x4,scalar,4) 
		EMIT_SIMD_SPLAT(i64x2,scalar,2)
		EMIT_SIMD_SPLAT(f32x4,scalar,4)
		EMIT_SIMD_SPLAT(f64x2,scalar,2)

		EMIT_STORE_OP(v128,store,value->getType(),4,identityConversion)
		EMIT_LOAD_OP(v128,load,llvmI64x2Type,4,identityConversion)

		#define EMIT_SIMD_BINARY_OP(name,llvmType,emitCode) \
			void name(NoImm) \
			{ \
				auto right = irBuilder.CreateBitCast(pop(),llvmType); SUPPRESS_UNUSED(right); \
				auto left = irBuilder.CreateBitCast(pop(),llvmType); SUPPRESS_UNUSED(left); \
				push(emitCode); \
			}
		#define EMIT_SIMD_UNARY_OP(name,llvmType,emitCode) \
			void name(NoImm) \
			{ \
				auto operand = irBuilder.CreateBitCast(pop(),llvmType); SUPPRESS_UNUSED(operand); \
				push(emitCode); \
			}
		#define EMIT_SIMD_INT_BINARY_OP(name,emitCode) \
			EMIT_SIMD_BINARY_OP(i8x16##_##name,llvmI8x16Type,emitCode) \
			EMIT_SIMD_BINARY_OP(i16x8##_##name,llvmI16x8Type,emitCode) \
			EMIT_SIMD_BINARY_OP(i32x4##_##name,llvmI32x4Type,emitCode) \
			EMIT_SIMD_BINARY_OP(i64x2##_##name,llvmI64x2Type,emitCode)
		#define EMIT_SIMD_FP_BINARY_OP(name,emitCode) \
			EMIT_SIMD_BINARY_OP(f32x4##_##name,llvmF32x4Type,emitCode) \
			EMIT_SIMD_BINARY_OP(f64x2##_##name,llvmF64x2Type,emitCode)
		#define EMIT_SIMD_INT_UNARY_OP(name,emitCode) \
			EMIT_SIMD_UNARY_OP(i8x16##_##name,llvmI8x16Type,emitCode) \
			EMIT_SIMD_UNARY_OP(i16x8##_##name,llvmI16x8Type,emitCode) \
			EMIT_SIMD_UNARY_OP(i32x4##_##name,llvmI32x4Type,emitCode) \
			EMIT_SIMD_UNARY_OP(i64x2##_##name,llvmI64x2Type,emitCode)
		#define EMIT_SIMD_FP_UNARY_OP(name,emitCode) \
			EMIT_SIMD_UNARY_OP(f32x4##_##name,llvmF32x4Type,emitCode) \
			EMIT_SIMD_UNARY_OP(f64x2##_##name,llvmF64x2Type,emitCode)
		EMIT_SIMD_INT_BINARY_OP(add,irBuilder.CreateAdd(left,right))
		EMIT_SIMD_INT_BINARY_OP(sub,irBuilder.CreateSub(left,right))

		EMIT_SIMD_INT_BINARY_OP(shl,irBuilder.CreateShl(left,right))
		EMIT_SIMD_INT_BINARY_OP(shr_s,irBuilder.CreateAShr(left,right))
		EMIT_SIMD_INT_BINARY_OP(shr_u,irBuilder.CreateLShr(left,right))
		EMIT_SIMD_INT_BINARY_OP(mul,irBuilder.CreateMul(left,right))
		EMIT_SIMD_INT_BINARY_OP(div_s,irBuilder.CreateSDiv(left,right))
		EMIT_SIMD_INT_BINARY_OP(div_u,irBuilder.CreateUDiv(left,right))

		EMIT_SIMD_INT_BINARY_OP(eq,irBuilder.CreateICmpEQ(left,right))
		EMIT_SIMD_INT_BINARY_OP(ne,irBuilder.CreateICmpNE(left,right))
		EMIT_SIMD_INT_BINARY_OP(lt_s,irBuilder.CreateICmpSLT(left,right))
		EMIT_SIMD_INT_BINARY_OP(lt_u,irBuilder.CreateICmpULT(left,right))
		EMIT_SIMD_INT_BINARY_OP(le_s,irBuilder.CreateICmpSLE(left,right))
		EMIT_SIMD_INT_BINARY_OP(le_u,irBuilder.CreateICmpULE(left,right))
		EMIT_SIMD_INT_BINARY_OP(gt_s,irBuilder.CreateICmpSGT(left,right))
		EMIT_SIMD_INT_BINARY_OP(gt_u,irBuilder.CreateICmpUGT(left,right))
		EMIT_SIMD_INT_BINARY_OP(ge_s,irBuilder.CreateICmpSGE(left,right))
		EMIT_SIMD_INT_BINARY_OP(ge_u,irBuilder.CreateICmpUGE(left,right))

		EMIT_SIMD_INT_UNARY_OP(neg,irBuilder.CreateNeg(operand))

		EMIT_SIMD_BINARY_OP(i8x16_add_saturate_s,llvmI8x16Type,unimplemented())
		EMIT_SIMD_BINARY_OP(i8x16_add_saturate_u,llvmI16x8Type,unimplemented())
		EMIT_SIMD_BINARY_OP(i8x16_sub_saturate_s,llvmI8x16Type,unimplemented())
		EMIT_SIMD_BINARY_OP(i8x16_sub_saturate_u,llvmI16x8Type,unimplemented())
		EMIT_SIMD_BINARY_OP(i16x8_add_saturate_s,llvmI8x16Type,unimplemented())
		EMIT_SIMD_BINARY_OP(i16x8_add_saturate_u,llvmI16x8Type,unimplemented())
		EMIT_SIMD_BINARY_OP(i16x8_sub_saturate_s,llvmI8x16Type,unimplemented())
		EMIT_SIMD_BINARY_OP(i16x8_sub_saturate_u,llvmI16x8Type,unimplemented())

		EMIT_SIMD_UNARY_OP(i32x4_trunc_s_f32x4_sat,llvmF32x4Type,unimplemented());
		EMIT_SIMD_UNARY_OP(i32x4_trunc_u_f32x4_sat,llvmF32x4Type,unimplemented());
		EMIT_SIMD_UNARY_OP(i64x2_trunc_s_f64x2_sat,llvmF64x2Type,unimplemented());
		EMIT_SIMD_UNARY_OP(i64x2_trunc_u_f64x2_sat,llvmF64x2Type,unimplemented());

		EMIT_SIMD_FP_BINARY_OP(add,irBuilder.CreateFAdd(left,right))
		EMIT_SIMD_FP_BINARY_OP(sub,irBuilder.CreateFSub(left,right))
		EMIT_SIMD_FP_BINARY_OP(mul,irBuilder.CreateFMul(left,right))
		EMIT_SIMD_FP_BINARY_OP(div,irBuilder.CreateFDiv(left,right))
			
		EMIT_SIMD_FP_BINARY_OP(eq,irBuilder.CreateFCmpOEQ(left,right))
		EMIT_SIMD_FP_BINARY_OP(ne,irBuilder.CreateFCmpUNE(left,right))
		EMIT_SIMD_FP_BINARY_OP(lt,irBuilder.CreateFCmpOLT(left,right))
		EMIT_SIMD_FP_BINARY_OP(le,irBuilder.CreateFCmpOLE(left,right))
		EMIT_SIMD_FP_BINARY_OP(gt,irBuilder.CreateFCmpOGT(left,right))
		EMIT_SIMD_FP_BINARY_OP(ge,irBuilder.CreateFCmpOGE(left,right))
		EMIT_SIMD_FP_BINARY_OP(min,unimplemented());
		EMIT_SIMD_FP_BINARY_OP(max,unimplemented());

		EMIT_SIMD_FP_UNARY_OP(neg,irBuilder.CreateFNeg(operand))
		EMIT_SIMD_FP_UNARY_OP(abs,irBuilder.CreateCall(getLLVMIntrinsic({operand->getType()},llvm::Intrinsic::fabs),llvm::ArrayRef<llvm::Value*>({operand})))
		EMIT_SIMD_FP_UNARY_OP(sqrt,irBuilder.CreateCall(getLLVMIntrinsic({operand->getType()},llvm::Intrinsic::sqrt),llvm::ArrayRef<llvm::Value*>({operand})))

		EMIT_SIMD_UNARY_OP(f32x4_convert_s_i32x4,llvmI32x4Type,irBuilder.CreateSIToFP(operand,llvmF32x4Type));
		EMIT_SIMD_UNARY_OP(f32x4_convert_u_i32x4,llvmI32x4Type,irBuilder.CreateUIToFP(operand,llvmF32x4Type));
		EMIT_SIMD_UNARY_OP(f64x2_convert_s_i64x2,llvmI64x2Type,irBuilder.CreateSIToFP(operand,llvmF64x2Type));
		EMIT_SIMD_UNARY_OP(f64x2_convert_u_i64x2,llvmI64x2Type,irBuilder.CreateUIToFP(operand,llvmF64x2Type));

		EMIT_SIMD_UNARY_OP(i8x16_any_true,llvmI8x16Type,emitAnyTrue(operand))
		EMIT_SIMD_UNARY_OP(i16x8_any_true,llvmI16x8Type,emitAnyTrue(operand))
		EMIT_SIMD_UNARY_OP(i32x4_any_true,llvmI32x4Type,emitAnyTrue(operand))
		EMIT_SIMD_UNARY_OP(i64x2_any_true,llvmI64x2Type,emitAnyTrue(operand))

		EMIT_SIMD_UNARY_OP(i8x16_all_true,llvmI8x16Type,emitAllTrue(operand))
		EMIT_SIMD_UNARY_OP(i16x8_all_true,llvmI16x8Type,emitAllTrue(operand))
		EMIT_SIMD_UNARY_OP(i32x4_all_true,llvmI32x4Type,emitAllTrue(operand))
		EMIT_SIMD_UNARY_OP(i64x2_all_true,llvmI64x2Type,emitAllTrue(operand))

		void v128_and(NoImm)
		{
			auto right = pop();
			auto left = irBuilder.CreateBitCast(pop(),right->getType());
			push(irBuilder.CreateAnd(left,right));
		}
		void v128_or(NoImm)
		{
			auto right = pop();
			auto left = irBuilder.CreateBitCast(pop(),right->getType());
			push(irBuilder.CreateOr(left,right));
		}
		void v128_xor(NoImm)
		{
			auto right = pop();
			auto left = irBuilder.CreateBitCast(pop(),right->getType());
			push(irBuilder.CreateXor(left,right));
		}
		void v128_not(NoImm)
		{
			auto operand = pop();
			push(irBuilder.CreateNot(operand));
		}

		#define EMIT_SIMD_EXTRACT_LANE_OP(name,llvmType,numLanes,coerceScalar) \
			void name(LaneIndexImm<numLanes> imm) \
			{ \
				auto operand = irBuilder.CreateBitCast(pop(),llvmType); \
				auto scalar = irBuilder.CreateExtractElement(operand,imm.laneIndex); \
				push(coerceScalar); \
			}
		EMIT_SIMD_EXTRACT_LANE_OP(i8x16_extract_lane_s,llvmI8x16Type,16,irBuilder.CreateSExt(scalar,llvmI32Type))
		EMIT_SIMD_EXTRACT_LANE_OP(i8x16_extract_lane_u,llvmI8x16Type,16,irBuilder.CreateZExt(scalar,llvmI32Type))
		EMIT_SIMD_EXTRACT_LANE_OP(i16x8_extract_lane_s,llvmI16x8Type,8,irBuilder.CreateSExt(scalar,llvmI32Type))
		EMIT_SIMD_EXTRACT_LANE_OP(i16x8_extract_lane_u,llvmI16x8Type,8,irBuilder.CreateZExt(scalar,llvmI32Type))
		EMIT_SIMD_EXTRACT_LANE_OP(i32x4_extract_lane,llvmI32x4Type,4,scalar)
		EMIT_SIMD_EXTRACT_LANE_OP(i64x2_extract_lane,llvmI64x2Type,2,scalar)

		EMIT_SIMD_EXTRACT_LANE_OP(f32x4_extract_lane,llvmF32x4Type,4,scalar)
		EMIT_SIMD_EXTRACT_LANE_OP(f64x2_extract_lane,llvmF64x2Type,2,scalar)
		
		#define EMIT_SIMD_REPLACE_LANE_OP(typePrefix,llvmType,numLanes,coerceScalar) \
			void typePrefix##_replace_lane(LaneIndexImm<numLanes> imm) \
			{ \
				auto vector = irBuilder.CreateBitCast(pop(),llvmType); \
				auto scalar = pop(); \
				push(irBuilder.CreateInsertElement(vector,coerceScalar,imm.laneIndex)); \
			}

		EMIT_SIMD_REPLACE_LANE_OP(i8x16,llvmI8x16Type,16,irBuilder.CreateTrunc(scalar,llvmI8Type))
		EMIT_SIMD_REPLACE_LANE_OP(i16x8,llvmI16x8Type,8,irBuilder.CreateTrunc(scalar,llvmI16Type))
		EMIT_SIMD_REPLACE_LANE_OP(i32x4,llvmI32x4Type,4,scalar)
		EMIT_SIMD_REPLACE_LANE_OP(i64x2,llvmI64x2Type,2,scalar)

		EMIT_SIMD_REPLACE_LANE_OP(f32x4,llvmF32x4Type,4,scalar)
		EMIT_SIMD_REPLACE_LANE_OP(f64x2,llvmF64x2Type,2,scalar)

		void v8x16_shuffle(ShuffleImm<16> imm)
		{
			auto right = irBuilder.CreateBitCast(pop(),llvmI8x16Type);
			auto left = irBuilder.CreateBitCast(pop(),llvmI8x16Type);
			unsigned int laneIndices[16];
			for(Uptr laneIndex = 0;laneIndex < 16;++laneIndex)
			{
				laneIndices[laneIndex] = imm.laneIndices[laneIndex];
			}
			push(irBuilder.CreateShuffleVector(left,right,llvm::ArrayRef<unsigned int>(laneIndices,16)));
		}
		
		void v128_const(LiteralImm<V128> imm)
		{
			push(llvm::ConstantVector::get({emitLiteral(imm.value.u64[0]),emitLiteral(imm.value.u64[1])}));
		}

		void v128_bitselect(NoImm)
		{
			auto mask = irBuilder.CreateBitCast(pop(),llvmI64x2Type);
			auto falseValue = irBuilder.CreateBitCast(pop(),llvmI64x2Type);
			auto trueValue = irBuilder.CreateBitCast(pop(),llvmI64x2Type);
			push(irBuilder.CreateOr(
				irBuilder.CreateAnd(trueValue,mask),
				irBuilder.CreateAnd(falseValue,irBuilder.CreateNot(mask))
				));
		}
		#endif

		#if ENABLE_THREADING_PROTOTYPE
		void is_lock_free(NoImm)
		{
			auto numBytes = pop();
			push(emitRuntimeIntrinsic(
				"wavmIntrinsics.isLockFree",
				FunctionType::get(ResultType::i32,{ValueType::i32}),
				{numBytes}));
		}
		void wake(AtomicLoadOrStoreImm<2>)
		{
			auto numWaiters = pop();
			auto address = pop();
			auto defaultMemoryObjectAsI64 = emitLiteral(reinterpret_cast<U64>(moduleContext.moduleInstance->defaultMemory));
			push(emitRuntimeIntrinsic(
				"wavmIntrinsics.wake",
				FunctionType::get(ResultType::i32,{ValueType::i32,ValueType::i32,ValueType::i64}),
				{address,numWaiters,defaultMemoryObjectAsI64}));
		}
		void i32_wait(AtomicLoadOrStoreImm<2>)
		{
			auto timeout = pop();
			auto expectedValue = pop();
			auto address = pop();
			auto defaultMemoryObjectAsI64 = emitLiteral(reinterpret_cast<U64>(moduleContext.moduleInstance->defaultMemory));
			push(emitRuntimeIntrinsic(
				"wavmIntrinsics.wait",
				FunctionType::get(ResultType::i32,{ValueType::i32,ValueType::i32,ValueType::f64,ValueType::i64}),
				{address,expectedValue,timeout,defaultMemoryObjectAsI64}));
		}
		void i64_wait(AtomicLoadOrStoreImm<3>)
		{
			auto timeout = pop();
			auto expectedValue = pop();
			auto address = pop();
			auto defaultMemoryObjectAsI64 = emitLiteral(reinterpret_cast<U64>(moduleContext.moduleInstance->defaultMemory));
			push(emitRuntimeIntrinsic(
				"wavmIntrinsics.wait",
				FunctionType::get(ResultType::i32,{ValueType::i32,ValueType::i64,ValueType::f64,ValueType::i64}),
				{address,expectedValue,timeout,defaultMemoryObjectAsI64}));
		}

		void launch_thread(LaunchThreadImm)
		{
			WAVM_ASSERT_THROW(moduleContext.moduleInstance->defaultTable);
			auto errorFunctionIndex = pop();
			auto argument = pop();
			auto functionIndex = pop();
			auto defaultTableAsI64 = emitLiteral(reinterpret_cast<U64>(moduleContext.moduleInstance->defaultTable));
			emitRuntimeIntrinsic(
				"wavmIntrinsics.launchThread",
				FunctionType::get(ResultType::none,{ValueType::i32,ValueType::i32,ValueType::i32,ValueType::i64}),
				{functionIndex,argument,errorFunctionIndex,defaultTableAsI64});
		}
		
		void trapIfMisalignedAtomic(llvm::Value* address,U32 naturalAlignmentLog2)
		{
			if(naturalAlignmentLog2 > 0)
			{
				emitConditionalTrapIntrinsic(
					irBuilder.CreateICmpNE(
						typedZeroConstants[(Uptr)ValueType::i32],
						irBuilder.CreateAnd(address,emitLiteral((U32(1) << naturalAlignmentLog2) - 1))),
					"wavmIntrinsics.misalignedAtomicTrap",
					FunctionType::get(ResultType::none,{ValueType::i32}),
					{address});
			}
		}

		EMIT_UNARY_OP(i32,extend_s_i8,irBuilder.CreateSExt(irBuilder.CreateTrunc(operand,llvmI8Type),llvmI32Type))
		EMIT_UNARY_OP(i32,extend_s_i16,irBuilder.CreateSExt(irBuilder.CreateTrunc(operand,llvmI16Type),llvmI32Type))
		EMIT_UNARY_OP(i64,extend_s_i8,irBuilder.CreateSExt(irBuilder.CreateTrunc(operand,llvmI8Type),llvmI64Type))
		EMIT_UNARY_OP(i64,extend_s_i16,irBuilder.CreateSExt(irBuilder.CreateTrunc(operand,llvmI16Type),llvmI64Type))

		#define EMIT_ATOMIC_LOAD_OP(valueTypeId,name,llvmMemoryType,naturalAlignmentLog2,conversionOp) \
			void valueTypeId##_##name(AtomicLoadOrStoreImm<naturalAlignmentLog2> imm) \
			{ \
				auto byteIndex = pop(); \
				trapIfMisalignedAtomic(byteIndex,naturalAlignmentLog2); \
				auto pointer = coerceByteIndexToPointer(byteIndex,imm.offset,llvmMemoryType); \
				auto load = irBuilder.CreateLoad(pointer); \
				load->setAlignment(1<<imm.alignmentLog2); \
				load->setVolatile(true); \
				load->setAtomic(llvm::AtomicOrdering::SequentiallyConsistent); \
				push(conversionOp(load,asLLVMType(ValueType::valueTypeId))); \
			}
		#define EMIT_ATOMIC_STORE_OP(valueTypeId,name,llvmMemoryType,naturalAlignmentLog2,conversionOp) \
			void valueTypeId##_##name(AtomicLoadOrStoreImm<naturalAlignmentLog2> imm) \
			{ \
				auto value = pop(); \
				auto byteIndex = pop(); \
				trapIfMisalignedAtomic(byteIndex,naturalAlignmentLog2); \
				auto pointer = coerceByteIndexToPointer(byteIndex,imm.offset,llvmMemoryType); \
				auto memoryValue = conversionOp(value,llvmMemoryType); \
				auto store = irBuilder.CreateStore(memoryValue,pointer); \
				store->setVolatile(true); \
				store->setAlignment(1<<imm.alignmentLog2); \
				store->setAtomic(llvm::AtomicOrdering::SequentiallyConsistent); \
			}
		EMIT_ATOMIC_LOAD_OP(i32,atomic_load,llvmI32Type,2,identityConversion)
		EMIT_ATOMIC_LOAD_OP(i64,atomic_load,llvmI64Type,3,identityConversion)
		EMIT_ATOMIC_LOAD_OP(f32,atomic_load,llvmF32Type,2,identityConversion)
		EMIT_ATOMIC_LOAD_OP(f64,atomic_load,llvmF64Type,3,identityConversion)

		EMIT_ATOMIC_LOAD_OP(i32,atomic_load8_s,llvmI8Type,0,irBuilder.CreateSExt)
		EMIT_ATOMIC_LOAD_OP(i32,atomic_load8_u,llvmI8Type,0,irBuilder.CreateZExt)
		EMIT_ATOMIC_LOAD_OP(i32,atomic_load16_s,llvmI16Type,1,irBuilder.CreateSExt)
		EMIT_ATOMIC_LOAD_OP(i32,atomic_load16_u,llvmI16Type,1,irBuilder.CreateZExt)
		EMIT_ATOMIC_LOAD_OP(i64,atomic_load8_s,llvmI8Type,0,irBuilder.CreateSExt)
		EMIT_ATOMIC_LOAD_OP(i64,atomic_load8_u,llvmI8Type,0,irBuilder.CreateZExt)
		EMIT_ATOMIC_LOAD_OP(i64,atomic_load16_s,llvmI16Type,1,irBuilder.CreateSExt)
		EMIT_ATOMIC_LOAD_OP(i64,atomic_load16_u,llvmI16Type,1,irBuilder.CreateZExt)
		EMIT_ATOMIC_LOAD_OP(i64,atomic_load32_s,llvmI32Type,2,irBuilder.CreateSExt)
		EMIT_ATOMIC_LOAD_OP(i64,atomic_load32_u,llvmI32Type,2,irBuilder.CreateZExt)

		EMIT_ATOMIC_STORE_OP(i32,atomic_store,llvmI32Type,2,identityConversion)
		EMIT_ATOMIC_STORE_OP(i64,atomic_store,llvmI64Type,3,identityConversion)
		EMIT_ATOMIC_STORE_OP(f32,atomic_store,llvmF32Type,2,identityConversion)
		EMIT_ATOMIC_STORE_OP(f64,atomic_store,llvmF64Type,3,identityConversion)
			
		EMIT_ATOMIC_STORE_OP(i32,atomic_store8,llvmI8Type,0,irBuilder.CreateTrunc)
		EMIT_ATOMIC_STORE_OP(i32,atomic_store16,llvmI16Type,1,irBuilder.CreateTrunc)
		EMIT_ATOMIC_STORE_OP(i64,atomic_store8,llvmI8Type,0,irBuilder.CreateTrunc)
		EMIT_ATOMIC_STORE_OP(i64,atomic_store16,llvmI16Type,1,irBuilder.CreateTrunc)
		EMIT_ATOMIC_STORE_OP(i64,atomic_store32,llvmI32Type,2,irBuilder.CreateTrunc)

		#define EMIT_ATOMIC_CMPXCHG(valueTypeId,name,llvmMemoryType,naturalAlignmentLog2,memoryToValueConversion,valueToMemoryConversion) \
			void valueTypeId##_##name(AtomicLoadOrStoreImm<naturalAlignmentLog2> imm) \
			{ \
				auto replacementValue = valueToMemoryConversion(pop(),llvmMemoryType); \
				auto expectedValue = valueToMemoryConversion(pop(),llvmMemoryType); \
				auto byteIndex = pop(); \
				trapIfMisalignedAtomic(byteIndex,naturalAlignmentLog2); \
				auto pointer = coerceByteIndexToPointer(byteIndex,imm.offset,llvmMemoryType); \
				auto atomicCmpXchg = irBuilder.CreateAtomicCmpXchg( \
					pointer, \
					expectedValue, \
					replacementValue, \
					llvm::AtomicOrdering::SequentiallyConsistent, \
					llvm::AtomicOrdering::SequentiallyConsistent); \
				atomicCmpXchg->setVolatile(true); \
				auto previousValue = irBuilder.CreateExtractValue(atomicCmpXchg,{0}); \
				push(memoryToValueConversion(previousValue,asLLVMType(ValueType::valueTypeId))); \
			}

		EMIT_ATOMIC_CMPXCHG(i32,atomic_rmw8_u_cmpxchg,llvmI8Type,0,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_CMPXCHG(i32,atomic_rmw16_u_cmpxchg,llvmI16Type,1,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_CMPXCHG(i32,atomic_rmw_cmpxchg,llvmI32Type,2,identityConversion,identityConversion)
			
		EMIT_ATOMIC_CMPXCHG(i64,atomic_rmw8_u_cmpxchg,llvmI8Type,0,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_CMPXCHG(i64,atomic_rmw16_u_cmpxchg,llvmI16Type,1,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_CMPXCHG(i64,atomic_rmw32_u_cmpxchg,llvmI32Type,2,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_CMPXCHG(i64,atomic_rmw_cmpxchg,llvmI64Type,3,identityConversion,identityConversion)
			
		#define EMIT_ATOMIC_RMW(valueTypeId,name,rmwOpId,llvmMemoryType,naturalAlignmentLog2,memoryToValueConversion,valueToMemoryConversion) \
			void valueTypeId##_##name(AtomicLoadOrStoreImm<naturalAlignmentLog2> imm) \
			{ \
				auto value = valueToMemoryConversion(pop(),llvmMemoryType); \
				auto byteIndex = pop(); \
				trapIfMisalignedAtomic(byteIndex,naturalAlignmentLog2); \
				auto pointer = coerceByteIndexToPointer(byteIndex,imm.offset,llvmMemoryType); \
				auto atomicRMW = irBuilder.CreateAtomicRMW( \
					llvm::AtomicRMWInst::BinOp::rmwOpId, \
					pointer, \
					value, \
					llvm::AtomicOrdering::SequentiallyConsistent); \
				atomicRMW->setVolatile(true); \
				push(memoryToValueConversion(atomicRMW,asLLVMType(ValueType::valueTypeId))); \
			}

		EMIT_ATOMIC_RMW(i32,atomic_rmw8_u_xchg,Xchg,llvmI8Type,0,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i32,atomic_rmw16_u_xchg,Xchg,llvmI16Type,1,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i32,atomic_rmw_xchg,Xchg,llvmI32Type,2,identityConversion,identityConversion)

		EMIT_ATOMIC_RMW(i64,atomic_rmw8_u_xchg,Xchg,llvmI8Type,0,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i64,atomic_rmw16_u_xchg,Xchg,llvmI16Type,1,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i64,atomic_rmw32_u_xchg,Xchg,llvmI32Type,2,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i64,atomic_rmw_xchg,Xchg,llvmI64Type,3,identityConversion,identityConversion)

		EMIT_ATOMIC_RMW(i32,atomic_rmw8_u_add,Add,llvmI8Type,0,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i32,atomic_rmw16_u_add,Add,llvmI16Type,1,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i32,atomic_rmw_add,Add,llvmI32Type,2,identityConversion,identityConversion)

		EMIT_ATOMIC_RMW(i64,atomic_rmw8_u_add,Add,llvmI8Type,0,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i64,atomic_rmw16_u_add,Add,llvmI16Type,1,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i64,atomic_rmw32_u_add,Add,llvmI32Type,2,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i64,atomic_rmw_add,Add,llvmI64Type,3,identityConversion,identityConversion)
			
		EMIT_ATOMIC_RMW(i32,atomic_rmw8_u_sub,Sub,llvmI8Type,0,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i32,atomic_rmw16_u_sub,Sub,llvmI16Type,1,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i32,atomic_rmw_sub,Sub,llvmI32Type,2,identityConversion,identityConversion)

		EMIT_ATOMIC_RMW(i64,atomic_rmw8_u_sub,Sub,llvmI8Type,0,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i64,atomic_rmw16_u_sub,Sub,llvmI16Type,1,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i64,atomic_rmw32_u_sub,Sub,llvmI32Type,2,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i64,atomic_rmw_sub,Sub,llvmI64Type,3,identityConversion,identityConversion)
			
		EMIT_ATOMIC_RMW(i32,atomic_rmw8_u_and,And,llvmI8Type,0,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i32,atomic_rmw16_u_and,And,llvmI16Type,1,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i32,atomic_rmw_and,And,llvmI32Type,2,identityConversion,identityConversion)

		EMIT_ATOMIC_RMW(i64,atomic_rmw8_u_and,And,llvmI8Type,0,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i64,atomic_rmw16_u_and,And,llvmI16Type,1,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i64,atomic_rmw32_u_and,And,llvmI32Type,2,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i64,atomic_rmw_and,And,llvmI64Type,3,identityConversion,identityConversion)
			
		EMIT_ATOMIC_RMW(i32,atomic_rmw8_u_or,Or,llvmI8Type,0,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i32,atomic_rmw16_u_or,Or,llvmI16Type,1,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i32,atomic_rmw_or,Or,llvmI32Type,2,identityConversion,identityConversion)

		EMIT_ATOMIC_RMW(i64,atomic_rmw8_u_or,Or,llvmI8Type,0,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i64,atomic_rmw16_u_or,Or,llvmI16Type,1,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i64,atomic_rmw32_u_or,Or,llvmI32Type,2,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i64,atomic_rmw_or,Or,llvmI64Type,3,identityConversion,identityConversion)
			
		EMIT_ATOMIC_RMW(i32,atomic_rmw8_u_xor,Xor,llvmI8Type,0,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i32,atomic_rmw16_u_xor,Xor,llvmI16Type,1,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i32,atomic_rmw_xor,Xor,llvmI32Type,2,identityConversion,identityConversion)

		EMIT_ATOMIC_RMW(i64,atomic_rmw8_u_xor,Xor,llvmI8Type,0,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i64,atomic_rmw16_u_xor,Xor,llvmI16Type,1,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i64,atomic_rmw32_u_xor,Xor,llvmI32Type,2,irBuilder.CreateZExt,irBuilder.CreateTrunc)
		EMIT_ATOMIC_RMW(i64,atomic_rmw_xor,Xor,llvmI64Type,3,identityConversion,identityConversion)
		#endif
	};
	
//一个“不做任何事情”访问者，用于解码过去无法访问的运算符（但支持日志记录和传递最终运算符）。
	struct UnreachableOpVisitor
	{
		typedef void Result;

		UnreachableOpVisitor(EmitFunctionContext& inContext): context(inContext), unreachableControlDepth(0) {}
		#define VISIT_OP(opcode,name,nameString,Imm,...) void name(Imm imm) {}
		ENUM_NONCONTROL_OPERATORS(VISIT_OP)
		VISIT_OP(_,unknown,"unknown",Opcode)
		#undef VISIT_OP

//跟踪不可访问代码中的控制结构嵌套级别，这样我们就知道何时到达不可访问代码的末尾。
		void block(ControlStructureImm) { ++unreachableControlDepth; }
		void loop(ControlStructureImm) { ++unreachableControlDepth; }
		void if_(ControlStructureImm) { ++unreachableControlDepth; }

//如果一个else或end操作码向无法到达的代码发出结束信号，然后将其传递给红外发射器。
		void else_(NoImm imm)
		{
			if(!unreachableControlDepth) { context.else_(imm); }
		}
		void end(NoImm imm)
		{
			if(!unreachableControlDepth) { context.end(imm); }
			else { --unreachableControlDepth; }
		}

	private:
		EmitFunctionContext& context;
		Uptr unreachableControlDepth;
	};

	void EmitFunctionContext::emit()
	{
//为函数创建调试信息。
		llvm::SmallVector<llvm::Metadata*,10> diFunctionParameterTypes;
		for(auto parameterType : functionType->parameters) { diFunctionParameterTypes.push_back(moduleContext.diValueTypes[(Uptr)parameterType]); }
		auto diFunctionType = moduleContext.diBuilder.createSubroutineType(moduleContext.diBuilder.getOrCreateTypeArray(diFunctionParameterTypes));
		diFunction = moduleContext.diBuilder.createFunction(
			moduleContext.diModuleScope,
			functionInstance->debugName,
			llvmFunction->getName(),
			moduleContext.diModuleScope,
			0,
			diFunctionType,
			false,
			true,
			0);
		llvmFunction->setSubprogram(diFunction);

//创建返回基本块，并推送函数的根控件上下文。
		auto returnBlock = llvm::BasicBlock::Create(context,"return",llvmFunction);
		auto returnPHI = createPHI(returnBlock,functionType->ret);
		pushControlStack(ControlContext::Type::function,functionType->ret,returnBlock,returnPHI);
		pushBranchTarget(functionType->ret,returnBlock,returnPHI);

//为函数创建初始基本块。
		auto entryBasicBlock = llvm::BasicBlock::Create(context,"entry",llvmFunction);
		irBuilder.SetInsertPoint(entryBasicBlock);

//如果启用，发出对wavm函数enter hook的调用（用于调试）。
		if(ENABLE_FUNCTION_ENTER_EXIT_HOOKS)
		{
			emitRuntimeIntrinsic(
				"wavmIntrinsics.debugEnterFunction",
				FunctionType::get(ResultType::none,{ValueType::i64}),
				{emitLiteral(reinterpret_cast<U64>(functionInstance))}
				);
		}

//为所有局部变量和参数创建并初始化allocas。
		auto llvmArgIt = llvmFunction->arg_begin();
		for(Uptr localIndex = 0;localIndex < functionType->parameters.size() + functionDef.nonParameterLocalTypes.size();++localIndex)
		{
			auto localType = localIndex < functionType->parameters.size()
				? functionType->parameters[localIndex]
				: functionDef.nonParameterLocalTypes[localIndex - functionType->parameters.size()];
			auto localPointer = irBuilder.CreateAlloca(asLLVMType(localType),nullptr,"");
			localPointers.push_back(localPointer);

			if(localIndex < functionType->parameters.size())
			{
//将参数值复制到存储它的本地。
				irBuilder.CreateStore((llvm::Argument*)&(*llvmArgIt),localPointer);
				++llvmArgIt;
			}
			else
			{
//将非参数局部变量初始化为零。
				irBuilder.CreateStore(typedZeroConstants[(Uptr)localType],localPointer);
			}
		}

//解码Webassembly操作码并为其发出llvm-ir。
		OperatorDecoderStream decoder(functionDef.code);
		UnreachableOpVisitor unreachableOpVisitor(*this);
		OperatorPrinter operatorPrinter(module,functionDef);
		Uptr opIndex = 0;
		while(decoder && controlStack.size())
		{
			irBuilder.SetCurrentDebugLocation(llvm::DILocation::get(context,(unsigned int)opIndex++,0,diFunction));
			if(ENABLE_LOGGING)
			{
				logOperator(decoder.decodeOpWithoutConsume(operatorPrinter));
			}

			if(controlStack.back().isReachable) { decoder.decodeOp(*this); }
			else { decoder.decodeOp(unreachableOpVisitor); }
		};
		WAVM_ASSERT_THROW(irBuilder.GetInsertBlock() == returnBlock);
		
//如果启用，发出对wavm函数enter hook的调用（用于调试）。
		if(ENABLE_FUNCTION_ENTER_EXIT_HOOKS)
		{
			emitRuntimeIntrinsic(
				"wavmIntrinsics.debugExitFunction",
				FunctionType::get(ResultType::none,{ValueType::i64}),
				{emitLiteral(reinterpret_cast<U64>(functionInstance))}
				);
		}

//发出函数返回。
		if(functionType->ret == ResultType::none) { irBuilder.CreateRetVoid(); }
		else { irBuilder.CreateRet(pop()); }
	}

	llvm::Module* EmitModuleContext::emit()
	{
		Timing::Timer emitTimer;

//为默认内存基和掩码创建文本。
		if(moduleInstance->defaultMemory)
		{
			defaultMemoryBase = emitLiteralPointer(moduleInstance->defaultMemory->baseAddress,llvmI8PtrType);
			const Uptr defaultMemoryEndOffsetValue = Uptr(moduleInstance->defaultMemory->endOffset);
			defaultMemoryEndOffset = emitLiteral(defaultMemoryEndOffsetValue);
		}
		else { defaultMemoryBase = defaultMemoryEndOffset = nullptr; }

//设置用于访问全局表的llvm值。
		if(moduleInstance->defaultTable)
		{
			auto tableElementType = llvm::StructType::get(context,{
				llvmI8PtrType,
				llvmI8PtrType
				});
			defaultTablePointer = emitLiteralPointer(moduleInstance->defaultTable->baseAddress,tableElementType->getPointerTo());
			defaultTableMaxElementIndex = emitLiteral(((Uptr)moduleInstance->defaultTable->endOffset)/sizeof(TableInstance::FunctionElement));
		}
		else
		{
			defaultTablePointer = defaultTableMaxElementIndex = nullptr;
		}

//为模块的导入函数创建llvm指针常量。
		for(Uptr functionIndex = 0;functionIndex < module.functions.imports.size();++functionIndex)
		{
			const FunctionInstance* functionInstance = moduleInstance->functions[functionIndex];
			importedFunctionPointers.push_back(emitLiteralPointer(functionInstance->nativeFunction,asLLVMType(functionInstance->type)->getPointerTo()));
		}

//为模块的全局创建llvm指针常量。
		for(auto global : moduleInstance->globals)
		{ globalPointers.push_back(emitLiteralPointer(&global->value,asLLVMType(global->type.valueType)->getPointerTo())); }
		
//创建LLVM函数。
		functionDefs.resize(module.functions.defs.size());
		for(Uptr functionDefIndex = 0;functionDefIndex < module.functions.defs.size();++functionDefIndex)
		{
			auto llvmFunctionType = asLLVMType(module.types[module.functions.defs[functionDefIndex].type.index]);
			auto externalName = getExternalFunctionName(moduleInstance,functionDefIndex);
			functionDefs[functionDefIndex] = llvm::Function::Create(llvmFunctionType,llvm::Function::ExternalLinkage,externalName,llvmModule);
		}

//编译模块中的每个函数。
		for(Uptr functionDefIndex = 0;functionDefIndex < module.functions.defs.size();++functionDefIndex)
		{ EmitFunctionContext(*this,module,module.functions.defs[functionDefIndex],moduleInstance->functionDefs[functionDefIndex],functionDefs[functionDefIndex]).emit(); }
		
//完成调试信息。
		diBuilder.finalize();

		Timing::logRatePerSecond("Emitted LLVM IR",emitTimer,(F64)llvmModule->size(),"functions");

		return llvmModule;
	}

	llvm::Module* emitModule(const Module& module,ModuleInstance* moduleInstance)
	{
		return EmitModuleContext(module,moduleInstance).emit();
	}
}
