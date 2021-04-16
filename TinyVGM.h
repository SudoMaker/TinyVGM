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

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	TinyVGM_OK = 0,
	TinyVGM_NO = -1,
} TinyVGMReturn;

typedef enum {
	TinyVGM_Event_HeaderParseDone = 1,
	TinyVGM_Event_PlaybackDone = 2
} TinyVGMEvent;

typedef enum {
	TinyVGM_CallBackType_Command = 0,
	TinyVGM_CallBackType_Header = 1,
	TinyVGM_CallBackType_Event = 2
} TinyVGMCallbackType;

typedef struct tinyvgm_gd3_info {
	uint32_t __read_bytes;
	uint32_t __total_length;

	int16_t* title;
	int16_t* title_jp;
	int16_t* album;
	int16_t* album_jp;
	int16_t* system_name;
	int16_t* system_name_jp;
	int16_t* composer;
	int16_t* composer_jp;
	int16_t* release_date;
	int16_t* converter;
	int16_t* note;

	int16_t* buf;
} TinyVGMGd3Info;

typedef struct tinyvgm_callback_info {
	uint8_t id;
	void *userp;

	int (*callback)(void *, uint8_t, const void *, uint32_t);
} TinyVGMCallbackInfo;

typedef struct timyvgm_header_info {
	uint32_t eof_offset;	// 0x04
	uint32_t version;	// 0x08
	uint32_t gd3_offset;	// 0x14
	uint32_t total_samples;	// 0x18
	uint32_t loop_offset;	// 0x1c
	uint32_t loop_samples;	// 0x20
	uint32_t rate;		// 0x24
	uint32_t data_offset;	// 0x34
} TinyVGMHeaderInfo;

typedef struct tinyvgm_context {
	TinyVGMCallbackInfo *callbacks[3];
	uint8_t nr_callbacks[3];

	TinyVGMHeaderInfo header_info;

	uint32_t read_bytes;

	uint8_t current_command;

	uint8_t buffer[8];
	uint8_t buffer_pos;
} TinyVGMContext;

/**
 * Calculate the length of a 16-bit, 0x0000-terminated string.
 *
 * @param strarg		The 16-bit string.
 *
 * @return			Length of the string.
 */
extern size_t tinyvgm_strlen16(const int16_t* strarg);

/**
 * Initialize a TinyVGM context.
 *
 * @param ctx			TinyVGM context pointer.
 *
 */
extern void tinyvgm_init(TinyVGMContext *ctx);

/**
 * Reset a TinyVGM context, to make it able to parse a new VGM stream.
 *
 * @param ctx			TinyVGM context pointer.
 *
 */
extern void tinyvgm_reset(TinyVGMContext *ctx);

/**
 * Destroy a TinyVGM context, frees all runtime allocated memory.
 *
 * @param ctx			TinyVGM context pointer.
 *
 */
extern void tinyvgm_destroy(TinyVGMContext *ctx);

/**
 * Parse a portion of VGM stream.
 *
 * @param ctx			TinyVGM context pointer.
 * @param buf			Buffer of the VGM stream.
 * @param len			Length of buffer.
 *
 * @return			Length of successfully processed bytes. If a callback fails, negated bytes drained from buffer will be returned. If the VGM stream is invalid, INT32_MIN will be returned.
 *
 * When a callback fails, you can get the command in TinyVGMContext::current_command and its params in TinyVGMContext::buffer, if needed.
 *
 */
extern int32_t tinyvgm_parse(TinyVGMContext *ctx, const void *buf, uint16_t len);

/**
 * Add an event callback.
 *
 * @param ctx			TinyVGM context pointer.
 * @param event			Event. See `TinyVGMEvent' type for available event types.
 * @param callback		Callback function pointer.
 * @param userp			User data pointer to supply as the first argument of the callback function. Can be a C++ class pointer or NULL.
 *
 */
extern void tinyvgm_add_event_callback(TinyVGMContext *ctx, TinyVGMEvent event, int (*callback)(void *, uint8_t, const void *, uint32_t), void *userp);

/**
 * Add a header callback.
 *
 * @param ctx			TinyVGM context pointer.
 * @param header_offset		Header offset. e.g. 0x08 for `Version number'.
 * @param callback		Callback function pointer.
 * @param userp			User data pointer to supply as the first argument of the callback function. Can be a C++ class pointer or NULL.
 *
 */
extern void tinyvgm_add_header_callback(TinyVGMContext *ctx, uint8_t header_offset, int (*callback)(void *, uint8_t, const void *, uint32_t), void *userp);

/**
 * Add a command callback.
 *
 * @param ctx			TinyVGM context pointer.
 * @param command		Command. e.g. 0x50 for `SN76489', 0x5e for `OPL3 Port0'.
 * @param callback		Callback function pointer.
 * @param userp			User data pointer to supply as the first argument of the callback function. Can be a C++ class pointer or NULL.
 *
 */
extern void tinyvgm_add_command_callback(TinyVGMContext *ctx, uint8_t command, int (*callback)(void *, uint8_t, const void *, uint32_t), void *userp);

/**
 * Initialize a TinyVGM GD3 context.
 *
 * @param ctx			TinyVGM GD3 context pointer.
 *
 */
extern void tinyvgm_init_gd3(TinyVGMGd3Info *ctx);

/**
 * Destroy a TinyVGM GD3 context, frees all runtime allocated memory.
 *
 * @param ctx			TinyVGM GD3 context pointer.
 *
 */
extern void tinyvgm_destroy_gd3(TinyVGMGd3Info *ctx);

/**
 * Parse a portion of GD3 stream.
 *
 * No data validation will be performed at this time.
 *
 * @param ctx			TinyVGM GD3 context pointer.
 * @param buf			Buffer of the GD3 stream.
 * @param len			Length of buffer.
 *
 * @return			Length of successfully processed bytes.
 *
 */
extern int32_t tinyvgm_parse_gd3(TinyVGMGd3Info *ctx, const void *buf, uint16_t len);

#ifdef __cplusplus
};
#endif