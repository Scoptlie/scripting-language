
#include <cstring>

#include "sl/heap.h"
#include "sl/thread.h"

int main(int argc, char **argv) {
	using namespace SL;
	
	Heap heap;
	heap.init();
	
	auto factorial = Func::create(&heap);
	factorial->nConsts = 2;
	factorial->consts = new Val[] {
		Val::newNumber(1.0),
		Val::newFunc(factorial),
	};
	factorial->nOps = 14;
	factorial->ops = new Op[] {
		Op{opcodePush, -1},
		Op{opcodePushc, 0},
		Op{opcodeCmpLtEq},
		Op{opcodeJmpN, 6},
			Op{opcodePushc, 0},
			Op{opcodeRet},
		Op{opcodePushc, 1},
		Op{opcodePush, -1},
		Op{opcodePushc, 0},
		Op{opcodeSub},
		Op{opcodeCall, 1},
		Op{opcodePush, -1},
		Op{opcodeMul},
		Op{opcodeRet},
	};
	factorial->nParams = 1;
	factorial->nVars = 0;
	
	auto func = Func::create(&heap);
	func->nConsts = 3;
	func->consts = new Val[] {
		Val::newFunc(factorial),
		Val::newNumber(12.0),
		Val::newNil()
	};
	func->nOps = 6;
	func->ops = new Op[] {
		Op{opcodePushc, 0},
		Op{opcodePushc, 1},
		Op{opcodeCall, 1},
		Op{opcodePrint},
		Op{opcodePushc, 2},
		Op{opcodeRet},
	};
	func->nParams = 0;
	func->nVars = 0;
	
	auto thread = Thread::create(&heap);
	
	Val result;
	thread->call(func, 0, nullptr, &result);
	
	heap.deinit();
}
