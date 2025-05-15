
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
		auto f = Compiler{}.run(&heap, file, nChars + 1, chars);
		
		auto t = Thread::create(&heap);
		
		Val result;
		t->call(f, 0, nullptr, &result);
		
		auto resultStr = String::createFromVal(&heap, result);
		printf("result: %s\n", resultStr->chars);
	} catch (...) { }
	
	heap.deinit();
}
