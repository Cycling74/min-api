/// @file
///	@ingroup 	minapi
///	@copyright	Copyright 2020 The Min-API Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#pragma once

namespace c74::min {


    /// A mouse or touch event

    class event {
    public:

        /// Represent message arguments as an instance of a notification
        ///
        /// @param  args    Five atoms passed as a notification in Max's object listener system, described as follows:
        /// * arg0    self
        /// * arg1    s         the registered name of the sending object
        /// * arg2    msg        then name of the notification/message sent
        /// * arg3    sender    the pointer to the sending object
        /// * arg4    data    optional argument sent with the notification/message

        event(const atoms& args)
        : m_target { args }
        {
            if (args.size() != 5)
                error("incorrect number of arguments for notification");

            m_self = args[0];
            // m_target = args[1];
            m_x = args[2];
            m_y = args[3];
            m_modifiers = args[4];
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
