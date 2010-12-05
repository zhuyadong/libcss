/*
 * This file is part of LibCSS
 * Licensed under the MIT License,
 *		  http://www.opensource.org/licenses/mit-license.php
 * Copyright 2009 John-Mark Bell <jmb@netsurf-browser.org>
 */

#include <assert.h>

#include "bytecode/bytecode.h"
#include "bytecode/opcodes.h"
#include "select/properties/properties.h"
#include "select/propget.h"
#include "select/propset.h"
#include "utils/utils.h"

#include "select/properties/helpers.h"

/* Generic destructors */

uint32_t generic_destroy_color(void *bytecode)
{
	return sizeof(uint32_t) + 
		((getValue(*((uint32_t*)bytecode)) == BACKGROUND_COLOR_SET) ? sizeof(css_color) : 0);
}

uint32_t generic_destroy_uri(void *bytecode)
{
	bool has_uri = (getValue(*((uint32_t*)bytecode)) & BACKGROUND_IMAGE_URI) == BACKGROUND_IMAGE_URI;
	
	if (has_uri) {
		void *vstr = (((uint8_t*)bytecode) + sizeof(uint32_t));
		lwc_string *str = *(lwc_string **) vstr;
		lwc_string_unref(str);
	}
	return sizeof(uint32_t) + (has_uri ? sizeof(lwc_string*) : 0);
}

uint32_t generic_destroy_length(void *bytecode)
{
	bool has_length = (getValue(*((uint32_t*)bytecode)) & BORDER_WIDTH_SET) == BORDER_WIDTH_SET;
	
	return sizeof(uint32_t) + (has_length ? sizeof(css_fixed) + sizeof(uint32_t) : 0);
}

uint32_t generic_destroy_number(void *bytecode)
{
	uint32_t value = getValue(*((uint32_t*)bytecode));
	bool has_number = (value == ORPHANS_SET);
	
	return sizeof(uint32_t) + (has_number ? sizeof(css_fixed) : 0);
}

/* Useful helpers */

css_unit to_css_unit(uint32_t u)
{
	switch (u) {
	case UNIT_PX: return CSS_UNIT_PX;
	case UNIT_EX: return CSS_UNIT_EX;
	case UNIT_EM: return CSS_UNIT_EM;
	case UNIT_IN: return CSS_UNIT_IN;
	case UNIT_CM: return CSS_UNIT_CM;
	case UNIT_MM: return CSS_UNIT_MM;
	case UNIT_PT: return CSS_UNIT_PT;
	case UNIT_PC: return CSS_UNIT_PC;
	case UNIT_PCT: return CSS_UNIT_PCT;
	case UNIT_DEG: return CSS_UNIT_DEG;
	case UNIT_GRAD: return CSS_UNIT_GRAD;
	case UNIT_RAD: return CSS_UNIT_RAD;
	case UNIT_MS: return CSS_UNIT_MS;
	case UNIT_S: return CSS_UNIT_S;
	case UNIT_HZ: return CSS_UNIT_HZ;
	case UNIT_KHZ: return CSS_UNIT_KHZ;
	}

	return 0;
}

/******************************************************************************
 * Utilities below here							      *
 ******************************************************************************/
css_error cascade_bg_border_color(uint32_t opv, css_style *style,
		css_select_state *state, 
		css_error (*fun)(css_computed_style *, uint8_t, css_color))
{
	uint16_t value = CSS_BACKGROUND_COLOR_INHERIT;
	css_color color = 0;

	assert(CSS_BACKGROUND_COLOR_INHERIT == CSS_BORDER_COLOR_INHERIT);
	assert(CSS_BACKGROUND_COLOR_TRANSPARENT == 
			CSS_BORDER_COLOR_TRANSPARENT);
	assert(CSS_BACKGROUND_COLOR_COLOR == CSS_BORDER_COLOR_COLOR);

	if (isInherit(opv) == false) {
		switch (getValue(opv)) {
		case BACKGROUND_COLOR_TRANSPARENT:
			value = CSS_BACKGROUND_COLOR_TRANSPARENT;
			break;
		case BACKGROUND_COLOR_SET:
			value = CSS_BACKGROUND_COLOR_COLOR;
			color = *((css_color *) style->bytecode);
			advance_bytecode(style, sizeof(color));
			break;
		}
	}

	if (outranks_existing(getOpcode(opv), isImportant(opv), state,
			isInherit(opv))) {
		return fun(state->result, value, color);
	}

	return CSS_OK;
}

css_error cascade_uri_none(uint32_t opv, css_style *style,
		css_select_state *state,
		css_error (*fun)(css_computed_style *, uint8_t, 
				lwc_string *))
{
	uint16_t value = CSS_BACKGROUND_IMAGE_INHERIT;
	lwc_string *uri = NULL;

	if (isInherit(opv) == false) {
		switch (getValue(opv)) {
		case BACKGROUND_IMAGE_NONE:
			value = CSS_BACKGROUND_IMAGE_NONE;
			break;
		case BACKGROUND_IMAGE_URI:
			value = CSS_BACKGROUND_IMAGE_IMAGE;
			uri = *((lwc_string **) style->bytecode);
			advance_bytecode(style, sizeof(uri));
			break;
		}
	}

	/** \todo lose fun != NULL once all properties have set routines */
	if (fun != NULL && outranks_existing(getOpcode(opv), 
			isImportant(opv), state, isInherit(opv))) {
		return fun(state->result, value, uri);
	}

	return CSS_OK;
}

css_error cascade_border_style(uint32_t opv, css_style *style,
		css_select_state *state, 
		css_error (*fun)(css_computed_style *, uint8_t))
{
	uint16_t value = CSS_BORDER_STYLE_INHERIT;

	UNUSED(style);

	if (isInherit(opv) == false) {
		switch (getValue(opv)) {
		case BORDER_STYLE_NONE:
			value = CSS_BORDER_STYLE_NONE;
			break;
		case BORDER_STYLE_HIDDEN:
			value = CSS_BORDER_STYLE_HIDDEN;
			break;
		case BORDER_STYLE_DOTTED:
			value = CSS_BORDER_STYLE_DOTTED;
			break;
		case BORDER_STYLE_DASHED:
			value = CSS_BORDER_STYLE_DASHED;
			break;
		case BORDER_STYLE_SOLID:
			value = CSS_BORDER_STYLE_SOLID;
			break;
		case BORDER_STYLE_DOUBLE:
			value = CSS_BORDER_STYLE_DOUBLE;
			break;
		case BORDER_STYLE_GROOVE:
			value = CSS_BORDER_STYLE_GROOVE;
			break;
		case BORDER_STYLE_RIDGE:
			value = CSS_BORDER_STYLE_RIDGE;
			break;
		case BORDER_STYLE_INSET:
			value = CSS_BORDER_STYLE_INSET;
			break;
		case BORDER_STYLE_OUTSET:
			value = CSS_BORDER_STYLE_OUTSET;
			break;
		}
	}

	if (outranks_existing(getOpcode(opv), isImportant(opv), state,
			isInherit(opv))) {
		return fun(state->result, value);
	}

	return CSS_OK;
}

css_error cascade_border_width(uint32_t opv, css_style *style,
		css_select_state *state, 
		css_error (*fun)(css_computed_style *, uint8_t, css_fixed, 
				css_unit))
{
	uint16_t value = CSS_BORDER_WIDTH_INHERIT;
	css_fixed length = 0;
	uint32_t unit = UNIT_PX;

	if (isInherit(opv) == false) {
		switch (getValue(opv)) {
		case BORDER_WIDTH_SET:
			value = CSS_BORDER_WIDTH_WIDTH;
			length = *((css_fixed *) style->bytecode);
			advance_bytecode(style, sizeof(length));
			unit = *((uint32_t *) style->bytecode);
			advance_bytecode(style, sizeof(unit));
			break;
		case BORDER_WIDTH_THIN:
			value = CSS_BORDER_WIDTH_THIN;
			break;
		case BORDER_WIDTH_MEDIUM:
			value = CSS_BORDER_WIDTH_MEDIUM;
			break;
		case BORDER_WIDTH_THICK:
			value = CSS_BORDER_WIDTH_THICK;
			break;
		}
	}

	unit = to_css_unit(unit);

	if (outranks_existing(getOpcode(opv), isImportant(opv), state,
			isInherit(opv))) {
		return fun(state->result, value, length, unit);
	}

	return CSS_OK;
}

css_error cascade_length_auto(uint32_t opv, css_style *style,
		css_select_state *state,
		css_error (*fun)(css_computed_style *, uint8_t, css_fixed,
				css_unit))
{
	uint16_t value = CSS_BOTTOM_INHERIT;
	css_fixed length = 0;
	uint32_t unit = UNIT_PX;

	if (isInherit(opv) == false) {
		switch (getValue(opv)) {
		case BOTTOM_SET:
			value = CSS_BOTTOM_SET;
			length = *((css_fixed *) style->bytecode);
			advance_bytecode(style, sizeof(length));
			unit = *((uint32_t *) style->bytecode);
			advance_bytecode(style, sizeof(unit));
			break;
		case BOTTOM_AUTO:
			value = CSS_BOTTOM_AUTO;
			break;
		}
	}

	unit = to_css_unit(unit);

	if (outranks_existing(getOpcode(opv), isImportant(opv), state,
			isInherit(opv))) {
		return fun(state->result, value, length, unit);
	}

	return CSS_OK;
}

css_error cascade_length_normal(uint32_t opv, css_style *style,
		css_select_state *state,
		css_error (*fun)(css_computed_style *, uint8_t, css_fixed,
				css_unit))
{
	uint16_t value = CSS_LETTER_SPACING_INHERIT;
	css_fixed length = 0;
	uint32_t unit = UNIT_PX;

	if (isInherit(opv) == false) {
		switch (getValue(opv)) {
		case LETTER_SPACING_SET:
			value = CSS_LETTER_SPACING_SET;
			length = *((css_fixed *) style->bytecode);
			advance_bytecode(style, sizeof(length));
			unit = *((uint32_t *) style->bytecode);
			advance_bytecode(style, sizeof(unit));
			break;
		case LETTER_SPACING_NORMAL:
			value = CSS_LETTER_SPACING_NORMAL;
			break;
		}
	}

	unit = to_css_unit(unit);

	if (outranks_existing(getOpcode(opv), isImportant(opv), state,
			isInherit(opv))) {
		return fun(state->result, value, length, unit);
	}

	return CSS_OK;
}

css_error cascade_length_none(uint32_t opv, css_style *style,
		css_select_state *state,
		css_error (*fun)(css_computed_style *, uint8_t, css_fixed,
				css_unit))
{
	uint16_t value = CSS_MAX_HEIGHT_INHERIT;
	css_fixed length = 0;
	uint32_t unit = UNIT_PX;

	if (isInherit(opv) == false) {
		switch (getValue(opv)) {
		case MAX_HEIGHT_SET:
			value = CSS_MAX_HEIGHT_SET;
			length = *((css_fixed *) style->bytecode);
			advance_bytecode(style, sizeof(length));
			unit = *((uint32_t *) style->bytecode);
			advance_bytecode(style, sizeof(unit));
			break;
		case MAX_HEIGHT_NONE:
			value = CSS_MAX_HEIGHT_NONE;
			break;
		}
	}

	unit = to_css_unit(unit);

	if (outranks_existing(getOpcode(opv), isImportant(opv), state,
			isInherit(opv))) {
		return fun(state->result, value, length, unit);
	}

	return CSS_OK;
}

css_error cascade_length(uint32_t opv, css_style *style,
		css_select_state *state,
		css_error (*fun)(css_computed_style *, uint8_t, css_fixed,
				css_unit))
{
	uint16_t value = CSS_MIN_HEIGHT_INHERIT;
	css_fixed length = 0;
	uint32_t unit = UNIT_PX;

	if (isInherit(opv) == false) {
		value = CSS_MIN_HEIGHT_SET;
		length = *((css_fixed *) style->bytecode);
		advance_bytecode(style, sizeof(length));
		unit = *((uint32_t *) style->bytecode);
		advance_bytecode(style, sizeof(unit));
	}

	unit = to_css_unit(unit);

	/** \todo lose fun != NULL once all properties have set routines */
	if (fun != NULL && outranks_existing(getOpcode(opv), 
			isImportant(opv), state, isInherit(opv))) {
		return fun(state->result, value, length, unit);
	}

	return CSS_OK;
}

css_error cascade_number(uint32_t opv, css_style *style,
		css_select_state *state,
		css_error (*fun)(css_computed_style *, uint8_t, css_fixed))
{
	uint16_t value = 0;
	css_fixed length = 0;

	/** \todo values */

	if (isInherit(opv) == false) {
		value = 0;
		length = *((css_fixed *) style->bytecode);
		advance_bytecode(style, sizeof(length));
	}

	/** \todo lose fun != NULL once all properties have set routines */
	if (fun != NULL && outranks_existing(getOpcode(opv), 
			isImportant(opv), state, isInherit(opv))) {
		return fun(state->result, value, length);
	}

	return CSS_OK;
}

css_error cascade_page_break_after_before(uint32_t opv, css_style *style,
		css_select_state *state,
		css_error (*fun)(css_computed_style *, uint8_t))
{
	uint16_t value = 0;

	UNUSED(style);

	if (isInherit(opv) == false) {
		switch (getValue(opv)) {
		case PAGE_BREAK_AFTER_AUTO:
		case PAGE_BREAK_AFTER_ALWAYS:
		case PAGE_BREAK_AFTER_AVOID:
		case PAGE_BREAK_AFTER_LEFT:
		case PAGE_BREAK_AFTER_RIGHT:
			/** \todo convert to public values */
			break;
		}
	}

	/** \todo lose fun != NULL */
	if (fun != NULL && outranks_existing(getOpcode(opv), 
			isImportant(opv), state, isInherit(opv))) {
		return fun(state->result, value);
	}

	return CSS_OK;
}

css_error cascade_counter_increment_reset(uint32_t opv, css_style *style,
		css_select_state *state,
		css_error (*fun)(css_computed_style *, uint8_t,
				css_computed_counter *))
{
	uint16_t value = CSS_COUNTER_INCREMENT_INHERIT;
	css_computed_counter *counters = NULL;
	uint32_t n_counters = 0;

	if (isInherit(opv) == false) {
		switch (getValue(opv)) {
		case COUNTER_INCREMENT_NAMED:
		{
			uint32_t v = getValue(opv);

			while (v != COUNTER_INCREMENT_NONE) {
				css_computed_counter *temp;
				lwc_string *name;
				css_fixed val = 0;

				name = *((lwc_string **)
						style->bytecode);
				advance_bytecode(style, sizeof(name));

				val = *((css_fixed *) style->bytecode);
				advance_bytecode(style, sizeof(val));

				temp = state->result->alloc(counters,
						(n_counters + 1) * 
						sizeof(css_computed_counter),
						state->result->pw);
				if (temp == NULL) {
					if (counters != NULL) {
						state->result->alloc(counters, 
							0, state->result->pw);
					}
					return CSS_NOMEM;
				}

				counters = temp;

				counters[n_counters].name = name;
				counters[n_counters].value = val;

				n_counters++;

				v = *((uint32_t *) style->bytecode);
				advance_bytecode(style, sizeof(v));
			}
		}
			break;
		case COUNTER_INCREMENT_NONE:
			value = CSS_COUNTER_INCREMENT_NONE;
			break;
		}
	}

	/* If we have some counters, terminate the array with a blank entry */
	if (n_counters > 0) {
		css_computed_counter *temp;

		temp = state->result->alloc(counters, 
				(n_counters + 1) * sizeof(css_computed_counter),
				state->result->pw);
		if (temp == NULL) {
			state->result->alloc(counters, 0, state->result->pw);
			return CSS_NOMEM;
		}

		counters = temp;

		counters[n_counters].name = NULL;
		counters[n_counters].value = 0;
	}

	if (outranks_existing(getOpcode(opv), isImportant(opv), state,
			isInherit(opv))) {
		css_error error;

		error = fun(state->result, value, counters);
		if (error != CSS_OK && n_counters > 0)
			state->result->alloc(counters, 0, state->result->pw);

		return error;
	} else if (n_counters > 0) {
		state->result->alloc(counters, 0, state->result->pw);
	}

	return CSS_OK;
}

