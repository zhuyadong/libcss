/*
 * This file is part of LibCSS.
 * Licensed under the MIT License,
 *                http://www.opensource.org/licenses/mit-license.php
 * Copyright 2008 John-Mark Bell <jmb@netsurf-browser.org>
 */

#include <assert.h>
#include <string.h>

#include "bytecode/bytecode.h"
#include "bytecode/opcodes.h"
#include "parse/properties/properties.h"
#include "parse/properties/utils.h"

/**
 * Dispatch table of property handlers, indexed by property enum
 */
const css_prop_handler property_handlers[LAST_PROP + 1 - FIRST_PROP] =
{
	parse_azimuth,
	parse_background_attachment,
	parse_background_color,
	parse_background_image,
	parse_background_position,
	parse_background_repeat,
	parse_border_bottom_color,
	parse_border_bottom_style,
	parse_border_bottom_width,
	parse_border_collapse,
	parse_border_left_color,
	parse_border_left_style,
	parse_border_left_width,
	parse_border_right_color,
	parse_border_right_style,
	parse_border_right_width,
	parse_border_spacing,
	parse_border_top_color,
	parse_border_top_style,
	parse_border_top_width,
	parse_bottom,
	parse_caption_side,
	parse_clear,
	parse_clip,
	parse_color,
	parse_content,
	parse_counter_increment,
	parse_counter_reset,
	parse_cue_after,
	parse_cue_before,
	parse_cursor,
	parse_direction,
	parse_display,
	parse_elevation,
	parse_empty_cells,
	parse_float,
	parse_font_family,
	parse_font_size,
	parse_font_style,
	parse_font_variant,
	parse_font_weight,
	parse_height,
	parse_left,
	parse_letter_spacing,
	parse_line_height,
	parse_list_style_image,
	parse_list_style_position,
	parse_list_style_type,
	parse_margin_bottom,
	parse_margin_left,
	parse_margin_right,
	parse_margin_top,
	parse_max_height,
	parse_max_width,
	parse_min_height,
	parse_min_width,
	parse_orphans,
	parse_outline_color,
	parse_outline_style,
	parse_outline_width,
	parse_overflow,
	parse_padding_bottom,
	parse_padding_left,
	parse_padding_right,
	parse_padding_top,
	parse_page_break_after,
	parse_page_break_before,
	parse_page_break_inside,
	parse_pause_after,
	parse_pause_before,
	parse_pitch_range,
	parse_pitch,
	parse_play_during,
	parse_position,
	parse_quotes,
	parse_richness,
	parse_right,
	parse_speak_header,
	parse_speak_numeral,
	parse_speak_punctuation,
	parse_speak,
	parse_speech_rate,
	parse_stress,
	parse_table_layout,
	parse_text_align,
	parse_text_decoration,
	parse_text_indent,
	parse_text_transform,
	parse_top,
	parse_unicode_bidi,
	parse_vertical_align,
	parse_visibility,
	parse_voice_family,
	parse_volume,
	parse_white_space,
	parse_widows,
	parse_width,
	parse_word_spacing,
	parse_z_index,
};

css_error parse_caption_side(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *ident;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;

	/* IDENT (top, bottom, inherit) */
	ident = parserutils_vector_iterate(vector, ctx);
	if (ident == NULL || ident->type != CSS_TOKEN_IDENT)
		return CSS_INVALID;

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	if (ident->ilower == c->strings[INHERIT]) {
		flags |= FLAG_INHERIT;
	} else if (ident->ilower == c->strings[TOP]) {
		value = CAPTION_SIDE_TOP;
	} else if (ident->ilower == c->strings[BOTTOM]) {
		value = CAPTION_SIDE_BOTTOM;
	} else
		return CSS_INVALID;

	opv = buildOPV(CSS_PROP_CAPTION_SIDE, flags, value);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, sizeof(opv), result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));

	return CSS_OK;
}

css_error parse_clear(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *ident;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;

	/* IDENT (left, right, both, none, inherit) */
	ident = parserutils_vector_iterate(vector, ctx);
	if (ident == NULL || ident->type != CSS_TOKEN_IDENT)
		return CSS_INVALID;

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	if (ident->ilower == c->strings[INHERIT]) {
		flags |= FLAG_INHERIT;
	} else if (ident->ilower == c->strings[RIGHT]) {
		value = CLEAR_RIGHT;
	} else if (ident->ilower == c->strings[LEFT]) {
		value = CLEAR_LEFT;
	} else if (ident->ilower == c->strings[BOTH]) {
		value = CLEAR_BOTH;
	} else if (ident->ilower == c->strings[NONE]) {
		value = CLEAR_NONE;
	} else
		return CSS_INVALID;

	opv = buildOPV(CSS_PROP_CLEAR, flags, value);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, sizeof(opv), result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));

	return CSS_OK;
}

css_error parse_clip(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *token;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;
	int num_lengths = 0;
	css_fixed length[4] = { 0 };
	uint32_t unit[4] = { 0 };
	uint32_t required_size;

	/* FUNCTION(rect) [ [ IDENT(auto) | length ] CHAR(,)? ]{3} 
	 *                [ IDENT(auto) | length ] CHAR{)} |
	 * IDENT(auto, inherit) */
	token = parserutils_vector_iterate(vector, ctx);
	if (token == NULL)
		return CSS_INVALID;

	if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[INHERIT]) {
		flags = FLAG_INHERIT;
	} else if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[AUTO]) {
		value = CLIP_AUTO;
	} else if (token->type == CSS_TOKEN_FUNCTION &&
			token->ilower == c->strings[RECT]) {
		int i;
		value = CLIP_SHAPE_RECT;

		for (i = 0; i < 4; i++) {
			consumeWhitespace(vector, ctx);

			token = parserutils_vector_peek(vector, *ctx);
			if (token == NULL)
				return CSS_INVALID;

			if (token->type == CSS_TOKEN_IDENT) {
				/* Slightly magical way of generating the auto 
				 * values. These are bits 3-6 of the value. */
				if (token->ilower == c->strings[AUTO])
					value |= 1 << (i + 3);
				else
					return CSS_INVALID;

				parserutils_vector_iterate(vector, ctx);
			} else {
				error = parse_unit_specifier(c, vector, ctx, 
						UNIT_PX, 
						&length[num_lengths], 
						&unit[num_lengths]);
				if (error != CSS_OK)
					return error;

				if (unit[num_lengths] & UNIT_ANGLE || 
						unit[num_lengths] & UNIT_TIME ||
						unit[num_lengths] & UNIT_FREQ ||
						unit[num_lengths] & UNIT_PCT)
					return CSS_INVALID;

				num_lengths++;
			}

			consumeWhitespace(vector, ctx);

			/* Consume optional comma after first 3 parameters */
			if (i < 3) {
				token = parserutils_vector_peek(vector, *ctx);
				if (token == NULL)
					return CSS_INVALID;

				if (tokenIsChar(token, ','))
					parserutils_vector_iterate(vector, ctx);
			}
		}

		consumeWhitespace(vector, ctx);

		/* Finally, consume closing parenthesis */
		token = parserutils_vector_iterate(vector, ctx);
		if (token == NULL || tokenIsChar(token, ')') == false)
			return CSS_INVALID;
	} else {
		return CSS_INVALID;
	}

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	opv = buildOPV(CSS_PROP_CLIP, flags, value);

	required_size = sizeof(opv);
	if ((flags & FLAG_INHERIT) == false && 
			(value & CLIP_SHAPE_MASK) == CLIP_SHAPE_RECT) {
		required_size += 
			num_lengths * (sizeof(length[0]) + sizeof(unit[0]));
	}

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, required_size, result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));
	if ((flags & FLAG_INHERIT) == false && 
			(value & CLIP_SHAPE_MASK) == CLIP_SHAPE_RECT) {
		int i;
		uint8_t *ptr = ((uint8_t *) (*result)->bytecode) + sizeof(opv);

		for (i = 0; i < num_lengths; i++) {
			memcpy(ptr, &length[i], sizeof(length[i]));
			ptr += sizeof(length[i]);
			memcpy(ptr, &unit[i], sizeof(unit[i]));
			ptr += sizeof(unit[i]);
		}
	}

	return CSS_OK;
}

css_error parse_color(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *token;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;
	uint32_t colour = 0;
	uint32_t required_size;

	/* colour | IDENT (inherit) */
	token= parserutils_vector_peek(vector, *ctx);
	if (token == NULL)
		return CSS_INVALID;

	if (token->type == CSS_TOKEN_IDENT && 
			token->ilower == c->strings[INHERIT]) {
		parserutils_vector_iterate(vector, ctx);
		flags |= FLAG_INHERIT;
	} else {
		error = parse_colour_specifier(c, vector, ctx, &colour);
		if (error != CSS_OK)
			return error;

		value = COLOR_SET;
	}

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	opv = buildOPV(CSS_PROP_COLOR, flags, value);

	required_size = sizeof(opv);
	if ((flags & FLAG_INHERIT) == false && value == COLOR_SET)
		required_size += sizeof(colour);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, required_size, result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));
	if ((flags & FLAG_INHERIT) == false && value == COLOR_SET) {
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv),
				&colour, sizeof(colour));
	}

	return CSS_OK;
}

css_error parse_cursor(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *token;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;
	uint32_t required_size = sizeof(opv);
	int temp_ctx = *ctx;
	uint8_t *ptr;

	/* [ (URI ',')* IDENT(auto, crosshair, default, pointer, move, e-resize,
	 *              ne-resize, nw-resize, n-resize, se-resize, sw-resize,
	 *              s-resize, w-resize, text, wait, help, progress) ] 
	 * | IDENT(inherit) 
	 */

	/* Pass 1: validate input and calculate bytecode size */
	token = parserutils_vector_iterate(vector, &temp_ctx);
	if (token == NULL || (token->type != CSS_TOKEN_IDENT &&
			token->type != CSS_TOKEN_URI))
		return CSS_INVALID;

	if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[INHERIT]) {
		flags = FLAG_INHERIT;
	} else {
		bool first = true;

		/* URI* */
		while (token != NULL && token->type == CSS_TOKEN_URI) {
			lwc_string *uri = token->idata;

			if (first == false) {
				required_size += sizeof(opv);
			} else {
				value = CURSOR_URI;
			}
			required_size += sizeof(uri);

			consumeWhitespace(vector, &temp_ctx);

			/* Expect ',' */
			token = parserutils_vector_iterate(vector, &temp_ctx);
			if (token == NULL || tokenIsChar(token, ',') == false)
				return CSS_INVALID;

			consumeWhitespace(vector, &temp_ctx);

			/* Expect either URI or IDENT */
			token = parserutils_vector_iterate(vector, &temp_ctx);
			if (token == NULL || (token->type != CSS_TOKEN_IDENT &&
					token->type != CSS_TOKEN_URI))
				return CSS_INVALID;

			first = false;
		}

		/* IDENT */
		if (token != NULL && token->type == CSS_TOKEN_IDENT) {
			if (token->ilower == c->strings[AUTO]) {
				if (first) {
					value = CURSOR_AUTO;
				}
			} else if (token->ilower == c->strings[CROSSHAIR]) {
				if (first) {
					value = CURSOR_CROSSHAIR;
				}
			} else if (token->ilower == c->strings[DEFAULT]) {
				if (first) {
					value = CURSOR_DEFAULT;
				}
			} else if (token->ilower == c->strings[POINTER]) {
				if (first) {
					value = CURSOR_POINTER;
				}
			} else if (token->ilower == c->strings[MOVE]) {
				if (first) {
					value = CURSOR_MOVE;
				}
			} else if (token->ilower == c->strings[E_RESIZE]) {
				if (first) {
					value = CURSOR_E_RESIZE;
				}
			} else if (token->ilower == c->strings[NE_RESIZE]) {
				if (first) {
					value = CURSOR_NE_RESIZE;
				}
			} else if (token->ilower == c->strings[NW_RESIZE]) {
				if (first) {
					value = CURSOR_NW_RESIZE;
				}
			} else if (token->ilower == c->strings[N_RESIZE]) {
				if (first) {
					value = CURSOR_N_RESIZE;
				}
			} else if (token->ilower == c->strings[SE_RESIZE]) {
				if (first) {
					value = CURSOR_SE_RESIZE;
				}
			} else if (token->ilower == c->strings[SW_RESIZE]) {
				if (first) {
					value = CURSOR_SW_RESIZE;
				}
			} else if (token->ilower == c->strings[S_RESIZE]) {
				if (first) {
					value = CURSOR_S_RESIZE;
				}
			} else if (token->ilower == c->strings[W_RESIZE]) {
				if (first) {
					value = CURSOR_W_RESIZE;
				}
			} else if (token->ilower == c->strings[TEXT]) {
				if (first) {
					value = CURSOR_TEXT;
				}
			} else if (token->ilower == c->strings[WAIT]) {
				if (first) {
					value = CURSOR_WAIT;
				}
			} else if (token->ilower == c->strings[HELP]) {
				if (first) {
					value = CURSOR_HELP;
				}
			} else if (token->ilower == c->strings[PROGRESS]) {
				if (first) {
					value = CURSOR_PROGRESS;
				}
			} else {
				return CSS_INVALID;
			}

			if (first == false) {
				required_size += sizeof(opv);
			}
		}

		consumeWhitespace(vector, &temp_ctx);

		token = parserutils_vector_peek(vector, temp_ctx);
		if (token != NULL && tokenIsChar(token, '!') == false)
			return CSS_INVALID;
	}

	error = parse_important(c, vector, &temp_ctx, &flags);
	if (error != CSS_OK)
		return error;

	opv = buildOPV(CSS_PROP_CURSOR, flags, value);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, required_size, result);
	if (error != CSS_OK)
		return error;

	/* Copy OPV to bytecode */
	ptr = (*result)->bytecode;
	memcpy(ptr, &opv, sizeof(opv));
	ptr += sizeof(opv);

	/* Pass 2: construct bytecode */
	token = parserutils_vector_iterate(vector, ctx);
	if (token == NULL || (token->type != CSS_TOKEN_IDENT &&
			token->type != CSS_TOKEN_URI))
		return CSS_INVALID;

	if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[INHERIT]) {
		/* Nothing to do */
	} else {
		bool first = true;

		/* URI* */
		while (token != NULL && token->type == CSS_TOKEN_URI) {
			lwc_string *uri = token->idata;

			if (first == false) {
				opv = CURSOR_URI;
				memcpy(ptr, &opv, sizeof(opv));
				ptr += sizeof(opv);
			}
                        
                        lwc_context_string_ref(c->sheet->dictionary, uri);
			memcpy(ptr, &uri, sizeof(uri));
			ptr += sizeof(uri);

			consumeWhitespace(vector, ctx);

			/* Expect ',' */
			token = parserutils_vector_iterate(vector, ctx);
			if (token == NULL || tokenIsChar(token, ',') == false)
				return CSS_INVALID;

			consumeWhitespace(vector, ctx);

			/* Expect either URI or IDENT */
			token = parserutils_vector_iterate(vector, ctx);
			if (token == NULL || (token->type != CSS_TOKEN_IDENT &&
					token->type != CSS_TOKEN_URI))
				return CSS_INVALID;

			first = false;
		}

		/* IDENT */
		if (token != NULL && token->type == CSS_TOKEN_IDENT) {
			if (token->ilower == c->strings[AUTO]) {
				opv = CURSOR_AUTO;
			} else if (token->ilower == c->strings[CROSSHAIR]) {
				opv = CURSOR_CROSSHAIR;
			} else if (token->ilower == c->strings[DEFAULT]) {
				opv = CURSOR_DEFAULT;
			} else if (token->ilower == c->strings[POINTER]) {
				opv = CURSOR_POINTER;
			} else if (token->ilower == c->strings[MOVE]) {
				opv = CURSOR_MOVE;
			} else if (token->ilower == c->strings[E_RESIZE]) {
				opv = CURSOR_E_RESIZE;
			} else if (token->ilower == c->strings[NE_RESIZE]) {
				opv = CURSOR_NE_RESIZE;
			} else if (token->ilower == c->strings[NW_RESIZE]) {
				opv = CURSOR_NW_RESIZE;
			} else if (token->ilower == c->strings[N_RESIZE]) {
				opv = CURSOR_N_RESIZE;
			} else if (token->ilower == c->strings[SE_RESIZE]) {
				opv = CURSOR_SE_RESIZE;
			} else if (token->ilower == c->strings[SW_RESIZE]) {
				opv = CURSOR_SW_RESIZE;
			} else if (token->ilower == c->strings[S_RESIZE]) {
				opv = CURSOR_S_RESIZE;
			} else if (token->ilower == c->strings[W_RESIZE]) {
				opv = CURSOR_W_RESIZE;
			} else if (token->ilower == c->strings[TEXT]) {
				opv = CURSOR_TEXT;
			} else if (token->ilower == c->strings[WAIT]) {
				opv = CURSOR_WAIT;
			} else if (token->ilower == c->strings[HELP]) {
				opv = CURSOR_HELP;
			} else if (token->ilower == c->strings[PROGRESS]) {
				opv = CURSOR_PROGRESS;
			} else {
				return CSS_INVALID;
			}

			if (first == false) {
				memcpy(ptr, &opv, sizeof(opv));
				ptr += sizeof(opv);
			}
		}

		consumeWhitespace(vector, ctx);

		token = parserutils_vector_peek(vector, *ctx);
		if (token != NULL && tokenIsChar(token, '!') == false)
			return CSS_INVALID;
	}

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	return CSS_OK;
}

css_error parse_direction(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *ident;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;

	/* IDENT (ltr, rtl, inherit) */
	ident = parserutils_vector_iterate(vector, ctx);
	if (ident == NULL || ident->type != CSS_TOKEN_IDENT)
		return CSS_INVALID;

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	if (ident->ilower == c->strings[INHERIT]) {
		flags |= FLAG_INHERIT;
	} else if (ident->ilower == c->strings[LTR]) {
		value = DIRECTION_LTR;
	} else if (ident->ilower == c->strings[RTL]) {
		value = DIRECTION_RTL;
	} else
		return CSS_INVALID;

	opv = buildOPV(CSS_PROP_DIRECTION, flags, value);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, sizeof(opv), result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));

	return CSS_OK;
}

css_error parse_display(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *ident;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;

	/* IDENT (inline, block, list-item, run-in, inline-block, table,
	 * inline-table, table-row-group, table-header-group, 
	 * table-footer-group, table-row, table-column-group, table-column,
	 * table-cell, table-caption, none, inherit) */
	ident = parserutils_vector_iterate(vector, ctx);
	if (ident == NULL || ident->type != CSS_TOKEN_IDENT)
		return CSS_INVALID;

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	if (ident->ilower == c->strings[INHERIT]) {
		flags |= FLAG_INHERIT;
	} else if (ident->ilower == c->strings[INLINE]) {
		value = DISPLAY_INLINE;
	} else if (ident->ilower == c->strings[BLOCK]) {
		value = DISPLAY_BLOCK;
	} else if (ident->ilower == c->strings[LIST_ITEM]) {
		value = DISPLAY_LIST_ITEM;
	} else if (ident->ilower == c->strings[RUN_IN]) {
		value = DISPLAY_RUN_IN;
	} else if (ident->ilower == c->strings[INLINE_BLOCK]) {
		value = DISPLAY_INLINE_BLOCK;
	} else if (ident->ilower == c->strings[TABLE]) {
		value = DISPLAY_TABLE;
	} else if (ident->ilower == c->strings[INLINE_TABLE]) {
		value = DISPLAY_INLINE_TABLE;
	} else if (ident->ilower == c->strings[TABLE_ROW_GROUP]) {
		value = DISPLAY_TABLE_ROW_GROUP;
	} else if (ident->ilower == c->strings[TABLE_HEADER_GROUP]) {
		value = DISPLAY_TABLE_HEADER_GROUP;
	} else if (ident->ilower == c->strings[TABLE_FOOTER_GROUP]) {
		value = DISPLAY_TABLE_FOOTER_GROUP;
	} else if (ident->ilower == c->strings[TABLE_ROW]) {
		value = DISPLAY_TABLE_ROW;
	} else if (ident->ilower == c->strings[TABLE_COLUMN_GROUP]) {
		value = DISPLAY_TABLE_COLUMN_GROUP;
	} else if (ident->ilower == c->strings[TABLE_COLUMN]) {
		value = DISPLAY_TABLE_COLUMN;
	} else if (ident->ilower == c->strings[TABLE_CELL]) {
		value = DISPLAY_TABLE_CELL;
	} else if (ident->ilower == c->strings[TABLE_CAPTION]) {
		value = DISPLAY_TABLE_CAPTION;
	} else if (ident->ilower == c->strings[NONE]) {
		value = DISPLAY_NONE;
	} else
		return CSS_INVALID;

	opv = buildOPV(CSS_PROP_DISPLAY, flags, value);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, sizeof(opv), result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));

	return CSS_OK;
}

css_error parse_empty_cells(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *ident;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;

	/* IDENT (show, hide, inherit) */
	ident = parserutils_vector_iterate(vector, ctx);
	if (ident == NULL || ident->type != CSS_TOKEN_IDENT)
		return CSS_INVALID;

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	if (ident->ilower == c->strings[INHERIT]) {
		flags |= FLAG_INHERIT;
	} else if (ident->ilower == c->strings[SHOW]) {
		value = EMPTY_CELLS_SHOW;
	} else if (ident->ilower == c->strings[HIDE]) {
		value = EMPTY_CELLS_HIDE;
	} else
		return CSS_INVALID;

	opv = buildOPV(CSS_PROP_EMPTY_CELLS, flags, value);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, sizeof(opv), result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));

	return CSS_OK;
}

css_error parse_float(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *ident;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;

	/* IDENT (left, right, none, inherit) */
	ident = parserutils_vector_iterate(vector, ctx);
	if (ident == NULL || ident->type != CSS_TOKEN_IDENT)
		return CSS_INVALID;

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	if (ident->ilower == c->strings[INHERIT]) {
		flags |= FLAG_INHERIT;
	} else if (ident->ilower == c->strings[LEFT]) {
		value = FLOAT_LEFT;
	} else if (ident->ilower == c->strings[RIGHT]) {
		value = FLOAT_RIGHT;
	} else if (ident->ilower == c->strings[NONE]) {
		value = FLOAT_NONE;
	} else
		return CSS_INVALID;

	opv = buildOPV(CSS_PROP_FLOAT, flags, value);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, sizeof(opv), result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));

	return CSS_OK;
}

css_error parse_height(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *token;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;
	css_fixed length = 0;
	uint32_t unit = 0;
	uint32_t required_size;

	/* length | percentage | IDENT(auto, inherit) */
	token = parserutils_vector_peek(vector, *ctx);
	if (token == NULL)
		return CSS_INVALID;

	if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[INHERIT]) {
		parserutils_vector_iterate(vector, ctx);
		flags = FLAG_INHERIT;
	} else if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[AUTO]) {
		parserutils_vector_iterate(vector, ctx);
		value = HEIGHT_AUTO;
	} else {
		error = parse_unit_specifier(c, vector, ctx, UNIT_PX,
				&length, &unit);
		if (error != CSS_OK)
			return error;

		if (unit & UNIT_ANGLE || unit & UNIT_TIME || unit & UNIT_FREQ)
			return CSS_INVALID;

		/* Negative height is illegal */
		if (length < 0)
			return CSS_INVALID;

		value = HEIGHT_SET;
	}

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	opv = buildOPV(CSS_PROP_HEIGHT, flags, value);

	required_size = sizeof(opv);
	if ((flags & FLAG_INHERIT) == false && value == HEIGHT_SET)
		required_size += sizeof(length) + sizeof(unit);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, required_size, result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));
	if ((flags & FLAG_INHERIT) == false && value == HEIGHT_SET) {
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv),
				&length, sizeof(length));
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv) +
				sizeof(length), &unit, sizeof(unit));
	}

	return CSS_OK;
}

css_error parse_letter_spacing(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *token;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;
	css_fixed length = 0;
	uint32_t unit = 0;
	uint32_t required_size;

	/* length | IDENT(normal, inherit) */
	token = parserutils_vector_peek(vector, *ctx);
	if (token == NULL)
		return CSS_INVALID;

	if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[INHERIT]) {
		parserutils_vector_iterate(vector, ctx);
		flags = FLAG_INHERIT;
	} else if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[NORMAL]) {
		parserutils_vector_iterate(vector, ctx);
		value = LETTER_SPACING_NORMAL;
	} else {
		error = parse_unit_specifier(c, vector, ctx, UNIT_PX,
				&length, &unit);
		if (error != CSS_OK)
			return error;

		if (unit & UNIT_ANGLE || unit & UNIT_TIME || unit & UNIT_FREQ ||
				unit & UNIT_PCT)
			return CSS_INVALID;

		value = LETTER_SPACING_SET;
	}

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	opv = buildOPV(CSS_PROP_LETTER_SPACING, flags, value);

	required_size = sizeof(opv);
	if ((flags & FLAG_INHERIT) == false && value == LETTER_SPACING_SET)
		required_size += sizeof(length) + sizeof(unit);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, required_size, result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));
	if ((flags & FLAG_INHERIT) == false && value == LETTER_SPACING_SET) {
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv),
				&length, sizeof(length));
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv) +
				sizeof(length), &unit, sizeof(unit));
	}

	return CSS_OK;
}

css_error parse_line_height(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *token;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;
	css_fixed length = 0;
	uint32_t unit = 0;
	uint32_t required_size;

	/* number | length | percentage | IDENT(normal, inherit) */
	token = parserutils_vector_peek(vector, *ctx);
	if (token == NULL)
		return CSS_INVALID;

	if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[INHERIT]) {
		parserutils_vector_iterate(vector, ctx);
		flags = FLAG_INHERIT;
	} else if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[NORMAL]) {
		parserutils_vector_iterate(vector, ctx);
		value = LINE_HEIGHT_NORMAL;
	} else if (token->type == CSS_TOKEN_NUMBER) {
		size_t consumed = 0;
		length = number_from_lwc_string(token->ilower, false, &consumed);
		if (consumed != lwc_string_length(token->ilower))
			return CSS_INVALID;

		/* Negative values are illegal */
		if (length < 0)
			return CSS_INVALID;

		parserutils_vector_iterate(vector, ctx);
		value = LINE_HEIGHT_NUMBER;
	} else {
		error = parse_unit_specifier(c, vector, ctx, UNIT_PX,
				&length, &unit);
		if (error != CSS_OK)
			return error;

		if (unit & UNIT_ANGLE || unit & UNIT_TIME || unit & UNIT_FREQ)
			return CSS_INVALID;

		/* Negative values are illegal */
		if (length < 0)
			return CSS_INVALID;

		value = LINE_HEIGHT_DIMENSION;
	}

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	opv = buildOPV(CSS_PROP_LINE_HEIGHT, flags, value);

	required_size = sizeof(opv);
	if ((flags & FLAG_INHERIT) == false && value == LINE_HEIGHT_NUMBER)
		required_size += sizeof(length);
	else if ((flags & FLAG_INHERIT) == false && 
			value == LINE_HEIGHT_DIMENSION)
		required_size += sizeof(length) + sizeof(unit);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, required_size, result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));
	if ((flags & FLAG_INHERIT) == false && (value == LINE_HEIGHT_NUMBER || 
			value == LINE_HEIGHT_DIMENSION))
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv),
				&length, sizeof(length));
	if ((flags & FLAG_INHERIT) == false && value == LINE_HEIGHT_DIMENSION)
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv) +
				sizeof(length), &unit, sizeof(unit));

	return CSS_OK;
}

css_error parse_max_height(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *token;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;
	css_fixed length = 0;
	uint32_t unit = 0;
	uint32_t required_size;

	/* length | percentage | IDENT(none, inherit) */
	token = parserutils_vector_peek(vector, *ctx);
	if (token == NULL)
		return CSS_INVALID;

	if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[INHERIT]) {
		parserutils_vector_iterate(vector, ctx);
		flags = FLAG_INHERIT;
	} else if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[NONE]) {
		parserutils_vector_iterate(vector, ctx);
		value = MAX_HEIGHT_NONE;
	} else {
		error = parse_unit_specifier(c, vector, ctx, UNIT_PX,
				&length, &unit);
		if (error != CSS_OK)
			return error;

		if (unit & UNIT_ANGLE || unit & UNIT_TIME || unit & UNIT_FREQ)
			return CSS_INVALID;

		/* Negative values are illegal */
		if (length < 0)
			return CSS_INVALID;

		value = MAX_HEIGHT_SET;
	}

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	opv = buildOPV(CSS_PROP_MAX_HEIGHT, flags, value);

	required_size = sizeof(opv);
	if ((flags & FLAG_INHERIT) == false && value == MAX_HEIGHT_SET)
		required_size += sizeof(length) + sizeof(unit);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, required_size, result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));
	if ((flags & FLAG_INHERIT) == false && value == MAX_HEIGHT_SET) {
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv),
				&length, sizeof(length));
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv) +
				sizeof(length), &unit, sizeof(unit));
	}

	return CSS_OK;
}

css_error parse_max_width(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *token;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;
	css_fixed length = 0;
	uint32_t unit = 0;
	uint32_t required_size;

	/* length | percentage | IDENT(none, inherit) */
	token = parserutils_vector_peek(vector, *ctx);
	if (token == NULL)
		return CSS_INVALID;

	if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[INHERIT]) {
		parserutils_vector_iterate(vector, ctx);
		flags = FLAG_INHERIT;
	} else if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[NONE]) {
		parserutils_vector_iterate(vector, ctx);
		value = MAX_WIDTH_NONE;
	} else {
		error = parse_unit_specifier(c, vector, ctx, UNIT_PX,
				&length, &unit);
		if (error != CSS_OK)
			return error;

		if (unit & UNIT_ANGLE || unit & UNIT_TIME || unit & UNIT_FREQ)
			return CSS_INVALID;

		/* Negative values are illegal */
		if (length < 0)
			return CSS_INVALID;

		value = MAX_WIDTH_SET;
	}

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	opv = buildOPV(CSS_PROP_MAX_WIDTH, flags, value);

	required_size = sizeof(opv);
	if ((flags & FLAG_INHERIT) == false && value == MAX_WIDTH_SET)
		required_size += sizeof(length) + sizeof(unit);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, required_size, result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));
	if ((flags && FLAG_INHERIT) == false && value == MAX_WIDTH_SET) {
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv),
				&length, sizeof(length));
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv) +
				sizeof(length), &unit, sizeof(unit));
	}

	return CSS_OK;
}

css_error parse_min_height(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *token;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;
	css_fixed length = 0;
	uint32_t unit = 0;
	uint32_t required_size;

	/* length | percentage | IDENT(inherit) */
	token = parserutils_vector_peek(vector, *ctx);
	if (token == NULL)
		return CSS_INVALID;

	if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[INHERIT]) {
		parserutils_vector_iterate(vector, ctx);
		flags = FLAG_INHERIT;
	} else {
		error = parse_unit_specifier(c, vector, ctx, UNIT_PX,
				&length, &unit);
		if (error != CSS_OK)
			return error;

		if (unit & UNIT_ANGLE || unit & UNIT_TIME || unit & UNIT_FREQ)
			return CSS_INVALID;

		/* Negative values are illegal */
		if (length < 0)
			return CSS_INVALID;

		value = MIN_HEIGHT_SET;
	}

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	opv = buildOPV(CSS_PROP_MIN_HEIGHT, flags, value);

	required_size = sizeof(opv);
	if ((flags & FLAG_INHERIT) == false && value == MIN_HEIGHT_SET)
		required_size += sizeof(length) + sizeof(unit);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, required_size, result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));
	if ((flags & FLAG_INHERIT) == false && value == MIN_HEIGHT_SET) {
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv),
				&length, sizeof(length));
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv) +
				sizeof(length), &unit, sizeof(unit));
	}

	return CSS_OK;
}

css_error parse_min_width(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *token;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;
	css_fixed length = 0;
	uint32_t unit = 0;
	uint32_t required_size;

	/* length | percentage | IDENT(inherit) */
	token = parserutils_vector_peek(vector, *ctx);
	if (token == NULL)
		return CSS_INVALID;

	if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[INHERIT]) {
		parserutils_vector_iterate(vector, ctx);
		flags = FLAG_INHERIT;
	} else {
		error = parse_unit_specifier(c, vector, ctx, UNIT_PX,
				&length, &unit);
		if (error != CSS_OK)
			return error;

		if (unit & UNIT_ANGLE || unit & UNIT_TIME || unit & UNIT_FREQ)
			return CSS_INVALID;

		/* Negative values are illegal */
		if (length < 0)
			return CSS_INVALID;

		value = MIN_WIDTH_SET;
	}

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	opv = buildOPV(CSS_PROP_MIN_WIDTH, flags, value);

	required_size = sizeof(opv);
	if ((flags & FLAG_INHERIT) == false && value == MIN_WIDTH_SET)
		required_size += sizeof(length) + sizeof(unit);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, required_size, result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));
	if ((flags & FLAG_INHERIT) == false && value == MIN_WIDTH_SET) {
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv),
				&length, sizeof(length));
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv) +
				sizeof(length), &unit, sizeof(unit));
	}

	return CSS_OK;
}

css_error parse_overflow(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *ident;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;

	/* IDENT (visible, hidden, scroll, auto, inherit) */
	ident = parserutils_vector_iterate(vector, ctx);
	if (ident == NULL || ident->type != CSS_TOKEN_IDENT)
		return CSS_INVALID;

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	if (ident->ilower == c->strings[INHERIT]) {
		flags |= FLAG_INHERIT;
	} else if (ident->ilower == c->strings[VISIBLE]) {
		value = OVERFLOW_VISIBLE;
	} else if (ident->ilower == c->strings[HIDDEN]) {
		value = OVERFLOW_HIDDEN;
	} else if (ident->ilower == c->strings[SCROLL]) {
		value = OVERFLOW_SCROLL;
	} else if (ident->ilower == c->strings[AUTO]) {
		value = OVERFLOW_AUTO;
	} else
		return CSS_INVALID;

	opv = buildOPV(CSS_PROP_OVERFLOW, flags, value);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, sizeof(opv), result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));

	return CSS_OK;
}

css_error parse_table_layout(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *ident;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;

	/* IDENT (auto, fixed, inherit) */
	ident = parserutils_vector_iterate(vector, ctx);
	if (ident == NULL || ident->type != CSS_TOKEN_IDENT)
		return CSS_INVALID;

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	if (ident->ilower == c->strings[INHERIT]) {
		flags |= FLAG_INHERIT;
	} else if (ident->ilower == c->strings[AUTO]) {
		value = TABLE_LAYOUT_AUTO;
	} else if (ident->ilower == c->strings[FIXED]) {
		value = TABLE_LAYOUT_FIXED;
	} else
		return CSS_INVALID;

	opv = buildOPV(CSS_PROP_TABLE_LAYOUT, flags, value);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, sizeof(opv), result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));

	return CSS_OK;
}

css_error parse_text_align(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *ident;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;

	/* IDENT (left, right, center, justify, inherit) */
	ident = parserutils_vector_iterate(vector, ctx);
	if (ident == NULL || ident->type != CSS_TOKEN_IDENT)
		return CSS_INVALID;

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	if (ident->ilower == c->strings[INHERIT]) {
		flags |= FLAG_INHERIT;
	} else if (ident->ilower == c->strings[LEFT]) {
		value = TEXT_ALIGN_LEFT;
	} else if (ident->ilower == c->strings[RIGHT]) {
		value = TEXT_ALIGN_RIGHT;
	} else if (ident->ilower == c->strings[CENTER]) {
		value = TEXT_ALIGN_CENTER;
	} else if (ident->ilower == c->strings[JUSTIFY]) {
		value = TEXT_ALIGN_JUSTIFY;
	} else
		return CSS_INVALID;

	opv = buildOPV(CSS_PROP_TEXT_ALIGN, flags, value);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, sizeof(opv), result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));

	return CSS_OK;
}

css_error parse_text_decoration(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *ident;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;

	/* IDENT([ underline || overline || line-through || blink ])
	 * | IDENT (none, inherit) */
	ident = parserutils_vector_iterate(vector, ctx);
	if (ident == NULL || ident->type != CSS_TOKEN_IDENT)
		return CSS_INVALID;

	if (ident->ilower == c->strings[INHERIT]) {
		flags |= FLAG_INHERIT;
	} else if (ident->ilower == c->strings[NONE]) {
		value = TEXT_DECORATION_NONE;
	} else {
		while (ident != NULL) {
			if (ident->ilower == c->strings[UNDERLINE]) {
				if ((value & TEXT_DECORATION_UNDERLINE) == 0)
					value |= TEXT_DECORATION_UNDERLINE;
				else
					return CSS_INVALID;
			} else if (ident->ilower == c->strings[OVERLINE]) {
				if ((value & TEXT_DECORATION_OVERLINE) == 0)
					value |= TEXT_DECORATION_OVERLINE;
				else
					return CSS_INVALID;
			} else if (ident->ilower == c->strings[LINE_THROUGH]) {
				if ((value & TEXT_DECORATION_LINE_THROUGH) == 0)
					value |= TEXT_DECORATION_LINE_THROUGH;
				else
					return CSS_INVALID;
			} else if (ident->ilower == c->strings[BLINK]) {
				if ((value & TEXT_DECORATION_BLINK) == 0)
					value |= TEXT_DECORATION_BLINK;
				else
					return CSS_INVALID;
			} else
				return CSS_INVALID;

			consumeWhitespace(vector, ctx);

			ident = parserutils_vector_peek(vector, *ctx);
			if (ident != NULL && ident->type != CSS_TOKEN_IDENT)
				break;
			ident = parserutils_vector_iterate(vector, ctx);
		}
	}

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	opv = buildOPV(CSS_PROP_TEXT_DECORATION, flags, value);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, sizeof(opv), result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));

	return CSS_OK;
}

css_error parse_text_indent(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *token;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;
	css_fixed length = 0;
	uint32_t unit = 0;
	uint32_t required_size;

	/* length | percentage | IDENT(inherit) */
	token = parserutils_vector_peek(vector, *ctx);
	if (token == NULL)
		return CSS_INVALID;

	if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[INHERIT]) {
		parserutils_vector_iterate(vector, ctx);
		flags = FLAG_INHERIT;
	} else {
		error = parse_unit_specifier(c, vector, ctx, UNIT_PX,
				&length, &unit);
		if (error != CSS_OK)
			return error;

		if (unit & UNIT_ANGLE || unit & UNIT_TIME || unit & UNIT_FREQ)
			return CSS_INVALID;

		value = TEXT_INDENT_SET;
	}

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	opv = buildOPV(CSS_PROP_TEXT_INDENT, flags, value);

	required_size = sizeof(opv);
	if ((flags & FLAG_INHERIT) == false && value == TEXT_INDENT_SET)
		required_size += sizeof(length) + sizeof(unit);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, required_size, result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));
	if ((flags & FLAG_INHERIT) == false && value == TEXT_INDENT_SET) {
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv),
				&length, sizeof(length));
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv) +
				sizeof(length), &unit, sizeof(unit));
	}

	return CSS_OK;
}

css_error parse_text_transform(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *ident;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;

	/* IDENT (capitalize, uppercase, lowercase, none, inherit) */
	ident = parserutils_vector_iterate(vector, ctx);
	if (ident == NULL || ident->type != CSS_TOKEN_IDENT)
		return CSS_INVALID;

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	if (ident->ilower == c->strings[INHERIT]) {
		flags |= FLAG_INHERIT;
	} else if (ident->ilower == c->strings[CAPITALIZE]) {
		value = TEXT_TRANSFORM_CAPITALIZE;
	} else if (ident->ilower == c->strings[UPPERCASE]) {
		value = TEXT_TRANSFORM_UPPERCASE;
	} else if (ident->ilower == c->strings[LOWERCASE]) {
		value = TEXT_TRANSFORM_LOWERCASE;
	} else if (ident->ilower == c->strings[NONE]) {
		value = TEXT_TRANSFORM_NONE;
	} else
		return CSS_INVALID;

	opv = buildOPV(CSS_PROP_TEXT_TRANSFORM, flags, value);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, sizeof(opv), result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));

	return CSS_OK;
}

css_error parse_unicode_bidi(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *ident;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;

	/* IDENT (normal, embed, bidi-override, inherit) */
	ident = parserutils_vector_iterate(vector, ctx);
	if (ident == NULL || ident->type != CSS_TOKEN_IDENT)
		return CSS_INVALID;

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	if (ident->ilower == c->strings[INHERIT]) {
		flags |= FLAG_INHERIT;
	} else if (ident->ilower == c->strings[NORMAL]) {
		value = UNICODE_BIDI_NORMAL;
	} else if (ident->ilower == c->strings[EMBED]) {
		value = UNICODE_BIDI_EMBED;
	} else if (ident->ilower == c->strings[BIDI_OVERRIDE]) {
		value = UNICODE_BIDI_BIDI_OVERRIDE;
	} else
		return CSS_INVALID;

	opv = buildOPV(CSS_PROP_UNICODE_BIDI, flags, value);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, sizeof(opv), result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));

	return CSS_OK;
}

css_error parse_vertical_align(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *token;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;
	css_fixed length = 0;
	uint32_t unit = 0;
	uint32_t required_size;

	/* length | percentage | IDENT(baseline, sub, super, top, text-top,
	 *                             middle, bottom, text-bottom, inherit)
	 */
	token = parserutils_vector_peek(vector, *ctx);
	if (token == NULL)
		return CSS_INVALID;

	if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[INHERIT]) {
		parserutils_vector_iterate(vector, ctx);
		flags = FLAG_INHERIT;
	} else if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[BASELINE]) {
		parserutils_vector_iterate(vector, ctx);
		value = VERTICAL_ALIGN_BASELINE;
	} else if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[SUB]) {
		parserutils_vector_iterate(vector, ctx);
		value = VERTICAL_ALIGN_SUB;
	} else if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[SUPER]) {
		parserutils_vector_iterate(vector, ctx);
		value = VERTICAL_ALIGN_SUPER;
	} else if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[TOP]) {
		parserutils_vector_iterate(vector, ctx);
		value = VERTICAL_ALIGN_TOP;
	} else if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[TEXT_TOP]) {
		parserutils_vector_iterate(vector, ctx);
		value = VERTICAL_ALIGN_TEXT_TOP;
	} else if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[MIDDLE]) {
		parserutils_vector_iterate(vector, ctx);
		value = VERTICAL_ALIGN_MIDDLE;
	} else if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[BOTTOM]) {
		parserutils_vector_iterate(vector, ctx);
		value = VERTICAL_ALIGN_BOTTOM;
	} else if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[TEXT_BOTTOM]) {
		parserutils_vector_iterate(vector, ctx);
		value = VERTICAL_ALIGN_TEXT_BOTTOM;
	} else {
		error = parse_unit_specifier(c, vector, ctx, UNIT_PX,
				&length, &unit);
		if (error != CSS_OK)
			return error;

		if (unit & UNIT_ANGLE || unit & UNIT_TIME || unit & UNIT_FREQ)
			return CSS_INVALID;

		value = VERTICAL_ALIGN_SET;
	}

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	opv = buildOPV(CSS_PROP_VERTICAL_ALIGN, flags, value);

	required_size = sizeof(opv);
	if ((flags & FLAG_INHERIT) == false && value == VERTICAL_ALIGN_SET)
		required_size += sizeof(length) + sizeof(unit);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, required_size, result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));
	if ((flags & FLAG_INHERIT) == false && value == VERTICAL_ALIGN_SET) {
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv),
				&length, sizeof(length));
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv) +
				sizeof(length), &unit, sizeof(unit));
	}

	return CSS_OK;
}

css_error parse_visibility(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *ident;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;

	/* IDENT (visible, hidden, collapse, inherit) */
	ident = parserutils_vector_iterate(vector, ctx);
	if (ident == NULL || ident->type != CSS_TOKEN_IDENT)
		return CSS_INVALID;

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	if (ident->ilower == c->strings[INHERIT]) {
		flags |= FLAG_INHERIT;
	} else if (ident->ilower == c->strings[VISIBLE]) {
		value = VISIBILITY_VISIBLE;
	} else if (ident->ilower == c->strings[HIDDEN]) {
		value = VISIBILITY_HIDDEN;
	} else if (ident->ilower == c->strings[COLLAPSE]) {
		value = VISIBILITY_COLLAPSE;
	} else
		return CSS_INVALID;

	opv = buildOPV(CSS_PROP_VISIBILITY, flags, value);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, sizeof(opv), result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));

	return CSS_OK;
}

css_error parse_white_space(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *ident;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;

	/* IDENT (normal, pre, nowrap, pre-wrap, pre-line, inherit) */
	ident = parserutils_vector_iterate(vector, ctx);
	if (ident == NULL || ident->type != CSS_TOKEN_IDENT)
		return CSS_INVALID;

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	if (ident->ilower == c->strings[INHERIT]) {
		flags |= FLAG_INHERIT;
	} else if (ident->ilower == c->strings[NORMAL]) {
		value = WHITE_SPACE_NORMAL;
	} else if (ident->ilower == c->strings[PRE]) {
		value = WHITE_SPACE_PRE;
	} else if (ident->ilower == c->strings[NOWRAP]) {
		value = WHITE_SPACE_NOWRAP;
	} else if (ident->ilower == c->strings[PRE_WRAP]) {
		value = WHITE_SPACE_PRE_WRAP;
	} else if (ident->ilower == c->strings[PRE_LINE]) {
		value = WHITE_SPACE_PRE_LINE;
	} else
		return CSS_INVALID;

	opv = buildOPV(CSS_PROP_WHITE_SPACE, flags, value);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, sizeof(opv), result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));

	return CSS_OK;
}

css_error parse_width(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *token;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;
	css_fixed length = 0;
	uint32_t unit = 0;
	uint32_t required_size;

	/* length | percentage | IDENT(auto, inherit) */
	token = parserutils_vector_peek(vector, *ctx);
	if (token == NULL)
		return CSS_INVALID;

	if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[INHERIT]) {
		parserutils_vector_iterate(vector, ctx);
		flags = FLAG_INHERIT;
	} else if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[AUTO]) {
		parserutils_vector_iterate(vector, ctx);
		value = WIDTH_AUTO;
	} else {
		error = parse_unit_specifier(c, vector, ctx, UNIT_PX,
				&length, &unit);
		if (error != CSS_OK)
			return error;

		if (unit & UNIT_ANGLE || unit & UNIT_TIME || unit & UNIT_FREQ)
			return CSS_INVALID;

		/* Must be positive */
		if (length < 0)
			return CSS_INVALID;

		value = WIDTH_SET;
	}

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	opv = buildOPV(CSS_PROP_WIDTH, flags, value);

	required_size = sizeof(opv);
	if ((flags & FLAG_INHERIT) == false && value == WIDTH_SET)
		required_size += sizeof(length) + sizeof(unit);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, required_size, result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));
	if ((flags & FLAG_INHERIT) == false && value == WIDTH_SET) {
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv),
				&length, sizeof(length));
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv) +
				sizeof(length), &unit, sizeof(unit));
	}

	return CSS_OK;
}

css_error parse_word_spacing(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *token;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;
	css_fixed length = 0;
	uint32_t unit = 0;
	uint32_t required_size;

	/* length | IDENT(normal, inherit) */
	token = parserutils_vector_peek(vector, *ctx);
	if (token == NULL)
		return CSS_INVALID;

	if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[INHERIT]) {
		parserutils_vector_iterate(vector, ctx);
		flags = FLAG_INHERIT;
	} else if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[NORMAL]) {
		parserutils_vector_iterate(vector, ctx);
		value = WORD_SPACING_NORMAL;
	} else {
		error = parse_unit_specifier(c, vector, ctx, UNIT_PX,
				&length, &unit);
		if (error != CSS_OK)
			return error;

		if (unit & UNIT_ANGLE || unit & UNIT_TIME || unit & UNIT_FREQ ||
				unit & UNIT_PCT)
			return CSS_INVALID;

		value = WORD_SPACING_SET;
	}

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	opv = buildOPV(CSS_PROP_WORD_SPACING, flags, value);

	required_size = sizeof(opv);
	if ((flags & FLAG_INHERIT) == false && value == WORD_SPACING_SET)
		required_size += sizeof(length) + sizeof(unit);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, required_size, result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));
	if ((flags & FLAG_INHERIT) == false && value == WORD_SPACING_SET) {
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv),
				&length, sizeof(length));
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv) +
				sizeof(length), &unit, sizeof(unit));
	}

	return CSS_OK;
}

css_error parse_z_index(css_language *c, 
		const parserutils_vector *vector, int *ctx, 
		css_style **result)
{
	css_error error;
	const css_token *token;
	uint8_t flags = 0;
	uint16_t value = 0;
	uint32_t opv;
	css_fixed num = 0;
	uint32_t required_size;

	/* <integer> | IDENT (auto, inherit) */
	token = parserutils_vector_iterate(vector, ctx);
	if (token == NULL || (token->type != CSS_TOKEN_IDENT &&
			token->type != CSS_TOKEN_NUMBER))
		return CSS_INVALID;

	error = parse_important(c, vector, ctx, &flags);
	if (error != CSS_OK)
		return error;

	if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[INHERIT]) {
		flags |= FLAG_INHERIT;
	} else if (token->type == CSS_TOKEN_IDENT &&
			token->ilower == c->strings[AUTO]) {
		value = Z_INDEX_AUTO;
	} else if (token->type == CSS_TOKEN_NUMBER) {
		size_t consumed = 0;
		num = number_from_lwc_string(token->ilower, true, &consumed);
		/* Invalid if there are trailing characters */
		if (consumed != lwc_string_length(token->ilower))
			return CSS_INVALID;

		value = Z_INDEX_SET;
	} else
		return CSS_INVALID;

	opv = buildOPV(CSS_PROP_Z_INDEX, flags, value);

	required_size = sizeof(opv);
	if ((flags & FLAG_INHERIT) == false && value == Z_INDEX_SET)
		required_size += sizeof(num);

	/* Allocate result */
	error = css_stylesheet_style_create(c->sheet, required_size, result);
	if (error != CSS_OK)
		return error;

	/* Copy the bytecode to it */
	memcpy((*result)->bytecode, &opv, sizeof(opv));
	if ((flags & FLAG_INHERIT) == false && value == Z_INDEX_SET) {
		memcpy(((uint8_t *) (*result)->bytecode) + sizeof(opv), 
				&num, sizeof(num));
	}

	return CSS_OK;
}

