// Copyright (c) 2013, German Neuroinformatics Node (G-Node)
//
// All rights reserved.
//
// Redistribution and use in Section and binary forms, with or without
// modification, are permitted under the terms of the BSD License. See
// LICENSE file in the root of the Project.

#include "TestSection.hpp"

#include <nix/util/util.hpp>
#include <nix/valid/validate.hpp>

#include <ctime>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

using namespace std;
using namespace nix;
using namespace valid;


void TestSection::setUp() {
    startup_time = time(NULL);
    file = File::open("test_section.h5", FileMode::Overwrite);

    section = file.createSection("section", "metadata");
    section_other = file.createSection("other_section", "metadata");
    section_null  = nullptr;

    file_fs = File::open("test_section", FileMode::Overwrite, Implementation::FileSys);
    section_fs = file_fs.createSection("section_fs", "metadata");
    section_fs_other = file_fs.createSection("other_section", "metadata");
}

void TestSection::tearDown() {
    file.close();
    boost::filesystem::remove_all(boost::filesystem::path(file_fs.location().c_str()));
}


void TestSection::testValidate() {
    valid::Result result = validate(section);
    CPPUNIT_ASSERT(result.getErrors().size() == 0);
    CPPUNIT_ASSERT(result.getWarnings().size() == 0);
}


void TestSection::testId() {
    CPPUNIT_ASSERT(section.id().size() == 36);
    CPPUNIT_ASSERT(section_fs.id().size() == 36);
}


void TestSection::testName() {
    CPPUNIT_ASSERT(section.name() == "section");
    CPPUNIT_ASSERT(section_fs.name() == "section_fs");
}


void TestSection::testType() {
    CPPUNIT_ASSERT(section.type() == "metadata");
    string typ = util::createId();
    section.type(typ);
    CPPUNIT_ASSERT(section.type() == typ);

    CPPUNIT_ASSERT(section_fs.type() == "metadata");
    section_fs.type(typ);
    CPPUNIT_ASSERT(section_fs.type() == typ);
}


void TestSection::testDefinition() {
    string def = util::createId();
    section.definition(def);
    CPPUNIT_ASSERT(*section.definition() == def);
    section.definition(nix::none);
    CPPUNIT_ASSERT(section.definition() == nix::none);

    section_fs.definition(def);
    CPPUNIT_ASSERT(*section_fs.definition() == def);
    section_fs.definition(nix::none);
    CPPUNIT_ASSERT(section_fs.definition() == nix::none);
}


void TestSection::testParent() {
    CPPUNIT_ASSERT((section.parent() == nullptr));
    Section child = section.createSection("child", "section");
    CPPUNIT_ASSERT(child.parent() != nullptr);
    CPPUNIT_ASSERT(child.parent().id() == section.id());
    CPPUNIT_ASSERT(child.parent().parent() == nullptr);

    CPPUNIT_ASSERT(section_fs.parent() == nullptr);
    Section child_fs = section_fs.createSection("child", "section");
    CPPUNIT_ASSERT(child_fs.parent() != nullptr);
    CPPUNIT_ASSERT(child_fs.parent().id() == section_fs.id());
    CPPUNIT_ASSERT(child_fs.parent().parent() == nullptr);

}


void TestSection::testRepository() {
    CPPUNIT_ASSERT(!section.repository());
    string rep = "http://foo.bar/" + util::createId();
    section.repository(rep);
    CPPUNIT_ASSERT(section.repository() == rep);
    section.repository(boost::none);
    CPPUNIT_ASSERT(!section.repository());

    CPPUNIT_ASSERT(!section_fs.repository());
    section_fs.repository(rep);
    CPPUNIT_ASSERT(section_fs.repository() == rep);
    section_fs.repository(boost::none);
    CPPUNIT_ASSERT(!section_fs.repository());
}


void TestSection::testLink() {
    CPPUNIT_ASSERT(!section.link());

    section.link(section_other);
    CPPUNIT_ASSERT(section.link());
    CPPUNIT_ASSERT(section.link().id() == section_other.id());

    // test none-unsetter
    section.link(none);
    CPPUNIT_ASSERT(!section.link());
    // test deleter removing link too
    section.link(section);
    file.deleteSection(section.id());
    CPPUNIT_ASSERT(!section.link());
    // re-create section
    section = file.createSection("foo_section", "metadata");

    CPPUNIT_ASSERT(!section_fs.link());
    section_fs.link(section_fs_other);
    CPPUNIT_ASSERT(section_fs.link());
    CPPUNIT_ASSERT(section_fs.link().id() == section_fs_other.id());
    // test none-unsetter
    section_fs.link(none);
    CPPUNIT_ASSERT(!section_fs.link());
}


void TestSection::testMapping() {
    CPPUNIT_ASSERT(!section.mapping());
    string map = "http://foo.bar/" + util::createId();
    section.mapping(map);
    CPPUNIT_ASSERT(section.mapping() == map);
    section.mapping(boost::none);
    CPPUNIT_ASSERT(!section.mapping());

    CPPUNIT_ASSERT(!section_fs.mapping());
    section_fs.mapping(map);
    CPPUNIT_ASSERT(section_fs.mapping() == map);
    section_fs.mapping(boost::none);
    CPPUNIT_ASSERT(!section_fs.mapping());
}


void TestSection::testSectionAccess() {
    vector<string> names = { "section_a", "section_b", "section_c", "section_d", "section_e" };

    CPPUNIT_ASSERT(section.sectionCount() == 0);
    CPPUNIT_ASSERT(section.sections().size() == 0);
    CPPUNIT_ASSERT(section.getSection("invalid_id") == false);
    CPPUNIT_ASSERT_EQUAL(false, section.hasSection("invalid_id"));

    vector<string> ids;
    for (auto name : names) {
        Section child_section = section.createSection(name, "metadata");
        CPPUNIT_ASSERT(child_section.name() == name);
        CPPUNIT_ASSERT_EQUAL(true, section.hasSection(name));

        ids.push_back(child_section.id());
    }
    CPPUNIT_ASSERT_THROW(section.createSection(names[0], "metadata"),
                         DuplicateName);

    CPPUNIT_ASSERT(section.sectionCount() == names.size());
    CPPUNIT_ASSERT(section.sections().size() == names.size());

    for (auto id : ids) {
        Section child_section = section.getSection(id);
        CPPUNIT_ASSERT(section.hasSection(id));
        CPPUNIT_ASSERT_EQUAL(id, child_section.id());

        section.deleteSection(id);

    }

    CPPUNIT_ASSERT(section.sectionCount() == 0);
    CPPUNIT_ASSERT(section.sections().size() == 0);
    CPPUNIT_ASSERT(section.getSection("invalid_id") == false);

    // Filesystem checks
    CPPUNIT_ASSERT(section_fs.sectionCount() == 0);
    CPPUNIT_ASSERT(section_fs.sections().size() == 0);
    CPPUNIT_ASSERT(section_fs.getSection("invalid_id") == false);
    CPPUNIT_ASSERT_EQUAL(false, section_fs.hasSection("invalid_id"));

    ids.clear();
    for (auto name : names) {
        Section child_section = section_fs.createSection(name, "metadata");
        CPPUNIT_ASSERT(child_section.name() == name);
        CPPUNIT_ASSERT_EQUAL(true, section_fs.hasSection(name));

        ids.push_back(child_section.id());
    }
    CPPUNIT_ASSERT_THROW(section_fs.createSection(names[0], "metadata"),
                         DuplicateName);

    CPPUNIT_ASSERT(section_fs.sectionCount() == names.size());
    CPPUNIT_ASSERT(section_fs.sections().size() == names.size());

    for (auto id : ids) {
        Section child_section = section_fs.getSection(id);
        CPPUNIT_ASSERT(section_fs.hasSection(id));
        CPPUNIT_ASSERT_EQUAL(id, child_section.id());

        section_fs.deleteSection(id);
    }

    CPPUNIT_ASSERT(section_fs.sectionCount() == 0);
    CPPUNIT_ASSERT(section_fs.sections().size() == 0);
    CPPUNIT_ASSERT(section_fs.getSection("invalid_id") == false);
}


void TestSection::testFindSection() {
    // prepare
    Section l1n1 = section.createSection("l1n1", "typ1");
    Section l1n2 = section.createSection("l1n2", "typ2");
    Section l1n3 = section.createSection("l1n3", "typ3");

    Section l2n1 = l1n1.createSection("l2n1", "typ1");
    Section l2n2 = l1n1.createSection("l2n2", "typ2");
    Section l2n3 = l1n1.createSection("l2n3", "typ2");
    Section l2n4 = l1n3.createSection("l2n4", "typ2");
    Section l2n5 = l1n3.createSection("l2n5", "typ2");
    Section l2n6 = l1n3.createSection("l2n6", "typ3");

    Section l3n1 = l2n1.createSection("l3n1", "typ1");
    Section l3n2 = l2n3.createSection("l3n2", "typ2");
    Section l3n3 = l2n3.createSection("l3n3", "typ2");
    Section l3n4 = l2n5.createSection("l3n4", "typ2");

    // test depth limit
    CPPUNIT_ASSERT(section.findSections().size() == 14);
    CPPUNIT_ASSERT(section.findSections(util::AcceptAll<Section>(), 2).size() == 10);
    CPPUNIT_ASSERT(section.findSections(util::AcceptAll<Section>(), 1).size() == 4);
    CPPUNIT_ASSERT(section.findSections(util::AcceptAll<Section>(), 0).size() == 1);

    // test filter
    auto filter_typ1 = util::TypeFilter<Section>("typ1");
    auto filter_typ2 = util::TypeFilter<Section>("typ2");

    CPPUNIT_ASSERT(section.findSections(filter_typ1).size() == 3);
    CPPUNIT_ASSERT(section.findSections(filter_typ2).size() == 8);
}

void TestSection::testFindRelated() {
    /* We create the following tree:
     * 
     * section---l1n1---l2n1---l3n1------------
     *            |      |                    |
     *            ------l2n2---l3n2---l4n1---l5n1
     *                   |      |      |
     *                   ------l3n3---l4n2
     * section_other------------|
     */
    Section l1n1 = section.createSection("l1n1", "typ1");

    Section l2n1 = l1n1.createSection("l2n1", "t1");
    Section l2n2 = l1n1.createSection("l2n2", "t2");
    Section l3n1 = l2n1.createSection("l3n1", "t3");
    Section l3n2 = l2n2.createSection("l3n2", "t3");
    Section l3n3 = l2n2.createSection("l3n3", "t4");
    Section l4n1 = l3n2.createSection("l4n1", "typ2");
    Section l4n2 = l3n3.createSection("l4n2", "typ2");
    Section l5n1 = l4n1.createSection("l5n1", "typ2");
    l2n1.link(l2n2.id());
    l3n1.link(l5n1.id());
    l3n2.link(l3n3.id());
    l4n1.link(l4n2.id());
    section_other.link(l3n3.id());

    string t1 = "t1";
    string t3 = "t3";
    string t4 = "t4";
    string typ2 = "typ2";
    string typ1 = "typ1";

    vector<Section> related = l1n1.findRelated(util::TypeFilter<Section>(t1));
    CPPUNIT_ASSERT(related.size() == 1);
    related = l1n1.findRelated(util::TypeFilter<Section>(t3));
    CPPUNIT_ASSERT(related.size() == 2);
    related = l1n1.findRelated(util::TypeFilter<Section>(t4));
    CPPUNIT_ASSERT(related.size() == 1);
    related = l1n1.findRelated(util::TypeFilter<Section>(typ2));
    CPPUNIT_ASSERT(related.size() == 2);
    related = l4n1.findRelated(util::TypeFilter<Section>(typ1));
    CPPUNIT_ASSERT(related.size() == 1);
    related = l4n1.findRelated(util::TypeFilter<Section>(t1));
    CPPUNIT_ASSERT(related.size() == 1);
    related = l3n2.findRelated(util::TypeFilter<Section>(t1));
    CPPUNIT_ASSERT(related.size() == 1);
    related = l3n2.findRelated(util::TypeFilter<Section>(t3));
    CPPUNIT_ASSERT(related.size() == 0);

    /* Chop the tree to:
     * 
     * section---l1n1---l2n1---l3n1
     * section_other
     *                   
     */
    l1n1.deleteSection(l2n2.id());
    CPPUNIT_ASSERT(section.findSections().size() == 4);
    // test that all (horizontal) links are gone too:
    CPPUNIT_ASSERT(!l2n1.link());
    CPPUNIT_ASSERT(!l3n1.link());
    CPPUNIT_ASSERT(!l3n2.link());
    CPPUNIT_ASSERT(!l4n1.link());
    CPPUNIT_ASSERT(!section_other.link());
    CPPUNIT_ASSERT(!l1n1.hasSection(l2n2));
    /* Extend the tree to:
     * 
     * section---l1n1---l2n1---l3n1
     * section_other-----|
     * 
     * and then chop it down to:
     * 
     * section_other
     *                   
     */
    section_other.link(l2n1.id());
    file.deleteSection(section.id());
    CPPUNIT_ASSERT(section_other.findSections().size() == 1);
    CPPUNIT_ASSERT(!section_other.link());

    // re-create section
    section = file.createSection("section", "metadata");
}


void TestSection::testPropertyAccess() {
    vector<string> names = { "property_a", "property_b", "property_c", "property_d", "property_e" };

    CPPUNIT_ASSERT(section.propertyCount() == 0);
    CPPUNIT_ASSERT(section.properties().size() == 0);
    CPPUNIT_ASSERT(section.getProperty("invalid_id") == false);
    CPPUNIT_ASSERT_EQUAL(false, section.hasProperty("invalid_id"));

    Property p = section.createProperty("empty_prop", DataType::Double);
    CPPUNIT_ASSERT(section.propertyCount() == 1);
    CPPUNIT_ASSERT(section.hasProperty(p));
    CPPUNIT_ASSERT(section.hasProperty("empty_prop"));
    Property prop = section.getProperty("empty_prop");
    CPPUNIT_ASSERT(prop.valueCount() == 0);
    CPPUNIT_ASSERT(prop.dataType() == nix::DataType::Double);
    section.deleteProperty(p.id());
    CPPUNIT_ASSERT(section.propertyCount() == 0);

    Value dummy(10);
    prop = section.createProperty("single value", dummy);
    CPPUNIT_ASSERT(section.hasProperty("single value"));
    CPPUNIT_ASSERT(section.propertyCount() == 1);
    section.deleteProperty(prop.id());
    CPPUNIT_ASSERT(section.propertyCount() == 0);

    vector<string> ids;
    for (auto name : names) {
        prop = section.createProperty(name, dummy);
        CPPUNIT_ASSERT(prop.name() == name);
        CPPUNIT_ASSERT(section.hasProperty(name));

        Property prop_copy = section.getProperty(name);

        CPPUNIT_ASSERT(prop_copy.id() == prop.id());

        ids.push_back(prop.id());
    }
    CPPUNIT_ASSERT_THROW(section.createProperty(names[0], dummy),
                         DuplicateName);

    CPPUNIT_ASSERT(section.propertyCount() == names.size());
    CPPUNIT_ASSERT(section.properties().size() == names.size());
    section_other.createProperty("some_prop", dummy);
    section_other.link(section);
    CPPUNIT_ASSERT(section_other.propertyCount() == 1);
    CPPUNIT_ASSERT(section_other.inheritedProperties().size() == names.size() + 1);

    for (auto id : ids) {
        Property prop = section.getProperty(id);
        CPPUNIT_ASSERT(section.hasProperty(id));
        CPPUNIT_ASSERT(prop.id() == id);

        section.deleteProperty(id);
    }

    CPPUNIT_ASSERT(section.propertyCount() == 0);
    CPPUNIT_ASSERT(section.properties().size() == 0);
    CPPUNIT_ASSERT(section.getProperty("invalid_id") == false);

    vector<Value> values;
    values.push_back(Value(10));
    values.push_back(Value(100));
    section.createProperty("another test", values);
    CPPUNIT_ASSERT(section.propertyCount() == 1);
    prop = section.getProperty("another test");
    CPPUNIT_ASSERT(prop.valueCount() == 2);

    // Filesystem checks
    CPPUNIT_ASSERT(section_fs.propertyCount() == 0);
    CPPUNIT_ASSERT(section_fs.properties().size() == 0);
    CPPUNIT_ASSERT(section_fs.getProperty("invalid_id") == false);
    CPPUNIT_ASSERT_EQUAL(false, section_fs.hasProperty("invalid_id"));

    p = section_fs.createProperty("empty_prop", DataType::Double);
    CPPUNIT_ASSERT(section_fs.propertyCount() == 1);
    CPPUNIT_ASSERT(section_fs.hasProperty(p));
    CPPUNIT_ASSERT(section_fs.hasProperty("empty_prop"));
    prop = section_fs.getProperty("empty_prop");
    CPPUNIT_ASSERT(prop.valueCount() == 0);
    CPPUNIT_ASSERT(prop.dataType() == nix::DataType::Double);
    section_fs.deleteProperty(p.id());
    CPPUNIT_ASSERT(section_fs.propertyCount() == 0);

    /*
    Value dummy(10);
    prop = section.createProperty("single value", dummy);
    CPPUNIT_ASSERT(section.hasProperty("single value"));
    CPPUNIT_ASSERT(section.propertyCount() == 1);
    section.deleteProperty(prop.id());
    CPPUNIT_ASSERT(section.propertyCount() == 0);
    */ // values are not yet supported in filesys backend
    ids.clear();
    for (auto name : names) {
        prop = section_fs.createProperty(name, dummy);
        CPPUNIT_ASSERT(prop.name() == name);
        CPPUNIT_ASSERT(section_fs.hasProperty(name));

        Property prop_copy = section_fs.getProperty(name);
        CPPUNIT_ASSERT(prop_copy.id() == prop.id());
        ids.push_back(prop.id());
    }
    CPPUNIT_ASSERT_THROW(section_fs.createProperty(names[0], dummy),
                         DuplicateName);

    CPPUNIT_ASSERT(section_fs.propertyCount() == names.size());
    CPPUNIT_ASSERT(section_fs.properties().size() == names.size());
    section_fs_other.createProperty("some_prop", dummy);
    section_fs_other.link(section_fs);
    CPPUNIT_ASSERT(section_fs_other.propertyCount() == 1);
    CPPUNIT_ASSERT(section_fs_other.inheritedProperties().size() == names.size() + 1);

    for (auto id : ids) {
        Property prop = section_fs.getProperty(id);
        CPPUNIT_ASSERT(section_fs.hasProperty(id));
        CPPUNIT_ASSERT(prop.id() == id);

        section_fs.deleteProperty(id);
    }

    CPPUNIT_ASSERT(section_fs.propertyCount() == 0);
    CPPUNIT_ASSERT(section_fs.properties().size() == 0);
    CPPUNIT_ASSERT(section_fs.getProperty("invalid_id") == false);
    /*
    vector<Value> values;
    values.push_back(Value(10));
    values.push_back(Value(100));
    section.createProperty("another test", values);
    CPPUNIT_ASSERT(section.propertyCount() == 1);
    prop = section.getProperty("another test");
    CPPUNIT_ASSERT(prop.valueCount() == 2);
    */ // Values are not supported yet
}


void TestSection::testOperators() {
    CPPUNIT_ASSERT(section_null == false);
    CPPUNIT_ASSERT(section_null == none);

    CPPUNIT_ASSERT(section != false);
    CPPUNIT_ASSERT(section != none);

    CPPUNIT_ASSERT(section == section);
    CPPUNIT_ASSERT(section != section_other);

    section_other = section;
    CPPUNIT_ASSERT(section == section_other);

    section_other = none;
    CPPUNIT_ASSERT(section_null == false);
    CPPUNIT_ASSERT(section_null == none);

    CPPUNIT_ASSERT(section_fs != false);
    CPPUNIT_ASSERT(section_fs != none);

    CPPUNIT_ASSERT(section_fs == section_fs);
    CPPUNIT_ASSERT(section_fs != section_fs_other);

    section_fs_other = section_fs;
    CPPUNIT_ASSERT(section_fs == section_fs_other);

}


void TestSection::testCreatedAt() {
    CPPUNIT_ASSERT(section.createdAt() >= startup_time);
    time_t past_time = time(NULL) - 10000000;
    section.forceCreatedAt(past_time);
    CPPUNIT_ASSERT(section.createdAt() == past_time);

    CPPUNIT_ASSERT(section_fs.createdAt() >= startup_time);
    section_fs.forceCreatedAt(past_time);
    CPPUNIT_ASSERT(section_fs.createdAt() == past_time);
}


void TestSection::testUpdatedAt() {
    CPPUNIT_ASSERT(section.updatedAt() >= startup_time);
    CPPUNIT_ASSERT(section_fs.updatedAt() >= startup_time);
}
