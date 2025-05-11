#pragma once

#include <cstddef>

struct Val;

struct String {
	size_t len, hash;
	char chars[];
	
	static String *create(size_t len);
	static String *create(size_t len, char const *chars);
	static String *create(char const *chars);
	
	static String *createFromNumber(double val);
	static String *createFromPtr(char const *tag, void const *p);
	static String *createFromVal(Val val);
	
	String *concat(String const *other) const;
	
};

struct Op;

struct Func {
	size_t nConsts, nOps,
		nVars, nParams;
	Val *consts;
	Op *ops;
	
	void destroy();
};

enum Type {
	typeNil,
	typeNumber,
	typeString,
	typeFunc,
};

struct Val {
	Type type;
	union {
		double numberVal;
		String *stringVal;
		Func *funcVal;
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
	
	bool isFunc() const {
		return type == typeFunc;
	}
	
	bool asBool() const {
		if (isNil()) {
			return false;
		} else if (isNumber()) {
			return numberVal != 0.0;
		}
		return true;
	}
	
	bool equals(Val other) const;
	
	static Val newNil() {
		return Val{.type = typeNil};
	}
	
	static Val newNumber(double val) {
		return Val{.type = typeNumber, .numberVal = val};
	}
	
	static Val newString(String *val) {
		return Val{.type = typeString, .stringVal = val};
	}
	
	static Val newFunc(Func *val) {
		return Val{.type = typeFunc, .funcVal = val};
	}
	
	static Val fromBool(bool val) {
		return Val::newNumber(val? 1.0 : 0.0);
	}
};
