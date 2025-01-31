// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2022, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html


#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/difference.hpp>
#include <boost/geometry/geometries/adapted/boost_variant2.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/io/wkt/wkt.hpp>


using pt_t = bg::model::point<double, 2, bg::cs::cartesian>;
using ls_t = bg::model::linestring<pt_t>;
using po_t = bg::model::polygon<pt_t>;
using mpt_t = bg::model::multi_point<pt_t>;
using mls_t = bg::model::multi_linestring<ls_t>;
using mpo_t = bg::model::multi_polygon<po_t>;
using var_t = boost::variant<pt_t, ls_t, po_t, mpt_t, mls_t, mpo_t>;
//using var_t = boost::variant2::variant<pt_t, ls_t, po_t, mpt_t, mls_t, mpo_t>;
using gc_t = bg::model::geometry_collection<var_t>;

template <typename GC1, typename GC2>
inline void check_one(GC1 const& out, GC2 const& expected)
{
    std::size_t out_size = boost::size(out);
    std::size_t expected_size = boost::size(expected);
    BOOST_CHECK(out_size == expected_size);
    if (out_size == expected_size)
    {
        for (std::size_t i = 0; i < out_size; ++i)
        {
            bg::traits::iter_visit<GC1>::apply([&](auto const& o)
            {
                bg::traits::iter_visit<GC2>::apply([&](auto const& e)
                {
                    using o_t = typename bg::util::remove_cref<decltype(o)>::type;
                    using e_t = typename bg::util::remove_cref<decltype(e)>::type;
                    int oid = bg::geometry_id<o_t>::value;
                    int eid = bg::geometry_id<e_t>::value;
                    BOOST_CHECK_MESSAGE(oid == eid, oid << " and " << eid);
                    bool elements_equal = bg::equals(o, e);
                    BOOST_CHECK_MESSAGE(elements_equal, bg::wkt(o) << " and " << bg::wkt(e));
                }, expected.begin() + i);
            }, out.begin() + i);
        }
    }
}


void test_gc_gc_gc()
{
    const char* gc1_cstr = "GEOMETRYCOLLECTION("
        "MULTIPOLYGON(((0 0, 0 10, 10 10, 10 0, 0 0), (1 1, 5 1, 5 5, 1 5, 1 1)), ((3 3, 3 4, 4 4, 4 3, 3 3))),"
        "POLYGON((3 3, 3 10, 10 10, 10 3, 3 3)),"
        "MULTILINESTRING((0 0, 5 5), (5 5, 11 5), (5 5, 5 11)),"
        "LINESTRING(0 0, 2 2, 2 11),"
        "MULTIPOINT(0 0, 1 1, 2 2, 3 3, 4 4, 5 5, 6 6, 7 7, 8 8, 9 9, 10 10),"
        "POINT(11 11)"
    ")";
    const char* gc2_cstr = "GEOMETRYCOLLECTION("
        "POLYGON((4 0, 4 10, 6 10, 6 0, 4 0)),"
        "POLYGON((0 4, 0 6, 10 6, 10 4, 0 4)),"
        "POLYGON((3 3, 3 5, 5 5, 5 3, 3 3)),"
        "MULTILINESTRING((1 1, 6 6), (5 1, 5 9), (2 2, 9 2)),"
        "MULTIPOINT(9 9, 11 11, 1 9),"
        "POINT(3 2)"
    ")";
    const char* expected1_cstr = "GEOMETRYCOLLECTION("
        "MULTIPOLYGON(((0 0, 0 4, 1 4, 1 1, 4 1, 4 0, 0 0)), ((0 6, 0 10, 4 10, 4 6, 0 6)), ((6 6, 6 10, 10 10, 10 6, 6 6)), ((6 0, 6 4, 10 4, 10 0, 6 0))),"
        "MULTIPOLYGON(((3 6, 3 10, 4 10, 4 6, 3 6)), ((6 3, 6 4, 10 4, 10 3, 6 3)), ((6 6, 6 10, 10 10, 10 6, 6 6))),"
        "MULTILINESTRING((0 0, 1 1), (10 5, 11 5), (5 10, 5 11)),"
        "MULTILINESTRING((0 0, 1 1), (2 2, 2 4), (2 6, 2 11)),"
        "MULTIPOINT(0 0, 7 7, 8 8, 10 10)"
    ")";
    const char* expected2_cstr = "GEOMETRYCOLLECTION("
        "POLYGON((4 1, 4 3, 5 3, 5 1, 4 1)),"
        "POLYGON((1 4, 1 5, 3 5, 3 4, 1 4)),"
        "LINESTRING(2 2, 5 2),"
        "POINT(3 2)"
        ")";

    gc_t gc1, gc2, expected1, expected2;
    bg::read_wkt(gc1_cstr, gc1);
    bg::read_wkt(gc2_cstr, gc2);
    bg::read_wkt(expected1_cstr, expected1);
    bg::read_wkt(expected2_cstr, expected2);

    gc_t out1, out2;
    bg::difference(gc1, gc2, out1);
    bg::difference(gc2, gc1, out2);

    check_one(out1, expected1);
    check_one(out2, expected2);
}

void test_gc_g_gc()
{
    const char* gc_cstr = "GEOMETRYCOLLECTION("
        "MULTIPOLYGON(((0 0, 0 10, 10 10, 10 0, 0 0), (1 1, 5 1, 5 5, 1 5, 1 1)), ((3 3, 3 4, 4 4, 4 3, 3 3))),"
        "POLYGON((3 3, 3 10, 10 10, 10 3, 3 3)),"
        "MULTILINESTRING((0 0, 5 5), (5 5, 11 5), (5 5, 5 11)),"
        "LINESTRING(0 0, 2 2, 2 11),"
        "MULTIPOINT(0 0, 1 1, 2 2, 3 3, 4 4, 5 5, 6 6, 7 7, 8 8, 9 9, 10 10),"
        "POINT(11 11)"
    ")";
    const char* po_cstr = "POLYGON((4 0, 4 10, 6 10, 6 0, 4 0))";
    const char* expected_cstr = "GEOMETRYCOLLECTION("
        "MULTIPOLYGON(((0 0, 0 10, 4 10, 4 5, 1 5, 1 1, 4 1, 4 0, 0 0)), ((6 0, 6 10, 10 10, 10 0, 6 0)), ((3 3, 3 4, 4 4, 4 3, 3 3))),"
        "MULTIPOLYGON(((3 3, 3 10, 4 10, 4 3, 3 3)), ((6 3, 6 10, 10 10, 10 3, 6 3))),"
        "MULTILINESTRING((0 0, 4 4), (6 5, 11 5), (5 10, 5 11)),"
        "LINESTRING(0 0, 2 2, 2 11),"
        "MULTIPOINT(0 0, 1 1, 2 2, 3 3, 7 7, 8 8, 9 9, 10 10),"
        "POINT(11 11)"
    ")";

    gc_t gc, expected;
    po_t po;
    bg::read_wkt(gc_cstr, gc);
    bg::read_wkt(po_cstr, po);
    bg::read_wkt(expected_cstr, expected);

    gc_t out;
    bg::difference(gc, po, out);

    check_one(out, expected);
}

void test_g_gc_gc()
{
    const char* mpo_cstr = "MULTIPOLYGON(((0 0, 0 10, 10 10, 10 0, 0 0), (1 1, 5 1, 5 5, 1 5, 1 1)), ((3 3, 3 4, 4 4, 4 3, 3 3)))";
    const char* gc_cstr = "GEOMETRYCOLLECTION("
        "POLYGON((4 0, 4 10, 6 10, 6 0, 4 0)),"
        "POLYGON((0 4, 0 6, 10 6, 10 4, 0 4)),"
        "POLYGON((3 3, 3 5, 5 5, 5 3, 3 3)),"
        "MULTILINESTRING((1 1, 6 6), (5 1, 5 9), (2 2, 9 2)),"
        "MULTIPOINT(9 9, 11 11, 1 9),"
        "POINT(3 2)"
    ")";
    const char* expected_cstr = "GEOMETRYCOLLECTION("
        "MULTIPOLYGON(((0 0, 0 4, 1 4, 1 1, 4 1, 4 0, 0 0)), ((0 6, 0 10, 4 10, 4 6, 0 6)), ((6 6, 6 10, 10 10, 10 6, 6 6)), ((6 0, 6 4, 10 4, 10 0, 6 0)))"
    ")";

    mpo_t mpo;
    gc_t gc, expected;    
    bg::read_wkt(mpo_cstr, mpo);
    bg::read_wkt(gc_cstr, gc);    
    bg::read_wkt(expected_cstr, expected);

    gc_t out;
    bg::difference(mpo, gc, out);

    check_one(out, expected);
}

void test_g_g_gc()
{
    const char* mpo_cstr = "MULTIPOLYGON(((0 0, 0 10, 10 10, 10 0, 0 0), (1 1, 5 1, 5 5, 1 5, 1 1)), ((3 3, 3 4, 4 4, 4 3, 3 3)))";
    const char* po_cstr = "POLYGON((4 0, 4 10, 6 10, 6 0, 4 0))";
    const char* expected_cstr = "GEOMETRYCOLLECTION("
        "MULTIPOLYGON(((0 0, 0 10, 4 10, 4 5, 1 5, 1 1, 4 1, 4 0, 0 0)), ((3 3, 3 4, 4 4, 4 3, 3 3)), ((6 0, 6 10, 10 10, 10 0, 6 0)))"
    ")";

    mpo_t mpo;
    po_t po;
    gc_t expected;    
    bg::read_wkt(mpo_cstr, mpo);
    bg::read_wkt(po_cstr, po);
    bg::read_wkt(expected_cstr, expected);

    gc_t out;
    bg::difference(mpo, po, out);

    check_one(out, expected);
}

int test_main(int, char* [])
{
    test_gc_gc_gc();
    test_gc_g_gc();
    test_g_gc_gc();
    test_g_g_gc();

    return 0;
}
