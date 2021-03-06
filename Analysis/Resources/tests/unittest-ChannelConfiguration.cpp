///@file unittest-ChannelConfiguration.cpp
///@brief Program that will test functionality of ChannelConfiguration
///@author S. V. Paulauskas
///@date November 25, 2016
#include <iostream>

#include <cmath>

#include <UnitTest++.h>

#include "ChannelConfiguration.hpp"

using namespace std;

///Testing the set/get for tags
TEST_FIXTURE (ChannelConfiguration, Test_GetAndSetTag) {
    string tag = "unittest";
    AddTag(tag);
    CHECK(HasTag(tag));

    set<string> testset;
    testset.insert(tag);
    CHECK (GetTags() == testset);
}

///Testing the case that we have a missing tag
TEST_FIXTURE (ChannelConfiguration, Test_HasMissingTag) {
    string tag = "unittest";
    CHECK(!HasTag(tag));
}

//Check the comparison operators
TEST_FIXTURE (ChannelConfiguration, Test_Comparison) {
    string type = "unit";
    string subtype = "test";
    unsigned int loc = 112;
    SetSubtype(subtype);
    SetType(type);
    SetLocation(loc);

    ChannelConfiguration id(type, subtype, loc);

    CHECK(*this == id);

    SetLocation(123);
    CHECK(*this > id);

    SetLocation(4);
    CHECK(*this < id);
}

///Testing that the place name is returning the proper value
TEST_FIXTURE (ChannelConfiguration, Test_PlaceName) {
    string type = "unit";
    string subtype = "test";
    unsigned int loc = 112;
    SetSubtype(subtype);
    SetType(type);
    SetLocation(loc);
    CHECK (GetPlaceName() == "unit_test_112");
}

///Test that the zero function is performing it's job properly
TEST_FIXTURE (ChannelConfiguration, Test_Zero) {
    ChannelConfiguration id("unit", "test", 123);
    id.Zero();
    CHECK(*this == id);
}

int main(int argv, char *argc[]) {
    return (UnitTest::RunAllTests());
}
