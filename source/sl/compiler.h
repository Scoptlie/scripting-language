#pragma once

#include <cstddef>

#include "token.h"

namespace SL {
	struct Lexer {
		Token eatToken();
		
		void init(char const *file, size_t bufLen, char const *buf);
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
}
