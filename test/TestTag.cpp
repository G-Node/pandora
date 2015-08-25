// Copyright © 2014 German Neuroinformatics Node (G-Node)
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted under the terms of the BSD License. See
// LICENSE file in the root of the Project.
//
// Author: Jan Grewe <jan.grewe@g-node.org>

#include "TestTag.hpp"

#include <nix/NDSize.hpp>
#include <nix/Exception.hpp>
#include <nix/valid/validate.hpp>

#include <sstream>
#include <ctime>

using namespace nix;
using namespace valid;
using namespace std;

void TestTag::setUp() {
    startup_time = time(NULL);
    file = File::open("test_multiTag.h5", FileMode::Overwrite);
    block = file.createBlock("block", "dataset");

    vector<string> array_names = { "data_array_a", "data_array_b", "data_array_c",
                                   "data_array_d", "data_array_e" };
    refs.clear();
    for (const auto & name : array_names) {
        refs.push_back(block.createDataArray(name, "reference",
                                             DataType::Double, nix::NDSize({ 0 })));
    }

    tag = block.createTag("tag_one", "test_tag", {0.0, 2.0, 3.4});
    tag_other = block.createTag("tag_two", "test_tag", {0.0, 2.0, 3.4});
    tag_null = nix::none;

    section = file.createSection("foo_section", "metadata");

    file_fs = File::open("test_tag", FileMode::Overwrite, Implementation::FileSys);
    block_fs = file_fs.createBlock("block", "dataset");

    refs_fs.clear();
    for (const auto & name : array_names) {
        refs_fs.push_back(block_fs.createDataArray(name, "reference",
                                                   DataType::Double, nix::NDSize({ 0 })));
    }
    tag_fs = block_fs.createTag("tag_one", "test_tag", {0.0, 2.0, 3.4});
    section_fs = file_fs.createSection("foo_section", "metadata");
}


void TestTag::tearDown() {
    file.deleteBlock(block.id());
    file.deleteSection(section.id());
    file.close();
}


void TestTag::testValidate() {
    valid::Result result = validate(tag);
    CPPUNIT_ASSERT(result.getErrors().size() == 0);
    CPPUNIT_ASSERT(result.getWarnings().size() == 0);

    valid::Result result_fs = validate(tag_fs);
    CPPUNIT_ASSERT(result_fs.getErrors().size() == 0);
    CPPUNIT_ASSERT(result_fs.getWarnings().size() == 0);

}


void TestTag::testId() {
    CPPUNIT_ASSERT(tag.id().size() == 36);
    CPPUNIT_ASSERT(tag_fs.id().size() == 36);
}


void TestTag::testName() {
    CPPUNIT_ASSERT(tag.name() == "tag_one");
    CPPUNIT_ASSERT(tag_fs.name() == "tag_one");
}


void TestTag::testType() {
    test_type(tag);
    test_type(tag_fs);
}

void TestTag::test_type(Tag &t) {
    CPPUNIT_ASSERT(t.type() == "test_tag");
    std::string type = util::createId();
    t.type(type);
    CPPUNIT_ASSERT(t.type() == type);
}


void TestTag::testDefinition() {
    test_definition(tag);
    test_definition(tag_fs);
}

void TestTag::test_definition(Tag  &t) {
    std::string def = util::createId();
    tag.definition(def);
    CPPUNIT_ASSERT(*tag.definition() == def);
    tag.definition(nix::none);
    CPPUNIT_ASSERT(tag.definition() == nix::none);
}

void TestTag::testCreateRemove() {
    test_create_remove(block, refs);
    test_create_remove(block_fs, refs_fs);
}

void TestTag::test_create_remove(Block &b, vector<DataArray> &r) {
    std::vector<std::string> ids;
    ndsize_t count = b.tagCount();
    const char *names[5] = { "tag_a", "tag_b", "tag_c", "tag_d", "tag_e" };

    for (int i = 0; i < 5; i++) {
        std::string type = "Event";
        Tag st1 = b.createTag(names[i], type, {0.0, 2.0, 3.4});
        st1.references(r);
        Tag st2 = b.getTag(st1.id());
        ids.push_back(st1.id());

        std::stringstream errmsg;
        errmsg << "Error while accessing tag: st1.id() = " << st1.id()
        << " / st2.id() = " << st2.id();
        CPPUNIT_ASSERT_MESSAGE(errmsg.str(), st1.id().compare(st2.id()) == 0);
    }
    std::stringstream errmsg2;
    errmsg2 << "Error creating Tags. Counts do not match!";
    CPPUNIT_ASSERT_MESSAGE(errmsg2.str(), b.tagCount() == (count+5));

    for (auto it = r.begin(); it != r.end(); it++) {
        b.deleteDataArray((*it).id());
    }
    for (const auto &id : ids) {
        b.deleteTag(id);
    }

    std::stringstream errmsg1;
    errmsg1 << "Error while removing tags!";
    CPPUNIT_ASSERT_MESSAGE(errmsg1.str(), b.tagCount() == count);
}


void TestTag::testExtent() {
    Tag st = block.createTag("TestTag1", "Tag", {0.0, 2.0, 3.4});
    st.references(refs);

    std::vector<double> extent = {1.0, 2.0, 3.0};
    st.extent(extent);

    std::vector<double> retrieved = st.extent();
    CPPUNIT_ASSERT(retrieved.size() == extent.size());
    for(size_t i = 0; i < retrieved.size(); i++){
        CPPUNIT_ASSERT(retrieved[i] == extent[i]);
    }

    st.extent(none);
    CPPUNIT_ASSERT(st.extent().size() == 0);
    for (auto it = refs.begin(); it != refs.end(); it++) {
        block.deleteDataArray((*it).id());
    }
    block.deleteTag(st.id());
}


void TestTag::testPosition() {
    Tag st = block.createTag("TestTag1", "Tag", {0.0, 2.0, 3.4});
    st.references(refs);

    std::vector<double> position = {1.0, 2.0, 3.0};
    std::vector<double> new_position = {2.0};

    st.position(position);
    std::vector<double> retrieved = st.position();
    CPPUNIT_ASSERT(retrieved.size() == position.size());

    for(size_t i = 0; i < retrieved.size(); i++){
        CPPUNIT_ASSERT(retrieved[i] == position[i]);
    }

    st.position(new_position);
    retrieved = st.position();
    CPPUNIT_ASSERT(retrieved.size() == new_position.size());

    for(size_t i = 0; i < retrieved.size(); i++){
        CPPUNIT_ASSERT(retrieved[i] == new_position[i]);
    }
    for (auto it = refs.begin(); it != refs.end(); it++) {
        block.deleteDataArray((*it).id());
    }
    block.deleteTag(st.id());
}


void TestTag::testMetadataAccess() {
    test_metadata_access(tag, file, section);
    test_metadata_access(tag_fs, file_fs, section_fs);
}

void TestTag::test_metadata_access(Tag &t, File &f, Section &s) {
    CPPUNIT_ASSERT(!t.metadata());

    t.metadata(s);
    CPPUNIT_ASSERT(t.metadata());
    // TODO This test fails due to operator== of Section
    CPPUNIT_ASSERT(t.metadata().id() == s.id());

    // test none-unsetter
    t.metadata(none);
    CPPUNIT_ASSERT(!t.metadata());
    // test deleter removing link too
    t.metadata(s);
    f.deleteSection(s.id());
    CPPUNIT_ASSERT(!t.metadata());
    // re-create section
    s = f.createSection("foo_section", "metadata");
}


void TestTag::testSourceAccess() {
    test_source_access(tag, block);
    test_source_access(tag_fs, block_fs);
}

void TestTag::test_source_access(Tag &t, Block &b) {
    std::vector<std::string> names = { "source_a", "source_b", "source_c", "source_d", "source_e" };
    CPPUNIT_ASSERT(t.sourceCount() == 0);
    CPPUNIT_ASSERT(t.sources().size() == 0);

    std::vector<std::string> ids;
    for (auto it = names.begin(); it != names.end(); it++) {
        Source child_source = b.createSource(*it,"channel");
        t.addSource(child_source);
        CPPUNIT_ASSERT(child_source.name() == *it);
        ids.push_back(child_source.id());
    }

    CPPUNIT_ASSERT(t.sourceCount() == names.size());
    CPPUNIT_ASSERT(t.sources().size() == names.size());

    std::string name = names[0];
    Source source = t.getSource(name);
    CPPUNIT_ASSERT(source.name() == name);

    for (auto it = ids.begin(); it != ids.end(); it++) {
        Source child_source = t.getSource(*it);
        CPPUNIT_ASSERT(t.hasSource(*it) == true);
        CPPUNIT_ASSERT(child_source.id() == *it);

        t.removeSource(*it);
        b.deleteSource(*it);
    }
    CPPUNIT_ASSERT(t.sourceCount() == 0);
}


void TestTag::testUnits() {
    test_unit(block, refs);
    test_unit(block_fs, refs_fs);
}

void TestTag::test_unit(Block &b, vector<DataArray> &r) {
    Tag st = b.createTag("TestTag1", "Tag", {0.0, 2.0, 3.4});
    st.references(r);

    std::vector<std::string> valid_units = {"mV", "cm", "m^2"};
    std::vector<std::string> invalid_units = {"mV", "haha", "qm^2"};
    std::vector<std::string> insane_units = {"muV ", " muS"};

    CPPUNIT_ASSERT_NO_THROW(st.units(valid_units));
    CPPUNIT_ASSERT(st.units().size() == valid_units.size());
    std::vector<std::string> retrieved_units = st.units();
    for(size_t i = 0; i < retrieved_units.size(); i++){
        CPPUNIT_ASSERT(retrieved_units[i] == valid_units[i]);
    }

    st.units(none);
    CPPUNIT_ASSERT(st.units().size() == 0);
    CPPUNIT_ASSERT_THROW(st.units(invalid_units), nix::InvalidUnit);
    CPPUNIT_ASSERT(st.units().size() == 0);
    for (auto it = r.begin(); it != r.end(); it++) {
        b.deleteDataArray((*it).id());
    }

    st.units(insane_units);
    retrieved_units = st.units();
    CPPUNIT_ASSERT(retrieved_units.size() == 2);
    CPPUNIT_ASSERT(retrieved_units[0] == "uV");
    CPPUNIT_ASSERT(retrieved_units[1] == "uS");

    b.deleteTag(st.id());
}


void TestTag::testReferences() {
    test_references(tag, refs);
    test_references(tag_fs, refs_fs);
}

void TestTag::test_references(Tag &t, vector<DataArray> &r) {
    CPPUNIT_ASSERT(t.referenceCount() == 0);
    for (size_t i = 0; i < r.size(); ++i) {
        CPPUNIT_ASSERT(!t.hasReference(r[i]));
        CPPUNIT_ASSERT_NO_THROW(t.addReference(r[i]));
        CPPUNIT_ASSERT(t.hasReference(r[i]));
    }
    CPPUNIT_ASSERT(t.referenceCount() == r.size());
    for (size_t i = 0; i < r.size(); ++i) {
        CPPUNIT_ASSERT_NO_THROW(t.removeReference(r[i]));
    }
    CPPUNIT_ASSERT(t.referenceCount() == 0);
    DataArray a;

    CPPUNIT_ASSERT_THROW(tag.hasReference(a), UninitializedEntity);
    CPPUNIT_ASSERT_THROW(tag.addReference(a), UninitializedEntity);
    CPPUNIT_ASSERT_THROW(tag.removeReference(a), UninitializedEntity);
}


void TestTag::testFeatures() {
    test_features(tag, refs[0]);
    test_features(tag_fs, refs_fs[0]);
}

void TestTag::test_features(Tag &t, DataArray &da) {
    DataArray a;
    Feature f;

    CPPUNIT_ASSERT(tag.featureCount() == 0);
    CPPUNIT_ASSERT_THROW(tag.hasFeature(f), UninitializedEntity);
    CPPUNIT_ASSERT_THROW(tag.deleteFeature(f), UninitializedEntity);
    CPPUNIT_ASSERT_THROW(tag.createFeature(a, nix::LinkType::Indexed), UninitializedEntity);
    
    CPPUNIT_ASSERT_NO_THROW(f = tag.createFeature(refs[0], nix::LinkType::Indexed));
    CPPUNIT_ASSERT(tag.featureCount() == 1);
    CPPUNIT_ASSERT_NO_THROW(tag.deleteFeature(f));
    CPPUNIT_ASSERT(tag.featureCount() == 0);
}


void TestTag::testDataAccess() {
    double samplingInterval = 1.0;
    vector<double> ticks {1.2, 2.3, 3.4, 4.5, 6.7};
    string unit = "ms";
    SampledDimension sampledDim;
    RangeDimension rangeDim;
    SetDimension setDim;
    vector<double> position {0.0, 2.0, 3.4};
    vector<double> extent {0.0, 6.0, 2.3};
    vector<string> units {"none", "ms", "ms"};

    DataArray data_array = block.createDataArray("dimensionTest",
                                                 "test",
                                                 DataType::Double,
                                                 NDSize({0, 0, 0}));
    vector<DataArray> reference;
    reference.push_back(data_array);

    typedef boost::multi_array<double, 3> array_type;
    typedef array_type::index index;
    array_type data(boost::extents[2][10][5]);
    int value;
    for(index i = 0; i != 2; ++i) {
        value = 0;
        for(index j = 0; j != 10; ++j) {
            for(index k = 0; k != 5; ++k) {
                data[i][j][k] = value++;
            }
        }
    }
    data_array.setData(data);

    setDim = data_array.appendSetDimension();
    std::vector<std::string> labels = {"label_a", "label_b"};
    setDim.labels(labels);

    sampledDim = data_array.appendSampledDimension(samplingInterval);
    sampledDim.unit(unit);

    rangeDim = data_array.appendRangeDimension(ticks);
    rangeDim.unit(unit);

    Tag position_tag = block.createTag("position tag", "event", position);
    position_tag.references(reference);
    position_tag.units(units);

    Tag segment_tag = block.createTag("region tag", "segment", position);
    segment_tag.references(reference);
    segment_tag.extent(extent);
    segment_tag.units(units);

    DataView retrieved_data = position_tag.retrieveData(0);
    NDSize data_size = retrieved_data.dataExtent();
    CPPUNIT_ASSERT(data_size.size() == 3);
    CPPUNIT_ASSERT(data_size[0] == 1 && data_size[1] == 1 &&  data_size[2] == 1);

    retrieved_data = segment_tag.retrieveData( 0);
    data_size = retrieved_data.dataExtent();
    CPPUNIT_ASSERT(data_size.size() == 3);
    CPPUNIT_ASSERT(data_size[0] == 1 && data_size[1] == 6 && data_size[2] == 2);

    block.deleteTag(position_tag);
    block.deleteTag(segment_tag);
}


void TestTag::testOperators() {
    CPPUNIT_ASSERT(tag_null == false);
    CPPUNIT_ASSERT(tag_null == none);

    CPPUNIT_ASSERT(tag != false);
    CPPUNIT_ASSERT(tag != none);

    CPPUNIT_ASSERT(tag == tag);
    CPPUNIT_ASSERT(tag != tag_other);

    tag_other = tag;
    CPPUNIT_ASSERT(tag == tag_other);

    tag_other = none;
    CPPUNIT_ASSERT(tag_null == false);
    CPPUNIT_ASSERT(tag_null == none);
}


void TestTag::testCreatedAt() {
    CPPUNIT_ASSERT(tag.createdAt() >= startup_time);
    time_t past_time = time(NULL) - 10000000;
    tag.forceCreatedAt(past_time);
    CPPUNIT_ASSERT(tag.createdAt() == past_time);
}


void TestTag::testUpdatedAt() {
    CPPUNIT_ASSERT(tag.updatedAt() >= startup_time);
}
