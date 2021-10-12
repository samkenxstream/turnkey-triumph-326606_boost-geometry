// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_PT_SEG_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_PT_SEG_HPP


#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>
#include <boost/geometry/strategies/closest_points/services.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace closest_points
{

template
<
    typename CalculationType = void,
    typename Strategy = distance::pythagoras<CalculationType>
>
class projected_point
{
public:
    // The three typedefs below are necessary to calculate distances
    // from segments defined in integer coordinates.

    // Integer coordinates can still result in FP distances.
    // There is a division, which must be represented in FP.
    // So promote.
    template <typename Point, typename PointOfSegment>
    struct calculation_type
        : promote_floating_point
          <
              typename strategy::distance::services::return_type
                  <
                      Strategy,
                      Point,
                      PointOfSegment
                  >::type
          >
    {};

    template <typename Point, typename PointOfSegment>
    inline auto
    apply(Point const& p, PointOfSegment const& p1, PointOfSegment const& p2) const
    {
        assert_dimension_equal<Point, PointOfSegment>();

        using calculation_type = typename calculation_type<Point, PointOfSegment>::type;

        // A projected point of points in Integer coordinates must be able to be
        // represented in FP.
        using fp_point_type = model::point
            <
                calculation_type,
                dimension<PointOfSegment>::value,
                typename coordinate_system<PointOfSegment>::type
            >;

        // For convenience
        using fp_vector_type = fp_point_type;

        /*
            Algorithm [p: (px,py), p1: (x1,y1), p2: (x2,y2)]
            VECTOR v(x2 - x1, y2 - y1)
            VECTOR w(px - x1, py - y1)
            c1 = w . v
            c2 = v . v
            b = c1 / c2
            RETURN POINT(x1 + b * vx, y1 + b * vy)
        */

        // v is multiplied below with a (possibly) FP-value, so should be in FP
        // For consistency we define w also in FP
        fp_vector_type v, w, projected;

        geometry::convert(p2, v);
        geometry::convert(p, w);
        geometry::convert(p1, projected);
        subtract_point(v, projected);
        subtract_point(w, projected);

        Strategy strategy;
        boost::ignore_unused(strategy);

        calculation_type const zero = calculation_type();
        calculation_type const c1 = dot_product(w, v);
        if (c1 <= zero)
        {
            return p1;
        }
        calculation_type const c2 = dot_product(v, v);
        if (c2 <= c1)
        {
            return p2;
        }

        // See above, c1 > 0 AND c2 > c1 so: c2 != 0
        calculation_type const b = c1 / c2;

        multiply_value(v, b);
        add_point(projected, v);

        PointOfSegment projected_point_of_segment;
        geometry::convert(projected, projected_point_of_segment);

        return projected_point_of_segment;
    }

};

}} // namespace strategy::closest_points


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_PT_SEG_HPP
