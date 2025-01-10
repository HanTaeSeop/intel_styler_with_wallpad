#include <Model_Mortor_Mode.h>

uint8_t mortor_Mode;

uint8_t Model_getMotorMode()
{
	return mortor_Mode;
}

void Model_setMotorMode(uint8_t mortor_data)
{
	mortor_Mode = mortor_data;
}
