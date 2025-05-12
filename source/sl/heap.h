#pragma once

#include <cstddef>

#include "thread.h"
#include "val.h"

namespace SL {
	struct Heap {
		Object *createObject(size_t size, Type type);
		
		void init() { }
		void deinit() { }
	};
}
