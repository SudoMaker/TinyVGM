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
	TinyVGM_FAIL = -1,
	TinyVGM_EIO = -2,
	TinyVGM_ECANCELED = -3,
	TinyVGM_EINVAL = -4,
} TinyVGMReturn;

typedef enum {
	// 0x00
	TinyVGM_HeaderField_Identity = 0,
	TinyVGM_HeaderField_EoF_Offset,
	TinyVGM_HeaderField_Version,
	TinyVGM_HeaderField_SN76489_Clock,

	// 0x10
	TinyVGM_HeaderField_YM2413_Clock,
	TinyVGM_HeaderField_GD3_Offset,
	TinyVGM_HeaderField_Total_Samples,
	TinyVGM_HeaderField_Loop_Offset,

	// 0x20
	TinyVGM_HeaderField_Loop_Samples,
	TinyVGM_HeaderField_Rate,
	TinyVGM_HeaderField_SN_Config,
	TinyVGM_HeaderField_YM2612_Clock,

	// 0x30
	TinyVGM_HeaderField_YM2151_Clock,
	TinyVGM_HeaderField_Data_Offset,
	TinyVGM_HeaderField_SegaPCM_Clock,
	TinyVGM_HeaderField_SPCM_Interface,

	// 0x40
	TinyVGM_HeaderField_RF5C68_Clock,
	TinyVGM_HeaderField_YM2203_Clock,
	TinyVGM_HeaderField_YM2608_Clock,
	TinyVGM_HeaderField_YM2610_Clock,

	// 0x50
	TinyVGM_HeaderField_YM3812_Clock,
	TinyVGM_HeaderField_YM3526_Clock,
	TinyVGM_HeaderField_Y8950_Clock,
	TinyVGM_HeaderField_YMF262_Clock,

	// 0x60
	TinyVGM_HeaderField_YMF278B_Clock,
	TinyVGM_HeaderField_YMF271_Clock,
	TinyVGM_HeaderField_YMZ280B_Clock,
	TinyVGM_HeaderField_RF5C164_Clock,

	// 0x70
	TinyVGM_HeaderField_PWM_Clock,
	TinyVGM_HeaderField_AY8910_Clock,
	TinyVGM_HeaderField_AY_Config,
	TinyVGM_HeaderField_Playback_Config,

	// 0x80
	TinyVGM_HeaderField_GBDMG_Clock,
	TinyVGM_HeaderField_NESAPU_Clock,
	TinyVGM_HeaderField_MultiPCM_Clock,
	TinyVGM_HeaderField_uPD7759_Clock,

	// 0x90
	TinyVGM_HeaderField_OKIM6258_Clock,
	TinyVGM_HeaderField_ArcadeChips_Config,
	TinyVGM_HeaderField_OKIM6295_Clock,
	TinyVGM_HeaderField_K051649_Clock,

	// 0xa0
	TinyVGM_HeaderField_K054539_Clock,
	TinyVGM_HeaderField_HuC6280_Clock,
	TinyVGM_HeaderField_C140_Clock,
	TinyVGM_HeaderField_K053260_Clock,

	// 0xb0
	TinyVGM_HeaderField_Pokey_Clock,
	TinyVGM_HeaderField_QSound_Clock,
	TinyVGM_HeaderField_SCSP_Clock,
	TinyVGM_HeaderField_ExtraHeader_Offset,

	// 0xc0
	TinyVGM_HeaderField_WonderSwan_Clock,
	TinyVGM_HeaderField_VSU_Clock,
	TinyVGM_HeaderField_SAA1099_Clock,
	TinyVGM_HeaderField_ES5503_Clock,

	// 0xd0
	TinyVGM_HeaderField_ES5506_Clock,
	TinyVGM_HeaderField_ES_Config,
	TinyVGM_HeaderField_X1010_Clock,
	TinyVGM_HeaderField_C352_Clock,

	// 0xe0
	TinyVGM_HeaderField_GA20_Clock,

	TinyVGM_HeaderField_MAX

} TinyVGMHeaderField;

typedef enum {
	TinyVGM_MetadataType_Title_EN = 0,
	TinyVGM_MetadataType_Title,
	TinyVGM_MetadataType_Album_EN,
	TinyVGM_MetadataType_Album,
	TinyVGM_MetadataType_SystemName_EN,
	TinyVGM_MetadataType_SystemName,
	TinyVGM_MetadataType_Composer_EN,
	TinyVGM_MetadataType_Composer,
	TinyVGM_MetadataType_ReleaseDate,
	TinyVGM_MetadataType_Converter,
	TinyVGM_MetadataType_Notes,

	TinyVGM_MetadataType_MAX
} TinyVGMMetadataType;

typedef struct tinyvgm_context {
	/*! Callbacks */
	struct {
		/*! Header callback. Params: user pointer, header field, header value */
		int (*header)(void *, TinyVGMHeaderField, uint32_t);

		/*! Metadata callback. Params: user pointer, metadata type, file offset, length */
		int (*metadata)(void *, TinyVGMMetadataType, uint32_t, uint32_t);

		/*! Command callback. Params: user pointer, command, command params, length */
		int (*command)(void *, unsigned int, const void *, uint32_t);

		/*! DataBlock callback. Params: user pointer, data block type, file offset, length */
		int (*data_block)(void *, unsigned int, uint32_t, uint32_t);

		/*! Read callback. Params: user pointer, buffer, length */
		int32_t (*read)(void *, uint8_t *, uint32_t);

		/*! Seek callback. Params: user pointer, file offset */
		int (*seek)(void *, uint32_t);
	} callback;

	/*! User pointer */
	void *userp;
} TinyVGMContext;

/**
 * Get absolute offset of a header item.
 *
 * @param x			Header item enum.
 *
 * @return			Offset
 *
 *
 */
#define tinyvgm_headerfield_offset(x)	((x) * sizeof(uint32_t))

/**
 * Parse the VGM header.
 *
 * @param ctx			TinyVGM context pointer.
 *
 * @return			TinyVGM_OK for success. Errors are reported accordingly.
 *
 *
 */
extern int tinyvgm_parse_header(TinyVGMContext *ctx);

/**
 * Parse the VGM metadata (GD3).
 *
 * @param ctx			TinyVGM context pointer.
 * @param offset_abs		Absolute offset of data in file.
 *
 * @return			TinyVGM_OK for success. Errors are reported accordingly.
 *
 *
 */
extern int tinyvgm_parse_metadata(TinyVGMContext *ctx, uint32_t offset_abs);

/**
 * Parse the VGM commands (incl. data blocks).
 *
 * @param ctx			TinyVGM context pointer.
 * @param offset_abs		Absolute offset of data in file.
 *
 * @return			TinyVGM_OK for success. Errors are reported accordingly.
 *
 *
 */
extern int tinyvgm_parse_commands(TinyVGMContext *ctx, uint32_t offset_abs);

#ifdef __cplusplus
};
#endif
