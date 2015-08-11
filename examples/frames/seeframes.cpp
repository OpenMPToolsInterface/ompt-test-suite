#include <stdio.h>

#define DUMP_FRAME(f,i) printf( # f ": builtin_frame_address(%d) = %p\n", i, __builtin_frame_address(i))

void foo_256()
{
	volatile char buffer[256];
	DUMP_FRAME(foo_256,1);
	DUMP_FRAME(foo_256,0);
}

void foo_128()
{
	volatile char buffer[128];
	DUMP_FRAME(foo_128,1);
	DUMP_FRAME(foo_128,0);
	foo_256();
}

void bar_1024()
{
	volatile char buffer[1024];
	DUMP_FRAME(bar_1024,1);
	DUMP_FRAME(bar_1024,0);
}


void bar_512()
{
	volatile char buffer[512];
	void *f_bar_512 = __builtin_frame_address(0);
	DUMP_FRAME(bar_512,1);
	DUMP_FRAME(bar_512,0);
	bar_1024();
}

int main(int argc, char **argv)
{
	void *f_main = __builtin_frame_address(0);
	DUMP_FRAME(main,1);
	DUMP_FRAME(main,0);
	foo_128();
	bar_512();
	return 0;
}
