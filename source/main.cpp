
#include <cstring>

#include "sl/heap.h"
#include "sl/thread.h"

int main(int argc, char **argv) {
	using namespace SL;
	
	Heap heap;
	heap.init();
	
	auto func = Func::create(&heap);
	func->nConsts = 4;
	func->consts = new Val[] {
		Val::newNumber(0.0),
		Val::newNumber(1.0),
		Val::newNumber(10.0),
		Val::newNil()
	};
	func->nOps = 13;
	func->ops = new Op[] {
		Op{opcodePushc, 0},
		Op{opcodePop, 0},
			Op{opcodePush, 0},
			Op{opcodePrint},
			Op{opcodePush, 0},
			Op{opcodePushc, 1},
			Op{opcodeAdd},
			Op{opcodePop, 0},
			Op{opcodePush, 0},
			Op{opcodePushc, 2},
			Op{opcodeCmpGtEq},
			Op{opcodeJmpN, 2},
		Op{opcodePushc, 3},
		Op{opcodeRet}
	};
	func->nParams = 0;
	func->nVars = 1;
	
	auto thread = Thread::create(&heap);
	
	Val result;
	thread->call(func, 0, nullptr, &result);
	
	heap.deinit();
}
