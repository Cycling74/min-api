/// @file
///	@ingroup 	minapi
///	@copyright	Copyright 2018 The Min-API Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#pragma once

namespace c74::min {

#define MIN_TAGS static constexpr const char* class_tags

using tags = std::vector<std::string>;

template <class T, class = void>
struct has_class_tags : std::false_type {};

template <class T>
struct has_class_tags<T, std::void_t<decltype(&T::class_tags)>> : std::true_type {};

template <class min_class_type>
void get_tags(tags& returned_tags)
{
    if constexpr (has_class_tags<min_class_type>::value) {
        returned_tags = str::split(min_class_type::class_tags, ',');
    }
    else {
        returned_tags = {};
    }
}

// Detects a member named mousedragdelta or m_mousedragdelta.
// NOTE: Matches the C++ member name, not the Max message name.
template <class T, class = void>
struct has_mousedragdelta : std::false_type {};

template <class T>
struct has_mousedragdelta<T, std::void_t<decltype(&T::mousedragdelta)>> : std::true_type {};

template <class T, class = void>
struct has_m_mousedragdelta : std::false_type {};

template <class T>
struct has_m_mousedragdelta<T, std::void_t<decltype(&T::m_mousedragdelta)>> : std::true_type {};

// Detects a member named focusgained or m_focusgained.
// NOTE: Matches the C++ member name, not the Max message name.
template <class T, class = void>
struct has_focusgained : std::false_type {};

template <class T>
struct has_focusgained<T, std::void_t<decltype(&T::focusgained)>> : std::true_type {};

template <class T, class = void>
struct has_m_focusgained : std::false_type {};

template <class T>
struct has_m_focusgained<T, std::void_t<decltype(&T::m_focusgained)>> : std::true_type {};

/// The base class for all first-class objects that are to be exposed in the Max environment.
///
/// We pass the class type as a template parameter to this class.
/// This allows for [static polymorphism](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern#Static_polymorphism).
/// One benefits of this are leveraged when instantiating class instances directly instead of through the Max interface,
/// such as when unit testing or embedding an object inside of another object.
///
/// @tparam min_class_type	The name of your class that is extenting min::object.
/// @tparam threadsafety	The default threadsafety assumption for all messages and attributes in this class.
template <class min_class_type, threadsafe threadsafety = threadsafe::no>
class object : public object_base
{
  public:
    /// Constructor.
    object()
    {

        // The way objects are created for the Max environment requires that memory be allocated first
        // using object_alloc() or jit_object_alloc(), which is followed by the use of placement-new to contruct the C++ class.
        //
        // When this occurs the m_maxobj member is already set prior to the constructor being run.
        // If there is no valid m_maxobj then that means this class was created outside of the Max environment.
        //
        // This could occur if a class uses another class directly or in the case of unit testing.
        // In such cases we need to do something reasonable so that our invariants can be held true
    }

    /// Destructor.
    virtual ~object()
    {
    }

    bool is_jitter_class() const override
    {
        return is_base_of<matrix_operator_base, min_class_type>::value;
    };

    bool is_ui_class() const override
    {
        return is_base_of<ui_operator_base, min_class_type>::value;
    }

    bool has_mousedragdelta() const override
    {
        return c74::min::has_mousedragdelta<min_class_type>::value || has_m_mousedragdelta<min_class_type>::value;
    }

    bool is_focusable() const override
    {
        return c74::min::has_focusgained<min_class_type>::value || has_m_focusgained<min_class_type>::value;
    }

    bool is_assumed_threadsafe() const override
    {
        return threadsafety == threadsafe::yes;
    }

    virtual strings tags() const override
    {
        strings t;
        get_tags<min_class_type>(t);
        for (auto& a_tag : t) {
            a_tag = str::trim(a_tag);
        }
        return t;
    }

  protected:
    logger cout{ this, logger::type::message };
    logger cwarn{ this, logger::type::warning };
    logger cerr{ this, logger::type::error };
};

} // namespace c74::min
