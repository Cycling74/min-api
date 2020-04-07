/// @file
///	@ingroup 	minapi
///	@copyright	Copyright 2020 The Min-API Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#pragma once

namespace c74::min {


    /// A mouse or touch event

    class event {
    public:

        event(const atoms& args)
        : m_target { args }
        {
            if (args.size() == 1) {
                // a single atom indicates we are being passed a pointer to an event
                auto* e = static_cast<event*>( static_cast<void*>(args[0]) );
                *this = *e;
            }
            else if (args.size() != 5) {
                error("incorrect number of arguments for notification");
            }
            else {
                m_self = args[0];
                //m_target = args[1];
                m_x = args[2];
                m_y = args[3];
                m_modifiers = args[4];
            }
        }


        event(max::t_object* o, max::t_object* a_patcherview, max::t_mouseevent& a_max_mouseevent)
        : m_target { o, a_patcherview }
        {
            m_self = o;
            // m_target
            m_x = a_max_mouseevent.position.x;
            m_y = a_max_mouseevent.position.y;
            m_modifiers = a_max_mouseevent.modifiers;
        }


        /// Is this notification for a local attribute being modified?
        /// @return true if is a local attribute modified notification, otherwise false.

        bool is_command_key_down() {
            return (m_modifiers & c74::max::eCommandKey);
        }

        bool is_shift_key_down() {
            return (m_modifiers & c74::max::eShiftKey);
        }


        /// The name of the notification message
        /// @return the name of the notification

        auto target() {
            return m_target;
        }


        /// The sender of the notification message
        /// @return A pointer to the sender of the notification.

        auto x() {
            return m_x;
        }


        /// The payload or data for the notification, if any.
        /// @return a pointer to the data of the notification.

        auto y() {
            return m_y;
        }


    private:
        max::t_object*  m_self;
        ui::target      m_target;
        number          m_x;
        number          m_y;
        int             m_modifiers;
    };


}    // namespace c74::min
