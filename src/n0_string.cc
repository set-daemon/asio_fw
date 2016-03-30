
#include "n0_string.h"

namespace n0_string {

unsigned long int strtoul(const char* s, int len, int base) {
	unsigned long int v = 0;

	if (len <= 0) {
		return v;
	}

	switch (base) {
	case 16:
	break;
	case 10: {
		int i = 0;
		while (i < len) {
			if (!(s[i] >= 0x30 && s[i] <= 0x39)) {
				return v;
			}
			v = v*10 + (0x39 - s[i]);
			++i;
		}
	}
	break;
	case 2:
	break;
	}

	return v;
}

}
