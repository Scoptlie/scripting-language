#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>

#include "heap.h"

namespace SL {
	enum Type {
		typeNil,
		typeNumber,
		typeString,
		typeArray,
		typeStruct,
		typeFunc,
		typeThread,
	};
	
	struct String;
	struct Array;
	struct Struct;
	struct Func;
	struct Thread;
	
	struct Val {
		Type type;
		union {
			double numberVal;
			void *ptrVal;
			String *stringVal;
			Array *arrayVal;
			Struct *structVal;
			Func *funcVal;
			Thread *threadVal;
		};
		
		bool isNil() const {
			return type == typeNil;
		}
		
		bool isNumber() const {
			return type == typeNumber;
		}
		
		bool isString() const {
			return type == typeString;
		}
		
		bool isArray() const {
			return type == typeArray;
		}
		
		bool isStruct() const {
			return type == typeStruct;
		}
		
		bool isFunc() const {
			return type == typeFunc;
		}
		
		bool isThread() const {
			return type == typeThread;
		}
		
		bool equals(Val other) const;
		
		bool asBool() const {
			if (type == typeNil) {
				return false;
			} else if (type == typeNumber) {
				return numberVal != 0.0;
			} else {
				return true;
			}
		}
		
		static Val newNil() {
			return Val{.type = typeNil};
		}
		
		static Val newNumber(double val) {
			return Val{.type = typeNumber, .numberVal = val};
		}
		
		static Val newString(String *val) {
			return Val{.type = typeString, .stringVal = val};
		}
		
		static Val newArray(Array *val) {
			return Val{.type = typeArray, .arrayVal = val};
		}
		
		static Val newStruct(Struct *val) {
			return Val{.type = typeStruct, .structVal = val};
		}
		
		static Val newFunc(Func *val) {
			return Val{.type = typeFunc, .funcVal = val};
		}
		
		static Val newThread(Thread *val) {
			return Val{.type = typeThread, .threadVal = val};
		}
		
		static Val fromBool(bool val) {
			return Val::newNumber(val? 1.0 : 0.0);
		}
	};
	
	struct String : public Object {
		size_t nChars;
		char chars[1];
		
		uint32_t hash();
		
		bool isEqual(String *other);
		
		static String *create(Heap *heap, size_t nChars);
		static String *create(Heap *heap, size_t nChars, char const *chars);
		static String *createFromVal(Heap *heap, Val val);
	};
	
	
}
