#pragma once

#include <cstddef>

namespace SL {
	enum ObjectType {
		objectTypeString,
		objectTypeArray,
		//objectTypeStruct,
		objectTypeFunc,
		objectTypeThread,
	};
	
	struct Object {
		ObjectType type;
	};
	
	struct Heap {
		Object *createObject(size_t size, ObjectType type);
		
		void init() { }
		void deinit() { }
	};
}
