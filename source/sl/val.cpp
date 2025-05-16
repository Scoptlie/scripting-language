#include "val.h"

#include <cassert>
#include <cstdio>
#include <cstring>

#include "heap.h"

namespace SL {
	bool Val::equals(Val other) const {
		if (type == other.type) {
			if (type == typeNil) {
				return true;
			} else if (type == typeNumber) {
				return numberVal == other.numberVal;
			} else if (type == typeString) {
				return stringVal->isEqual(other.stringVal);
			} else {
				return ptrVal == other.ptrVal;
			}
		} else {
			return false;
		}
	}
	
	uint32_t String::hash() {
		auto r = uint32_t(nChars);
		for (auto i = size_t(0); i < nChars; i++) {
			r ^= ((r << 5) + (r >> 2) + uint32_t(chars[i]));
		}
		return r * 2654435769u;
	}
	
	bool String::isEqual(String *other) {
		if (this == other) {
			return true;
		}
		
		if (nChars != other->nChars) {
			return false;
		}
		
		return memcmp(chars, other->chars, nChars) == 0;
	}
	
	String *String::create(Heap *heap, size_t nChars) {
		auto r = (String*)heap->createObject(
			sizeof(String) + nChars,
			objectTypeString
		);
		r->nChars = nChars;
		r->chars[nChars] = 0;
		
		return r;
	}
	
	String *String::create(Heap *heap, size_t nChars, char const *chars) {
		auto r = create(heap, nChars);
		memcpy(r->chars, chars, nChars);
		
		return r;
	}
	
	String *String::createFromVal(Heap *heap, Val val) {
		thread_local static char buf[32];
		
		if (val.isNil()) {
			return create(heap, 3, "nil");
		} else if (val.isNumber()) {
			auto len = snprintf(buf, sizeof(buf), "%.14g", val.numberVal);
			return create(heap, (len >= 0)? len : 0, buf);
		} else if (val.isString()) {
			return val.stringVal;
		} else if (val.isArray()) {
			auto len = snprintf(buf, sizeof(buf), "array@%p", val.arrayVal);
			return create(heap, (len >= 0)? len : 0, buf);
		} else if (val.isStruct()) {
			auto len = snprintf(buf, sizeof(buf), "struct@%p", val.structVal);
			return create(heap, (len >= 0)? len : 0, buf);
		} else if (val.isFunc()) {
			auto len = snprintf(buf, sizeof(buf), "func@%p", val.funcVal);
			return create(heap, (len >= 0)? len : 0, buf);
		} else if (val.isThread()) {
			auto len = snprintf(buf, sizeof(buf), "thread@%p", val.threadVal);
			return create(heap, (len >= 0)? len : 0, buf);
		}
		
		assert(!"unreachable");
		return nullptr;
	}
}
