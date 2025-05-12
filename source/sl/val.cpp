#include "val.h"

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
				return stringVal == other.stringVal || (
					stringVal->nChars == other.stringVal->nChars &&
					memcmp(stringVal->chars, other.stringVal->chars, stringVal->nChars) == 0
				);
			} else {
				return ptrVal == other.ptrVal;
			}
		} else {
			return false;
		}
	}
	
	String *String::create(Heap *heap, size_t nChars) {
		auto r = (String*)heap->createObject(
			sizeof(String) + nChars,
			typeString
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
	
	Func *Func::create(Heap *heap) {
		return (Func*)heap->createObject(sizeof(Func), typeFunc);
	}
}
