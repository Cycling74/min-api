/// @file
///	@ingroup 	minapi
///	@copyright	Copyright 2018 The Min-API Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#pragma once

namespace c74::min {

// Type definition for what the legacy C Max SDK uses to represent an inlet.
using t_max_inlet = void*;

/// Represents any type of inlet.
/// Used internally to allow heterogenous containers of inlets.
class inlet_base : public port
{
    friend void object_base::create_inlets();

  public:
    inlet_base(object_base* an_owner, const std::string& a_description, const std::string& a_type = "")
        : port{ an_owner, a_description, a_type }
    {
    }

    virtual ~inlet_base() {}

    virtual bool has_attribute_mapping() const = 0;
    virtual attribute_base* attribute() const = 0;

    /// Is this inlet drawn as hot in the patcher?
    /// This only affects how the inlet is drawn. Whether input triggers output is up to the object.
    bool hot() const
    {
        return m_hot;
    }

    /// Set whether this inlet is drawn as hot or cold, redrawing the object's box if it is in a patcher.
    void hot(bool is_hot)
    {
        m_hot = is_hot;

        max::t_object* box{};
        if (m_owner->maxobj() && max::object_obex_lookup(m_owner->maxobj(), k_sym__pound_b, &box) == max::MAX_ERR_NONE && box) {
            max::jbox_redraw(reinterpret_cast<max::t_jbox*>(box));
        }
    }

  private:
    t_max_inlet m_instance{ nullptr };
    bool m_hot{ true };
};

/// An inlet.
/// The actual inlet class is a templated type.
/// This is done for consistency of coding style with outlet<>, message<>, attribute<>, etc.
/// At the moment there is no actual function for the template parameter.
/// @tparam inlet_placeholder_type	Unused. You should supply no arguments. For example, `inlet<>`.
template <placeholder inlet_placeholder_type = placeholder::none>
class inlet : public inlet_base
{
  public:
    /// Create an Inlet.
    /// @param	an_owner		The owning object for the inlet. Typically you will pass `this`.
    /// @param	a_description	Description of the inlet for documentation and UI assistance.
    /// @param	a_type			Optional string to create a type-checked inlet.
    inlet(object_base* an_owner, const std::string& a_description, const std::string& a_type = "")
        : inlet_base{ an_owner, a_description, a_type }
    {
        m_owner->inlets().push_back(this);
    }

    inlet(object_base* an_owner, const std::string& a_description, attribute_base& an_attribute_to_which_to_map_audio_input, const std::string& a_type = "")
        : inlet_base{ an_owner, a_description, a_type }
        , m_attribute{ &an_attribute_to_which_to_map_audio_input }
    {
        m_owner->inlets().push_back(this);
    }

    bool has_attribute_mapping() const override
    {
        return m_attribute != nullptr;
    }

    attribute_base* attribute() const override
    {
        return m_attribute;
    }

  private:
    attribute_base* m_attribute{ nullptr };
};

} // namespace c74::min
