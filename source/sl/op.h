#pragma once

#include <cstdint>

namespace SL {
	enum Opcode : uint8_t {
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
		
		opcodePrint,
		
		opcodeJmp,
		opcodeJmpN,
		
		opcodeCall,
		opcodeRet,
	};
	
	struct Op {
		int32_t opcode: 8, arg: 24;
	};
}
