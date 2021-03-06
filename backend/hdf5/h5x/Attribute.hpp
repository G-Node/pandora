// Copyright © 2015 German Neuroinformatics Node (G-Node)
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under the terms of the BSD License. See
// LICENSE file in the root of the Project.
//
// Author: Christian Kellner <kellner@bio.lmu.de>

#ifndef NIX_ATTRIBUTE_H5_H
#define NIX_ATTRIBUTE_H5_H

#include "DataSpace.hpp"
#include "H5DataType.hpp"
#include "H5PList.hpp"

namespace nix {

namespace hdf5 {


class NIXAPI Attribute : public H5Object {
public:
    Attribute();
    Attribute(hid_t hid);
    Attribute(const Attribute &other);

    void read(h5x::DataType mem_type, const NDSize &size, void *data);
    void read(h5x::DataType mem_type, const NDSize &size, std::string *data);

    void write(h5x::DataType mem_type, const NDSize &size, const void *data);
    void write(h5x::DataType mem_type, const NDSize &size, const std::string *data);

    PList createPList() const;
    h5x::DataType dataType() const;
    DataSpace getSpace() const;
    NDSize extent() const;

    Attribute &operator=(const Attribute &other) {
        H5Object::operator=(other);
        return *this;
    }
};


}

}


#endif
