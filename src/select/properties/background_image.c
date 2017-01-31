/*
 * This file is part of LibCSS
 * Licensed under the MIT License,
 *		  http://www.opensource.org/licenses/mit-license.php
 * Copyright 2009 John-Mark Bell <jmb@netsurf-browser.org>
 */

#include <libcss/computed.h>
#include "bytecode/bytecode.h"
#include "bytecode/opcodes.h"
#include "select/propset.h"
#include "select/propget.h"
#include "utils/utils.h"

#include "select/properties/properties.h"
#include "select/properties/helpers.h"

css_error css__cascade_background_image(uint32_t opv, css_style *style, 
		css_select_state *state)
{
	return css__cascade_image(opv, style, state, set_background_image);
}

css_error css__set_background_image_from_hint(const css_hint *hint, 
		css_computed_style *style)
{
	css_error error;

	error = set_background_image(style, hint->status, &hint->data.image);

	return error;
}

css_error css__initial_background_image(css_select_state *state)
{
	return set_background_image(state->computed, 
			CSS_BACKGROUND_IMAGE_NONE, NULL);
}

css_error css__compose_background_image(const css_computed_style *parent,
		const css_computed_style *child,
		css_computed_style *result)
{
  css_error error;
	css_computed_image *image = NULL;
	uint8_t type = get_background_image(child, &image);

	if ((child->i.background_image == NULL && parent->i.background_image != NULL) ||
			type == CSS_BACKGROUND_IMAGE_INHERIT ||
			(child->i.background_image != NULL && result != child))
	{
		css_computed_image *copy = NULL;

		if ((child->i.background_image == NULL && parent->i.background_image != NULL) ||
				type == CSS_BACKGROUND_IMAGE_INHERIT)
		{
			type = get_background_image(parent, &image);
		}

		if (type == CSS_BACKGROUND_IMAGE_IMAGE && image)
		{
			copy = malloc(sizeof(css_computed_image));
			if (copy == NULL)
				return CSS_NOMEM;
			*copy = *image;
			if (image->data.uri)
			{
				if (image->type == CSS_COMPUTED_IMAGE_LINEAR_GRADIENT ||
						image->type == CSS_COMPUTED_IMAGE_REPEATING_LINEAR_GRADIENT)
				{
					copy->data.linear = malloc(sizeof(css_computed_linear_gradient));
					if (!copy->data.linear)
					{
						free(copy);
						return CSS_NOMEM;
					}
					*copy->data.linear = *image->data.linear;
					if (image->data.linear->nstop)
					{
						size_t size = image->data.linear->nstop * sizeof(css_computed_color_stop);
						copy->data.linear->stops = malloc(size);
						if (!copy->data.linear->stops)
						{
							free(copy->data.linear);
							free(copy);
							return CSS_NOMEM;
						}
						memcpy(copy->data.linear->stops, image->data.linear->stops, size);
					}
				}
			}
		}

		error = set_background_image(result, type, copy);
		if (error != CSS_OK && copy != NULL)
      css__computed_image_destroy(copy);

		return error;
	}

	return CSS_OK;
}

