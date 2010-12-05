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

css_error cascade_width(uint32_t opv, css_style *style, 
		css_select_state *state)
{
	return cascade_length_auto(opv, style, state, set_width);
}

css_error set_width_from_hint(const css_hint *hint,
		css_computed_style *style)
{
	return set_width(style, hint->status,
			hint->data.length.value, hint->data.length.unit);
}

css_error initial_width(css_select_state *state)
{
	return set_width(state->result, CSS_WIDTH_AUTO, 0, CSS_UNIT_PX);
}

css_error compose_width(const css_computed_style *parent,
		const css_computed_style *child,
		css_computed_style *result)
{
	css_fixed length = 0;
	css_unit unit = CSS_UNIT_PX;
	uint8_t type = get_width(child, &length, &unit);

	if (type == CSS_WIDTH_INHERIT) {
		type = get_width(parent, &length, &unit);
	}

	return set_width(result, type, length, unit);
}

uint32_t destroy_width(void *bytecode)
{
	return generic_destroy_length(bytecode);
}
