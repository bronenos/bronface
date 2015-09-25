#include "logic.h"


static Logic g_logic;


void logic_reset() {
	memset(&g_logic, 0, sizeof g_logic);
}


Logic *logic() {
	return &g_logic;
}
