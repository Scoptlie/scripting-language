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
	size_t stackBufLen, stackLen;
	Val *stack;
	
	void push(Val val);
	Val pop();
	
	void neg();
	void add();
	void sub();
	void mul();
	void div();
	void mod();
	
	void cmpEq();
	void cmpNEq();
	void cmpLt();
	void cmpGt();
	void cmpLtEq();
	void cmpGtEq();
	
	void notL();
	void andL();
	void orL();
	
	void call(size_t nArgs);
	
	void create();
	void destroy();
	
};
