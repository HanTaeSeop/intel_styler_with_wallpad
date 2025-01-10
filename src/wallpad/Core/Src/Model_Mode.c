#include "Model_Mode.h"

uint8_t Mode;

uint8_t Model_getMode()
{
	return Mode;
}

void Model_setMode(uint8_t data)
{
	Mode = data;
}
