/*
    This file is part of TinyVGM.

    Copyright (C) 2021 ReimuNotMoe <reimu@sudomaker.com>
    Copyright (C) 2021 Yukino Song <yukino@sudomaker.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "TinyVGM.h"

#include <assert.h>
#include <stdio.h>

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__))
#include <unistd.h>
#else
static int usleep (uint32_t __useconds) {
	puts("sleep is not emulated on non-POSIX platforms.");
	return 0;
}
#endif

time_t sample_rate = 1000000 / 44100;

int callback(void *userp, uint8_t value, const void *buf, uint32_t len) {
	printf("callback: value=0x%02x, len=%" PRIu32 ", data:", value, len);

	for (uint32_t i=0; i<len; i++) {
		printf("%02x ", ((uint8_t *)buf)[i]);
	}

	puts("");

	return TinyVGM_OK;
}

int callback_sleep(void *userp, uint8_t value, const void *buf, uint32_t len) {
	assert(len == 2);
	usleep(sample_rate * *((uint16_t *)buf));

	return TinyVGM_OK;
}

int callback_sleep_62(void *userp, uint8_t value, const void *buf, uint32_t len) {
	usleep(sample_rate * 735);

	return TinyVGM_OK;
}

int callback_sleep_63(void *userp, uint8_t value, const void *buf, uint32_t len) {
	usleep(sample_rate * 882);

	return TinyVGM_OK;
}

int main(int argc, char **argv) {
	if (!argv[1]) {
		puts("Usage: TinyVGM_Test <file>");
		exit(2);
	}

	FILE *file = fopen(argv[1], "rb");
	assert(file);

	uint8_t buf[32];

	TinyVGMContext tvc;

	tinyvgm_init(&tvc);

	tinyvgm_add_command_callback(&tvc, 0x61, callback_sleep, NULL);
	tinyvgm_add_command_callback(&tvc, 0x62, callback_sleep_62, NULL);
	tinyvgm_add_command_callback(&tvc, 0x63, callback_sleep_63, NULL);

	uint8_t cmds[] = {0x50, 0x51, 0x5a, 0x5e, 0x5f};

	for (uint8_t i=0; i<sizeof(cmds); i++) {
		tinyvgm_add_command_callback(&tvc, cmds[i], callback, NULL);
	}

	while (1) {
		uint32_t rc = fread(buf, 1, sizeof(buf), file);

		if (rc > 0) {
			int32_t rc2 = tinyvgm_parse(&tvc, buf, rc);
			printf("tinyvgm_parse() returned %d\n", rc2);
		} else {
			break;
		}
	}

	fclose(file);

	return 0;
}