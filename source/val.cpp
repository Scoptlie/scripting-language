#include "val.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "vm.h"

String *String::create(size_t len) {
	auto r = (String*)new uintptr_t[(offsetof(String, chars) + len + 8) / 8];
	r->len = len;
	r->hash = 0;
	r->chars[len] = 0;
	
	return r;
}

String *String::create(size_t len, char const *chars) {
	auto r = create(len);
	memcpy(r->chars, chars, len);
	
	return r;
}

String *String::create(char const *chars) {
	return create(strlen(chars), chars);
}

String *String::createFromNumber(double val) {
	static char chars[32];
	auto len = snprintf(chars, 32, "%.14g", val);
	return String::create(len, chars);
}

String *String::createFromPtr(char const *tag, void const *p) {
	static char chars[32];
	auto len = snprintf(chars, 32, "%s@%p", tag, p);
	return String::create(len, chars);
}

String *String::createFromVal(Val val) {
	if (val.isNil()) {
		return String::create(3, "nil");
	} else if (val.isNumber()) {
		return String::createFromNumber(val.numberVal);
	} else if (val.isString()) {
		return val.stringVal;
	} else if (val.isFunc()) {
		return String::createFromPtr("func", val.funcVal);
	}
	return String::create("");
}

String *String::concat(String const *other) const {
	auto r = create(len + other->len);
	memcpy(r->chars, chars, len);
	memcpy(r->chars + len, other->chars, other->len);
	
	return r;
}

void Func::destroy() {
	delete[] consts;
	delete[] ops;
}

bool Val::equals(Val other) const {
	if (type == other.type) {
		if (type == typeNil) {
			return true;
		} else if (type == typeNumber) {
			return numberVal == other.numberVal;
		} else if (type == typeString) {
			return stringVal == other.stringVal || (
				stringVal->len == other.stringVal->len &&
				stringVal->hash == other.stringVal->hash &&
				memcmp(stringVal->chars, other.stringVal->chars, stringVal->len) == 0
			);
		} else if (type == typeFunc) {
			return funcVal == other.funcVal;
		}
	}
	return false;
}
