#include "thread.h"

#include <cassert>
#include <cmath>
#include <cstdio>

#include "array.h"
#include "struct.h"

namespace SL {
	void Thread::call(Func *func, Val inst, size_t nInps, size_t nArgs) {
		assert(func != nullptr);
		assert(stack.len >= nInps);
		
		// Adjust to the correct number of arguments
		if (nArgs > func->nParams) {
			// If too many arguments were provided,
			// pop the extra ones off the stack
			stack.len = stack.len - nArgs + func->nParams;
		} else if (nArgs < func->nParams) {
			// If not enough arguments were provided,
			// push nil onto the stack for the missing ones
			for (auto i = size_t(0); i < func->nParams - nArgs; i++) {
				stack.push(Val::newNil());
			}
		}
		
		nArgs = func->nParams;
		
		callStack.push(Call{
			.func = func,
			.inst = inst,
			.opIt = func->ops,
			.nInps = nInps,
			.nArgs = nArgs,
			.baseStackIdx = stack.len
		});
		
		// Push local variables onto the stack,
		// defaulted to nils
		for (auto i = size_t(0); i < func->nLocals; i++) {
			stack.push(Val::newNil());
		}
	}
	
	Val Thread::getElem(Val base, Val subscript) {
		if (base.isArray() && subscript.isNumber()) {
			auto array = base.arrayVal;
			auto idxF = subscript.numberVal;
			
			if (idxF == trunc(idxF) && !isnan(idxF) && !isinf(idxF)) {
				auto idx = ptrdiff_t(idxF);
				
				if (idx >= 0 && idx < array->nElems) {
					return array->elems[idx];
				}
			}
		} else if (base.isStruct()) {
			auto key = String::createFromVal(heap, subscript);
			Val v;
			if (base.structVal->get(key, &v)) {
				return v;
			}
		}
		
		return Val::newNil();
	}
	
	void Thread::setElem(Val base, Val subscript, Val val) {
		if (base.isArray() && subscript.isNumber()) {
			auto array = base.arrayVal;
			auto idxF = subscript.numberVal;
			
			if (idxF == trunc(idxF) && !isnan(idxF) && !isinf(idxF)) {
				auto idx = ptrdiff_t(idxF);
				
				if (idx >= 0 && idx < array->nElems) {
					array->elems[idx] = val;
				}
			}
		} else if (base.isStruct()) {
			auto key = String::createFromVal(heap, subscript);
			if (val.isNil()) {
				base.structVal->remove(key);
			} else {
				base.structVal->set(key, val);
			}
		}
	}
	
	bool Thread::call(Func *func, Val inst, size_t nArgs, Val const *args, Val *oResult) {
		assert(nArgs == 0 || args != nullptr);
		assert(oResult != nullptr);
		
		// Push arguments onto the stack
		for (auto i = size_t(0); i < nArgs; i++) {
			stack.push(args[i]);
		}
		
		// Call the function, run until it returns to us
		call(func, inst, 0, nArgs);
		return runUntilReturnToHost(oResult);
	}
	
	bool Thread::runUntilReturnToHost(Val *oResult) {
		Call *topCall;
		Func *func;
		Val inst;
		Val *consts;
		Op *opIt;
		size_t nInps;
		size_t nArgs;
		size_t baseStackIdx;
		
		auto refreshLocals = [&]() {
			topCall = &callStack.buf[callStack.len - 1];
			func = topCall->func;
			inst = topCall->inst;
			consts = func->consts;
			opIt = topCall->opIt;
			nInps = topCall->nInps;
			nArgs = topCall->nArgs;
			baseStackIdx = topCall->baseStackIdx;
		};
		refreshLocals();
		
		for (;;) {
			assert(opIt < func->ops + func->nOps);
			auto op = *opIt++;
			switch (op.opcode) {
			case opcodeGetInst: {
				stack.push(inst);
				break;
			}
			case opcodeGetGlobal: {
				stack.push(global);
				break;
			}
			case opcodeGetConst: {
				assert(op.arg >= 0 && op.arg < func->nConsts);
				stack.push(consts[op.arg]);
				break;
			}
			case opcodeGetVar: {
				assert(baseStackIdx + op.arg < stack.len);
				stack.push(stack.buf[baseStackIdx + op.arg]);
				break;
			}
			case opcodeSetVar: {
				assert(stack.len > 0);
				assert(baseStackIdx + op.arg < stack.len);
				stack.buf[baseStackIdx + op.arg] = stack.pop();
				break;
			}
			case opcodeGetElem: {
				assert(stack.len >= 2);
				
				auto subscript = stack.pop();
				auto base = stack.pop();
				stack.push(getElem(base, subscript));
				
				break;
			}
			case opcodeSetElem: {
				assert(stack.len >= 3);
				
				auto val = stack.pop();
				auto subscript = stack.pop();
				auto base = stack.pop();
				setElem(base, subscript, val);
				
				break;
			}
			case opcodeEat: {
				assert(stack.len > 0);
				stack.pop();
				break;
			}
			case opcodeNeg: {
				auto v = stack.pop();
				if (v.isNumber()) {
					stack.push(Val::newNumber(-v.numberVal));
				} else {
					stack.push(Val::newNil());
				}
				break;
			}
			case opcodeAdd: {
				auto b = stack.pop(), a = stack.pop();
				if (a.isNumber() && b.isNumber()) {
					stack.push(Val::newNumber(a.numberVal + b.numberVal));
				} else if (a.isString() || b.isString()) {
					auto aStr = String::createFromVal(heap, a);
					auto bStr = String::createFromVal(heap, b);
					
					auto len = aStr->nChars + bStr->nChars;
					auto r = String::create(heap, len);
					memcpy(r->chars, aStr->chars, aStr->nChars);
					memcpy(r->chars + aStr->nChars, bStr->chars, bStr->nChars);
					
					stack.push(Val::newString(r));
				} else {
					stack.push(Val::newNil());
				}
				break;
			}
			case opcodeSub: {
				auto b = stack.pop(), a = stack.pop();
				if (a.isNumber() && b.isNumber()) {
					stack.push(Val::newNumber(a.numberVal - b.numberVal));
				} else {
					stack.push(Val::newNil());
				}
				break;
			}
			case opcodeMul: {
				auto b = stack.pop(), a = stack.pop();
				if (a.isNumber() && b.isNumber()) {
					stack.push(Val::newNumber(a.numberVal * b.numberVal));
				} else {
					stack.push(Val::newNil());
				}
				break;
			}
			case opcodeDiv: {
				auto b = stack.pop(), a = stack.pop();
				if (a.isNumber() && b.isNumber()) {
					stack.push(Val::newNumber(a.numberVal / b.numberVal));
				} else {
					stack.push(Val::newNil());
				}
				break;
			}
			case opcodeMod: {
				auto b = stack.pop(), a = stack.pop();
				if (a.isNumber() && b.isNumber()) {
					stack.push(Val::newNumber(fmod(a.numberVal, b.numberVal)));
				} else {
					stack.push(Val::newNil());
				}
				break;
			}
			case opcodeCmpEq: {
				auto b = stack.pop(), a = stack.pop();
				stack.push(Val::fromBool(a.equals(b)));
				break;
			}
			case opcodeCmpNEq: {
				auto b = stack.pop(), a = stack.pop();
				stack.push(Val::fromBool(!a.equals(b)));
				break;
			}
			case opcodeCmpLt: {
				auto b = stack.pop(), a = stack.pop();
				if (a.isNumber() && b.isNumber()) {
					stack.push(Val::fromBool(a.numberVal < b.numberVal));
				} else {
					stack.push(Val::fromBool(false));
				}
				break;
			}
			case opcodeCmpGt: {
				auto b = stack.pop(), a = stack.pop();
				if (a.isNumber() && b.isNumber()) {
					stack.push(Val::fromBool(a.numberVal > b.numberVal));
				} else {
					stack.push(Val::fromBool(false));
				}
				break;
			}
			case opcodeCmpLtEq: {
				auto b = stack.pop(), a = stack.pop();
				if (a.isNumber() && b.isNumber()) {
					stack.push(Val::fromBool(a.numberVal <= b.numberVal));
				} else {
					stack.push(Val::fromBool(false));
				}
				break;
			}
			case opcodeCmpGtEq: {
				auto b = stack.pop(), a = stack.pop();
				if (a.isNumber() && b.isNumber()) {
					stack.push(Val::fromBool(a.numberVal >= b.numberVal));
				} else {
					stack.push(Val::fromBool(false));
				}
				break;
			}
			case opcodeNotL: {
				auto v = stack.pop();
				stack.push(Val::fromBool(!v.asBool()));
				break;
			}
			case opcodeAndL: {
				auto b = stack.pop(), a = stack.pop();
				stack.push(Val::fromBool(a.asBool() && b.asBool()));
				break;
			}
			case opcodeOrL: {
				auto b = stack.pop(), a = stack.pop();
				stack.push(Val::fromBool(a.asBool() || b.asBool()));
				break;
			}
			case opcodeMakeArray: {
				auto nElems = op.arg;
				assert(stack.len >= nElems);
				
				auto r = Array::create(heap, nElems);
				memcpy(r->elems, stack.buf + stack.len - nElems, sizeof(Val) * nElems);
				stack.len -= nElems;
				
				stack.push(Val::newArray(r));
				
				break;
			}
			case opcodeMakeStruct: {
				auto nElems = op.arg;
				assert(stack.len >= nElems * 2);
				
				auto r = Struct::create(heap, 16);
				
				for (auto i = size_t(0); i < nElems; i++) {
					auto v = stack.pop();
					auto key = stack.pop();
					
					r->set(String::createFromVal(heap, key), v);
				}
				
				stack.push(Val::newStruct(r));
				
				break;
			}
			case opcodePrint: {
				auto v = stack.pop();
				auto str = String::createFromVal(heap, v);
				puts(str->chars);
				break;
			}
			case opcodeJmp: {
				assert(op.arg >= 0 && op.arg < func->nOps);
				opIt = func->ops + op.arg;
				break;
			}
			case opcodeJmpN: {
				assert(op.arg >= 0 && op.arg < func->nOps);
				auto v = stack.pop();
				if (!v.asBool()) {
					opIt = func->ops + op.arg;
				}
				break;
			}
			case opcodeCall: {
				auto nArgs = op.arg;
				assert(nArgs >= 0);
				
				// Access the called function from the stack,
				// try to call it
				auto tFunc = stack.buf[stack.len - nArgs - 1];
				if (tFunc.isFunc()) {
					topCall->opIt = opIt;
					
					call(tFunc.funcVal, inst, nArgs + 1, nArgs);
					refreshLocals();
				} else {
					// Value called wasn't a function,
					// return nil
					stack.len = stack.len - nArgs - 1;
					stack.push(Val::newNil());
				}
				break;
			}
			case opcodeInstCall: {
				auto nArgs = op.arg;
				assert(nArgs >= 0);
				
				// Retrieve the function from the instance,
				// try to call it
				auto base = stack.buf[stack.len - nArgs - 2];
				auto subscript = stack.buf[stack.len - nArgs - 1];
				auto tFunc = getElem(base, subscript);
				if (tFunc.isFunc()) {
					topCall->opIt = opIt;
					
					call(tFunc.funcVal, base, nArgs + 2, nArgs);
					refreshLocals();
				} else {
					// Value called wasn't a function,
					// return nil
					stack.len = stack.len - nArgs - 1;
					stack.push(Val::newNil());
				}
				break;
			}
			case opcodeRet: {
				assert(stack.len == baseStackIdx + func->nLocals + 1);
				
				// Pop the return value off the stack
				auto v = stack.pop();
				// Pop local variables, arguments, and any any other inputs off the stack
				stack.len = baseStackIdx - nInps;
				
				// Pop the current call off the call stack
				callStack.pop();
				if (callStack.len > 0) {
					refreshLocals();
					// If returning into a VM function,
					// push the return value back onto the stack
					stack.push(v);
				} else {
					// Return the return value to the host
					*oResult = v;
					return true;
				}
				break;
			}
			default: {
				assert(!"unknown opcode");
			}
			}
		}
	}
	
	Thread *Thread::create(Heap *heap, Val global) {
		auto r = (Thread*)heap->createObject(sizeof(Thread), objectTypeThread);
		r->heap = heap;
		r->global = global;
		r->stack.init(64);
		r->callStack.init(8);
		
		return r;
	}
	
	void Thread::deinit() {
		callStack.deinit();
		stack.deinit();
	}
}
