#include "vm.h"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "val.h"

void Vm::push(Val val) {
	if (stackLen == stackBufLen) {
		stackBufLen *= 2;
		
		auto newStack = new Val[stackBufLen];
		memcpy(newStack, stack, sizeof(Val) * stackLen);
		
		delete[] stack;
		stack = newStack;
	}
	
	stack[stackLen++] = val;
}

Val Vm::pop() {
	assert(stackLen > 0);
	return stack[--stackLen];
}

void Vm::call(size_t nArgs) {
	assert(stackLen >= nArgs + 1);
	
	auto funcVal = stack[stackLen - nArgs - 1];
	if (!funcVal.isFunc()) {
		stackLen -= nArgs - 1;
		push(Val::newNil());
		return;
	}
	
	auto func = funcVal.funcVal;
	
	auto consts = func->consts;
	auto ops = func->ops;
	
	if (nArgs > func->nParams) {
		stackLen -= nArgs - func->nParams;
	} else {
		while (nArgs < func->nParams) {
			nArgs++;
			push(Val::newNil());
		}
	}
	
	auto frameIdx = stackLen;
	
	for (auto i = size_t(0); i < func->nVars; i++) {
		push(Val::newNil());
	}
	
	auto opIt = func->ops;
	for (;;) {
		auto op = *opIt++;
		switch (op.opcode) {
		case opcodePushc: {
			assert(op.n >= 0 && op.n < func->nConsts);
			push(consts[op.n]);
			break;
		}
		case opcodePush: {
			assert(frameIdx + op.n < stackLen);
			push(stack[frameIdx + op.n]);
			break;
		}
		case opcodePop: {
			assert(frameIdx + op.n < stackLen);
			stack[frameIdx + op.n] = pop();
			break;
		}
		case opcodeNeg: {
			auto v = pop();
			if (v.isNumber()) {
				push(Val::newNumber(-v.numberVal));
			} else {
				push(Val::newNil());
			}
			break;
		}
		case opcodeAdd: {
			auto b = pop(), a = pop();
			if (a.isNumber() && b.isNumber()) {
				push(Val::newNumber(a.numberVal + b.numberVal));
			} else if (a.isString() || b.isString()) {
				auto aStr = String::createFromVal(a);
				auto bStr = String::createFromVal(b);
				
				push(Val::newString(aStr->concat(bStr)));
			} else {
				push(Val::newNil());
			}
			break;
		}
		case opcodeSub: {
			auto b = pop(), a = pop();
			if (a.isNumber() && b.isNumber()) {
				push(Val::newNumber(a.numberVal - b.numberVal));
			} else {
				push(Val::newNil());
			}
			break;
		}
		case opcodeMul: {
			auto b = pop(), a = pop();
			if (a.isNumber() && b.isNumber()) {
				push(Val::newNumber(a.numberVal * b.numberVal));
			} else {
				push(Val::newNil());
			}
			break;
		}
		case opcodeDiv: {
			auto b = pop(), a = pop();
			if (a.isNumber() && b.isNumber()) {
				push(Val::newNumber(a.numberVal / b.numberVal));
			} else {
				push(Val::newNil());
			}
			break;
		}
		case opcodeMod: {
			auto b = pop(), a = pop();
			if (a.isNumber() && b.isNumber()) {
				push(Val::newNumber(fmod(a.numberVal, b.numberVal)));
			} else {
				push(Val::newNil());
			}
			break;
		}
		case opcodeCmpEq: {
			auto b = pop(), a = pop();
			push(Val::fromBool(a.equals(b)));
			break;
		}
		case opcodeCmpNEq: {
			auto b = pop(), a = pop();
			push(Val::fromBool(!a.equals(b)));
			break;
		}
		case opcodeCmpLt: {
			auto b = pop(), a = pop();
			if (a.isNumber() && b.isNumber()) {
				push(Val::fromBool(a.numberVal < b.numberVal));
			} else {
				push(Val::fromBool(false));
			}
			break;
		}
		case opcodeCmpGt: {
			auto b = pop(), a = pop();
			if (a.isNumber() && b.isNumber()) {
				push(Val::fromBool(a.numberVal > b.numberVal));
			} else {
				push(Val::fromBool(false));
			}
			break;
		}
		case opcodeCmpLtEq: {
			auto b = pop(), a = pop();
			if (a.isNumber() && b.isNumber()) {
				push(Val::fromBool(a.numberVal <= b.numberVal));
			} else {
				push(Val::fromBool(false));
			}
			break;
		}
		case opcodeCmpGtEq: {
			auto b = pop(), a = pop();
			if (a.isNumber() && b.isNumber()) {
				push(Val::fromBool(a.numberVal >= b.numberVal));
			} else {
				push(Val::fromBool(false));
			}
			break;
		}
		case opcodeNotL: {
			auto v = pop();
			push(Val::fromBool(!v.asBool()));
			break;
		}
		case opcodeAndL: {
			auto b = pop(), a = pop();
			push(Val::fromBool(a.asBool() && b.asBool()));
			break;
		}
		case opcodeOrL: {
			auto b = pop(), a = pop();
			push(Val::fromBool(a.asBool() || b.asBool()));
			break;
		}
		case opcodeJmp: {
			opIt = ops + op.n;
			break;
		}
		case opcodeJmpN: {
			auto v = pop();
			if (!v.asBool()) {
				opIt = ops + op.n;
			}
			break;
		}
		case opcodeCall: {
			assert(op.n >= 0);
			call(op.n);
			break;
		}
		case opcodeRet: {
			auto v = pop();
			stackLen = frameIdx - nArgs - 1;
			push(v);
			return;
		}
		case opcodePrint: {
			auto v = pop();
			if (v.isNil()) {
				puts("nil");
			} else if (v.isNumber()) {
				printf("%.14g\n", v.numberVal);
			} else if (v.isString()) {
				printf("%.*s\n", int(v.stringVal->len), v.stringVal->chars);
			} else if (v.isFunc()) {
				printf("func@%p\n", v.funcVal);
			}
			break;
		}
		}
	}
}

Val Vm::call(Func *func, size_t nArgs, Val *args) {
	push(Val::newFunc(func));
	while (nArgs > 0) {
		push(*args++);
		nArgs--;
	}
	call(nArgs);
	return pop();
}

void Vm::create() {
	stackBufLen = 1024;
	stackLen = 0;
	stack = new Val[stackBufLen];
}

void Vm::destroy() {
	delete[] stack;
}
