#pragma once

#include <cstddef>

struct Val;

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
	typeFunc,
};

struct Val {
	Type type;
	union {
		double numberVal;
		Func *funcVal;
	};
	
	bool isNil() const {
		return type == typeNil;
	}
	
	bool isNumber() const {
		return type == typeNumber;
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
	
	bool equals(Val other) const {
		if (type == other.type) {
			return isNil() ||
				(isNumber() && numberVal == other.numberVal) ||
				(isFunc() && funcVal == other.funcVal);
		}
		return false;
	}
	
	static Val newNil() {
		return Val{.type = typeNil};
	}
	
	static Val newNumber(double val) {
		return Val{.type = typeNumber, .numberVal = val};
	}
	
	static Val newFunc(Func *val) {
		return Val{.type = typeFunc, .funcVal = val};
	}
	
	static Val fromBool(bool val) {
		return Val::newNumber(val? 1.0 : 0.0);
	}
};
