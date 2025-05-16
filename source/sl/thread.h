#pragma once

#include "darray.h"
#include "func.h"
#include "heap.h"

namespace SL {
	struct Call {
		Func *func;
		Op *opIt;
		size_t nArgs;
		size_t baseStackIdx;
	};
	
	struct Val;
	
	struct Thread : public Object {
		Heap *heap;
		
		DArray<Val> stack;
		DArray<Call> callStack;
		
		bool call(Func *func, size_t nArgs, Val const *args, Val *oResult);
		
		static Thread *create(Heap *heap);
		void deinit();
		
	private:
		void call(Func *func, size_t nArgs);
		
		bool runUntilReturnToHost(Val *oResult);
		
	};
}
