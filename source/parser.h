#pragma once

#include <cstddef>

#include "token.h"
#include "val.h"
#include "vm.h"

struct Lexer {
	char nextChar() const;
	char eatChar();
	
	bool eatWhitespace();
	bool eatComment();
	void eatPadding();
	
	Token eatWordToken();
	Token eatNumberToken();
	Token eatSymbolToken();
	Token eatToken();
	
	void create(char const *file, size_t bufLen, char const *buf);
	
private:
	char const *file;
	size_t line;
	
	size_t bufLen;
	char const *buf;
	char const *it;
	
	bool eolIsWs;
	
	static bool charIsDigit(char c);
	static bool charIsWordStart(char c);
	static bool charIsWordPart(char c);
	
};

class Parser {
public:
	Token eatToken();
	
	bool eatSepToken();
	
	Token expectToken(Token::Kind kind, char const *kindStr);
	
	void pushFunc();
	size_t pushOp(Op op);
	int32_t pushConst(Val val);
	int32_t pushVar(size_t nameLen, char const *name, int32_t idx);
	void pushScope(bool loop);
	void pushBreak(size_t opIdx);
	
	Func *popFunc();
	void popScope();
	
	bool findVar(size_t nameLen, char const *name, int32_t *oIdx);
	
	bool parseExpr(size_t minPrecedence = 0);
	void expectExpr(size_t minPrecedence = 0);
	
	size_t parseExprList();
	
	bool parseStmt();
	void expectStmt();
	
	bool parseStmtList();
	void parseFuncStmtList();
	
	Func *parseOuterFunc();
	
	void create(char const *file, size_t bufLen, char const *buf);
	
private:
	struct Var {
		size_t nameLen;
		char const *name;
		int32_t idx;
	};
	
	struct Scope {
		size_t nActiveVars,
			continueAddr;
		bool loop;
	};
	
	struct FuncBuilder {
		size_t constsBufLen, nConsts,
			opsBufLen, nOps,
			activeVarsBufLen, nActiveVars, nParams,
			scopesBufLen, nScopes,
			breaksBufLen, nBreaks;
		
		int32_t nextVarIdx;
		
		Val *consts;
		Op *ops;
		Var *activeVars;
		Scope *scopes;
		size_t *breaks;
		
		void create();
		Func *build();
	};
	
	char const *file;
	
	Lexer lexer;
	Token nextToken;
	
	FuncBuilder fb;
	
	size_t fbStackBufLen, fbStackLen;
	FuncBuilder *fbStack;
	
};
