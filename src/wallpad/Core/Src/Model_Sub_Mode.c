#include <Model_Sub_Mode.h>

uint8_t sub_Mode;

uint8_t Model_getSubMode()
{
	return sub_Mode;
}

void Model_setSubMode(uint8_t sub_data)
{
	sub_Mode = sub_data;
}
