// Copyright (c) 2013 - 2015, German Neuroinformatics Node (G-Node)
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under the terms of the BSD License. See
// LICENSE file in the root of the Project.

#ifndef NIX_FILE_FS_HPP
#define NIX_FILE_FS_HPP

#include <nix/base/IFile.hpp>
#include <string>
#include <memory>
#include <boost/filesystem.hpp>
#include "DirectoryWithAttributes.hpp"
#include <nix/Exception.hpp>

namespace nix {
namespace file {

class FileFS : public base::IFile, public DirectoryWithAttributes, public std::enable_shared_from_this<FileFS> {

private:
    Directory data_dir, metadata_dir;
    Compression compr;
    FileMode mode;

    void create_subfolders(const std::string &loc);

public:
    FileFS(const std::string &name, const FileMode mode = FileMode::ReadWrite, const Compression compression = Compression::Auto);


    bool flush() { return true; };


    ndsize_t blockCount() const;


    bool hasBlock(const std::string &name_or_id) const;


    std::shared_ptr<base::IBlock> getBlock(const std::string &name_or_id) const;


    std::shared_ptr<base::IBlock> getBlock(ndsize_t index) const;


    std::shared_ptr<base::IBlock> createBlock(const std::string &name, const std::string &type);


    bool deleteBlock(const std::string &name_or_id);

    //--------------------------------------------------
    // Methods concerning sections
    //--------------------------------------------------

    bool hasSection(const std::string &name_or_id) const;


    std::shared_ptr<base::ISection> getSection(const std::string &name_or_id) const;


    std::shared_ptr<base::ISection> getSection(ndsize_t index) const;


    ndsize_t sectionCount() const;


    std::shared_ptr<base::ISection> createSection(const std::string &name, const std::string &type);


    bool deleteSection(const std::string &name_or_id);

    //--------------------------------------------------
    // Methods for file attribute access.
    //--------------------------------------------------


    std::vector<int> version() const;


    std::string format() const;


    std::string id() const;


    void forceId();
    
    
    std::string location() const;


    time_t createdAt() const;


    time_t updatedAt() const;


    void setUpdatedAt();


    void forceUpdatedAt();


    void setCreatedAt();


    void forceCreatedAt(time_t t);


    void close();


    bool isOpen() const;


    FileMode fileMode() const;


    Compression compression() const;


    bool operator==(const FileFS &other) const;


    bool operator!=(const FileFS &other) const;


    virtual ~FileFS();

    private:

    std::shared_ptr<base::IFile> file() const;

    // check for existence
    bool fileExists(const std::string &name) const;

    // check if the header of the file is valid
    bool checkHeader();

};

} // namespace file
} // namespace nix



#endif //NIX_FILE_FS_H
