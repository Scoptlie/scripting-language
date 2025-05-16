#include "array.h"

#include "val.h"

namespace SL {
	Array *Array::create(Heap *heap, size_t nElems) {
		auto r = (Array*)heap->createObject(sizeof(Array), objectTypeArray);
		r->bufLen = nElems;
		r->nElems = nElems;
		r->elems = new Val[nElems];
		
		return r;
	}
}
