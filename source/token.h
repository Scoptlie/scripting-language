#pragma once

#include <cstddef>

struct Token {
	enum Kind {
		kindEof = 256,
		kindEol,
		
		kindName,
		
		kindNumber,
		kindString,
		
		kindKwNil,
		kindKwTrue,
		kindKwFalse,
		kindKwFunc,
		kindKwVar,
		kindKwIf,
		kindKwElse,
		kindKwWhile,
		kindKwPrint,
		kindKwBreak,
		kindKwContinue,
		kindKwReturn,
		
		kindEq, // ==
		kindNEq, // !=
		kindAndL, // &&
		kindOrL, // ||
		kindLtEq, // <=
		kindGtEq, // >=
	};
	
	Kind kind;
	size_t line;
	union {
		double numberVal;
		struct { size_t strValLen; char const *strVal; };
	};
	
	static char const *kindAsStr(Kind kind);
};
