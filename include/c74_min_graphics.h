/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

#include "c74_ui.h"
#include "c74_ui_graphics.h"

namespace c74 {
namespace min {
namespace graphics {


	class context {
	public:
		explicit context(const atoms& args) {
			assert(args.size() == 1);

			max::t_object* view = args[0];
			m_graphics_context = (max::t_jgraphics*)max::patcherview_get_jgraphics(view);
		}

	private:
		max::t_jgraphics* m_graphics_context; // g

	};



	class font {
	public:
		font(context ctx, symbol name) {

		}

	private:

	};



	class fontsize {
	public:
		fontsize(context ctx, double a_size) {

		}

	private:
		
	};



	enum class colors {
		black
	};

	static max::t_jrgba k_color_black = {0.0, 0.0, 0.0, 1.0};


	class color {
	public:
		color(context a_context, colors a_color)
		: m_context { a_context }
		{
//			max::jgraphics_get_source

//			switch (a_color) {
//				case colors::black:	max::jgraphics_set_source_jrgba(m_context, &k_color_black);
//			}
		}

	private:
		context m_context;
	};


	class position {
	public:
		position(context ctx, double x, double y) {

		}

	private:
		

	};




	class text {
	public:
		text(context ctx, std::string str) {

		}

	private:


	};




	// Provide a stateless API for drawing elements
	// http://robert.ocallahan.org/2011/09/graphics-api-design.html
	// https://github.com/mikebmcl/N3888_RefImpl/issues/31
	// https://en.wikipedia.org/wiki/Skia_Graphics_Engine






// TODO: use variadic template processing to do everything specified in the arguments


	void draw(const atoms& graphics_context, ... ) {
		context g(graphics_context);


	}


/*

							jgraphics_set_source_jrgba(g, &s_color_red_button);
							else
								jgraphics_set_source_jrgba(g, &s_color_background_button);

								x->rect_mute.x = right_side;
								x->rect_mute.width = 13.0;

								jgraphics_set_line_width(g, 1.5);
								jgraphics_arc(g, right_side+6.5, 9.5, 6.5, 0., JGRAPHICS_2PI);
								jgraphics_fill(g);

								if (x->highlight && x->highlight_mute)
									jgraphics_set_source_jrgba(g, &s_color_selected);
									else
										jgraphics_set_source_jrgba(g, &s_color_border_button);
										jgraphics_arc(g, right_side+6.5, 9.5, 6.5, 0., JGRAPHICS_2PI);
										jgraphics_stroke(g);

										// m
										if (x->is_muted)
											jgraphics_set_source_jrgba(g, &s_color_text_button_on);
											else
												jgraphics_set_source_jrgba(g, &s_color_text_button_off);

												jgraphics_set_line_width(g, 2.0);
												jgraphics_move_to(g, right_side + 2.5, 13.0);
												jgraphics_select_font_face(g, JAMOMA_BUTTON_FONT, JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_BOLD);
												jgraphics_set_font_size(g, 7.0);
												jgraphics_show_text(g, "m");
												}

	*/

}}} // namespace c74::min:::graphics
