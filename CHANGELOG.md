# Changelog

This file records breaking changes, additions, and bug fixes in Min-API. Before
0.6.0, it lists only breaking changes and history before v0.2.0 is not covered.

## [Unreleased]

### Breaking

- `atom_reference` (the result of `dict::operator[]` and `dict::at()`) gained
  conversions to `std::vector<number>` and `std::vector<symbol>`. Assigning a
  dict entry to an `attribute<numbers>` is now ambiguous and fails to compile,
  as does passing one to any overload set that takes both `numbers` and
  `atoms`.
  ```cpp
  // before
  my_attr = d["key"];
  // after
  my_attr = static_cast<atoms>(d["key"]);
  ```
- Setting a non-list attribute (anything other than `numbers` or `ints`) with
  no arguments is now ignored and the setter is not called. Previously the
  setter ran and read `args[0]` from an empty list. ([#204])
- `quittask` is now a reserved message name. A `message<>` with that name is
  no longer callable from a patcher; it is called when Max quits.
- `dict::keys()` returns keys in insertion order rather than the dictionary's
  internal order.
- Assigning an `atom` to a `dict` copies the atom's dictionary contents into
  the existing dictionary instead of rebinding to it, matching assignment from
  another `dict`. Code that wrote through the dict to modify the incoming
  dictionary should construct a new `dict` from the atom instead.
  ```cpp
  // before
  d = args[0];
  // after
  dict incoming { args[0] };
  ```
- `dict`'s conversion to `max::t_object*` is `explicit`. It retains the
  dictionary, so implicit conversions, including constructing an `atom` from a
  `dict`, leaked a reference. Use `instance()` for a non-retaining pointer, or
  cast explicitly and release the result with `object_free()`.

### Added

- `atom_reference` converts to `std::vector<number>` and
  `std::vector<symbol>`. ([#203], [#206])
- `min::event` has `is_left_button_down()`, `is_right_button_down()`,
  `is_middle_button_down()`, and `modifiers()`. ([#195])
- `inlet<>::hot()` gets and sets whether an inlet is drawn as hot or cold.
  ([#139])
- A `message<>` named `quittask` is called when Max quits. ([#121])
- `dict::append_atoms()` stores a list of atoms under a key. ([#82])
- `dict::for_each()` visits every entry in insertion order, and
  `dict::read()` and `dict::write()` load and save JSON files.
- `dict::instance()` returns the underlying `max::t_dictionary*` without
  retaining it.
- `dict` has a move constructor.
- Defining `C74_NO_MIN_LIB_SUBMODULE` stops `c74_min.h` from picking up a
  sibling `min-lib` checkout through `__has_include`. ([#211])

### Fixed

- Copying a `dict` that owns its dictionary no longer frees the dictionary
  twice; the copy now retains it.
- Self-assignment of a `dict` no longer clones the dictionary onto itself.
- A `dict` constructed from the name of an existing dictionary releases it
  with `dictobj_release()`, matching `dictobj_findregistered_retain()`.
- Constructing a `dict` from empty dict-syntax creates an empty dictionary
  instead of indexing an empty vector.
- Vector operators update the signal connection state of their outlets.
  Previously outlets were skipped when an object had at least as many inlets
  as outlets, and non-signal outlets threw off the indexing. ([#160])
- Matrix operators value-initialize their temporary plane arrays. ([#143])
- Sending to an outlet that has no Max counterpart is a no-op instead of a
  crash.
- Unit tests that call `object_register` now link against the mock kernel.
  ([#181])

## [0.6.11]

### Breaking

- Enum attributes now set the Max `min` and `max` attribute attributes from
  their `range`, so the inspector and `attrui` constrain them. See 0.6.8.
- Builds on platforms other than macOS and Windows now produce externals
  with the suffix `.mxl_<processor>`, and `max-linker-flags.txt` is applied
  only on macOS (from max-sdk-base).

### Added

- `attribute_base::set_visibility()`.
- `dict::entrycount()` and `dict::keys()`. ([#202], [min-devkit#222])

### Fixed

- The `assist` method checks the inlet or outlet index, fixing a crash when
  MC Wrapper adds an extra outlet.
- Reference pages list attributes in alphabetical order.
- `c74_min_limit.h` includes `<algorithm>`.
- MSVC warning C4130 (logical operation on address of string constant) in the
  object wrapper. ([#153])

## [0.6.10]

### Breaking

- Integer and bool attributes now set Max `min` and `max` attribute attributes
  from their `range`. See 0.6.8.

## [0.6.9]

### Fixed

- `c74_min_api.h` includes `<iterator>`.

## [0.6.8]

### Breaking

- Float attributes now set Max `min` and `max` attribute attributes from their
  `range`. Previously a `range` was only enforced when the attribute had a
  clamping `limit`; now the inspector and `attrui` constrain values to the
  range even without one.

## [0.6.7]

### Breaking

- `attribute_base` has a new pure virtual `atoms get_atoms() const`. Classes
  deriving directly from `attribute_base` must implement it. `attribute<T>`
  already does.

## [0.6.6]

### Added

- `attribute<T>::get_atoms()` returns the value as `atoms`, using the getter
  if there is one.

### Fixed

- `attribute<T>::set_range()` no longer fails to compile on Windows because of
  an ambiguous conversion to `atoms`.

## [0.6.4]

### Breaking

- The `#undef` of `object_method`, `object_new`, and `jit_object_new` added in
  0.6.2 was removed; code that worked around it can drop the workaround.

## [0.6.3]

### Breaking

- macOS externals are now code-signed after every build by default (from
  max-sdk-base), using an ad-hoc identity unless `MAX_SDK_CODESIGN_IDENTITY`
  is set. Packages with their own signing step can pass
  `-DMAX_SDK_CODESIGN_EXTERNS=OFF`.

## [0.6.2]

### Breaking

- `c74_min_api.h` `#undef`s the Max SDK macros `object_method`, `object_new`,
  and `jit_object_new`. Code using them must call the `c74::max::` functions
  directly. Reverted in 0.6.4.

## [0.6.1]

### Breaking

- Assigning an unscoped enum to an existing `atom` now stores a float instead
  of an int, because `atom::operator=` for integers only accepts integral
  types and enums fall through to the `double` overload. Constructing an atom
  from an enum (`atom a = e;`, `atoms{e}`) still stores an int.
  ```cpp
  // before
  a = my_enum;
  // after
  a = static_cast<long>(my_enum);
  ```
- `vector_size()` on `sample_operator`, `vector_operator`, and `mc_operator`
  returns `int` instead of `double`. Expressions such as `n / vector_size()`
  with an integer `n` now use integer division. ([#196])
- `mousewheel` is now a reserved message name. A `message<>` with that name is
  no longer callable from a patcher; it is called on mouse wheel events with
  `{event, delta_x, delta_y}`.

### Added

- UI objects can handle `mousewheel`; `min::event` has `wheel_delta_x()` and
  `wheel_delta_y()`. ([#193])
- `dict::register_as()` registers an existing dictionary under a name.
  ([#182])

### Fixed

- Clang diagnostic pragmas are guarded by `__clang__` instead of
  `MAC_VERSION`, so they also apply to Clang on Windows.
- `object_base::patcher()` and `object_base::box()` no longer clash with the
  `patcher` and `box` type names.
- Calls to `max::object_method` and `max::object_new` with a `min::symbol`
  argument compile against the current Max SDK.

## [0.6.0]

### Breaking

CMake and the Max SDK:

- The `max-api` submodule was replaced by `max-sdk-base`. `C74_MAX_API_DIR` is
  now `C74_MAX_SDK_DIR`, and `C74_INCLUDES` points at the
  `max-sdk-base/c74support` include directories. Paths built from the old
  variable must be updated.
- CMake 3.19 or later is required.
- To change where externals are built, set `C74_LIBRARY_OUTPUT_DIRECTORY`;
  `CMAKE_LIBRARY_OUTPUT_DIRECTORY` is ignored.
- `CMAKE_OSX_DEPLOYMENT_TARGET` is forced to 10.11, overriding any value the
  package sets.
- `add_subdirectory(min-api)` no longer includes `min-package.cmake`, adds
  `test/mock`, or adds unit tests. Packages must include
  `script/min-package.cmake` themselves. `test/min-api-unittest.cmake` was
  removed.
- `-Werror` is no longer added automatically; set `C74_WARNINGS_AS_ERRORS` to
  opt in.
- The `c74::max` namespace now wraps the real Max SDK C headers rather than
  hand-written C++ declarations. Many constants (for example `JBOX_*` and
  `Z_NO_INPLACE`) are now preprocessor macros and must not be qualified with
  `max::`. Several functions such as `object_new` now return `void*` and take
  non-`const` pointers, so some calls need casts.

Objects:

- `MIN_EXTERNAL_CUSTOM(cls, name)` now registers the class under `name`.
  Previously it used the source file name, so such objects may change their
  Max class name.
- A message named `setup` is now called on every new instance right after
  construction.
- `maxclass_setup` runs before the class is registered and before Min adds its
  own `assist` method, so an `assist` method added there is replaced.
- `object_base::maxobj() const` returns `const max::t_object*`.
- UI objects get `JBOX_MOUSEDRAGDELTA` only when the class has a member named
  `mousedragdelta` or `m_mousedragdelta`, and `JBOX_HILITE` only when it has a
  member named `focusgained` or `m_focusgained`. Previously every UI object got
  drag-delta mouse handling.
- `key` is now an A_CANT message, and its handler's return value is read as
  the result. Handlers must return `{1}` (handled) or `{0}`, not `{}`.
- `timer<>` now fires on the scheduler of its owning object, which can differ
  from the global scheduler in hosts such as Live.

Attributes:

- An attribute with a clamping, wrapping, or folding `limit` must have an
  arithmetic type; for example `attribute<numbers, threadsafe::no,
  limit::clamp>` no longer compiles.
- `set()` and `operator=` take `const atoms&`, so the caller's arguments are no
  longer modified by limiting.
- `visibility::hide` now hides the attribute from the inspector and the
  reference page.
- Every attribute now registers its default value with Max, which affects the
  inspector and saving of default values. ([#146])

Other:

- `symbol::operator void*()` was removed. Use `sym.object()`. ([#155])
- `buffer_reference` can no longer be copied or moved.

### Added

Platforms and builds:

- Apple Silicon (arm64) builds, through max-sdk-base.
- `C74_WARNINGS_AS_ERRORS` CMake option.
- A `min-tests` target that builds and runs the unit tests from this
  repository, and a GitHub Actions workflow that runs it.
- A Conan recipe and example (`Conan.md`, `examples/min.conan`).

Objects and UI:

- UI objects can take keyboard focus by defining `focusgained` and
  `focuslost` messages. The member must be named `focusgained` or
  `m_focusgained`.
- A `dspstate` message is called when audio is turned on or off.
- A vector operator's `dspsetup` handler is found when the member is named
  `m_dspsetup` as well as `dspsetup`.
- `ui::color` has `operator==` and `operator!=`.
- `position`, `origin`, and `destination` can be constructed from a
  `max::t_pt`.
- `atom` can be assigned from a `const void*`.

Attributes:

- `attribute<ints>` is fully supported.
- `attribute<T>::set_range()` changes the range at runtime and applies it to
  the current value.
- The `live_color` attribute option maps a color attribute to a Max for Live
  theme color.

Buffers:

- `buffer_reference` takes an optional `create_messages` argument. Passing
  `false` stops it from adding `set`, `dblclick`, and `notify` messages to its
  owner, for objects with several buffers or their own `notify` handling.
- `buffer_reference::name()` returns the name of the bound buffer, and the
  static `buffer_reference::handle_notification(owner, args, begin, end)`
  dispatches a notification to the matching references in a collection.

### Fixed

- Constructing a UI object with no arguments no longer throws.
- Calling a method with no arguments through `instance::operator()` compiles.
- `atom` equality compared the left atom's type with itself instead of with
  the right atom's type. ([#166])
- `to_atoms()` handles an empty string returned by the Max API.
- Messages no longer dereference a null owner or Max object when looking up
  the current inlet during class setup.
- Vector operators set `vector_size()` during DSP setup, before `dspsetup` is
  called. Previously it was never updated.
- Vector operators no longer crash when Max passes a null connection count.
- An attribute's range is applied before comparing against the current value
  when repetitions are not allowed.
- `scale()` no longer divides by zero when the input range is empty.
- Outlets using `thread_action::first` passed the wrong arguments when
  sending their deferred value.

## [0.5.0]

### Breaking

- C++17 is required, along with Xcode 10 or later and Visual Studio 2017 or
  later.
- UI mouse messages (`mouseenter`, `mouseleave`, `mousedown`, `mouseup`,
  `mousemove`, `mousedragdelta`, `mousedoubleclick`) now receive a single
  atom wrapping a `min::event` instead of the raw Max arguments.
  ```cpp
  // before
  MIN_FUNCTION {
      ui::target t {args};
      double x {args[2]};
      double y {args[3]};
      ...
  }
  // after
  MIN_FUNCTION {
      event e {args};
      auto t {e.target()};
      double x {e.x()};
      double y {e.y()};
      ...
  }
  ```
- The class tag `multitouch` in `MIN_TAGS` now switches UI objects to
  multitouch mouse handling.
- `ui::color` predefined colors are now a scoped enum: `color::black` becomes
  `color::predefined::black`.
- `logger::type` is now an `enum class`: `logger::warning` becomes
  `logger::type::warning`.
- `doc_get_tags<T>()` was renamed `get_tags<T>()`, and `MIN_TAGS` moved from
  `c74_min_doc.h` to `c74_min_object.h`.
- `min::error()` no longer throws while the dummy instance is being
  constructed during class setup.

## [0.4.1]

### Breaking

Messages and callbacks:

- `min::function` takes the inlet number as a second argument. `MIN_FUNCTION`
  was updated to match, but hand-written lambdas and `std::function`s passed
  to `message<>`, attribute setters, `timer`, `queue`, and `buffer_reference`
  must add the parameter.
  ```cpp
  // before
  message<> m {this, "foo", [this](const atoms& args) -> atoms { ... }};
  // after
  message<> m {this, "foo", [this](const atoms& args, const int inlet) -> atoms { ... }};
  ```
- `object_base::current_inlet()` was removed. Use the `inlet` parameter of the
  callback.
- `message_type` values were renamed: `nothing` to `no_argument`, `long_arg`
  to `int_argument`, `float_arg` to `float_argument`, `symbol_arg` to
  `symbol_argument`, and `long_optional` to `int_optional`.
- The `message_type` argument to `message<>` is now honored; it was previously
  ignored.
- Deferred messages are delivered through `max::defer()` and a fixed-size
  queue instead of one qelem per message, which changes their ordering
  relative to other low-priority events.
- `threadsafe` gained a first value, `undefined`, which is now the default for
  `message<>` and `attribute<>` and means "inherit from the object".
  `object<T>` became `object<T, threadsafe = threadsafe::no>`.

Timers and queues:

- `timer` is a class template and its options are a template argument.
  `timer::options` became `timer_options`.
  ```cpp
  // before
  timer t {this, MIN_FUNCTION { ... }, timer::options::defer_delivery};
  // after
  timer<timer_options::defer_delivery> t {this, MIN_FUNCTION { ... }};
  ```
- `queue` is a class template and must be written `queue<>`.

Atoms and symbols:

- On 64-bit builds, constructing or assigning an `atom` from `long` is
  ambiguous and fails to compile. Cast to `max::t_atom_long` or `int`.
- Attribute `order` values are `int` instead of `long`; an `order` passed as a
  `long` is not recognized.
- `atom` comparison operators are members, so a plain `max::t_atom` on the
  left of `==` no longer compiles.
- `symbol::operator float()` is deleted, so `float f = sym;` and
  `double d = sym;` no longer compile.
- Converting a `symbol` to `max::t_dictionary*` returns `nullptr` unless the
  symbol is bound to a dictionary.

Attributes and state:

- `attribute` has a fourth template parameter, `allow_repetitions`. This only
  affects forward declarations and template-template arguments.
- An attribute named `value` now gets automatic preset, pattr, and snapshot
  support, and the object gains `getvalueof`, `setvalueof`, and `preset`
  methods.
- The special handling of a message named `appendtodictionary` was removed.
  Use `savestate`, which now works and receives the dictionary as its
  argument.

UI:

- UI support returned (see 0.4.0). Compared with 0.3.0, UI objects now call
  `jbox_notify` after a user `notify` returns `0`, and `ui::color` attributes
  are refreshed from the style system on every paint, which can overwrite
  values set directly.

CMake and testing:

- Xcode 9 or later is required.
- Catch was updated to the single-header Catch2 v2.5.0 at `test/catch.hpp`.
  Include paths that referenced `test/catch/include` must change, and
  `Approx` follows Catch2 semantics.
- `min-object-unittest.cmake` no longer calls `project()`, so the test target
  lives in the object's project.
- `CMAKE_OSX_ARCHITECTURES` defaults to `x86_64`; 32-bit Mac builds are no
  longer the default.

## [0.4.0]

### Breaking

- UI support was removed: `c74_min_operator_ui.h` was deleted and
  `c74_min_graphics.h` is no longer included. Restored in 0.4.1.
- `samplerate_set(double)` on audio operators was renamed `samplerate(double)`.
- `MIN_CLAMP` expands to an unqualified `clamp`, so it needs
  `c74::min` in scope.
- The unit test CMake helpers link `mock_kernel` with the keyword form of
  `target_link_libraries`, so packages adding libraries to the test target
  must also use the keyword form.

## [0.3.0]

### Breaking

Objects:

- The global `define_min_external*` functions became
  `c74::min::wrap_as_max_external*`. `MIN_EXTERNAL` is unchanged.
- `minwrap<T>` members were renamed: `obj` to `m_min_object` and `base` to
  `m_max_header`. `maxobject_base` became `maxobject_header`.
- On `object_base`, `methods()` became `messages()`, `m_maxobj` became private
  (use `maxobj()`), and `inlets()` and `outlets()` hold `inlet_base*` and
  `outlet_base*`.
- A `notify` handler must return a value such as `{0}`; returning `{}` throws.
- Objects attach to themselves, so `notify` also fires for their own attribute
  changes.
- The `savestate` message is no longer called; the wrapper only handles a
  message named `appendtodictionary`. (Restored in 0.4.1.)
- The `toggle` special message was removed.
- Messages named `setup`, `fileusage`, `maxob_setup`, `mop_setup`,
  `jitclass_setup`, `mouseenter`, `mouseleave`, `mousedown`, `mouseup`,
  `mousedragdelta`, and `mousedoubleclick` are now A_CANT and cannot be sent
  from a patcher.
- A reference page `<name>.maxref.xml` is written to the package's `docs`
  folder when the external is newer than the existing page. Opt out with
  `MIN_FLAGS { documentation_flags::do_not_generate };`.

Inlets, outlets, and messages:

- `inlet` became `inlet<>`, and `outlet` became `outlet<>`.
- `method` became `message<>`, and `c74_min_method.h` became
  `c74_min_message.h`.
- The public fields of ports and messages (`owner`, `description`, `type`,
  `function`, `instance`, `signal_connection`) were removed or made private.
  `description()` and `type()` are now methods.
- `outlet::send()` with an `int` or `bool` outputs an int instead of a float.
- In debug builds, the default outlet asserts when called from a thread other
  than the main or scheduler thread.
- A default `message<>` called off the main thread is deferred to the main
  thread and returns immediately. Use `message<threadsafe::yes>` for the old
  behavior.

Attributes:

- `attribute_base::set()` takes `(atoms&, bool notify, bool override_readonly)`,
  so a braced temporary such as `a.set({x})` no longer compiles.
- `readonly{true}` is enforced: neither Max nor the object's own `operator=`
  can change the value. Use `set(as, false, true)` from inside the object.
- An attribute set off the main thread is applied later on the main thread.
  Use `attribute<T, threadsafe::yes>` for the old behavior.
- `attribute` has three template parameters, `<T, threadsafe, limit_type>`.
- `title` is its own type and no longer accepts a plain `std::string`.
- A `getter` is now used by `operator atoms()`.

Atoms, symbols, dictionaries, and paths:

- `atom` can no longer be constructed from `const t_object*`,
  `const t_class*`, or `const t_dictionary*`, and its broader implicit
  conversions can make user overloads taking `atom` ambiguous.
- `symbol::operator bool` returns whether the symbol is bound (`s_thing` is
  non-null), so `if (sym)` is no longer true for every non-null symbol.
- Converting a `symbol` to `void*` yields its `s_thing`, not the `t_symbol*`.
- `dict::at()` throws when the key is missing.
- Converting a `path` to `std::string` gives an absolute system path rather
  than a Max-style path.
- `timer` can no longer be copied.
- `texteditor` calls its callback when the text is empty.

Audio:

- `perform_operator` became `vector_operator<>`, and
  `c74_min_operator_perform.h` became `c74_min_operator_vector.h`.
  `perform(in, out)` became the pure virtual
  `void operator()(audio_bundle input, audio_bundle output)`.
- `audio_bundle` fields became methods: `samples()`, `samples(channel)`,
  `channel_count()`, and `frame_count()`.
- In `sample_operator`, `calculate(...)` became `operator()(...)`, and
  `inputcount()` and `outputcount()` became `input_count()` and
  `output_count()`.
- `Z_NO_INPLACE` is set on all audio objects, so input and output buffers no
  longer alias.
- `sample_operator<1,1>` does not compile in 0.3.0; fixed in 0.3.1.

Buffers:

- `buffer_lock` became `buffer_lock<>`.
- `framecount()` became `frame_count()`, and `channelcount()` became
  `channel_count()`, which returns `size_t`.

Jitter:

- `matrix_operator` became `matrix_operator<>`.
- `matrix_info::planecount()` and `dimcount()` became `plane_count()` and
  `dim_count()`. The public fields `in_info`, `bip`, `out_info`, and `bop`
  became `m_in_info`, `m_bip`, `m_out_info`, and `m_bop`, and the byte
  pointers changed from `char*` to `uchar*`.
- Defining `jitclass_setup` or `maxclass_setup` on a `matrix_operator`
  replaces the standard setup instead of running in addition to it.
- `gl_operator`, `glchunk`, and `c74_min_operator_gl.h` were removed.

Removed helpers:

- `c74_min_accessories.h` was removed. Math, filter, and easing helpers moved
  to Min-Lib. `MIN_CLAMP` is in `c74_min_limit.h`.
- `min::map` and `c74_min_map.h` were removed.

CMake:

- Xcode builds add `-Wall -Werror -Wmissing-field-initializers`.
- If `min-lib` exists next to the package's `min-api`, `C74_USE_MIN_LIB` is
  defined and `c74_min.h` includes Min-Lib automatically.

[Unreleased]: https://github.com/Cycling74/min-api/compare/v0.6.11...HEAD
[0.6.11]: https://github.com/Cycling74/min-api/compare/v0.6.10...v0.6.11
[0.6.10]: https://github.com/Cycling74/min-api/compare/v0.6.9...v0.6.10
[0.6.8]: https://github.com/Cycling74/min-api/compare/v0.6.7...v0.6.8
[0.6.7]: https://github.com/Cycling74/min-api/compare/v0.6.6...v0.6.7
[0.6.4]: https://github.com/Cycling74/min-api/compare/v0.6.3...v0.6.4
[0.6.3]: https://github.com/Cycling74/min-api/compare/v0.6.2...v0.6.3
[0.6.2]: https://github.com/Cycling74/min-api/compare/v0.6.1...v0.6.2
[0.6.1]: https://github.com/Cycling74/min-api/compare/v0.6.0...v0.6.1
[0.6.0]: https://github.com/Cycling74/min-api/compare/v0.5.0...v0.6.0
[0.5.0]: https://github.com/Cycling74/min-api/compare/v0.4.1...v0.5.0
[0.4.1]: https://github.com/Cycling74/min-api/compare/v0.4.0...v0.4.1
[0.4.0]: https://github.com/Cycling74/min-api/compare/v0.3.0...v0.4.0
[0.3.0]: https://github.com/Cycling74/min-api/compare/v0.2.0...v0.3.0

[#82]: https://github.com/Cycling74/min-api/issues/82
[#121]: https://github.com/Cycling74/min-api/issues/121
[#139]: https://github.com/Cycling74/min-api/issues/139
[#143]: https://github.com/Cycling74/min-api/issues/143
[#146]: https://github.com/Cycling74/min-api/issues/146
[#153]: https://github.com/Cycling74/min-api/issues/153
[#155]: https://github.com/Cycling74/min-api/issues/155
[#160]: https://github.com/Cycling74/min-api/issues/160
[#166]: https://github.com/Cycling74/min-api/issues/166
[#181]: https://github.com/Cycling74/min-api/issues/181
[#182]: https://github.com/Cycling74/min-api/pull/182
[#193]: https://github.com/Cycling74/min-api/pull/193
[#195]: https://github.com/Cycling74/min-api/issues/195
[#196]: https://github.com/Cycling74/min-api/pull/196
[#202]: https://github.com/Cycling74/min-api/pull/202
[#203]: https://github.com/Cycling74/min-api/issues/203
[#204]: https://github.com/Cycling74/min-api/issues/204
[#206]: https://github.com/Cycling74/min-api/pull/206
[#211]: https://github.com/Cycling74/min-api/pull/211
[min-devkit#222]: https://github.com/Cycling74/min-devkit/issues/222
