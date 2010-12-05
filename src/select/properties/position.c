/*
 * This file is part of LibCSS
 * Licensed under the MIT License,
 *		  http://www.opensource.org/licenses/mit-license.php
 * Copyright 2009 John-Mark Bell <jmb@netsurf-browser.org>
 */

#include "bytecode/bytecode.h"
#include "bytecode/opcodes.h"
#include "select/propset.h"
#include "select/propget.h"
#include "utils/utils.h"

#include "select/properties/properties.h"
#include "select/properties/helpers.h"

css_error cascade_position(uint32_t opv, css_style *style, 
		css_select_state *state)
{
	uint16_t value = CSS_POSITION_INHERIT;

	UNUSED(style);

	if (isInherit(opv) == false) {
		switch (getValue(opv)) {
		case POSITION_STATIC:
			value = CSS_POSITION_STATIC;
			break;
		case POSITION_RELATIVE:
			value = CSS_POSITION_RELATIVE;
			break;
		case POSITION_ABSOLUTE:
			value = CSS_POSITION_ABSOLUTE;
			break;
		case POSITION_FIXED:
			value = CSS_POSITION_FIXED;
			break;
		}
	}

	if (outranks_existing(getOpcode(opv), isImportant(opv), state,
			isInherit(opv))) {
		return set_position(state->result, value);
	}

	return CSS_OK;
}

css_error set_position_from_hint(const css_hint *hint,
		css_computed_style *style)
{
	return set_position(style, hint->status);
}

css_error initial_position(css_select_state *state)
{
	return set_position(state->result, CSS_POSITION_STATIC);
}

css_error compose_position(const css_computed_style *parent,
		const css_computed_style *child,
		css_computed_style *result)
{
	uint8_t type = get_position(child);

	if (type == CSS_POSITION_INHERIT) {
		type = get_position(parent);
	}

	return set_position(result, type);
}

uint32_t destroy_position(void *bytecode)
{
	UNUSED(bytecode);
	
	return sizeof(uint32_t);
}
