/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

#include <sstream>
#include <unordered_map>

namespace c74 {
namespace min {

	
	/// @defgroup attributes Attributes


	/// A callback function used by attributes the provide an optional customized "setter" routine.
	/// @ingroup attributes

	using setter = function;


	/// A callback function used by attributes the provide an optional customized "getter" routine.
	/// Typically this is provided to argument as a lamba function using the #MIN_GETTER_FUNCTION macro.
	/// @ingroup	attributes
	/// @return		A vector of atoms that represent the current state of the attribute.
	/// @see		MIN_GETTER_FUNCTION

	using getter = std::function<atoms()>;


	/// Provide the correct lamba function prototype for a getter parameter to the min::attribute constructor.
	/// @ingroup	attributes
	/// @see		getter
	/// @see		attribute

	#define MIN_GETTER_FUNCTION [this]()->atoms


	/// A high-level meta-type that is associated with an attribute.
	/// These are used to enhance the experience of editing the attribute value using the inspector or attr ui.
	/// @ingroup attributes

	enum class style {
		none,			/// No special style.
        text,			/// Provide a text editor.
        onoff,			/// Edit using an on/off switch or toggle.
 		enum_symbol,	/// Provide a list or menu of options, the actual stored attribute is a symbol.
		enum_index,		/// Provide a list or menu of options, the actual stored attribute is an int.
		rect,			/// Rectangular coordinate editor.
		font,			/// Provide a font dialog.
		file,			/// Provide a file chooser.
		time,			/// ITM time attributes many also specify a type of time (e.g. notes, bars-beats-units, etc.)
		color			/// Provide high-level color editors and swatches.
	};


	/// Symbolic names associated with the values of the enum min::style.
	/// @ingroup attributes
	/// @see style

	static std::unordered_map<style, symbol> style_symbols {
			{ style::text,			"text"},
			{ style::onoff,			"onoff"},
			{ style::enum_symbol,	"enum"},
			{ style::enum_index,	"enumindex"},
			{ style::rect,			"rect"},
			{ style::font,			"font"},
			{ style::file,			"file"},
			{ style::color,			"rgba"},
	};


	/// If the style of an attribute is one of the 'enum' types then an 'enum_map' maybe be
	/// supplied which provides symbolic names for each of the enum options.
	/// @ingroup attributes
	/// @see style

	using enum_map = std::vector<std::string>;


	/// Declare an attribute's visibility to the user.
	/// @ingroup attributes

	enum class visibility {
		show,	///< standard behavior: show the attribute to the user
		hide,	///< hide the attribute from the user
		disable	///< don't create the attribute at all
	};


	/// Defines an attribute's category (group) in the inspector.
	/// @ingroup attributes

	using category = symbol;


	/// Defines attribute ordering in the inspector.
	/// A value zero means there is no special order and Max will take care of the ordering automatically.
	/// @ingroup attributes

	using order = long;


	/// The range provides a definition of acceptable or 'normal' values for an attribute.
	/// Unless specified as a template-parameter to the attribute, the range is only a suggestion to the user.
	/// Ranges for numeric types will be two numbers (a low bound and a high bound).
	/// For symbols or enums the range will provide the available options.
	/// @ingroup attributes

	using range = atoms;


	/// Defines whether an attribute is readonly (true) or if it is also writable (false).
	/// @ingroup attributes

	using readonly = bool;


	// Represents any type of attribute.
	// Used internally to allow heterogenous containers of attributes for the Min class.
	/// @ingroup attributes

	class attribute_base {
	protected:

		// Constructor. See the constructor documention for min::attribute<> to get more details on the arguments.

		attribute_base(object_base& an_owner, std::string a_name)
		: m_owner	{ an_owner }
		, m_name	{ a_name }
		, m_title	{ a_name }
		{}

	public:

		attribute_base(const deferred_message& other) = delete; // no copying allowed!
		attribute_base(const deferred_message&& other) = delete; // no moving allowed!


		// All attributes must define what happens when you set their value.

		virtual attribute_base& operator = (atoms& args) = 0;


		// All attributes must define what happens when you set their value.
		// NOTE: args may be modified after this call due to range limiting behavior

		virtual void set(atoms& args, bool notify = true, bool override_readonly = false) = 0;


		// All attributes must define what happens when you get their value.

		virtual operator atoms() const = 0;


		// All attributes must define what happens when asked for their range of values.
		// The range must be in string format, values separated by spaces.

		virtual std::string range_string() = 0;


		// All attributes must define what happens in the Max wrapper to
		// create the Max attribute and add it to the Max class.
		// Not intended for public use, but made a public member due to the
		// difficulty of making friends of the heavily templated SFINAE wrapper code.

		virtual void create(max::t_class* c, max::method getter, max::method setter, bool isjitclass = 0) = 0;


		/// Determine the name of the datatype
		/// @return	The name of the datatype of the attribute.

		symbol datatype() const {
			return m_datatype;
		}


		/// Determine the name of the attribute
		/// @return	The name of the attribute.

		symbol name() const {
			return m_name;
		}


		/// Is the attribute writable (meaning settable)?
		/// @return	True if it is writable. Otherwise false.

		// note: cannot call it "readonly" because that is the name of a type and
		// it thus confuses the variadic template parsing below

		bool writable() const {
			return !m_readonly;
		}


		/// Is the attribute visible?
		/// @return The attribute's current visibility flag.

		visibility visible() {
			return m_visibility;
		}


		/// Fetch the title/label as a string.
		/// This is how the name appears in the inspector.
		/// @return The attribute's label.
		///			If the attribute has no label then the name of the object is used as the default.

		const char* label_string() {
			return m_title;
		}


		/// Return the provided description for use in documentation generation, auto-complete, etc.
		/// @return	The description string supplied when the attribute was created.

		std::string description_string() const {
			return m_description;
		}


		/// Return the style that is to be used for attribute editors such as the attrui object and the Max inspector.
		/// @return	The style supplied when the attribute was created.

		style editor_style() const {
			return m_style;
		}


		/// Return the category under which the attribute should appear in the Max inspector.
		/// @return	The category supplied when the attribute was created.

		symbol editor_category() const {
			return m_category;
		}


		/// Return the ordering priority for the attribute when listed in the inspector.
		/// @return	The order priority supplied when the attribute was created.

		long editor_order() const {
			return m_order;
		}


		/// Touch the attribute to force an update and notification of its value to any listeners.

		void touch() {
			max::object_attr_touch(m_owner, m_name);
		}

	protected:
		object_base&	m_owner;
		symbol			m_name;
		symbol			m_title;
		symbol			m_datatype;
		setter			m_setter;
		getter			m_getter;
		bool			m_readonly { false };
		visibility		m_visibility { visibility::show };
		description		m_description;
		size_t			m_size;			// size of array/vector if attr is array/vector

		style			m_style;		// display style in Max
		symbol			m_category;		// Max inspector category
		long 			m_order { 0 };	// Max inspector ordering


		// calculate the offset of the size member as required for array/vector attributes

		size_t size_offset() {
			return (&m_size) - reinterpret_cast<size_t*>(&m_owner);
		}


		// return flags required by the max/obex attribute to get the correct behavior

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
	};


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

		void set(atoms& args, bool notify = true, bool override_readonly = false) {
			if (!writable() && !override_readonly)
				return; // we're all done... unless this is a readonly attr that we are forcing to update

#ifndef MIN_TEST // At this time the Mock Kernel does not implement object_attr_setvalueof(), so we can't use it for unit tests
			if (notify && this_class) {		// Use the Max API to set the attribute value
				max::object_attr_setvalueof(m_owner, m_name, static_cast<long>(args.size()), static_cast<const c74::max::t_atom*>(&args[0]));
			}
			else
#endif // !MIN_TEST
			{								// Set the value ourselves
				// currently all jitter attributes bypass the defer mechanism here opting to instead use the default jitter handling
				// were we to simply call `m_helper.set(args);` then our defer mechanism would be called **in addition to** jitter's deferring

				if (m_owner.is_jitter_class())
					attribute_threadsafe_helper_do_set<T,threadsafety>(&m_helper, args);
				else
					m_helper.set(args);
			}
		}


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



	//	Native Max methods for the wrapper class to perform getting of attribute values

	template<class T>
	max::t_max_err min_attr_getter(minwrap<T>* self, max::t_object* maxattr, long* ac, max::t_atom** av) {
		symbol	attr_name	= static_cast<max::t_symbol*>(max::object_method(maxattr, k_sym_getname));
		auto&	attr		= self->m_min_object.attributes()[attr_name.c_str()];
		atoms	rvals		= *attr;
		
		*ac = (long)rvals.size();
		if (!(*av)) // otherwise use memory passed in
			*av = (max::t_atom*)max::sysmem_newptr(sizeof(max::t_atom) * *ac);
		for (auto i=0; i<*ac; ++i)
			(*av)[i] = rvals[i];
		
		return 0;
	}
	

	//	Native Max methods for the wrapper class to perform setting of attribute values

	template<class T>
	max::t_max_err min_attr_setter(minwrap<T>* self, max::t_object* maxattr, long ac, max::t_atom* av) {
		atom_reference	args(ac,av);
		symbol			attr_name	= static_cast<max::t_symbol*>(max::object_method(maxattr, k_sym_getname));
		auto			attr		= self->m_min_object.attributes()[attr_name.c_str()];

		if (attr) {
			atoms as(args.begin(), args.end());
			attr->set(as, false, false);
		}
		return 0;
	}
	
}} // namespace c74::min
