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

css_error cascade_background_color(uint32_t opv, css_style *style, 
		css_select_state *state)
{
	return cascade_bg_border_color(opv, style, state, set_background_color);
}

css_error set_background_color_from_hint(const css_hint *hint, 
		css_computed_style *style)
{
	return set_background_color(style, hint->status, hint->data.color);
}

css_error initial_background_color(css_select_state *state)
{
	return set_background_color(state->result, 
			CSS_BACKGROUND_COLOR_TRANSPARENT, 0);
}

css_error compose_background_color(const css_computed_style *parent,
		const css_computed_style *child,
		css_computed_style *result)
{
	css_color color;
	uint8_t type = get_background_color(child, &color);

	if (type == CSS_BACKGROUND_COLOR_INHERIT) {
		type = get_background_color(parent, &color);
	}

	return set_background_color(result, type, color);
}

uint32_t destroy_background_color(void *bytecode)
{
	return generic_destroy_color(bytecode);
}
