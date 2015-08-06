//
// Created by jan on 7/30/15.
//

#include "nix/file/AttributesFS.hpp"


using namespace nix::file;
using namespace boost::filesystem;
using namespace YAML;

namespace nix {
namespace file {

#define ATTRIBUTES_FILE std::string("attributes")

AttributesFS::AttributesFS() { }


AttributesFS::AttributesFS(const std::string &file_path, FileMode mode)
    : AttributesFS(path(file_path.c_str()), mode)
{ }

AttributesFS::AttributesFS(const path &file_path, FileMode mode)
    : loc(file_path), mode(mode) {
    if (exists(file_path)) {
        open_or_create();
    }
}


void AttributesFS::open_or_create() {
    path attr(ATTRIBUTES_FILE);
    path temp = location() / attr;
    if (!exists(temp)) {
        if (mode > FileMode::ReadOnly) {
            std::ofstream ofs;
            ofs.open(location().string() + "/" + ATTRIBUTES_FILE, std::ofstream::out | std::ofstream::app);
            ofs.close();
        } else {
            throw std::logic_error("Trying to create new attributes in ReadOnly mode!");
        }
    }
    node = LoadFile(location().string() + "/" + ATTRIBUTES_FILE);
}


bool AttributesFS::has(const std::string &name) {
    open_or_create();
    return (node.size() > 0) && (node[name]);
}


void AttributesFS::flush() {
    std::ofstream ofs;
    ofs.open(location().string() + "/" + ATTRIBUTES_FILE, std::ofstream::trunc);
    if (ofs.is_open())
        ofs << node << std::endl;
    else
        std::cerr << "Failure!!!" << std::endl;
    ofs.close();
}

path AttributesFS::location() const {
    return loc;
}

nix::ndsize_t AttributesFS::attributeCount() {
    open_or_create();
    return node.size();
}

void AttributesFS::remove(const std::string &name) {
    open_or_create();
    if (mode == FileMode::ReadOnly) {
        throw std::logic_error("Trying to remove an attributes in ReadOnly mode!");
    }
    if (node[name]) {
        node.remove(name);
    }
    flush();
}

} //namespace file
} //namespace nix