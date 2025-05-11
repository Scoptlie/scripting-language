#pragma once

#include <cstdint>

#include "val.h"

enum Opcode {
	opcodePushc,
	opcodePush,
	opcodePop,
	
	opcodeNeg,
	opcodeAdd,
	opcodeSub,
	opcodeMul,
	opcodeDiv,
	opcodeMod,
	
	opcodeCmpEq,
	opcodeCmpNEq,
	opcodeCmpLt,
	opcodeCmpGt,
	opcodeCmpLtEq,
	opcodeCmpGtEq,
	
	opcodeNotL,
	opcodeAndL,
	opcodeOrL,
	
	opcodeJmp,
	opcodeJmpN,
	
	opcodeCall,
	opcodeRet,
	
	opcodePrint,
};

struct Op {
	int32_t opcode: 8, n: 24;
};

struct Vm {
	Val call(Func *func, size_t nArgs, Val *args);
	
	void create();
	void destroy();
	
private:
	size_t stackBufLen, stackLen;
	Val *stack;
	
	void push(Val val);
	Val pop();
	
	void call(size_t nArgs);
	
};
