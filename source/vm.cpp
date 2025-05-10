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

void Vm::neg() {
	auto v = pop();
	if (v.isNumber()) {
		push(Val::newNumber(-v.numberVal));
	} else {
		push(Val::newNil());
	}  
}

void Vm::add() {
	auto b = pop(), a = pop();
	if (a.isNumber() && b.isNumber()) {
		push(Val::newNumber(a.numberVal + b.numberVal));
	} else {
		push(Val::newNil());
	}
}

void Vm::sub() {
	auto b = pop(), a = pop();
	if (a.isNumber() && b.isNumber()) {
		push(Val::newNumber(a.numberVal - b.numberVal));
	} else {
		push(Val::newNil());
	}
}

void Vm::mul() {
	auto b = pop(), a = pop();
	if (a.isNumber() && b.isNumber()) {
		push(Val::newNumber(a.numberVal * b.numberVal));
	} else {
		push(Val::newNil());
	}
}

void Vm::div() {
	auto b = pop(), a = pop();
	if (a.isNumber() && b.isNumber()) {
		push(Val::newNumber(a.numberVal / b.numberVal));
	} else {
		push(Val::newNil());
	}
}

void Vm::mod() {
	auto b = pop(), a = pop();
	if (a.isNumber() && b.isNumber()) {
		push(Val::newNumber(fmod(a.numberVal, b.numberVal)));
	} else {
		push(Val::newNil());
	}
}

void Vm::cmpEq() {
	auto b = pop(), a = pop();
	push(Val::fromBool(a.equals(b)));
}

void Vm::cmpNEq() {
	auto b = pop(), a = pop();
	push(Val::fromBool(!a.equals(b)));
}

void Vm::cmpLt() {
	auto b = pop(), a = pop();
	if (a.isNumber() && b.isNumber()) {
		push(Val::fromBool(a.numberVal < b.numberVal));
	} else {
		push(Val::fromBool(false));
	}
}

void Vm::cmpGt() {
	auto b = pop(), a = pop();
	if (a.isNumber() && b.isNumber()) {
		push(Val::fromBool(a.numberVal > b.numberVal));
	} else {
		push(Val::fromBool(false));
	}
}

void Vm::cmpLtEq() {
	auto b = pop(), a = pop();
	if (a.isNumber() && b.isNumber()) {
		push(Val::fromBool(a.numberVal <= b.numberVal));
	} else {
		push(Val::fromBool(false));
	}
}

void Vm::cmpGtEq() {
	auto b = pop(), a = pop();
	if (a.isNumber() && b.isNumber()) {
		push(Val::fromBool(a.numberVal >= b.numberVal));
	} else {
		push(Val::fromBool(false));
	}
}

void Vm::notL() {
	auto v = pop();
	push(Val::fromBool(!v.asBool()));
}

void Vm::andL() {
	auto b = pop(), a = pop();
	push(Val::fromBool(a.asBool() && b.asBool()));
}

void Vm::orL() {
	auto b = pop(), a = pop();
	push(Val::fromBool(a.asBool() || b.asBool()));
}

void Vm::call(Func *func, size_t nParams) {
	assert(stackLen >= nParams);
	
	auto consts = func->consts;
	auto ops = func->ops;
	
	if (nParams > func->nArgs) {
		stackLen -= nParams - func->nArgs;
	} else {
		while (nParams < func->nArgs) {
			nParams++;
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
			neg();
			break;
		}
		case opcodeAdd: {
			add();
			break;
		}
		case opcodeSub: {
			sub();
			break;
		}
		case opcodeMul: {
			mul();
			break;
		}
		case opcodeDiv: {
			div();
			break;
		}
		case opcodeMod: {
			mod();
			break;
		}
		case opcodeCmpEq: {
			cmpEq();
			break;
		}
		case opcodeCmpNEq: {
			cmpNEq();
			break;
		}
		case opcodeCmpLt: {
			cmpLt();
			break;
		}
		case opcodeCmpGt: {
			cmpGt();
			break;
		}
		case opcodeCmpLtEq: {
			cmpLtEq();
			break;
		}
		case opcodeCmpGtEq: {
			cmpGtEq();
			
			break;
		}
		case opcodeNotL: {
			notL();
			break;
		}
		case opcodeAndL: {
			andL();
			break;
		}
		case opcodeOrL: {
			orL();
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
			auto v = stack[stackLen - 1 - op.n];
			if (v.isFunc()) {
				call(v.funcVal, op.n);
			} else {
				push(Val::newNil());
			}
			break;
		}
		case opcodeRet: {
			auto v = pop();
			stackLen = frameIdx - nParams;
			push(v);
			return;
		}
		case opcodePrint: {
			auto v = pop();
			if (v.isNil()) {
				puts("nil");
			} else if (v.isNumber()) {
				printf("%f\n", v.numberVal);
			} else if (v.isFunc()) {
				printf("func.%p\n", v.funcVal);
			}
			break;
		}
		}
	}
}

void Vm::create() {
	stackBufLen = 1024;
	stackLen = 0;
	stack = new Val[stackBufLen];
}

void Vm::destroy() {
	delete[] stack;
}
