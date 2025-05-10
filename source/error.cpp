#include "error.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

void error(char const *file, size_t line, char const *messageFmt, ...) {
	va_list args1, args2;
	va_start(args1, messageFmt);
	va_start(args2, messageFmt);
	
	auto messageLen = vsnprintf(nullptr, 0, messageFmt, args1);
	auto message = new char[messageLen + 1];
	vsprintf(message, messageFmt, args2);
	message[messageLen] = 0;
	
	printf("%s:%llu: %s\n", file, (unsigned long long)(line + 1), message);
	exit(1);
}
