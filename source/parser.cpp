#include "parser.h"

#include <cassert>
#include <cstdlib>
#include <cstring>

#include "error.h"
#include "token.h"
#include "vm.h"

char Lexer::nextChar() const {
	assert(it < buf + bufLen);
	return *it;
}

char Lexer::eatChar() {
	assert(it < buf + bufLen);
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
		while (it != buf + bufLen && nextChar() != '\n') {
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

bool Lexer::charIsDigit(char c) {
	return c >= '0' && c <= '9';
}

bool Lexer::charIsWordStart(char c) {
	return (c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		c == '_';
}

bool Lexer::charIsWordPart(char c) {
	return charIsWordStart(c) ||
		charIsDigit(c);
}

Token Lexer::eatWordToken() {
	auto val = it;
	
	while (charIsWordPart(nextChar())) {
		eatChar();
	}
	
	auto valLen = size_t(it - val);

	if (valLen == 3 && memcmp(val, "nil", 3) == 0) {
		eolIsWs = true;
		return Token{Token::kindKwNil, line};	
	} else if (valLen == 4 && memcmp(val, "true", 4) == 0) {
		eolIsWs = true;
		return Token{Token::kindKwTrue, line};
	} else if (valLen == 5 && memcmp(val, "false", 5) == 0) {
		eolIsWs = true;
		return Token{Token::kindKwFalse, line};
	} else if (valLen == 4 && memcmp(val, "func", 4) == 0) {
		eolIsWs = true;
		return Token{Token::kindKwFunc, line};
	} else if (valLen == 3 && memcmp(val, "var", 3) == 0) {
		eolIsWs = true;
		return Token{Token::kindKwVar, line};
	} else if (valLen == 2 && memcmp(val, "if", 2) == 0) {
		eolIsWs = true;
		return Token{Token::kindKwIf, line};
	} else if (valLen == 4 && memcmp(val, "else", 4) == 0) {
		eolIsWs = true;
		return Token{Token::kindKwElse, line};
	} else if (valLen == 5 && memcmp(val, "while", 5) == 0) {
		eolIsWs = true;
		return Token{Token::kindKwWhile, line};
	} else if (valLen == 5 && memcmp(val, "print", 5) == 0) {
		eolIsWs = true;
		return Token{Token::kindKwPrint, line};
	} else if (valLen == 5 && memcmp(val, "break", 5) == 0) {
		eolIsWs = false;
		return Token{Token::kindKwBreak, line};
	} else if (valLen == 8 && memcmp(val, "continue", 8) == 0) {
		eolIsWs = false;
		return Token{Token::kindKwContinue, line};
	} else if (valLen == 6 && memcmp(val, "return", 6) == 0) {
		eolIsWs = false;
		return Token{Token::kindKwReturn, line};
	}
	
	eolIsWs = false;
	return Token{
		.kind = Token::kindName,
		.line = line,
		.strValLen = valLen,
		.strVal = val
	};
}

Token Lexer::eatNumberToken() {
	auto startIt = it;
	
	while (charIsDigit(nextChar())) {
		eatChar();
	}
	
	if (nextChar() == '.') {
		eatChar();
		
		while (charIsDigit(nextChar())) {
			eatChar();
		}
	}
	
	if (charIsWordPart(nextChar())) {
		error(file, line, "invalid character in number constant");
	}
	
	auto val = strtod(startIt, nullptr);
	
	eolIsWs = false;
	return Token{
		.kind = Token::kindNumber,
		.line = line,
		.numberVal = val
	};
}

Token Lexer::eatSymbolToken() {
	if (nextChar() == '=') {
		eatChar();
		
		eolIsWs = true;
		if (nextChar() == '=') {
			eatChar();
			
			return Token{Token::kindEq, line};
		} else {
			return Token{Token::Kind('='), line};
		}
	} else if (nextChar() == '!') {
		eatChar();
		
		eolIsWs = true;
		if (nextChar() == '=') {
			eatChar();
			
			return Token{Token::kindNEq, line};
		} else {
			return Token{Token::Kind('!'), line};
		}
	} else if (nextChar() == '&') {
		eatChar();
		
		eolIsWs = true;
		if (nextChar() == '&') {
			eatChar();
			
			return Token{Token::kindAndL, line};
		} else {
			return Token{Token::Kind('&'), line};
		}
	} else if (nextChar() == '|') {
		eatChar();
		
		eolIsWs = true;
		if (nextChar() == '|') {
			eatChar();
			
			return Token{Token::kindOrL, line};
		} else {
			return Token{Token::Kind('|'), line};
		}
	} else if (nextChar() == '<') {
		eatChar();
		
		eolIsWs = true;
		if (nextChar() == '=') {
			eatChar();
			
			return Token{Token::kindLtEq, line};
		} else {
			return Token{Token::Kind('<'), line};
		}
	} else if (nextChar() == '>') {
		eatChar();
		
		eolIsWs = true;
		if (nextChar() == '=') {
			eatChar();
			
			return Token{Token::kindLtEq, line};
		} else {
			return Token{Token::Kind('>'), line};
		}
	}
	
	auto c = eatChar();
	
	eolIsWs = c != ')' && c != ']' && c != '}';
	return Token{Token::Kind(c), line};
}

Token Lexer::eatToken() {
	eatPadding();
	
	auto c = nextChar();
	if (c == 0) {
		if (eolIsWs) {
			return Token{Token::kindEof, line};
		} else {
			eolIsWs = true;
			return Token{Token::kindEol, line};
		}
	} else if (c == '\n') {
		eolIsWs = true;
		return Token{Token::kindEol, line};
	} else if (charIsWordStart(c)) {
		return eatWordToken();
	} else if (charIsDigit(c)) {
		return eatNumberToken();
	}
	return eatSymbolToken();
}

void Lexer::create(char const *file, size_t bufLen, char const *buf) {
	this->file = file;
	line = 0;
	
	this->bufLen = bufLen;
	this->buf = buf;
	it = buf;
	
	eolIsWs = true;
}

void Parser::FuncBuilder::create() {
	constsBufLen = 8;
	nConsts = 0;
	opsBufLen = 32;
	nOps = 0;
	activeVarsBufLen = 8;
	nActiveVars = 0;
	scopesBufLen = 8;
	nScopes = 0;
	breaksBufLen = 4;
	nBreaks = 0;
	
	nextVarIdx = 0;
	
	ops = new Op[opsBufLen];
	consts = new Val[constsBufLen];
	activeVars = new Var[activeVarsBufLen];
	scopes = new Scope[scopesBufLen];
	breaks = new size_t[breaksBufLen];
}

Func *Parser::FuncBuilder::build() {
	delete[] activeVars;
	
	return new Func{
		.nConsts = nConsts,
		.nOps = nOps,
		.nArgs = 0,
		.nVars = size_t(nextVarIdx),
		.consts = consts,
		.ops = ops
	};
}

Token Parser::eatToken() {
	auto r = nextToken;
	nextToken = lexer.eatToken();
	return r;
}

bool Parser::eatSepToken() {
	if (nextToken.kind == Token::Kind(',') || nextToken.kind == Token::kindEol) {
		eatToken();
		return true;
	}
	return false;
}

Token Parser::expectToken(Token::Kind kind, char const *kindStr) {
	if (nextToken.kind != kind) {
		error(file, nextToken.line, "expected %s before %s",
			kindStr, Token::kindAsStr(nextToken.kind)
		);
	}
	return eatToken();
}

size_t Parser::pushOp(Op op) {
	if (fb.nOps == fb.opsBufLen) {
		fb.opsBufLen *= 2;
		
		auto newOps = new Op[fb.opsBufLen];
		memcpy(newOps, fb.ops, sizeof(Op) * fb.nOps);
		
		delete[] fb.ops;
		fb.ops = newOps;
	}
	
	fb.ops[fb.nOps] = op;
	return fb.nOps++;
}

int32_t Parser::pushConst(Val val) {
	for (auto i = size_t(0); i < fb.nConsts; i++) {
		if (fb.consts[i].equals(val)) {
			return i;
		}
	}
	
	assert(fb.nConsts != INT32_MAX);
	
	if (fb.nConsts == fb.constsBufLen) {
		fb.constsBufLen *= 2;
		
		auto newConsts = new Val[fb.constsBufLen];
		memcpy(newConsts, fb.consts, sizeof(Val) * fb.nConsts);
		
		delete[] fb.consts;
		fb.consts = newConsts;
	}
	
	fb.consts[fb.nConsts] = val;
	return fb.nConsts++;
}

int32_t Parser::pushVar(size_t nameLen, char const *name, int32_t idx) {
	assert(fb.nActiveVars != INT32_MAX);
	
	if (fb.nActiveVars == fb.activeVarsBufLen) {
		fb.activeVarsBufLen *= 2;
		
		auto newVars = new Var[fb.activeVarsBufLen];
		memcpy(newVars, fb.activeVars, sizeof(Var) * fb.nActiveVars);
		
		delete[] fb.activeVars;
		fb.activeVars = newVars;
	}
	
	fb.activeVars[fb.nActiveVars++] = Var{
		.nameLen = nameLen,
		.name = name,
		.idx = idx
	};
	return idx;
}

void Parser::pushScope(bool loop) {
	if (fb.nScopes == fb.scopesBufLen) {
		fb.scopesBufLen *= 2;
		
		auto newScopes = new Scope[fb.scopesBufLen];
		memcpy(newScopes, fb.scopes, sizeof(Scope) * fb.scopesBufLen);
		
		delete[] fb.scopes;
		fb.scopes = newScopes;
	}
	
	size_t continueAddr;
	if (loop || fb.nScopes == 0) {
		continueAddr = fb.nOps;
	} else {
		continueAddr = fb.scopes[fb.nScopes - 1].continueAddr;
	}
	
	fb.scopes[fb.nScopes++] = Scope{
		.nActiveVars = fb.nActiveVars,
		.continueAddr = continueAddr,
		.loop = loop
	};
}

void Parser::pushBreak(size_t opIdx) {
	if (fb.nBreaks == fb.breaksBufLen) {
		fb.breaksBufLen *= 2;
		
		auto newBreaks = new size_t[fb.breaksBufLen];
		memcpy(newBreaks, fb.breaks, sizeof(size_t) * fb.breaksBufLen);
		
		delete[] fb.breaks;
		fb.breaks = newBreaks;
	}
	
	fb.breaks[fb.nBreaks++] = opIdx;
}

void Parser::popScope() {
	auto scope = fb.scopes + --fb.nScopes;
	
	if (scope->loop) {
		while (fb.nBreaks > 0) {
			auto opIdx = fb.breaks[--fb.nBreaks];
			
			fb.ops[opIdx].n = fb.nOps;
		}
	}
	
	fb.nActiveVars = scope->nActiveVars;
}

bool Parser::findVar(size_t nameLen, char const *name, int32_t *oIdx) {
	for (auto i = fb.nActiveVars; i-- > 0;) {
		auto var = fb.activeVars + i;
		if (
			nameLen == var->nameLen &&
			memcmp(name, var->name, nameLen) == 0
		) {
			*oIdx = var->idx;
			return true;
		}
	}
	
	return false;
}

bool Parser::parseExpr(size_t minPrecedence) {
	auto hasLhs = false;
	if (nextToken.kind == '(') {
		eatToken();
		
		expectExpr();
		expectToken(Token::Kind(')'), "')'");
		
		hasLhs = true;
	} else if (nextToken.kind == Token::kindKwNil) {
		auto n = pushConst(Val::newNil());
		eatToken();
		
		pushOp(Op{opcodePushc, n});
		
		hasLhs = true;
	} else if (nextToken.kind == Token::kindNumber) {
		auto n = pushConst(Val::newNumber(nextToken.numberVal));
		eatToken();
		
		pushOp(Op{opcodePushc, n});
		
		hasLhs = true;
	} else if (nextToken.kind == Token::kindName) {
		int32_t n;
		if (!findVar(nextToken.strValLen, nextToken.strVal, &n)) {
			error(file, nextToken.line, "unresolved name '%.*s'",
				int(nextToken.strValLen), nextToken.strVal
			);
		}
		eatToken();
		
		pushOp(Op{opcodePush, n});
		
		hasLhs = true;
	}
	
	for (;;) {
		auto op = nextToken;
		
		size_t precedence;
		if (hasLhs) {
			if (op.kind == '*' || op.kind == '/' || op.kind == '%') {
				precedence = 5;
			} else if (op.kind == '+' || op.kind == '-') {
				precedence = 4;
			} else if (op.kind == Token::kindEq || op.kind == Token::kindNEq ||
				op.kind == '<' || op.kind == '>' || op.kind == Token::kindLtEq || op.kind == Token::kindGtEq
			) {
				precedence = 3;
			} else if (op.kind == Token::kindAndL) {
				precedence = 2;
			} else if (op.kind == Token::kindOrL) {
				precedence = 1;
			} else {
				return true;
			}
		} else if (op.kind == '-' || op.kind == '!') {
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
			if (op.kind == '*') {
				pushOp(Op{opcodeMul});
			} else if (op.kind == '/') {
				pushOp(Op{opcodeDiv});
			} else if (op.kind == '%') {
				pushOp(Op{opcodeMod});
			} else if (op.kind == '+') {
				pushOp(Op{opcodeAdd});
			} else if (op.kind == '-') {
				pushOp(Op{opcodeSub});
			} else if (op.kind == Token::kindEq) {
				pushOp(Op{opcodeCmpEq});
			} else if (op.kind == Token::kindNEq) {
				pushOp(Op{opcodeCmpNEq});
			} else if (op.kind == '<') {
				pushOp(Op{opcodeCmpLt});
			} else if (op.kind == '>') {
				pushOp(Op{opcodeCmpGt});
			} else if (op.kind == Token::kindLtEq) {
				pushOp(Op{opcodeCmpLtEq});
			} else if (op.kind == Token::kindGtEq) {
				pushOp(Op{opcodeCmpGtEq});
			} else if (op.kind == Token::kindAndL) {
				pushOp(Op{opcodeAndL});
			} else if (op.kind == Token::kindOrL) {
				pushOp(Op{opcodeOrL});
			}
		} else if (op.kind == '-') {
			pushOp(Op{opcodeNeg});
		} else if (op.kind == '!') {
			pushOp(Op{opcodeNotL});
		}
		
		hasLhs = true;
	}
}

void Parser::expectExpr(size_t minPrecedence) {
	if (!parseExpr(minPrecedence)) {
		error(file, nextToken.line, "expected expression before %s",
			Token::kindAsStr(nextToken.kind)
		);
	}
}

bool Parser::parseStmt() {
	if (nextToken.kind == Token::Kind('{')) {
		eatToken();
		
		pushScope(false);
		
		parseStmtList();
		
		popScope();
		
		expectToken(Token::Kind('}'), "'}'");
		
		return true;
	} else if (nextToken.kind == Token::kindKwVar) {
		eatToken();
		
		auto nameToken = expectToken(Token::kindName, "name");
		auto nameLen = nameToken.strValLen;
		auto name = nameToken.strVal;
		
		auto n = pushVar(nameLen, name, fb.nextVarIdx++);
		if (nextToken.kind == Token::Kind('=')) {
			eatToken();
			
			expectExpr();
			pushOp(Op{opcodePop, n});
		}
		
		return true;
	} else if (nextToken.kind == Token::kindKwIf) {
		eatToken();
		
		pushScope(false);
		
		expectExpr();
		
		auto jmpElse = pushOp(Op{opcodeJmpN, 0});
		
		expectStmt();
		
		if (nextToken.kind == Token::kindKwElse) {
			eatToken();
			
			auto jmpEnd = pushOp(Op{opcodeJmp, 0});
			fb.ops[jmpElse].n = fb.nOps;
			
			expectStmt();
			
			fb.ops[jmpEnd].n = fb.nOps;
		} else {
			fb.ops[jmpElse].n = fb.nOps;
		}
		
		popScope();
		
		return true;
	} else if (nextToken.kind == Token::kindKwWhile) {
		eatToken();
		
		pushScope(true);
		
		auto jmpStart = Op{opcodeJmp, int32_t(fb.nOps)};
		
		expectExpr();
		
		auto jmpEnd = pushOp(Op{opcodeJmpN, 0});
		
		expectStmt();
		
		pushOp(jmpStart);
		fb.ops[jmpEnd].n = fb.nOps;
		
		popScope();
		
		return true;
	} else if (nextToken.kind == Token::kindKwPrint) {
		eatToken();
		
		expectExpr();
		pushOp(Op{opcodePrint});
		
		return true;
	} else if (nextToken.kind == Token::kindKwBreak) {
		eatToken();
		
		pushOp(Op{opcodeJmp, 0});
		pushBreak(fb.nOps - 1);
		
		return true;
	} else if (nextToken.kind == Token::kindKwContinue) {
		for (auto i = fb.nScopes; i-- > 0;) {
			auto scope = fb.scopes + i;
			if (scope->loop) {
				pushOp(Op{opcodeJmp, int32_t(scope->continueAddr)});
				goto foundScope;
			}
		}
		error(file, nextToken.line, "'continue' not within loop");
		foundScope:
		
		eatToken();
		
		return true;
	} else if (nextToken.kind == Token::kindKwReturn) {
		eatToken();
		
		if (!parseExpr()) {
			auto n = pushConst(Val::newNil());
			pushOp(Op{opcodePushc, n});
		}
		pushOp(Op{opcodeRet});
		
		return true;
	} else if (parseExpr()) {
		if (nextToken.kind == Token::Kind('=')) {
			auto readOp = fb.ops[fb.nOps - 1];
			if (readOp.opcode != opcodePush) {
				error(file, nextToken.line, "assignment to unassignable expression");
			}
			fb.nOps--;
			
			eatToken();
			
			expectExpr();
			
			if (readOp.opcode == opcodePush) {
				pushOp(Op{opcodePop, readOp.n});
			}
		}
		return true;
	}
	return false;
}

void Parser::expectStmt() {
	if (!parseStmt()) {
		error(file, nextToken.line, "expected expression before %s",
			Token::kindAsStr(nextToken.kind)
		);
	}
}

bool Parser::parseStmtList() {
	auto r = false;
	while (parseStmt()) {
		r = true;
		if (!eatSepToken()) {
			break;
		}
	}
	return r;
}

void Parser::parseFuncStmtList() {
	if (!parseStmtList() ||
		fb.ops[fb.nOps - 1].opcode != opcodeRet
	) {
		auto n = pushConst(Val::newNil());
		pushOp(Op{opcodePushc, n});
		pushOp(Op{opcodeRet});
	}
}

Func *Parser::parseOuterFunc() {
	pushScope(false);
	parseFuncStmtList();
	popScope();
	
	expectToken(Token::kindEof, "end of file");
	
	return fb.build();
}

void Parser::create(char const *file, size_t bufLen, char const *buf) {
	this->file = file;
	
	lexer.create(file, bufLen, buf);
	nextToken = lexer.eatToken();
	
	fb.create();
}
