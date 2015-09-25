#include "logic.h"


static Logic_t g_logic;


void logic_reset() {
	memset(&g_logic, 0, sizeof g_logic);
}


Logic_t *logic() {
	return &g_logic;
}
