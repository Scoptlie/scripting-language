#pragma once

#include "heap.h"

namespace SL {
	struct Val;
	
	struct Array : public Object {
		size_t bufLen, nElems;
		Val *elems;
		
		static Array *create(Heap *heap, size_t nElems);
	};
}
