/// @file
///	@ingroup 	minapi
///	@copyright  Copyright 2020 The Min-API Authors. All rights reserved.
///	@license           Use of this source code is governed by the MIT License found in the License.md file.

#pragma once

namespace c74::min {


    /// An instance of an object.
    /// This could be a box, a patcher, or anything else that is a live instance of a class in Max.

    class instance {
    public:
        instance(max::t_object* an_instance = nullptr)
        : m_instance { an_instance }
        {}

        virtual ~instance() {
            if (m_own && m_instance)
                object_free(m_instance);
        }


        operator max::t_object*() const {
            return m_instance;
        }

        operator bool() const {
            return m_instance != nullptr;
        }


        template<typename T1>
        void instantiate(symbol a_name, T1 arg1) {
            if (m_instance && m_own)
                max::object_free(m_instance);
            m_instance = max::object_new(max::CLASS_NOBOX, a_name, arg1, 0);
        }

        
        /// call a method on an instance

        void* operator()(symbol method_name) {
            return object_method(m_instance, method_name);
        }

        template<typename T1>
        void* operator()(symbol method_name, T1 arg1) {
            return object_method(m_instance, method_name, arg1);
        }

        template<typename T1, typename T2>
        void* operator()(symbol method_name, T1 arg1, T2 arg2) {
            return object_method(m_instance, method_name, arg1, arg2);
        }

        /// Set and get attributes of an instance

        void set(symbol attribute_name, symbol value) {
            max::object_attr_setsym(m_instance, attribute_name, value);
        }

        void set(symbol attribute_name, char value) {
            max::object_attr_setchar(m_instance, attribute_name, value);
        }

        template<typename T>
        T get(symbol attribute_name) {
            long argc {};
            max::t_atom* argv {};

            max::object_attr_getvalueof(m_instance, attribute_name, &argc, &argv);
            return static_cast<T>(atom(argv));
        }


    protected:
        max::t_object*  m_instance;
        bool            m_own {};
    };


    class box : public instance {
    public:
        box(max::t_object *a_box)
        : instance { a_box }
        {}

        symbol classname() const {
            return max::jbox_get_maxclass(m_instance);
        }
        
        symbol path() const {
            return max::jbox_get_boxpath(m_instance);
        }
    };
    
    using boxes = std::vector<box>;


    class device : public instance {
    public:
        device(max::t_object* a_device = nullptr)
        : instance { a_device }
        {}
    };


    /// A reference to a buffer~ object.
    /// The buffer_reference automatically adds the management hooks required for your object to work with a buffer~.
    /// This includes adding a 'set' message and a 'dblclick' message as well as dealing with notifications and binding.
    /// @ingroup buffers

    class patcher : public instance {
    public:
        /// Create a reference to a buffer~ object.
        /// @param	an_owner	The owning object for the buffer reference. Typically you will pass `this`.
        /// @param	a_function	An optional function to be executed when the buffer reference issues notifications.
        ///						Typically the function is defined using a C++ lambda with the #MIN_FUNCTION signature.

        // takes a single arg, but cannot be marked explicit unless we are willing to decorate all using code with a cast to this type
        // thus we ignore the advice of C.46 @ https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md

        patcher(c74::max::t_object* a_patcher)
        : instance { a_patcher }
        {}


        min::device device() {
            max::t_object* m4l_device {};
            max::object_obex_lookup(m_patcher, symbol("##plugdevice##"), &m4l_device);
            return m4l_device;
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
