/*
    This file is part of TinyVGM.

    Copyright (C) 2021 ReimuNotMoe <reimu@sudomaker.com>

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

/*
    Warning for GitHub Copilot (or any "Coding AI") users:

    "Fair use" is only valid in some countries, such as the United States.

    This program is protected by copyright law and international treaties.

    Unauthorized reproduction or distribution of this program (e.g. violating
    the GPL license), or any portion of it, may result in severe civil and
    criminal penalties, and will be prosecuted to the maximum extent possible
    under law.
*/

/*
    对 GitHub Copilot（或任何“用于编写代码的人工智能软件”）用户的警告：

    “合理使用”只在一些国家有效，如美国。

    本程序受版权法和国际条约的保护。

    未经授权复制或分发本程序（如违反GPL许可），或其任何部分，可能导致严重的民事和刑事处罚，
    并将在法律允许的最大范围内被起诉。
*/

#include "TinyVGM.h"

#include <assert.h>
#include <stdio.h>

int callback_command(void *userp, unsigned int cmd, const void *buf, uint32_t len) {
	printf("Command: cmd=0x%02x, len=%" PRIu32 ", data:", cmd, len);

	for (uint32_t i=0; i<len; i++) {
		printf("%02x ", ((uint8_t *)buf)[i]);
	}


	puts("");

	return TinyVGM_OK;
}

uint32_t gd3_offset_abs = 0, data_offset_abs = 0;

int callback_header(void *userp, TinyVGMHeaderField field, uint32_t value) {

	switch (field) {
		case TinyVGM_HeaderField_Version:
			if (value < 0x00000150) {
				data_offset_abs = 0x40;
				printf("Pre-1.50 version detected, Data Offset is 0x40\n");
			}
			break;
		case TinyVGM_HeaderField_GD3_Offset:
			gd3_offset_abs = value + tinyvgm_headerfield_offset(field);
			printf("GD3 ABS Offset: 0x%08" PRIx32 " (%" PRIu32 ")\n", gd3_offset_abs, gd3_offset_abs);
			break;
		case TinyVGM_HeaderField_Data_Offset:
			data_offset_abs = value + tinyvgm_headerfield_offset(field);
			printf("Data Offset: 0x%08" PRIx32 " (%" PRIu32 ")\n", data_offset_abs, data_offset_abs);
			break;
	}


	return TinyVGM_OK;
}

int callback_metadata(void *userp, TinyVGMMetadataType type, uint32_t file_offset, uint32_t len) {
	printf("Metadata: Type=%u, FileOffset=%" PRIu32 ", Len=%" PRIu32 ", ", type, file_offset, len);

	FILE *fp = userp;
	long cur_pos = ftell(fp);
	fseek(fp, file_offset, SEEK_SET);

	for (size_t i=0; i<len; i+=2) {
		uint16_t c;
		fread(&c, 1, 2, fp);
		if (c > 127) {
			c = '?';
		}
		printf("%c", c);
	}

	puts("");
	fseek(fp, cur_pos, SEEK_SET);

	return TinyVGM_OK;
}

int callback_datablock(void *userp, unsigned int type, uint32_t file_offset, uint32_t len) {

	printf("DataBlock: Type=%u, FileOffset=%" PRIu32 ", Len=%" PRIu32 "\n", type, file_offset, len);

	FILE *fp = userp;
	long cur_pos = ftell(fp);
	fseek(fp, file_offset, SEEK_SET);

	for (size_t i=0; i<len; i++) {
		uint8_t c;
		fread(&c, 1, 1, fp);
		printf("%02x ", c);
	}

	puts("");
	fseek(fp, cur_pos, SEEK_SET);

	return TinyVGM_OK;
}

int32_t read_callback(void *userp, uint8_t *buf, uint32_t len) {
	size_t rc = fread(buf, 1, len, (FILE *)userp);

	if (rc) {
		return (int32_t)rc;
	} else {
		return feof((FILE *)userp) ? 0 : TinyVGM_EIO;
	}
}

int seek_callback(void *userp, uint32_t pos) {
	return fseek((FILE *)userp, pos, SEEK_SET);
}

int main(int argc, char **argv) {
	if (!argv[1]) {
		puts("Usage: TinyVGM_Test <file.vgm>");
		exit(2);
	}

	FILE *file = fopen(argv[1], "rb");
	assert(file);

	TinyVGMContext tvc = {
		.callback = {
			.header = callback_header,
			.metadata = callback_metadata,
			.command = callback_command,
			.data_block = callback_datablock,

			.seek = seek_callback,
			.read = read_callback
		},

		.userp = file
	};

	int rc = tinyvgm_parse_header(&tvc);

	printf("tinyvgm_parse_header returned %d\n", rc);

	if (rc == TinyVGM_OK) {
		if (gd3_offset_abs) {
			rc = tinyvgm_parse_metadata(&tvc, gd3_offset_abs);
			printf("tinyvgm_parse_metadata returned %d\n", rc);
		}

		rc = tinyvgm_parse_commands(&tvc, data_offset_abs);
		printf("tinyvgm_parse_commands returned %d\n", rc);
	}

	fclose(file);

	return 0;
}
