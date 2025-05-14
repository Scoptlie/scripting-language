#include "compiler.h"

#include <cassert>
#include <cstdarg>
#include <cstdio>
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
				printError(file, line, "unclosed string constant");
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
	
	void Lexer::init(char const *file, size_t bufLen, char const *buf) {
		assert(buf[bufLen - 1] == 0);
		
		this->file = file;
		line = 0;
		
		this->it = buf;
		this->end = buf + bufLen;
		
		eolIsWs = true;
	}
}
