#include "Print.h"

extern "C" void __cxa_pure_virtual() { while (1); }

void Print::print(const char *str) {
	while (*str) {
		write(*str++);
	}
}

void Print::println() {
	print("\n\r");
}

void Print::println(const char *str) {
	print(str);
	println();
}

void Print::printf(const char *format, ...) {
	
	char buffer[256];
	memset(buffer, 0, 256);
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	
	int i=0;
	while (buffer[i]) {
		write(buffer[i++]);
	}

// test this
//	print((const char *)buffer);
	
	va_end(args);
}