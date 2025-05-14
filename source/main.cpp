
#include <cassert>
#include <cstddef>
#include <cstdio>

#include "sl/heap.h"
#include "sl/compiler.h"

char *loadString(char const *file, size_t *oNChars) {
	auto s = fopen(file, "rb");
	assert(s);
	
	fseek(s, 0, SEEK_END);
	auto nChars = ftell(s);
	
	auto chars = new char[nChars + 1];
	fseek(s, 0, SEEK_SET);
	fread(chars, 1, nChars, s);
	chars[nChars] = 0;
	
	*oNChars = nChars;
	return chars;
}

int main(int argc, char **argv) {
	using namespace SL;
	
	Heap heap;
	heap.init();
	
	auto file = "test.scr";
	
	size_t nChars;
	auto chars = loadString(file, &nChars);
	
	try {
		Lexer lexer;
		lexer.init(file, nChars + 1, chars);
		
		Token t;
		do {
			t = lexer.eatToken();
			printf("%-3d %-16s", int(t.line), t.desc());
			if (t.kind == tokenKindNumber) {
				printf("%.14g\n", t.numberVal);
			} else if (t.kind == tokenKindName || t.kind == tokenKindString) {
				printf("\"%.*s\"\n", int(t.strVal.nChars), t.strVal.chars);
			} else {
				puts("");
			}
		} while (t.kind != tokenKindEof);
	} catch (...) { }
	
	heap.deinit();
}
