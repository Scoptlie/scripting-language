#include "compiler.h"
#include "sl/op.h"
#include "sl/token.h"

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static void printError(char const *file, size_t line, char const *msgFmt, ...) {
	va_list args1, args2;
	va_start(args1, msgFmt);
	va_start(args2, msgFmt);
	
	auto msgLen = vsnprintf(nullptr, 0, msgFmt, args1);
	auto msg = new char[msgLen + 1];
	vsprintf(msg, msgFmt, args2);
	msg[msgLen] = 0;
	
	va_end(args2);
	va_end(args1);
	
	printf("%s:%llu: %s\n", file, (unsigned long long)(line + 1), msg);
	
	delete[] msg;
}

namespace SL {
	static bool charIsDigit(char c) {
		return c >= '0' && c <= '9';
	}
	
	static bool charIsWordStart(char c) {
		return (c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ||
			c == '_';
	}
	
	static bool charIsWordPart(char c) {
		return charIsWordStart(c) ||
			charIsDigit(c);
	}
	
	char Lexer::nextChar() const {
		assert(it < end);
		return *it;
	}
	
	char Lexer::eatChar() {
		assert(it < end);
		auto r = *it++;
		if (r == '\n') {
			line++;
		}
		return r;
	}
	
	bool Lexer::eatWhitespace() {
		auto isWs = [&](char c) {
			return c == ' ' || c == '\t' || c == '\r' ||
				(eolIsWs && c == '\n');
		};
		
		if (isWs(nextChar())) {
			while (isWs(nextChar())) {
				eatChar();
			}
			return true;
		}
		return false;
	}
	
	bool Lexer::eatComment() {
		if (nextChar() == '#') {
			while (nextChar() != '\n' && nextChar() != 0) {
				eatChar();
			}
			return true;
		}
		return false;
	}
	
	void Lexer::eatPadding() {
		while (
			eatWhitespace() ||
			eatComment()
		) { }
	}
	
	Token Lexer::eatWordToken() {
		auto chars = it;
		
		while (charIsWordPart(nextChar())) {
			eatChar();
		}
		
		auto nChars = size_t(it - chars);
		
		auto isKw = [&](char const *kw) {
			auto kwNChars = strlen(kw);
			return nChars == kwNChars &&
				memcmp(kw, chars, kwNChars) == 0;
		};
		
		eolIsWs = false;
		if (isKw("nil")) {
			return Token{tokenKindKwNil, line};
		} else if (isKw("true")) {
			return Token{tokenKindKwTrue, line};
		} else if (isKw("false")) {
			return Token{tokenKindKwFalse, line};
		} else if (isKw("func")) {
			return Token{tokenKindKwFunc, line};
		} else if (isKw("print")) {
			return Token{tokenKindKwPrint, line};
		} else if (isKw("var")) {
			return Token{tokenKindKwVar, line};
		} else if (isKw("if")) {
			return Token{tokenKindKwIf, line};
		} else if (isKw("else")) {
			return Token{tokenKindKwElse, line};
		} else if (isKw("while")) {
			return Token{tokenKindKwWhile, line};
		} else if (isKw("break")) {
			return Token{tokenKindKwBreak, line};
		} else if (isKw("continue")) {
			return Token{tokenKindKwContinue, line};
		} else if (isKw("return")) {
			return Token{tokenKindKwReturn, line};
		}
		return Token{
			.kind = tokenKindName,
			.line = line,
			.strVal = {nChars, chars},
		};
	}
	
	Token Lexer::eatNumberToken() {
		auto chars = it;
		
		// Consume integer part
		while (charIsDigit(nextChar())) {
			eatChar();
		}
		
		// Consume fraction part
		if (nextChar() == '.') {
			eatChar();
			
			while (charIsDigit(nextChar())) {
				eatChar();
			}
		}
		
		if (charIsWordPart(nextChar())) {
			printError(file, line, "invalid character in number constant");
			throw 0;
		}
		
		auto val = strtod(chars, nullptr);
		
		eolIsWs = false;
		return Token{
			.kind = tokenKindNumber,
			.line = line,
			.numberVal = val,
		};
	}
	
	Token Lexer::eatStringToken() {
		eatChar();
		
		auto startLine = line;
		auto chars = it;
		
		do {
			if (nextChar() == 0) {
				printError(file, startLine, "unclosed string constant");
				throw 0;
			}
			if (nextChar() == '\\') {
				eatChar();
				if (nextChar() != 0) {
					eatChar();
				}
				continue;
			}
		} while (eatChar() != '"');
		
		auto nChars = size_t(it - 1 - chars);
		
		eolIsWs = false;
		return Token{
			.kind = tokenKindString,
			.line = startLine,
			.strVal = {nChars, chars},
		};
	}
	
	Token Lexer::eatSymbolToken() {
		eolIsWs = true;
		if (nextChar() == '=') {
			eatChar();
			if (nextChar() == '=') {
				eatChar();
				return Token{tokenKindEq, line};
			} else {
				return Token{TokenKind('='), line};
			}
		} else if (nextChar() == '!') {
			eatChar();
			if (nextChar() == '=') {
				eatChar();
				return Token{tokenKindNEq, line};
			} else {
				return Token{TokenKind('!'), line};
			}
		} else if (nextChar() == '&') {
			eatChar();
			if (nextChar() == '&') {
				eatChar();
				return Token{tokenKindAndL, line};
			} else {
				return Token{TokenKind('&'), line};
			}
		} else if (nextChar() == '|') {
			eatChar();
			if (nextChar() == '|') {
				eatChar();
				return Token{tokenKindOrL, line};
			} else {
				return Token{TokenKind('|'), line};
			}
		} else if (nextChar() == '<') {
			eatChar();
			if (nextChar() == '=') {
				eatChar();
				return Token{tokenKindLtEq, line};
			} else {
				return Token{TokenKind('<'), line};
			}
		} else if (nextChar() == '>') {
			eatChar();
			if (nextChar() == '=') {
				eatChar();
				return Token{tokenKindGtEq, line};
			} else {
				return Token{TokenKind('>'), line};
			}
		} else {
			auto c = eatChar();
			
			eolIsWs = c != ')' && c != ']' && c != '}';
			return Token{TokenKind(c), line};
		}
	}
	
	Token Lexer::eatToken() {
		eatPadding();
		
		auto c = nextChar();
		if (c == 0) {
			if (eolIsWs) {
				return Token{tokenKindEof, line};
			} else {
				return Token{tokenKindEol, line};
			}
		} else if (c == '\n') {
			eolIsWs = true;
			return Token{tokenKindEol, line};
		} else if (charIsWordStart(c)) {
			return eatWordToken();
		} else if (charIsDigit(c)) {
			return eatNumberToken();
		} else if (c == '"') {
			return eatStringToken();
		} else {
			return eatSymbolToken();
		}
	}
	
	void Lexer::init(char const *file, size_t nChars, char const *chars) {
		assert(chars[nChars - 1] == 0);
		
		this->file = file;
		line = 0;
		
		this->it = chars;
		this->end = chars + nChars;
		
		eolIsWs = true;
	}
	
	size_t Compiler::getConst(Val val) {
		for (auto i = size_t(0); i < consts.len; i++) {
			if (consts.buf[i].equals(val)) {
				return i;
			}
		}
		consts.push(val);
		return consts.len - 1;
	}
	
	int32_t Compiler::createVar(size_t nameNChars, char const *nameChars) {
		nVars++;
		activeLocals.push(Local{
			.idx = int32_t(nVars - 1),
			.name = {nameNChars, nameChars}
		});
		return nVars - 1;
	}
	
	bool Compiler::getLocal(size_t nameNChars, char const *nameChars, int32_t *oIdx) {
		for (auto i = activeLocals.len; i-- > 0;) {
			auto v = &activeLocals.buf[i];
			if (nameNChars == v->name.nChars &&
				memcmp(nameChars, v->name.chars, nameNChars) == 0
			) {
				*oIdx = v->idx;
				return true;
			}
		}
		return false;
	}
	
	void Compiler::enterScope(bool isLoop) {
		scopes.push(Scope{
			.firstOp = ops.len,
			.firstActiveLocal = activeLocals.len,
			.isLoop = isLoop,
		});
	}
	
	void Compiler::exitScope() {
		auto s = scopes.pop();
		
		if (s.isLoop) {
			for (auto i = size_t(0); i < breakOps.len; i++) {
				ops.buf[breakOps.buf[i]].arg = int32_t(ops.len);
			}
			breakOps.len = 0;
		}
		
		// Pop locals defined within this scope
		activeLocals.len = s.firstActiveLocal;
	}
	
	Token Compiler::eatToken() {
		auto r = nextToken;
		nextToken = lexer.eatToken();
		return r;
	}
	
	Token Compiler::expectToken(TokenKind kind, char const *desc) {
		if (nextToken.kind != kind) {
			printError(file, nextToken.line, "expected %s before %s",
				desc, nextToken.desc()
			);
			throw 0;
		}
		return eatToken();
	}
	
	bool Compiler::eatSepToken() {
		if (nextToken.kind == ',' || nextToken.kind == tokenKindEol) {
			eatToken();
			return true;
		} else {
			return false;
		}
	}
	
	bool Compiler::eatExpr(size_t minPrecedence) {
		auto hasLhs = false;
		if (nextToken.kind == '(') {
			eatToken();
			
			expectExpr();
			
			expectToken(TokenKind(')'), "')'");
			
			hasLhs = true;
		} else if (nextToken.kind == tokenKindKwNil) {
			eatToken();
			
			auto arg = getConst(Val::newNil());
			ops.push(Op{opcodeConst, int32_t(arg)});
			
			hasLhs = true;
		} else if (nextToken.kind == tokenKindKwTrue) {
			auto arg = getConst(Val::fromBool(true));
			eatToken();
			
			ops.push(Op{opcodeConst, int32_t(arg)});
			
			hasLhs = true;
		} else if (nextToken.kind == tokenKindKwFalse) {
			auto arg = getConst(Val::fromBool(false));
			eatToken();
			
			ops.push(Op{opcodeConst, int32_t(arg)});
			
			hasLhs = true;
		} else if (nextToken.kind == tokenKindNumber) {
			auto arg = getConst(Val::newNumber(nextToken.numberVal));
			eatToken();
			
			ops.push(Op{opcodeConst, int32_t(arg)});
			
			hasLhs = true;
		} else if (nextToken.kind == tokenKindString) {
			auto tokenVal = nextToken.strVal;
			
			Val val;
			if (tokenVal.nChars > 0) {
				DArray<char> chars;
				chars.init(tokenVal.nChars);
				
				auto i = size_t(0);
				while (i < tokenVal.nChars) {
					auto c = tokenVal.chars[i++];
					if (c == '\\') {
						// Okay not to do a bounds check here because
						// '\' will never be the last character in a string constant
						c = tokenVal.chars[i++];
						if (c == '"') {
							chars.push('"');
						} else if (c == '\\') {
							chars.push('\\');
						} else if (c == 'n') {
							chars.push('\n');
						} else if (c == 't') {
							chars.push('\t');
						} else if (c == 'f') {
							chars.push('\f');
						} else if (c == 'r') {
							chars.push('\r');
						} else if (c == 'b') {
							chars.push('\b');
						} else {
							chars.deinit();
							
							printError(file, nextToken.line, "invalid escape sequence");
							throw 0;
						}
					} else {
						chars.push(c);
					}
				}
				
				eatToken();
				
				val = Val::newString(
					String::create(heap, chars.len, chars.buf)
				);
				
				chars.deinit();
			} else {
				val = Val::newString(
					String::create(heap, 0, nullptr)
				);
			}
			
			auto arg = getConst(val);
			ops.push(Op{opcodeConst, int32_t(arg)});
			
			hasLhs = true;
		} else if (nextToken.kind == tokenKindKwFunc) {
			eatToken();
			
			auto prevConsts = consts;
			auto prevOps = ops;
			auto prevNParams = nParams;
			auto prevNVars = nVars;
			auto prevActiveLocals = activeLocals;
			auto prevScopes = scopes;
			
			consts.init(8);
			ops.init(32);
			nParams = 0;
			nVars = 0;
			activeLocals.init(8);
			prevScopes.init(8);
			
			expectToken(TokenKind('('), "'('");
			
			while (nextToken.kind == tokenKindName) {
				activeLocals.push(Local{
					.idx = 0,
					.name = {nextToken.strVal.nChars, nextToken.strVal.chars},
				});
				eatToken();
				
				if (!eatSepToken()) {
					break;
				}
			}
			
			expectToken(TokenKind(')'), "')'");
			
			nParams = activeLocals.len;
			
			for (auto i = size_t(0); i < activeLocals.len; i++) {
				activeLocals.buf[i].idx = int32_t(i) - int32_t(activeLocals.len);
			}
			
			expectToken(TokenKind('{'), "");
			
			eatFuncStmtList();
			
			expectToken(TokenKind('}'), "");
			
			auto func = Func::create(heap);
			func->nConsts = consts.len;
			func->consts = consts.buf;
			func->nOps = ops.len;
			func->ops = ops.buf;
			func->nParams = nParams;
			func->nVars = nVars;
			
			scopes = prevScopes;
			activeLocals = prevActiveLocals;
			nVars = prevNVars;
			nParams = prevNParams;
			ops = prevOps;
			consts = prevConsts;
			
			auto arg = getConst(Val::newFunc(func));
			ops.push(Op{opcodeConst, int32_t(arg)});
			
			hasLhs = true;
		} else if (nextToken.kind == tokenKindName) {
			int32_t idx;
			if (getLocal(nextToken.strVal.nChars, nextToken.strVal.chars, &idx)) {
				ops.push(Op{opcodeVar, idx});
			} else {
				printError(file, nextToken.line, "unresolved name '%.*s'",
					int(nextToken.strVal.nChars),
					nextToken.strVal.chars
				);
				throw 0;
			}
			
			eatToken();
			
			hasLhs = true;
		}
		
		for (;;) {
			if (nextToken.kind == '(') {
				eatToken();
				
				auto nArgs = eatExprList();
				
				expectToken(TokenKind(')'), "')'");
				
				ops.push(Op{opcodeCall, int32_t(nArgs)});
			} else {
				auto op = nextToken.kind;
				
				size_t precedence;
				if (hasLhs) {
					if (op == '*' || op == '/' || op == '%') {
						precedence = 5;
					} else if (op == '+' || op == '-') {
						precedence = 4;
					} else if (op == tokenKindEq || op == tokenKindNEq ||
						op == '<' || op == '>' || op == tokenKindLtEq || op == tokenKindGtEq
					) {
						precedence = 3;
					} else if (op == tokenKindAndL) {
						precedence = 2;
					} else if (op == tokenKindOrL) {
						precedence = 1;
					} else {
						return true;
					}
				} else if (op == '-' || op == '!') {
					precedence = 6;
				} else {
					return false;
				}
				
				if (precedence < minPrecedence) {
					return hasLhs;
				}
				
				eatToken();
				
				expectExpr(precedence);
				
				if (hasLhs) {
					if (op == '*') {
						ops.push(Op{opcodeMul});
					} else if (op == '/') {
						ops.push(Op{opcodeDiv});
					} else if (op == '%') {
						ops.push(Op{opcodeMod});
					} else if (op == '+') {
						ops.push(Op{opcodeAdd});
					} else if (op == '-') {
						ops.push(Op{opcodeSub});
					} else if (op == tokenKindEq) {
						ops.push(Op{opcodeCmpEq});
					} else if (op == tokenKindNEq) {
						ops.push(Op{opcodeCmpNEq});
					} else if (op == '<') {
						ops.push(Op{opcodeCmpLt});
					} else if (op == '>') {
						ops.push(Op{opcodeCmpGt});
					} else if (op == tokenKindLtEq) {
						ops.push(Op{opcodeCmpLtEq});
					} else if (op == tokenKindGtEq) {
						ops.push(Op{opcodeCmpGtEq});
					} else if (op == tokenKindAndL) {
						ops.push(Op{opcodeAndL});
					} else if (op == tokenKindOrL) {
						ops.push(Op{opcodeOrL});
					} else {
						assert(!"unreachable");
					}
				} else if (op == '-') {
					ops.push(Op{opcodeNeg});
				} else if (op == '!') {
					ops.push(Op{opcodeNotL});
				} else {
					assert(!"unreachable");
				}
			}
			
			hasLhs = true;
		}
	}
	
	void Compiler::expectExpr(size_t minPrecedence) {
		if (!eatExpr(minPrecedence)) {
			printError(file, nextToken.line, "expected expression before %s",
				nextToken.desc()
			);
			throw 0;
		}
	}
	
	size_t Compiler::eatExprList() {
		auto r = size_t(0);
		while (eatExpr()) {
			r++;
			if (!eatSepToken()) {
				break;
			}
		}
		return r;
	}
	
	bool Compiler::eatStmt() {
		if (nextToken.kind == '{') {
			enterScope();
			
			eatToken();
			
			eatStmtList();
			
			expectToken(TokenKind('}'), "'}'");
			
			exitScope();
			
			return true;
		} else if (nextToken.kind == tokenKindKwPrint) {
			eatToken();
			
			expectExpr();
			
			ops.push(Op{opcodePrint});
			
			return true;
		} else if (nextToken.kind == tokenKindKwVar) {
			eatToken();
			
			auto nameToken = expectToken(tokenKindName, "name");
			
			auto idx = createVar(nameToken.strVal.nChars, nameToken.strVal.chars);
			
			if (nextToken.kind == '=') {
				eatToken();
				
				expectExpr();
				
				ops.push(Op{opcodeSetVar, idx});
			}
			
			return true;
		} else if (nextToken.kind == tokenKindKwIf) {
			enterScope();
			
			eatToken();
			
			expectExpr();
			
			auto jmpElseOp = ops.len;
			ops.push(Op{opcodeJmpN, -1});
			
			expectStmt();
			
			if (nextToken.kind == tokenKindKwElse) {
				eatToken();
				
				auto jmpEndOp = ops.len;
				ops.push(Op{opcodeJmp, -1});
				
				ops.buf[jmpElseOp].arg = ops.len;
				
				expectStmt();
				
				ops.buf[jmpEndOp].arg = ops.len;
			} else {
				ops.buf[jmpElseOp].arg = ops.len;
			}
			
			exitScope();
			
			return true;
		} else if (nextToken.kind == tokenKindKwWhile) {
			enterScope(true);
			
			eatToken();
			
			auto start = int32_t(ops.len);
			
			eatExpr();
			
			auto jmpEndOp = ops.len;
			ops.push(Op{opcodeJmpN, -1});
			
			eatStmt();
			
			ops.push(Op{opcodeJmp, start});
			
			ops.buf[jmpEndOp].arg = int32_t(ops.len);
			
			exitScope();
			
			return true;
		} else if (nextToken.kind == tokenKindKwBreak) {
			auto inLoop = false;
			for (auto i = scopes.len; i-- > 0;) {
				auto s = &scopes.buf[i];
				if (s->isLoop) {
					inLoop = true;
					break;
				}
			}
			
			if (!inLoop) {
				printError(file, nextToken.line, "'break' not within loop");
				throw 0;
			}
			
			eatToken();
			
			breakOps.push(ops.len);
			ops.push(Op{opcodeJmp, -1});
			
			return true;
		} else if (nextToken.kind == tokenKindKwContinue) {
			auto inLoop = false;
			for (auto i = scopes.len; i-- > 0;) {
				auto s = &scopes.buf[i];
				if (s->isLoop) {
					ops.push(Op{opcodeJmp, int32_t(s->firstOp)});
					inLoop = true;
					break;
				}
			}
			
			if (!inLoop) {
				printError(file, nextToken.line, "'continue' not within loop");
				throw 0;
			}
			
			eatToken();
			
			return true;
		} else if (nextToken.kind == tokenKindKwReturn) {
			eatToken();
			
			if (!eatExpr()) {
				auto arg = getConst(Val::newNil());
				ops.push(Op{opcodeConst, int32_t(arg)});
			}
			
			ops.push(Op{opcodeRet});
			
			return true;
		} else if (eatExpr()) {
			if (nextToken.kind == '=') {
				auto getOp = ops.pop();
				if (getOp.opcode != opcodeVar) {
					printError(file, nextToken.line, "assignment to unassignable expression");
					throw 0;
				}
				
				eatToken();
				
				expectExpr();
				
				if (getOp.opcode == opcodeVar) {
					ops.push(Op{opcodeSetVar, getOp.arg});
				}
			} else {
				ops.push(Op{opcodeEat});
			}
			
			return true;
		} else {
			return false;
		}
	}
	
	void Compiler::expectStmt() {
		if (!eatStmt()) {
			printError(file, nextToken.line, "expected statement before %s",
				nextToken.desc()
			);
			throw 0;
		}
	}
	
	size_t Compiler::eatStmtList() {
		auto r = size_t(0);
		while (eatStmt()) {
			r++;
			if (!eatSepToken()) {
				break;
			}
		}
		return r;
	}
	
	void Compiler::eatFuncStmtList() {
		eatStmtList();
		
		if (ops.len == 0 || ops.buf[ops.len - 1].opcode != opcodeRet) {
			auto arg = getConst(Val::newNil());
			ops.push(Op{opcodeConst, int32_t(arg)});
			ops.push(Op{opcodeRet});
		}
	}
	
	Func *Compiler::run(Heap *heap, char const *file, size_t nChars, char const *chars) {
		this->heap = heap;
		
		this->file = file;
		
		lexer.init(file, nChars, chars);
		nextToken = lexer.eatToken();
		
		consts.init(8);
		ops.init(32);
		nParams = 0;
		nVars = 0;
		activeLocals.init(8);
		scopes.init(8);
		breakOps.init(8);
		
		try {
			eatFuncStmtList();
			
			expectToken(tokenKindEof, "end of file");
			
			auto r = Func::create(heap);
			r->nConsts = consts.len;
			r->consts = consts.buf;
			r->nOps = ops.len;
			r->ops = ops.buf;
			r->nParams = nParams;
			r->nVars = nVars;
			
			breakOps.deinit();
			scopes.deinit();
			activeLocals.deinit();
			lexer.deinit();
			
			return r;
		} catch (...) {
			breakOps.deinit();
			scopes.deinit();
			activeLocals.deinit();
			ops.deinit();
			consts.deinit();
			lexer.deinit();
			
			return nullptr;
		}
	}
}
