#include "val.h"

#include "vm.h"

void Func::destroy() {
	delete[] consts;
	delete[] ops;
}
