#include "heap.h"

#include <cassert>

namespace SL {
	Object *Heap::createObject(size_t size, ObjectType type) {
		assert(size >= sizeof(Object));
		auto r = (Object*)::operator new(size);
		r->type = type;
		return r;
	}
}
