/// @file
///	@ingroup 	minapi
///	@copyright	Copyright 2018 The Min-API Authors. All rights reserved.
///	@license	Use of this source code is governed by the MIT License found in the License.md file.

#pragma once

namespace c74::min {

class dict
{
  public:
    /// Create (or reference an existing) dictionary by name
    dict(const symbol name)
    {
        auto d = max::dictobj_findregistered_retain(name);

        if (!d) { // didn't find a dictionary with that name, so create it
            d = max::dictionary_new();
            max::t_symbol* s = name;
            m_instance = max::dictobj_register(d, &s);
        }
        else {
            m_instance = d;
            m_retained_from_registry = true;
        }
    }

    /// Create an unregistered dictionary from dict-syntax, or an empty one if content is empty.
    dict(const atoms& content)
    {
        if (content.empty()) {
            m_instance = max::dictionary_new();
        }
        else {
            max::dictobj_dictionaryfromatoms(&m_instance, static_cast<long>(content.size()), content.data());
        }
    }

    /// Create an unregistered dictionary
    /// @param d				optionally getting a handle to an old-school t_dictionary
    /// @param retain			true to share ownership of d; false to borrow it, in which case d must outlive this dict
    dict(max::t_dictionary* d = nullptr, const bool retain = true)
    {
        if (d == nullptr) {
            m_instance = max::dictionary_new();
        }
        else {
            if (retain) {
                max::object_retain((max::t_object*)d);
            }
            else {
                m_has_ownership = false;
            }
            m_instance = d;
        }
    }

    dict(const atom an_atom_containing_a_dict)
    {
        auto a = static_cast<const max::t_atom*>(&an_atom_containing_a_dict);
        m_instance = static_cast<max::t_dictionary*>(max::atom_getobj(a));
        if (!m_instance) {
            error("no dictionary in atom");
        }
        auto err = max::object_retain((max::t_object*)m_instance);
        error(err, "failed to retain dictionary instance");
    }

    /// Refer to the same dictionary as other, retaining it with object_retain() if other owns it.
    dict(const dict& other)
        : m_instance{ other.m_instance }
        , m_has_ownership{ other.m_has_ownership }
    {
        if (m_has_ownership && m_instance) {
            max::object_retain((max::t_object*)m_instance);
        }
    }

    dict(dict&& other) noexcept
        : m_instance{ std::exchange(other.m_instance, nullptr) }
        , m_has_ownership{ other.m_has_ownership }
        , m_retained_from_registry{ other.m_retained_from_registry }
    {
    }

    ~dict()
    {
        if (m_has_ownership && m_instance) {
            if (m_retained_from_registry) {
                max::dictobj_release(m_instance);
            }
            else {
                object_free(m_instance);
            }
        }
    }

    /// Replace the contents with a copy of value's contents, keeping this dictionary's identity and registered name.
    /// There is deliberately no move assignment, so assigning a temporary also copies contents.
    dict& operator=(const dict& value)
    {
        if (this != &value) {
            max::dictionary_clone_to_existing(value.m_instance, m_instance);
        }
        return *this;
    }

    /// Replace the contents with a copy of the contents of the dictionary in value.
    dict& operator=(const atom& value)
    {
        auto a = static_cast<const max::t_atom*>(&value);
        if (!max::atomisdictionary((max::t_atom*)a)) {
            error("no dictionary in atom");
            return *this;
        }

        auto d = static_cast<max::t_dictionary*>(max::atom_getobj(a));
        if (d != m_instance) {
            max::dictionary_clone_to_existing(d, m_instance);
        }
        return *this;
    }

    /// The underlying dictionary for use with the C Max API, without retaining it.
    /// The pointer is only valid while this dict, or another reference to the dictionary, is alive.
    max::t_dictionary* instance() const
    {
        return m_instance;
    }

    /// The underlying dictionary, retained. The caller must release it with object_free(); prefer instance() to avoid leaking.
    operator max::t_object*() const
    {
        max::object_retain((max::t_object*)m_instance);
        return (max::t_object*)m_instance;
    }

    // bounds check: if key doesn't exist, throw
    atom_reference at(const symbol key)
    {
        long argc = 0;
        max::t_atom* argv = nullptr;
        auto err = max::dictionary_getatoms(m_instance, key, &argc, &argv);

        error(err, "could not get key from dictionary");
        return atom_reference(argc, argv);
    }

    // bounds check: if key doesn't exist, create it
    atom_reference operator[](symbol key)
    {
        if (!max::dictionary_hasentry(m_instance, key)) {
            max::dictionary_appendatom(m_instance, key, &atoms{ 0 }[0]);
        }
        return at(key);
    };

    atom_reference operator[](int key)
    {
        symbol skey{ key };
        return (*this)[skey];
    };

    /// Store a list of atoms under key, replacing any existing value for that key.
    void append_atoms(const symbol key, const atoms& values)
    {
        auto err = max::dictionary_appendatoms(m_instance, key, static_cast<long>(values.size()), const_cast<atom*>(values.data()));
        error(err, "could not append atoms to dictionary");
    }

    symbol name() const
    {
        return dictobj_namefromptr(m_instance);
    }

    bool valid() const
    {
        return m_instance != nullptr;
    }

    void clear()
    {
        dictionary_clear(m_instance);
    }

    void copyunique(const dict& source)
    {
        dictionary_copyunique(m_instance, source.m_instance);
    }

    void touch()
    {
        object_notify(m_instance, k_sym_modified, nullptr);
    }

    /// Register an existing dictionary
    /// @param name   name to register the dictionary under
    void register_as(const symbol name)
    {
        if (m_instance != nullptr) {
            max::t_symbol* s = name;
            m_instance = max::dictobj_register(m_instance, &s);
        }
    }

    /// Get number of entries of dictionary
    long entrycount()
    {
        return static_cast<long>(max::dictionary_getentrycount(m_instance));
    }

    /// Returns the dictionary keys in insertion order
    std::vector<symbol> keys()
    {
        std::vector<symbol> k;
        max::t_symbol** keys = NULL;
        long numkeys = 0;

        max::dictionary_getkeys_ordered(m_instance, &numkeys, &keys);
        for (long i = 0; i < numkeys; i++) {
            k.push_back(symbol(keys[i]));
        }
        if (keys) {
            max::dictionary_freekeys(m_instance, numkeys, keys);
        }
        return k;
    }

    /// Call fn for every entry in insertion order. Assigning to the atom_reference updates the entry in place.
    void for_each(const std::function<void(const symbol key, atom_reference value)>& fn)
    {
        for (const auto& key : keys()) {
            fn(key, at(key));
        }
    }

    /// Replace the contents with a JSON file found by name in the Max search path, or by absolute path.
    void read(const std::string& name)
    {
        char filename[max::MAX_PATH_CHARS]{};
        strncpy(filename, name.c_str(), max::MAX_PATH_CHARS - 1);

        short path_id{};
        max::t_fourcc type{};
        if (max::locatefile_extended(filename, &path_id, &type, nullptr, 0)) {
            error("file not found");
            return;
        }

        max::t_dictionary* d{ nullptr };
        auto err = max::dictionary_read(filename, path_id, &d);
        if (err || !d) {
            error("could not read dictionary from file");
            return;
        }

        // Clone into the existing instance so a registered name stays attached.
        max::dictionary_clone_to_existing(d, m_instance);
        object_free(d);
    }

    /// Write the contents as JSON to an absolute path, creating or overwriting the file.
    void write(const std::string& filepath)
    {
        char fullpath[max::MAX_PATH_CHARS]{};
        max::path_nameconform(filepath.c_str(), fullpath, max::PATH_STYLE_MAX, max::PATH_TYPE_ABSOLUTE);

        // The file may not exist yet, so resolve its parent folder instead of the file itself.
        char* filename = strrchr(fullpath, '/');
        if (!filename) {
            error("no filename provided");
            return;
        }
        *filename++ = 0;

        short folder_path{};
        char unused[max::MAX_FILENAME_CHARS]{};
        if (max::path_frompathname(fullpath, &folder_path, unused)) {
            error("folder not found");
            return;
        }

        auto err = max::dictionary_write(m_instance, filename, folder_path);
        error(err, "could not write dictionary to file");
    }

  private:
    max::t_dictionary* m_instance{ nullptr };
    bool m_has_ownership{ true };
    bool m_retained_from_registry{ false }; ///< retained by dictobj_findregistered_retain(), so released with dictobj_release()
};

} // namespace c74::min
