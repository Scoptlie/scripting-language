#pragma once

#include <cstddef>
#include <cstdint>

#include "darray.h"
#include "func.h"
#include "heap.h"
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
		// Record of a local or param
		struct Var {
			// Offset on the stack relative to the function's
			// base stack index
			int32_t idx;
			struct {
				size_t nChars;
				char const *chars;
			} name;
		};
		
		// Record of a lexical scope
		struct Scope {
			size_t firstOp;
			size_t firstActiveVar;
			bool isLoop;
		};
		
		Heap *heap;
		
		char const *file;
		
		Lexer lexer;
		Token nextToken;
		
		DArray<Val> consts;
		DArray<Op> ops;
		size_t nParams, nLocals;
		DArray<Var> activeVars;
		DArray<Scope> scopes;
		DArray<size_t> breakOps;
		
		size_t getConst(Val val);
		int32_t createLocal(size_t nameNChars, char const *nameChars);
		bool getVar(size_t nameNChars, char const *nameChars, int32_t *oIdx);
		void enterScope(bool isLoop = false);
		void exitScope();
		
		Token eatToken();
		Token expectToken(TokenKind kind, char const *desc);
		
		bool eatSepToken();
		
		bool eatExpr(size_t minPrecedence = 0);
		void expectExpr(size_t minPrecedence = 0);
		
		size_t eatExprList();
		
		bool eatStmt();
		void expectStmt();
		
		size_t eatStmtList();
		void eatFuncStmtList();
		
	};
}
