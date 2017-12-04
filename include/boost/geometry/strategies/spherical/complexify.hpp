// Boost.Geometry

// Copyright (c) 2017, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_COMPLEXIFY_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_COMPLEXIFY_HPP


#include <boost/geometry/algorithms/detail/convert_point_to_point.hpp>
#include <boost/geometry/algorithms/detail/signed_size_type.hpp>
#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/geometry/arithmetic/cross_product.hpp>
#include <boost/geometry/arithmetic/dot_product.hpp>
#include <boost/geometry/arithmetic/normalize.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/formulas/spherical.hpp>
#include <boost/geometry/strategies/complexify.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/range.hpp>
#include <boost/geometry/util/select_most_precise.hpp>

#include <boost/range/value_type.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace complexify
{


template
<
    typename Sphere = srs::sphere<double>,
    typename CalculationType = void
>
struct spherical
{
    // For consistency with area strategy the radius is set to 1
    spherical()
        : m_radius(1.0)
    {}

    explicit spherical(Sphere const& sphere)
        : m_radius(geometry::get_radius<0>(sphere))
    {}

    // TODO: use enable_if/disable_if to distinguish between Sphere and Radius?
    template <typename OtherSphere>
    explicit spherical(OtherSphere const& sphere)
        : m_radius(geometry::get_radius<0>(sphere))
    {}

    template <typename Point, typename RangeOut, typename T>
    inline void apply(Point const& p0, Point const& p1, RangeOut & rng, T const& length_threshold) const
    {
        typedef typename boost::range_value<RangeOut>::type rng_point_t;
        typedef typename select_most_precise
            <
                typename coordinate_type<Point>::type,
                typename coordinate_type<rng_point_t>::type,
                CalculationType
            >::type calc_t;

        calc_t const c0 = 0;
        calc_t const c1 = 1;
        calc_t const pi = math::pi<calc_t>();

        typedef model::point<calc_t, 3, cs::cartesian> point3d_t;
        point3d_t const xyz0 = formula::sph_to_cart3d<point3d_t>(p0);
        point3d_t const xyz1 = formula::sph_to_cart3d<point3d_t>(p1);
        calc_t const dot01 = geometry::dot_product(xyz0, xyz1);
        calc_t const angle01 = acos(dot01);

        signed_size_type n = signed_size_type(angle01 * m_radius / length_threshold);
        if (n <= 0)
            return;

        point3d_t axis;
        if (! math::equals(angle01, pi))
        {
            axis = geometry::cross_product(xyz0, xyz1);
            geometry::detail::vec_normalize(axis);
        }
        else // antipodal
        {
            calc_t const half_pi = math::half_pi<calc_t>();
            calc_t const lat = geometry::get_as_radian<1>(p0);

            if (math::equals(lat, half_pi))
            {
                // pointing east, segment lies on prime meridian, going south
                axis = point3d_t(c0, c1, c0);
            }
            else if (math::equals(lat, -half_pi))
            {
                // pointing west, segment lies on prime meridian, going north
                axis = point3d_t(c0, -c1, c0);
            }
            else
            {
                // lon rotated west by pi/2 at equator
                calc_t const lon = geometry::get_as_radian<0>(p0);
                axis = point3d_t(sin(lon), -cos(lon), c0);
            }
        }

        calc_t step = angle01 / (n + 1);

        calc_t a = step;
        for (signed_size_type i = 0 ; i < n ; ++i, a += step)
        {
            // Axis-Angle rotation
            // see: https://en.wikipedia.org/wiki/Axis-angle_representation
            calc_t const cos_a = cos(a);
            calc_t const sin_a = sin(a);
            // cos_a * v
            point3d_t s1 = xyz0;
            geometry::multiply_value(s1, cos_a);
            // sin_a * (n x v)
            point3d_t s2 = geometry::cross_product(axis, xyz0);
            geometry::multiply_value(s2, sin_a);
            // (1 - cos_a)(n.v) * n
            point3d_t s3 = axis;
            geometry::multiply_value(s3, (c1 - cos_a) * geometry::dot_product(axis, xyz0));
            // v_rot = cos_a * v + sin_a * (n x v) + (1 - cos_a)(n.v) * e
            point3d_t v_rot = s1;
            geometry::add_point(v_rot, s2);
            geometry::add_point(v_rot, s3);
            
            rng_point_t p = formula::cart3d_to_sph<rng_point_t>(v_rot);
            geometry::detail::conversion::point_to_point
                <
                    Point, rng_point_t,
                    2, dimension<rng_point_t>::value
                >::apply(p0, p);

            range::push_back(rng, p);
        }
    }

private:
    typename geometry::radius_type<Sphere>::type m_radius;
};


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <>
struct default_strategy<spherical_equatorial_tag>
{
    typedef strategy::complexify::spherical<> type;
};


} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::complexify


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_COMPLEXIFY_HPP
