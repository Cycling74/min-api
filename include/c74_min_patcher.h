/// @file
///	@ingroup 	minapi
///	@copyright  Copyright 2020 The Min-API Authors. All rights reserved.
///	@license           Use of this source code is governed by the MIT License found in the License.md file.

#pragma once

namespace c74::min {
    
    class box {
    public:
        box(max::t_object *a_box)
        : m_box { a_box }
        {}
        
        operator max::t_object*() const {
            return m_box;
        }

        symbol classname() const {
            return max::jbox_get_maxclass(m_box);
        }
        
        symbol path() const {
            return max::jbox_get_boxpath(m_box);
        }

    private:
        max::t_object* m_box {};
    };
    
    using boxes = std::vector<box>;


    class device {
    public:
        device(max::t_object* a_device = nullptr)
        : m_device { a_device }
        {}

        operator bool() const {
            return m_device != nullptr;
        }

        operator max::t_object*() const {
            return m_device;
        }


    private:
        max::t_object*  m_device;
    };


    /// @defgroup buffers Buffer Objects

    /// A reference to a buffer~ object.
    /// The buffer_reference automatically adds the management hooks required for your object to work with a buffer~.
    /// This includes adding a 'set' message and a 'dblclick' message as well as dealing with notifications and binding.
    /// @ingroup buffers

    class patcher {
    public:
        /// Create a reference to a buffer~ object.
        /// @param	an_owner	The owning object for the buffer reference. Typically you will pass `this`.
        /// @param	a_function	An optional function to be executed when the buffer reference issues notifications.
        ///						Typically the function is defined using a C++ lambda with the #MIN_FUNCTION signature.

        // takes a single arg, but cannot be marked explicit unless we are willing to decorate all using code with a cast to this type
        // thus we ignore the advice of C.46 @ https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md

        patcher(c74::max::t_object* a_patcher)
        : m_patcher { a_patcher }
        {}


        min::device device() {
            max::t_object* m4l_device {};
            max::object_obex_lookup(m_patcher, symbol("##plugdevice##"), &m4l_device);
            return m4l_device;
        }

        operator max::t_object*() const {
            return m_patcher;
        }


        /// Bind the buffer reference to a buffer with a different name.
        /// @param	name	The name of the buffer~ with which to bind the reference.

        min::boxes boxes() {
            m_boxes.clear();

            auto box = max::jpatcher_get_firstobject(m_patcher);
            while (box) {
                m_boxes.push_back(box);
                box = max::jbox_get_nextobject(box);
            }
            return m_boxes;
        }
        
        symbol name() {
             return max::jpatcher_get_name(m_patcher);
        }

    private:
        max::t_object*  m_patcher   {};
        min::boxes      m_boxes     {};
    };


}    // namespace c74::min
