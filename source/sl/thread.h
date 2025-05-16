#pragma once

#include "darray.h"
#include "func.h"
#include "heap.h"
#include "val.h"

namespace SL {
	struct Call {
		Func *func;
		Val inst;
		Op *opIt;
		size_t nInps, nArgs;
		size_t baseStackIdx;
	};
	
	struct Val;
	
	struct Thread : public Object {
		Heap *heap;
		Val global;
		
		DArray<Val> stack;
		DArray<Call> callStack;
		
		bool call(Func *func, Val inst, size_t nArgs, Val const *args, Val *oResult);
		
		static Thread *create(Heap *heap, Val global);
		void deinit();
		
	private:
		Val getElem(Val base, Val subscript);
		void setElem(Val base, Val subscript, Val val);
		
		void call(Func *func, Val inst, size_t nInps, size_t nArgs);
		
		bool runUntilReturnToHost(Val *oResult);
		
	};
}
