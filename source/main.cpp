
#include <cassert>
#include <cstdio>
#include <cstring>

#include "parser.h"

char *loadStr(char const *file, size_t *oLen) {
	auto s = fopen(file, "rb");
	assert(s);
	
	fseek(s, 0, SEEK_END);
	auto len = ftell(s);
	
	auto r = new char[len + 1];
	fseek(s, 0, SEEK_SET);
	fread(r, 1, len, s);
	r[len] = 0;
	
	*oLen = len;
	return r;
}

int main(int argc, char **argv) {
	if (argc == 1) {
		puts("no inputs");
		
		return 1;
	} else {
		auto vm = Vm{};
		vm.create();
		
		for (auto i = 1; i < argc; i++) {
			auto file = argv[i];
			
			size_t bufLen;
			auto buf = loadStr(file, &bufLen);
			
			auto p = Parser{};
			p.create(file, bufLen + 1, buf);
			
			auto f = p.parseOuterFunc();
			vm.call(f, 0);
		}
		
		return 0;
	}
}
