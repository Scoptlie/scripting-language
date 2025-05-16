#include "func.h"

#include "heap.h"

namespace SL {
	Func *Func::create(Heap *heap) {
		return (Func*)heap->createObject(sizeof(Func), objectTypeFunc);
	}
}
