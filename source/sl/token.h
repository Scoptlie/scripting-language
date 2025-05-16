#pragma once

#include <cstddef>

namespace SL {
	enum TokenKind {
		// Values [0, 255] used for single ASCII
		// character tokens
		
		tokenKindEof = 256,
		tokenKindEol,
		
		// Constants
		tokenKindNumber,
		tokenKindString,
		
		tokenKindName, // Identifier
		
		// Keywords
		tokenKindKwNil,
		tokenKindKwTrue,
		tokenKindKwFalse,
		tokenKindKwFunc,
		tokenKindKwThis,
		tokenKindKwGlobal,
		tokenKindKwPrint,
		tokenKindKwVar,
		tokenKindKwIf,
		tokenKindKwElse,
		tokenKindKwWhile,
		tokenKindKwBreak,
		tokenKindKwContinue,
		tokenKindKwReturn,
		
		tokenKindEq, // ==
		tokenKindNEq, // !=
		tokenKindAndL, // &&
		tokenKindOrL, // ||
		tokenKindLtEq, // <=
		tokenKindGtEq, // >=
	};
	
	struct Token {
		TokenKind kind;
		size_t line;
		union {
			double numberVal;
			struct {
				size_t nChars;
				char const *chars;
			} strVal;
		};
		
		// Returns a description of this token
		// for diagnostics
		char const *desc();
	};
}
