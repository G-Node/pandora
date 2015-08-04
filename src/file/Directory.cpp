// Copyright (c) 2013, German Neuroinformatics Node (G-Node)
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under the terms of the BSD License. See
// LICENSE file in the root of the Project.

#include <iostream>
#include <nix/file/Directory.hpp>

using namespace std;

using namespace boost::filesystem;

namespace nix {
namespace file {


Directory::Directory(const path &parent, const string &name): loc(parent / path(name.c_str())) {
    open_or_create();
    attributes = AttributesFS(loc);
}


void Directory::open_or_create() {
    if (!exists(loc)) {
        create_directories(loc);
    }

}

bool Directory::hasAttr(const string &name) const  {
    return attributes.has(name);
}

string Directory::location() const {
    return loc.string();
}

} // nix::file::
} // nix