#pragma once

#include <cstddef>
#include <cstdint>

#include "heap.h"

namespace SL {
	enum Opcode : uint8_t {
		opcodeGetConst,
		opcodeGetVar,
		opcodeSetVar,
		opcodeGetElem,
		opcodeSetElem,
		
		opcodeEat,
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
		
		opcodeMakeArray,
		opcodeMakeStruct,
		
		opcodePrint,
		
		opcodeJmp,
		opcodeJmpN,
		
		opcodeCall,
		opcodeRet,
	};
	
	struct Op {
		int32_t opcode: 8, arg: 24;
	};
	
	struct Val;
	
	struct Func : public Object {
		size_t nConsts;
		Val *consts;
		
		size_t nOps;
		Op *ops;
		
		size_t nParams, nLocals;
		
		static Func *create(Heap *heap);
	};
}
