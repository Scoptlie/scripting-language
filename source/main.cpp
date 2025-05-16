
#include <cassert>
#include <cstddef>
#include <cstdio>

#include "sl/compiler.h"
#include "sl/darray.h"
#include "sl/heap.h"
#include "sl/thread.h"
#include "sl/val.h"

char *loadString(char const *file, size_t *oNChars) {
	auto s = fopen(file, "rb");
	if (!s) {
		printf("cannot open file '%s' for reading", file);
		return nullptr;
	}
	
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
	
	if (argc <= 1) {
		puts("no inputs");
		return 1;
	}
	
	Heap heap;
	heap.init();
	
	auto thread = Thread::create(&heap);
	
	for (auto i = 1; i < argc; i++) {
		auto file = argv[i];
		
		size_t nChars;
		auto chars = loadString(file, &nChars);
		if (!chars) {
			return 1;
		}
		
		auto func = Compiler{}.run(&heap, file, nChars + 1, chars);
		if (!func) {
			return 1;
		}
		
		Val result;
		thread->call(func, 0, nullptr, &result);
	}
	
	heap.deinit();
	
	return 0;
}
