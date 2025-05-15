#pragma once

#include <cstddef>

#include "darray.h"
#include "heap.h"
#include "op.h"
#include "token.h"
#include "val.h"

namespace SL {
	struct Lexer {
		Token eatToken();
		
		void init(char const *file, size_t nChars, char const *chars);
		void deinit() { }
		
	private:
		// File and line for diagnostics
		char const *file;
		size_t line;
		
		char const *it, *end;
		
		// Whether line feeds should be skipped
		// as whitespace (true) or lexed as eol
		// tokens (false)
		bool eolIsWs;
		
		char nextChar() const;
		char eatChar();
		
		bool eatWhitespace();
		bool eatComment();
		// Eat a run of whitespace and/or comments
		void eatPadding();
		
		Token eatWordToken();
		Token eatNumberToken();
		Token eatStringToken();
		Token eatSymbolToken();
		
	};
	
	struct Compiler {
		Func *run(Heap *heap, char const *file, size_t nChars, char const *chars);
		
	private:
		struct ActiveLocal {
			size_t idx;
			struct {
				size_t nChars;
				char const *chars;
			} name;
		};
		
		Heap *heap;
		
		char const *file;
		
		Lexer lexer;
		Token nextToken;
		
		DArray<Val> consts;
		DArray<Op> ops;
		size_t nParams, nVars;
		DArray<ActiveLocal> activeLocals;
		
		size_t getConst(Val val);
		size_t createVar(size_t nameNChars, char const *nameChars);
		bool getLocal(size_t nameNChars, char const *nameChars, size_t *oIdx);
		
		Token eatToken();
		Token expectToken(TokenKind kind, char const *desc);
		
		bool eatSepToken();
		
		bool eatExpr(size_t minPrecedence = 0);
		void expectExpr(size_t minPrecedence = 0);
		
		size_t eatExprList();
		
		bool eatStmt();
		void expectStmt();
		
		size_t eatStmtList();
		
	};
}
