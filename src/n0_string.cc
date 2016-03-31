#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
			v = v*10 + (s[i] - 0x30);
			++i;
		}
	}
	break;
	case 2:
	break;
	}

	return v;
}

void hex_print(const char* s, int len, const char* title, int line_width) {
	fprintf(stdout, "%s>>>\n", title);

	int line_num = len / line_width;
	if (len % line_width != 0) {
		line_num += 1;
	}

	for (int i = 0; i < line_num; ++i) {
		int line_start = i * line_width; 
		int line_end = line_start + line_width;
		if (line_end > len) {
			line_end = len;
		}
		fprintf(stdout, "0x%08x ", i*line_width);
		for (int j = line_start; j < line_end; ++j) {
			fprintf(stdout, "%02x ", *(const unsigned char*)(s+j));
		}
		for (int j = 0; j < line_width -(line_end - line_start); ++j) {
			fprintf(stdout, "   ");
		}
		fprintf(stdout, "       ");
		for (int j = line_start; j < line_end; ++j) {
			unsigned char c = *(unsigned char*)(s+j);
			if (c > 0x20 && c < 0x7F) {
				fprintf(stdout, "%c", c);
			} else {
				fprintf(stdout, "%c", '.');
			}
		}
		fprintf(stdout, "\n");
	
	}
	fprintf(stdout, "<<<%s\n", title);
	fflush(stdout);
}

}
