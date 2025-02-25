# Copyright 2018 The Min-API Authors. All rights reserved.
# Use of this source code is governed by the MIT License found in the License.md file.


# enforce a strict warning policy for xcode
function(c74_set_target_xcode_warning_flags target)
    if (APPLE)
        set(C74_XCODE_WARNING_CFLAGS "-Wall -Wmissing-field-initializers -Wno-unused-lambda-capture -Wno-unknown-warning-option")
        if (${C74_WARNINGS_AS_ERRORS})
            set(C74_XCODE_WARNING_CFLAGS "${C74_XCODE_WARNING_CFLAGS} -Werror")
        endif ()

        set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_WARNING_CFLAGS ${C74_XCODE_WARNING_CFLAGS})
    endif ()
endfunction()
