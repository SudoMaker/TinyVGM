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

#ifndef TinyVGM_DEBUG
#define TinyVGM_DEBUG	1
#endif

#if TinyVGM_DEBUG != 1
#define printf
#define fprintf
#endif

// -1: Unused, -2: Data block
static const int8_t vgm_cmd_length_table[256] = {
	//0	1	2	3	4	5	6	7	8	9	A	B	C	D	E	F
	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	// 00 - 0F
	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	// 10 - 1F
	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	// 20 - 2F
	1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	// 30 - 3F
	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	1,	// 40 - 4F
	1,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	// 50 - 5F
	-1,	2,	0,	0,	-1,	-1,	0,	-2,	11,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	// 60 - 6F
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	// 70 - 7F
	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	// 80 - 8F
	4,	4,	5,	10,	1,	4,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	// 90 - 9F
	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	// A0 - AF
	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	// B0 - BF
	3,	3,	3,	3,	3,	3,	3,	3,	3,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	// C0 - CF
	3,	3,	3,	3,	3,	3,	3,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	// D0 - DF
	4,	4,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	// E0 - EF
	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	// F0 - FF
};

static inline int32_t tinyvgm_io_read(TinyVGMContext *ctx, uint8_t *buf, uint32_t len) {
	return ctx->callback.read(ctx->userp, buf, len);
}

static int32_t tinyvgm_io_readall(TinyVGMContext *ctx, uint8_t *buf, uint32_t len) {
	uint32_t bytes_read = 0;

	while (1) {
		int32_t rc = tinyvgm_io_read(ctx, buf+bytes_read, len-bytes_read);

		if (rc > 0) {
			bytes_read += rc;

			if (bytes_read == len) {
				return (int32_t)bytes_read;
			}
		} else if (rc == 0) {
			return (int32_t)bytes_read;
		} else {
			if (bytes_read) {
				return (int32_t)bytes_read;
			} else {
				return rc;
			}
		}
	}
}

static inline int tinyvgm_io_seek(TinyVGMContext *ctx, uint32_t pos) {
	return ctx->callback.seek(ctx->userp, pos);
}

int tinyvgm_parse_header(TinyVGMContext *ctx) {
	if (tinyvgm_io_seek(ctx, 0) != 0) {
		return TinyVGM_EIO;
	}

	uint8_t buf[4];
	uint32_t val;

	unsigned int loop_end = TinyVGM_HeaderField_MAX;

	for (unsigned int i=0; i<loop_end; i++) {

		if (tinyvgm_io_readall(ctx, buf, sizeof(uint32_t)) != sizeof(uint32_t)) {
			return TinyVGM_EIO;
		}
		val=(uint_fast32_t)buf[0] | ((uint_fast32_t)buf[1] << 8) | ((uint_fast32_t)buf[2] << 16) | ((uint_fast32_t)buf[3] << 24);

		fprintf(stderr, "tinyvgm_parse_header: offset: 0x%04x, value: 0x%08" PRIx32 " (%" PRId32 ")\n", (unsigned int)(i * sizeof(uint32_t)), val, val);

		if (i == TinyVGM_HeaderField_Identity) {
			if (val != 0x206d6756) {
				return TinyVGM_EINVAL;
			}

			fprintf(stderr, "tinyvgm_parse_header: valid VGM ident\n");
		} else if (i == TinyVGM_HeaderField_Version) {
			if (val < 0x00000151) {
				loop_end = TinyVGM_HeaderField_SegaPCM_Clock;
				if (val < 0x00000150) {
					loop_end = TinyVGM_HeaderField_Data_Offset;
					if (val < 0x00000110) {
						loop_end = TinyVGM_HeaderField_YM2612_Clock;
						if (val < 0x00000101) {
							loop_end = TinyVGM_HeaderField_Rate;
						}
					}
				}
			}

			if (ctx->callback.header) {
				int rc = ctx->callback.header(ctx->userp, i, val);

				if (rc != TinyVGM_OK) {
					return rc;
				}
			}
		} else {
			if (ctx->callback.header) {
				int rc = ctx->callback.header(ctx->userp, i, val);
				
				if (rc != TinyVGM_OK) {
					return rc;
				}
			}
		}
	}

	return TinyVGM_OK;
}

int tinyvgm_parse_metadata(TinyVGMContext *ctx, uint32_t offset_abs) {
	if (tinyvgm_io_seek(ctx, offset_abs) != 0) {
		return TinyVGM_EIO;
	}

	uint32_t metadata_len = 0;

	uint8_t buf[8]; /* MAX(sizeof(uint32_t), 4 * sizeof(uint16_t)) */
	uint32_t val;

	// 0: "Gd3 ", 1: version, 2: data len
	for (unsigned int i=0; i<3; i++) {
		
		if (tinyvgm_io_readall(ctx, buf, sizeof(uint32_t)) != sizeof(uint32_t)) {
			return TinyVGM_EIO;
		}
		val=(uint_fast32_t)buf[0] | ((uint_fast32_t)buf[1] << 8) | ((uint_fast32_t)buf[2] << 16) | ((uint_fast32_t)buf[3] << 24);

		fprintf(stderr, "tinyvgm_parse_metadata: offset: 0x%04x, value: 0x%08" PRIx32 " (%" PRId32 ")\n", (unsigned int)(i * sizeof(uint32_t)), val, val);

		switch (i) {
			case 0:
				if (val != 0x20336447) {
					return TinyVGM_EINVAL;
				}

				fprintf(stderr, "tinyvgm_parse_metadata: valid GD3 ident\n");
				break;

			case 1:
				fprintf(stderr, "tinyvgm_parse_metadata: GD3 version: 0x%08" PRIx32 "\n", val);
				break;

			case 2:
				fprintf(stderr, "tinyvgm_parse_metadata: data len: %" PRIu32 "\n", val);
				metadata_len = val;
				break;
		}

	}

	if (metadata_len) {
		uint32_t cur_pos = offset_abs + 3 * sizeof(uint32_t);
		uint32_t gd3_field_len = 0;
		unsigned int meta_type = TinyVGM_MetadataType_Title_EN;
		uint16_t data[4];
		uint32_t end_pos = cur_pos + metadata_len;

		while (1) {
			int32_t rc = tinyvgm_io_readall(ctx, buf, sizeof(data));
			data[0]=(uint_fast32_t)buf[0] | ((uint_fast32_t)buf[1] << 8);
			data[1]=(uint_fast32_t)buf[2] | ((uint_fast32_t)buf[3] << 8);
			data[2]=(uint_fast32_t)buf[4] | ((uint_fast32_t)buf[5] << 8);
			data[3]=(uint_fast32_t)buf[6] | ((uint_fast32_t)buf[7] << 8);

			if (rc > 0) {
				for (unsigned int i=0; i<(rc/2); i++) {
					if (buf[i]) {
						gd3_field_len += 2;
					} else {
						if (ctx->callback.metadata) {
							int rcc = ctx->callback.metadata(ctx->userp, meta_type, cur_pos, gd3_field_len);

							if (rcc != TinyVGM_OK) {
								return rcc;
							}
						}

						cur_pos += gd3_field_len + 2;
						gd3_field_len = 0;
						meta_type++;
					}
				}

				if (cur_pos >= end_pos) {
					break;
				}
			} else {
				return TinyVGM_EIO;
			}
		}

	}

	return TinyVGM_OK;
}

int tinyvgm_parse_commands(TinyVGMContext *ctx, uint32_t offset_abs) {
	if (tinyvgm_io_seek(ctx, offset_abs) != 0) {
		return TinyVGM_EIO;
	}

	uint32_t cur_pos = offset_abs;

	uint8_t buf[16];

	while (1) {
		uint8_t cmd;

		if (tinyvgm_io_read(ctx, &cmd, 1) != 1) {
			return TinyVGM_EIO;
		}

		if (cmd == 0x66) {
			return TinyVGM_OK;
		}

		int8_t cmd_val_len = vgm_cmd_length_table[cmd];

		if (cmd_val_len == -1) { // Unused
			fprintf(stderr, "tinyvgm_parse_commands: Unknown command 0x%x\n", cmd);
			return TinyVGM_EINVAL;
		} else if (cmd_val_len == -2) { // Data block
			if (tinyvgm_io_read(ctx, buf, 6) != 6) {
				return TinyVGM_EIO;
			}

			uint32_t pdblen = buf[2];
			pdblen |= ((uint32_t)buf[3] << 8);
			pdblen |= ((uint32_t)buf[4] << 16);
			pdblen |= ((uint32_t)buf[5] << 24);

			cur_pos += 1 + 6;

			if (ctx->callback.data_block) {
				int rcc = ctx->callback.data_block(ctx->userp, buf[1], cur_pos, pdblen);
				if (rcc != TinyVGM_OK) {
					return rcc;
				}
			}

			cur_pos += pdblen;

			if (tinyvgm_io_seek(ctx, cur_pos) != 0) {
				return TinyVGM_EIO;
			}
		} else { // Ordinary commands
			if (cmd_val_len) {
				if (tinyvgm_io_read(ctx, buf, cmd_val_len) != cmd_val_len) {
					return TinyVGM_EIO;
				}
			}

			int rcc = ctx->callback.command(ctx->userp, cmd, buf, cmd_val_len);
			if (rcc != TinyVGM_OK) {
				return rcc;
			}

			cur_pos += 1 + cmd_val_len;
		}

	}

}
