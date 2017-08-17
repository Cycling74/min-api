/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

#include <sstream>
#include <unordered_map>

namespace c74 {
namespace min {

	class inlet_base;
	class outlet_base;
	class message_base;
	class attribute_base;

	/// @defgroup attributes Attributes

	/// @ingroup attributes
	using setter = function;

	/// @ingroup attributes
	using getter = std::function<atoms()>;

	/// @ingroup attributes
	#define MIN_GETTER_FUNCTION [this]()->atoms

	/// @ingroup attributes
	enum class style {
		none,
        text,
        onoff,
        rgba,
		enum_symbol,
		enum_index,
		rect,
		font,
		file,
		time,
		color
	};


	/// @ingroup attributes
	static std::unordered_map<style, symbol> style_symbols {
			{ style::text, "text"},
			{ style::onoff, "onoff"},
			{ style::rgba, "rgba"},
			{ style::enum_symbol, "enum"},
			{ style::enum_index, "enumindex"},
			{ style::rect, "rect"},
			{ style::font, "font"},
			{ style::file, "file"},
			{ style::color, "rgba"},
	};


	enum class visibility {
		show,	///< standard behavior: show the attribute to the user
		hide,	///< hide the attribute from the user
		disable	///< don't create the attribute at all
	};


	using category = symbol;
	using order = long;

	/// @ingroup attributes
	class attribute_base {

	public:
		attribute_base(object_base& an_owner, std::string a_name)
		: m_owner	{ an_owner }
		, m_name	{ a_name }
		, m_title	{ a_name }
		{}

		attribute_base(const deferred_message& other) = delete; // no copying allowed!
		attribute_base(const deferred_message&& other) = delete; // no moving allowed!


		/// set the value of the attribute
		virtual attribute_base& operator = (atoms& args) = 0;
		
		/// set the value of the attribute
		/// note that args may be modified after this call due to range limiting behavior
		virtual void set(atoms& args, bool notify = true, bool override_readonly = false) = 0;
		
		/// get the value of the attribute
		virtual operator atoms() const = 0;

		/// fetch the name of the datatype
		symbol datatype() const {
			return m_datatype;
		}

		symbol name() const {
			return m_name;
		}

		// note: cannot call it "readonly" because that is the name of a type and
		// it thus confuses the variadic template parsing below
		bool writable() const {
			return !m_readonly;
		}


		visibility visible() {
			return m_visibility;
		}


		/// fetch the title/label as a string
		const char* label_string() {
			return m_title;
		}

		std::string description_string() const {
			return m_description;
		}

		style editor_style() const {
			return m_style;
		}

		symbol editor_category() const {
			return m_category;
		}

		long editor_order() const {
			return m_order;
		}

		/// fetch the range in string format, values separated by spaces
		virtual std::string range_string() = 0;

		
		/// Create the Max attribute and add it to the Max class
		virtual void create(max::t_class* c, max::method getter, max::method setter, bool isjitclass = 0) = 0;
		
		/// calculate the offset of the size member as required for array/vector attributes
		size_t size_offset() {
			return (&m_size) - reinterpret_cast<size_t*>(&m_owner);
		}

		void touch() {
			max::object_attr_touch(m_owner, m_name);
		}

	protected:

		std::size_t flags(bool isjitclass) {
			auto attrflags = 0;

			if (!writable()) {
				attrflags |= max::ATTR_SET_OPAQUE_USER;
				attrflags |= max::ATTR_SET_OPAQUE;
			}
			if (isjitclass) {
				attrflags |= max::ATTR_GET_DEFER_LOW;
				attrflags |= max::ATTR_SET_USURP_LOW;
			}
			return attrflags;
		}

		object_base&	m_owner;
		symbol			m_name;
        symbol			m_title;
		symbol			m_datatype;
		setter			m_setter;
		getter			m_getter;
		bool			m_readonly { false };
		visibility		m_visibility { visibility::show };
		size_t			m_size;		/// size of array/vector if attr is array/vector
		description		m_description;

		style			m_style;		// display style in Max
		symbol			m_category;		// Max inspector category
		long 			m_order { 0 };	// Max inspector ordering
	};
	
	
	using range = atoms;
	using enum_map = std::vector<std::string>;
	using readonly = bool;


	/// @ingroup attributes
	template<typename T, threadsafe threadsafety, template<typename> class limit_type>
	class attribute_threadsafe_helper;

	/// @ingroup attributes
	template<typename T, threadsafe threadsafety, template<typename> class limit_type>
	void attribute_threadsafe_helper_do_set(attribute_threadsafe_helper<T,threadsafety,limit_type>* helper, atoms& args);






	/// default is `threadsafe::no`
	/// @ingroup attributes

	template<typename T, threadsafe threadsafety, template<typename> class limit_type>
	class attribute : public attribute_base {
	private:

		/// constructor utility: handle an argument defining an attribute's title / label
		template<typename argument_type>
		constexpr typename enable_if<is_same<argument_type, title>::value>::type
		assign_from_argument(const argument_type& arg) noexcept {
			const_cast<symbol&>(m_title) = arg;
		}
		
		/// constructor utility: handle an argument defining an attribute's description
		template<typename argument_type>
		constexpr typename enable_if<is_same<argument_type, description>::value>::type
		assign_from_argument(const argument_type& arg) noexcept {
			const_cast<argument_type&>(m_description) = arg;
		}

		/// constructor utility: handle an argument defining a attribute's range
		template<typename argument_type>
		constexpr typename enable_if<is_same<argument_type, range>::value>::type
		assign_from_argument(const argument_type& arg) noexcept {
			const_cast<argument_type&>(m_range_args) = arg;
		}

		/// constructor utility: handle an argument defining an enum mapping to associate strings with enum constants
		/// this is used in place of the range for index enum attributes.
		template<typename argument_type>
		constexpr typename enable_if<is_same<argument_type, enum_map>::value>::type
		assign_from_argument(const argument_type& arg) noexcept {
			const_cast<argument_type&>(m_enum_map) = arg;
		}
		
		/// constructor utility: handle an argument defining a attribute's setter function
		template<typename argument_type>
		constexpr typename enable_if<is_same<argument_type, setter>::value>::type
		assign_from_argument(const argument_type& arg) noexcept {
			const_cast<argument_type&>(m_setter) = arg;
		}

		/// constructor utility: handle an argument defining a attribute's getter function
		template<typename argument_type>
		constexpr typename enable_if<is_same<argument_type, getter>::value>::type
		assign_from_argument(const argument_type& arg) noexcept {
			const_cast<argument_type&>(m_getter) = arg;
		}
		
		/// constructor utility: handle an argument defining a attribute's readonly property
		template<typename argument_type>
		constexpr typename enable_if<is_same<argument_type, readonly>::value>::type
		assign_from_argument(const argument_type& arg) noexcept {
			const_cast<argument_type&>(m_readonly) = arg;
		}

		/// constructor utility: handle an argument defining a attribute's visibility property
		template<typename argument_type>
		constexpr typename enable_if<is_same<argument_type, visibility>::value>::type
		assign_from_argument(const argument_type& arg) noexcept {
			const_cast<argument_type&>(m_visibility) = arg;
		}

		/// constructor utility: handle an argument defining a attribute's style property
		template<typename argument_type>
		constexpr typename enable_if<is_same<argument_type, style>::value>::type
		assign_from_argument(const argument_type& arg) noexcept {
			const_cast<argument_type&>(m_style) = arg;
		}

		/// constructor utility: handle an argument defining a attribute's category property
		template<typename argument_type>
		constexpr typename enable_if<is_same<argument_type, category>::value>::type
		assign_from_argument(const argument_type& arg) noexcept {
			const_cast<argument_type&>(m_category) = arg;
		}

		/// constructor utility: handle an argument defining a attribute's order property
		template<typename argument_type>
		constexpr typename enable_if<is_same<argument_type, order>::value>::type
		assign_from_argument(const argument_type& arg) noexcept {
			const_cast<argument_type&>(m_order) = arg;
		}


		/// constructor utility: empty argument handling (required for recursive variadic templates)
		constexpr void handle_arguments() noexcept {
			;
		}
		
		/// constructor utility: handle N arguments of any type by recursively working through them
		///	and matching them to the type-matched routine above.
		template <typename FIRST_ARG, typename ...REMAINING_ARGS>
		constexpr void handle_arguments(FIRST_ARG const& first, REMAINING_ARGS const& ...args) noexcept {
			assign_from_argument(first);
			if (sizeof...(args) > 0)
				handle_arguments(args...); // recurse
		}
		
		
	public:
		/// Constructor
		/// @param an_owner			The instance pointer for the owning C++ class, typically you will pass 'this'
		/// @param a_name			A string specifying the name of the attribute when dynamically addressed or inspected.
		/// @param a_default_value	The default value of the attribute, which will be set when the instance is created.
		/// @param ...args			N arguments specifying optional properties of an attribute such as setter, label, style, etc.
		template<typename ...ARGS>
		attribute(object_base* an_owner, std::string a_name, T a_default_value, ARGS... args);

		attribute(const deferred_message& other) = delete; // no copying allowed!
		attribute(const deferred_message&& other) = delete; // no moving allowed!

		
		/// Set the attribute value using the native type of the attribute.
		attribute& operator = (const T arg) {
			atoms as = { atom(arg) };
			*this = as;
			return *this;
		}
		
		/// Set the attribute value using atoms.
		attribute& operator = (atoms& args) {
			set(args);
			return *this;
		}

		/// Set the attribute value using atoms.
		attribute& operator = (const atoms& args) {
			atoms as {args};
			set(as);
			return *this;
		}

		// special setter for enum attributes
		template<class U=T, typename enable_if< is_enum<U>::value, int>::type = 0>
		attribute& operator = (symbol arg) {
			for (auto i=0; i<m_enum_map.size(); ++i) {
				if (arg == m_enum_map[i]) {
					*this = static_cast<T>(i);
					break;
				}
			}
			return *this;
		}
		
		

		friend bool operator == (const attribute& lhs, const T& rhs) {
			return lhs.m_value == rhs;
		}


		template<class U=T, typename enable_if< is_same<limit_type<U>, limit::none<U>>::value, int>::type = 0>
		void constrain(atoms& args) {
			// no limiting, so do nothing
		}

		template<class U=T, typename enable_if< !is_same<limit_type<U>, limit::none<U>>::value, int>::type = 0>
		void constrain(atoms& args) {
			// TODO: type checking on the above so that it is not applied to vectors or colors
			args[0] = limit_type<T>::apply(args[0], m_range[0], m_range[1]);
		}


		template<class U=T, typename enable_if< !is_enum<U>::value, int>::type = 0>
		void assign(const atoms& args) {
			m_value = from_atoms<T>(args);
		}

		template<class U=T, typename enable_if< is_enum<U>::value, int>::type = 0>
		void assign(const atoms& args) {
			const atom& a = args[0];

			if (a.a_type == max::A_SYM) {
				for (auto i=0; i<m_enum_map.size(); ++i) {
					if (a == m_enum_map[i]) {
						m_value = static_cast<T>(i);
						break;
					}
				}
			}
			else
				m_value = from_atoms<T>(args);
		}

		
		/// Set the attribute value

		void set(atoms& args, bool notify = true, bool override_readonly = false) {
			if (notify && this_class)
				max::object_attr_setvalueof(m_owner, m_name, static_cast<long>(args.size()), static_cast<const c74::max::t_atom*>(&args[0]));

			if (!writable() && !override_readonly)
				return; // we're all done... unless this is a readonly attr that we are forcing to update

			// currently all jitter attributes bypass the defer mechanism here opting to instead use the default jitter handling
			// were we to simply call `m_helper.set(args);` then our defer mechanism would be called **in addition to** jitter's deferring

			if (m_owner.is_jitter_class())
				attribute_threadsafe_helper_do_set<T,threadsafety>(&m_helper, args);
			else
				m_helper.set(args);
		}


		// Apply range limiting, if any, to input
		T range_apply(const T& value);
		
		
		/// Create the Max attribute and add it to the Max class
		void create(max::t_class* c, max::method getter, max::method setter, bool isjitclass = 0);
		
		
		operator atoms() const {
			if (m_getter)
				return m_getter();
			else
				return to_atoms(m_value);
		}


		// we need to return by const reference due to cases where the type of the attribute is a class
		// for example, a time_value attribute cannot be copy constructed

		operator const T&() const {
			if (m_getter)
				assert(false); // at the moment there is no easy way to support this
			return m_value;
		}

		// getting a writable reference to the underlying data is of particular importance
		// for e.g. vector<number> attributes

		operator T&() {
			if (m_getter)
				assert(false); // at the moment there is no easy way to support this
			return m_value;
		}


		// simplify getting millisecond time from a time_value attribute

		template<class U=T, typename enable_if< is_same<U, time_value>::value, int>::type = 0>
		operator double() const {
			return m_value;
		}

		
		template<class U=T, typename enable_if< is_same<U, numbers>::value, int>::type = 0>
		double& operator[](size_t index) {
			return m_value[index];
		}


		std::string range_string();


		enum_map get_enum_map() {
			return m_enum_map;
		}

		atoms get_range_args() {
			return m_range_args;
		}

		std::vector<T>& range_ref() {
			return m_range;
		}
        
        bool disabled() const {
            return c74::max::object_attr_getdisabled(m_owner, m_name);
        }
        
        void disable(bool value) {
            c74::max::object_attr_setdisabled(m_owner, m_name, value);
        }

	private:
		T				m_value;
		atoms			m_range_args;	// the range/enum as provided by the subclass
		std::vector<T>	m_range;		// the range/enum translated into the native datatype
		enum_map		m_enum_map;		// the range/enum mapping for indexed enums (as opposed to symbol enums)
		attribute_threadsafe_helper<T,threadsafety,limit_type>	m_helper { this };

		void copy_range();				// copy m_range_args to m_range

		friend void attribute_threadsafe_helper_do_set<T,threadsafety,limit_type>(attribute_threadsafe_helper<T,threadsafety,limit_type>* helper, atoms& args);
	};

/*	The setter/getter for the wrapper are below.
	Regarding thread-safety...

	First, lets just consider the getter.
	We can't defer here -- it must happen at a higher level.  Like in Max somewhere.  
	Or we lock Max with a critical region?  Ick?
	This is because we are required to return the value syncronously.
	So getters are out of our control.
 
	Second, let's consider the setter.
	We can defer this (and should if the setter is not threadsafe).
	
	If we defer, we need to do it in the attribut<>::set() method
	because it is common to set the attr value from calls other than just the outside Max call.
	Unfortunately, we cannot do a partial template specialization for a member function in C++.
	So the set method is then required to call a templated class (which can be partially specialized) as a functor.

	That is what we have here:
 */

	/// @ingroup attributes
	template<typename T, threadsafe threadsafety, template<typename> class limit_type>
	void attribute_threadsafe_helper_do_set(attribute_threadsafe_helper<T,threadsafety,limit_type>* helper, atoms& args) {
		auto& attr = *helper->m_attribute;

		attr.constrain(args);

		if (attr.m_setter)
			attr.m_value = from_atoms<T>(attr.m_setter(args));
		else
			attr.assign(args);
	}

	/// @ingroup attributes
	template<typename T, template<typename> class limit_type>
	class attribute_threadsafe_helper<T,threadsafe::yes,limit_type> {
		friend void attribute_threadsafe_helper_do_set<T,threadsafe::yes,limit_type>(attribute_threadsafe_helper<T,threadsafe::yes,limit_type>* helper, atoms& args);
	public:
		explicit attribute_threadsafe_helper(attribute<T,threadsafe::yes,limit_type>* an_attribute)
		: m_attribute ( an_attribute )
		{}

		void set(atoms& args) {
			attribute_threadsafe_helper_do_set(this, args);
		}

	private:
		attribute<T,threadsafe::yes,limit_type>*	m_attribute;
	};


	/// @ingroup attributes
	template<typename T, threadsafe threadsafety, template<typename> class limit_type>
	void attribute_threadsafe_helper_qfn(attribute_threadsafe_helper<T,threadsafety,limit_type>* helper) {
		static_assert(threadsafety == threadsafe::no, "helper function should not be called by threadsafe attrs");
		attribute_threadsafe_helper_do_set<T,threadsafety,limit_type>(helper, helper->m_value);
	}


	/// @ingroup attributes
	template<typename T, template<typename> class limit_type>
	class attribute_threadsafe_helper<T,threadsafe::no,limit_type> {
		friend void attribute_threadsafe_helper_do_set<T,threadsafe::no,limit_type>(attribute_threadsafe_helper<T,threadsafe::no,limit_type>* helper, atoms& args);
		friend void attribute_threadsafe_helper_qfn<T,threadsafe::no,limit_type>(attribute_threadsafe_helper<T,threadsafe::no,limit_type>* helper);
	public:
		explicit attribute_threadsafe_helper(attribute<T,threadsafe::no,limit_type>* an_attribute)
		: m_attribute ( an_attribute )
		{
			m_qelem = (max::t_qelem*)max::qelem_new(this, (max::method)attribute_threadsafe_helper_qfn<T,threadsafe::no,limit_type>);
		}

		~attribute_threadsafe_helper() {
			max::qelem_free(m_qelem);
		}

		void set(atoms& args) {
			if (max::systhread_ismainthread())
				attribute_threadsafe_helper_do_set(this, args);
			else {
				m_value = args;
				max::qelem_set(m_qelem);
			}
		}

	private:
		attribute<T,threadsafe::no,limit_type>*	m_attribute;
		max::t_qelem*							m_qelem;
		atoms									m_value;
	};



	/*	Native Max methods for the wrapper class
		to perform getting /setting attributes
	 */

	/// @ingroup attributes
	template<class T>
	max::t_max_err min_attr_getter(minwrap<T>* self, max::t_object* maxattr, long* ac, max::t_atom** av) {
		symbol	attr_name	= static_cast<max::t_symbol*>(max::object_method(maxattr, k_sym_getname));
		auto&	attr		= self->min_object.attributes()[attr_name.c_str()];
		atoms	rvals		= *attr;
		
		*ac = (long)rvals.size();
		if (!(*av)) // otherwise use memory passed in
			*av = (max::t_atom*)max::sysmem_newptr(sizeof(max::t_atom) * *ac);
		for (auto i=0; i<*ac; ++i)
			(*av)[i] = rvals[i];
		
		return 0;
	}
	
	
	/// @ingroup attributes
	template<class T>
	max::t_max_err min_attr_setter(minwrap<T>* self, max::t_object* maxattr, long ac, max::t_atom* av) {
		atom_reference	args(ac,av);
		symbol			attr_name	= static_cast<max::t_symbol*>(max::object_method(maxattr, k_sym_getname));
		auto			attr		= self->min_object.attributes()[attr_name.c_str()];

		if (attr) {
			atoms as(args.begin(), args.end());
			attr->set(as, false, false);
		}
		return 0;
	}
	
}} // namespace c74::min
