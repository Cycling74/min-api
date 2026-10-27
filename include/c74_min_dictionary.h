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
        }
    }

    /// Create an unregistered dictionary from dict-syntax
    dict(const atoms& content)
    {
        max::dictobj_dictionaryfromatoms(&m_instance, static_cast<long>(content.size()), &content[0]);
    }

    /// Create an unregistered dictionary
    /// @param d				optionally getting a handle to an old-school t_dictionary
    /// @param take_ownership	defaults to true, change to false only in exceptional cases
    dict(max::t_dictionary* d = nullptr, const bool take_ownership = true)
    {
        if (d == nullptr) {
            m_instance = max::dictionary_new();
        }
        else {
            if (take_ownership) {
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

    ~dict()
    {
        if (m_has_ownership) {
            object_free(m_instance);
        }
    }

    dict& operator=(const dict& value)
    {
        max::dictionary_clone_to_existing(value.m_instance, m_instance);
        return *this;
    }

    dict& operator=(const atom& value)
    {
        auto a = static_cast<const max::t_atom*>(&value);
        if (max::atomisdictionary((max::t_atom*)a)) {
            m_instance = static_cast<max::t_dictionary*>(max::atom_getobj(a));
        }
        return *this;
    }

    /// Cast the dictionary to a Max t_object so that, e.g., it can be used with the C Max API.
    /// Be exceedingly careful with this!
    /// When you pass the pointer out are you pointing the instance without incrementing the reference count?

    // TODO: We need copy (retain) and move (don't retain) semantics ????
    // TODO: we don't have a copy constructor!

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
};

} // namespace c74::min
