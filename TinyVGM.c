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

#ifdef __cplusplus
extern "C" {
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
	2,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	// A0 - AF
	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	// B0 - BF
	3,	3,	3,	3,	3,	3,	3,	3,	3,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	// C0 - CF
	3,	3,	3,	3,	3,	3,	3,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	// D0 - DF
	4,	4,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	// E0 - EF
	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	-1,	// F0 - FF
};

void tinyvgm_add_callback(TinyVGMContext *ctx, TinyVGMCallbackType callback_type, uint8_t id, int (*callback)(void *, uint8_t, const void *, uint32_t), void *userp) {
	if (!ctx->callbacks[callback_type]) {
		ctx->callbacks[callback_type] = (TinyVGMCallbackInfo *)(malloc(1));
	}

	ctx->callbacks[callback_type] = (TinyVGMCallbackInfo *)realloc(ctx->callbacks[callback_type], (ctx->nr_callbacks[callback_type] + 1) * sizeof(TinyVGMCallbackInfo));
	TinyVGMCallbackInfo *newcb = &ctx->callbacks[callback_type][ctx->nr_callbacks[callback_type]];
	newcb->id = id;
	newcb->callback = callback;
	newcb->userp = userp;
	ctx->nr_callbacks[callback_type]++;
}

void tinyvgm_add_event_callback(TinyVGMContext *ctx, TinyVGMEvent event, int (*callback)(void *, uint8_t, const void *, uint32_t), void *userp) {
	tinyvgm_add_callback(ctx, TinyVGM_CallBackType_Event, event, callback, userp);
}

void tinyvgm_add_header_callback(TinyVGMContext *ctx, uint8_t header_offset, int (*callback)(void *, uint8_t, const void *, uint32_t), void *userp) {
	tinyvgm_add_callback(ctx, TinyVGM_CallBackType_Header, header_offset, callback, userp);
}

void tinyvgm_add_command_callback(TinyVGMContext *ctx, uint8_t command, int (*callback)(void *, uint8_t, const void *, uint32_t), void *userp) {
	tinyvgm_add_callback(ctx, TinyVGM_CallBackType_Command, command, callback, userp);
}

int tinyvgm_invoke_callback(TinyVGMContext *ctx, TinyVGMCallbackType callback_type, uint8_t value, const void *buf, uint32_t len) {
//	printf("tinyvgm_invoke_callback: ctx=%p, callback_type=%u, value=0x%02x, buf=%p, len=%" PRIu32 "\n", ctx, callback_type, value, buf, len);

	TinyVGMCallbackInfo *cbs = ctx->callbacks[callback_type];
	uint8_t nr_cbs = ctx->nr_callbacks[callback_type];

	if (callback_type > 2) {
		fprintf(stderr, "tinyvgm FATAL: invalid callback type: %d, check your code!\n", callback_type);
		abort();
	}

	for (uint8_t i = 0; i < nr_cbs; i++) {
		TinyVGMCallbackInfo *cur_cb = &cbs[i];
		if (cur_cb->id == value) {
			return cur_cb->callback(cur_cb->userp, value, buf, len);
		}
	}

	return TinyVGM_OK;
}

static int tinyvgm_buffer_push(TinyVGMContext *ctx, uint8_t data) {
	ctx->buffer[ctx->buffer_pos] = data;
	ctx->buffer_pos++;
	return ctx->buffer_pos;
}

static void tinyvgm_buffer_clear(TinyVGMContext *ctx) {
	ctx->buffer_pos = 0;
	ctx->current_command = 0;
}

size_t tinyvgm_strlen16(const int16_t* strarg) {
	if (!strarg)
		return -1; //strarg is NULL pointer
	const int16_t* str = strarg;
	while(*str) {
		str++;
	}
	return str - strarg;
}

void tinyvgm_init(TinyVGMContext *ctx) {
	memset(ctx, 0, sizeof(TinyVGMContext));
	ctx->header_info.data_offset = 0x00000040;
}

void tinyvgm_reset(TinyVGMContext *ctx) {
	ctx->read_bytes = 0;
	ctx->buffer_pos = 0;
	ctx->current_command = 0;

	memset(&ctx->header_info, 0, sizeof(TinyVGMHeaderInfo));
	ctx->header_info.data_offset = 0x00000040;
}

void tinyvgm_destroy(TinyVGMContext *ctx) {
	for (int i = 0; i < (sizeof(ctx->callbacks) / sizeof(TinyVGMCallbackInfo *)); i++) {
		if (ctx->callbacks[i]) {
			free(ctx->callbacks[i]);
		}
	}
}

int32_t tinyvgm_parse(TinyVGMContext *ctx, const void *buf, uint16_t len) {
	for (uint16_t i = 0; i < len; i++) {
		uint8_t cur_byte = ((uint8_t *) buf)[i];

		// TODO Header Len Chk
		/*
		typedef struct timyvgm_header_info {
			uint32_t eof_offset;	// 0x04 - 0x07
			uint32_t version;	// 0x08 - 0x11
			uint32_t gd3_offset;	// 0x14 - 0x17
			uint32_t total_samples;	// 0x18 - 0x1b
			uint32_t loop_offset;	// 0x1c - 0x1f
			uint32_t loop_samples;	// 0x20 - 0x23
			uint32_t rate;		// 0x24 - 0x27
			uint32_t data_offset;	// 0x34 - 0x37
		} TinyVGMHeaderInfo;
		 */
		if (ctx->read_bytes < ctx->header_info.data_offset) {
			if (tinyvgm_buffer_push(ctx, cur_byte) == 4) {
				if (ctx->read_bytes == 0x03) {
					static const char hdr[] = "Vgm ";
					if (memcmp(ctx->buffer, hdr, 4) != 0) {
						return -1;
					}
				} else if (ctx->read_bytes == 0x07) {
					memcpy(&ctx->header_info.eof_offset, ctx->buffer, 4);
				} else if (ctx->read_bytes == 0x11) {
					// TODO:
					// 1. Parse BCD-version
					memcpy(&ctx->header_info.version, ctx->buffer, 4);
				} else if (ctx->read_bytes == 0x17) {
					memcpy(&ctx->header_info.gd3_offset, ctx->buffer, 4);
					ctx->header_info.gd3_offset += 0x14;
				} else if (ctx->read_bytes == 0x1b) {
					memcpy(&ctx->header_info.total_samples, ctx->buffer, 4);
				} else if (ctx->read_bytes == 0x1f) {
					memcpy(&ctx->header_info.loop_offset, ctx->buffer, 4);
					ctx->header_info.loop_offset += 0x1c;
				} else if (ctx->read_bytes == 0x23) {
					memcpy(&ctx->header_info.loop_samples, ctx->buffer, 4);
				} else if (ctx->read_bytes == 0x27) {
					memcpy(&ctx->header_info.rate, ctx->buffer, 4);
				} else if (ctx->read_bytes == 0x37) {
					uint32_t data_offset = *((uint32_t *) ctx->buffer);

					// TODO:
					// 1. Check version first?
					if (data_offset != 0 && data_offset != 0x0000000c) {
						memcpy(&ctx->header_info.data_offset, ctx->buffer, 4);
						ctx->header_info.data_offset += 0x34;
					}
				}

				tinyvgm_invoke_callback(ctx, TinyVGM_CallBackType_Header, ctx->read_bytes - 3, ctx->buffer, 4);
				tinyvgm_buffer_clear(ctx);
			}

			if (ctx->read_bytes == (ctx->header_info.data_offset - 1)) {
				tinyvgm_invoke_callback(ctx, TinyVGM_CallBackType_Event, TinyVGM_Event_HeaderParseDone, NULL, 0);
			}
		} else {
			if (!ctx->current_command) {
				if (vgm_cmd_length_table[cur_byte] == 0) {
					if (TinyVGM_OK != tinyvgm_invoke_callback(ctx, TinyVGM_CallBackType_Command, cur_byte, NULL, 0)) {
						return i;
					}
				} else if (vgm_cmd_length_table[cur_byte] == -1) {
					// Do nothing
					printf("FATAL: Unknown command: 0x%02x\n", cur_byte);
//					abort();
				} else if (vgm_cmd_length_table[cur_byte] == -2) {
					// TODO
				} else {
					ctx->current_command = cur_byte;
				}
			} else {
				if (tinyvgm_buffer_push(ctx, cur_byte) == vgm_cmd_length_table[ctx->current_command]) {
					int rc = tinyvgm_invoke_callback(ctx, TinyVGM_CallBackType_Command, ctx->current_command, ctx->buffer, ctx->buffer_pos);

					if (rc == TinyVGM_OK) {
						tinyvgm_buffer_clear(ctx);
					} else {
						int32_t deducted_offset = vgm_cmd_length_table[ctx->current_command] - 1;
						ctx->read_bytes -= deducted_offset;
						tinyvgm_buffer_clear(ctx);
						return i - deducted_offset;
					}
				}
			}
		}

		if (ctx->read_bytes == ctx->header_info.gd3_offset - 1) {
			tinyvgm_invoke_callback(ctx, TinyVGM_CallBackType_Event, TinyVGM_Event_PlaybackDone, NULL, 0);
			return i;
		}

		ctx->read_bytes++;
	}

	return len;
}

void tinyvgm_init_gd3(TinyVGMGd3Info *ctx) {
	memset(ctx, 0, sizeof(TinyVGMGd3Info));
}

void tinyvgm_destroy_gd3(TinyVGMGd3Info *ctx) {
	if (ctx->buf) {
		free(ctx->buf);
		memset(ctx, 0, sizeof(TinyVGMGd3Info));
	}
}

int32_t tinyvgm_parse_gd3(TinyVGMGd3Info *ctx, const void *buf, uint16_t len) {
	uint16_t read_len = 0;

	while (read_len < len) {
		if (ctx->__read_bytes < 8) {
			read_len += 1;
			ctx->__read_bytes += 1;
		} else if (ctx->__read_bytes >= 8 && ctx->__read_bytes < 12) {
			((uint8_t*)&ctx->__total_length)[ctx->__read_bytes - 8] = *((uint8_t*)buf + read_len);;
			read_len += 1;
			ctx->__read_bytes += 1;

			if (ctx->__read_bytes == 12) {
				ctx->buf = (int16_t *)malloc(ctx->__total_length);
			}
		} else if (ctx->__read_bytes >= 12 && ctx->__read_bytes - 12 < ctx->__total_length) {
			uint16_t cpy_length = len - read_len;
			if (ctx->__read_bytes - 12 + cpy_length > ctx->__total_length) {
				cpy_length -= ctx->__read_bytes - 12 + cpy_length - ctx->__total_length;
			}
			memcpy((uint8_t *)ctx->buf + (ctx->__read_bytes - 12), ((uint8_t*)buf + read_len), cpy_length);
			read_len += cpy_length;
			ctx->__read_bytes += cpy_length;

			if (ctx->__read_bytes - 12 == ctx->__total_length) {
				ctx->title = ctx->buf;
				ctx->title_jp = ctx->title + tinyvgm_strlen16(ctx->title) + 1;
				ctx->album = ctx->title_jp + tinyvgm_strlen16(ctx->title_jp) + 1;
				ctx->album_jp = ctx->album + tinyvgm_strlen16(ctx->album) + 1;
				ctx->system_name = ctx->album_jp + tinyvgm_strlen16(ctx->album_jp) + 1;
				ctx->system_name_jp = ctx->system_name + tinyvgm_strlen16(ctx->system_name) + 1;
				ctx->composer = ctx->system_name_jp + tinyvgm_strlen16(ctx->system_name_jp) + 1;
				ctx->composer_jp = ctx->composer + tinyvgm_strlen16(ctx->composer) + 1;
				ctx->release_date = ctx->composer_jp + tinyvgm_strlen16(ctx->composer_jp) + 1;
				ctx->converter = ctx->release_date + tinyvgm_strlen16(ctx->release_date) + 1;
				ctx->note = ctx->converter + tinyvgm_strlen16(ctx->converter) + 1;
			}
		} else {
			return read_len;
		}
	}

	return read_len;
}

#ifdef __cplusplus
};
#endif