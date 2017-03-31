/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once


namespace c74 {
namespace min {
namespace ui {


	class target {
	public:
		explicit target(const atoms& args) {
			assert(args.size() > 1);

			m_box = (max::t_jbox*)(max::t_object*)args[0];
			m_view = args[1];
			m_graphics_context = (max::t_jgraphics*)max::patcherview_get_jgraphics(m_view);
			jbox_get_rect_for_view((max::t_object*)m_box, m_view, &m_rect);
		}

		operator max::t_jgraphics*() const {
			return m_graphics_context;
		}

		number x() const {
			return m_rect.x;
		}

		number y() const {
			return m_rect.y;
		}

		number width() const {
			return m_rect.width;
		}

		number height() const {
			return m_rect.height;
		}

	private:
		max::t_jbox*		m_box;
		max::t_object*		m_view;
		max::t_jgraphics*	m_graphics_context;
		max::t_rect			m_rect;
	};


	class color {
	public:

		enum colors {
			black,
			white,
			gray
		};

		color(max::t_jrgba a_color)
		: m_color { a_color }
		{}

		color(colors a_color) {
			switch (a_color) {
				case black	: m_color = { 0.0, 0.0, 0.0, 1.0}; break;
				case white	: m_color = { 1.0, 1.0, 1.0, 1.0}; break;
				case gray	: m_color = { 0.7, 0.7, 0.7, 1.0}; break;
			}
		}

		void operator()(target& g) {
			max::jgraphics_set_source_jrgba(g, &m_color);
		}

	private:
		max::t_jrgba m_color;
	};


	class position {
	public:
		position(double x, double y)
		: m_position {x,y}
		{}

		void operator()(max::t_rect& r) {
			r.x = m_position.x;
			r.y = m_position.y;
		}

	private:
		max::t_pt m_position;
	};


	class size {
	public:
		size(double x, double y)
		: m_size {x,y}
		{}

		void operator()(max::t_rect& r) {
			r.width = m_size.width;
			r.height = m_size.height;
		}

	private:
		max::t_size m_size;
	};


	class line_width {
	public:
		line_width(number a_width)
		: m_width { a_width }
		{}

		void operator()(const target& g) {
			max::jgraphics_set_line_width(g, m_width);
		}

	private:
		number m_width;
	};


	/// textual content
	class content {
	public:
		content(string str)
		: m_text {str}
		{}

		void operator()(string& s) {
			s = m_text;
		}

	private:
		string m_text;
	};


	class fontface {
	public:
		fontface(symbol a_name, bool bold = false, bool italic = false)
		: m_name	{ a_name }
		, m_weight	{ bold ? max::JGRAPHICS_FONT_WEIGHT_BOLD : max::JGRAPHICS_FONT_WEIGHT_NORMAL }
		, m_slant	{ italic ? max::JGRAPHICS_FONT_SLANT_ITALIC : max::JGRAPHICS_FONT_SLANT_NORMAL }
		{}

		void operator()(const target& g) {
			max::jgraphics_select_font_face(g, m_name, m_slant, m_weight);
		}

	private:
		symbol							m_name;
		max::t_jgraphics_font_weight	m_weight;
		max::t_jgraphics_font_slant		m_slant;
	};


	// NOTE: this is the model to follow for the other draw commands

	class fontsize {
	public:
		fontsize(double a_value)
		: m_value { a_value }
		{}

		void operator()(const target& g) {
			max::jgraphics_set_font_size(g, m_value);
		}

	private:
		number m_value;
	};
	

	class element {
	protected:

		/// constructor utility: context
		template<typename argument_type>
		constexpr typename enable_if<is_same<argument_type, target>::value>::type
		assign_from_argument(const argument_type& arg) noexcept {
			//const_cast<symbol&>(m_title) = arg;
			m_target = std::make_unique<target>(arg);
		}

		/// constructor utility: color
		template<typename argument_type>
		constexpr typename enable_if<is_same<argument_type, color>::value>::type
		assign_from_argument(const argument_type& arg) noexcept {
			const_cast<argument_type&>(arg)( const_cast<target&>(*m_target) );
		}

		/// constructor utility: position
		template<typename argument_type>
		constexpr typename enable_if<is_same<argument_type, position>::value>::type
		assign_from_argument(const argument_type& arg) noexcept {
			const_cast<argument_type&>(arg)(m_rect);
		}

		/// constructor utility: size
		template<typename argument_type>
		constexpr typename enable_if<is_same<argument_type, size>::value>::type
		assign_from_argument(const argument_type& arg) noexcept {
			const_cast<argument_type&>(arg)(m_rect);
		}

		/// constructor utility: fontface
		template<typename argument_type>
		constexpr typename enable_if<is_same<argument_type, fontface>::value>::type
		assign_from_argument(const argument_type& arg) noexcept {
			const_cast<argument_type&>(arg)( const_cast<target&>(*m_target) );
		}

		/// constructor utility: fontsize
		template<typename argument_type>
		constexpr typename enable_if<is_same<argument_type, fontsize>::value>::type
		assign_from_argument(const argument_type& arg) noexcept {
			const_cast<argument_type&>(arg)( const_cast<target&>(*m_target) );
		}

		/// constructor utility: line_width
		template<typename argument_type>
		constexpr typename enable_if<is_same<argument_type, line_width>::value>::type
		assign_from_argument(const argument_type& arg) noexcept {
			const_cast<argument_type&>(arg)( const_cast<target&>(*m_target) );
		}

		/// constructor utility: content
		template<typename argument_type>
		constexpr typename enable_if<is_same<argument_type, content>::value>::type
		assign_from_argument(const argument_type& arg) noexcept {
			const_cast<argument_type&>(arg)(m_text);
		}

		/// constructor utility
		constexpr void handle_arguments() noexcept {
			;
		}

		/// constructor utility
		template <typename FIRST_ARG, typename ...REMAINING_ARGS>
		constexpr void handle_arguments(FIRST_ARG const& first, REMAINING_ARGS const& ...args) noexcept {
			assign_from_argument(first);
			if (sizeof...(args))
				handle_arguments(args...); // recurse
		}


		void update() {
			if (m_rect.x <= 0.0)
				m_rect.x = 0 + m_rect.x;
			if (m_rect.y <= 0.0)
				m_rect.y = 0 + m_rect.y;
			if (m_rect.width <= 0.0)
				m_rect.width = m_target->width() + m_rect.width;
			if (m_rect.height <= 0.0)
				m_rect.height = m_target->height() + m_rect.height;
		}


		std::unique_ptr<target>		m_target;
		max::t_rect					m_rect {};
		string						m_text;
	};


	enum draw_style {
		stroke,
		fill
	};


	template<draw_style style>
	void draw(target& a_target);

	template<>
	void draw<stroke>(target& a_target) {
		max::jgraphics_stroke(a_target);
	}

	template<>
	void draw<fill>(target& a_target) {
		max::jgraphics_fill(a_target);
	}


	template<draw_style style = stroke>
	class rect : public element {
	public:
		template<typename ...ARGS>
		rect(ARGS... args) {
			handle_arguments(args...);
			update();
			max::jgraphics_rectangle(*m_target, m_rect.x, m_rect.y, m_rect.width, m_rect.height);
			draw<style>(*m_target);
		}
	};


	class text : public element {
	public:
		template<typename ...ARGS>
		text(ARGS... args) {
			handle_arguments(args...);
			update();
			max::jgraphics_move_to(*m_target, m_rect.x, m_rect.y);
			max::jgraphics_show_text(*m_target, m_text.c_str());
		}
	};


}}} // namespace c74::min:::graphics
