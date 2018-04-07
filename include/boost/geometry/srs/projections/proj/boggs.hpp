// Boost.Geometry - gis-projections (based on PROJ4)

// Copyright (c) 2008-2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017, 2018.
// Modifications copyright (c) 2017-2018, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This file is converted from PROJ4, http://trac.osgeo.org/proj
// PROJ4 is originally written by Gerald Evenden (then of the USGS)
// PROJ4 is maintained by Frank Warmerdam
// PROJ4 is converted to Boost.Geometry by Barend Gehrels

// Last updated version of proj: 5.0.0

// Original copyright notice:

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef BOOST_GEOMETRY_PROJECTIONS_BOGGS_HPP
#define BOOST_GEOMETRY_PROJECTIONS_BOGGS_HPP

#include <boost/geometry/util/math.hpp>

#include <boost/geometry/srs/projections/impl/base_static.hpp>
#include <boost/geometry/srs/projections/impl/base_dynamic.hpp>
#include <boost/geometry/srs/projections/impl/projects.hpp>
#include <boost/geometry/srs/projections/impl/factory_entry.hpp>

namespace boost { namespace geometry
{

namespace srs { namespace par4
{
    struct boggs {};

}} //namespace srs::par4

namespace projections
{
    #ifndef DOXYGEN_NO_DETAIL
    namespace detail { namespace boggs
    {

            static const int NITER = 20;
            static const double EPS = 1e-7;
            static const double FXC = 2.00276;
            static const double FXC2 = 1.11072;
            static const double FYC = 0.49931;

            // template class, using CRTP to implement forward/inverse
            template <typename CalculationType, typename Parameters>
            struct base_boggs_spheroid : public base_t_f<base_boggs_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>
            {

                typedef CalculationType geographic_type;
                typedef CalculationType cartesian_type;


                inline base_boggs_spheroid(const Parameters& par)
                    : base_t_f<base_boggs_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>(*this, par) {}

                // FORWARD(s_forward)  spheroid
                // Project coordinates from geographic (lon, lat) to cartesian (x, y)
                inline void fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y) const
                {
                    static const CalculationType HALFPI = detail::HALFPI<CalculationType>();
                    static const CalculationType ONEPI = detail::ONEPI<CalculationType>();
                    static const CalculationType SQRT2 = boost::math::constants::root_two<CalculationType>();

                    CalculationType theta, th1, c;
                    int i;

                    theta = lp_lat;
                    if (fabs(fabs(lp_lat) - HALFPI) < EPS)
                        xy_x = 0.;
                    else {
                        c = sin(theta) * ONEPI;
                        for (i = NITER; i; --i) {
                            theta -= th1 = (theta + sin(theta) - c) /
                                (1. + cos(theta));
                            if (fabs(th1) < EPS) break;
                        }
                        theta *= 0.5;
                        xy_x = FXC * lp_lon / (1. / cos(lp_lat) + FXC2 / cos(theta));
                    }
                    xy_y = FYC * (lp_lat + SQRT2 * sin(theta));
                }

                static inline std::string get_name()
                {
                    return "boggs_spheroid";
                }

            };

            // Boggs Eumorphic
            template <typename Parameters>
            inline void setup_boggs(Parameters& par)
            {
                par.es = 0.;
            }

    }} // namespace detail::boggs
    #endif // doxygen

    /*!
        \brief Boggs Eumorphic projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Pseudocylindrical
         - no inverse
         - Spheroid
        \par Example
        \image html ex_boggs.gif
    */
    template <typename CalculationType, typename Parameters>
    struct boggs_spheroid : public detail::boggs::base_boggs_spheroid<CalculationType, Parameters>
    {
        inline boggs_spheroid(const Parameters& par) : detail::boggs::base_boggs_spheroid<CalculationType, Parameters>(par)
        {
            detail::boggs::setup_boggs(this->m_par);
        }
    };

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {

        // Static projection
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(srs::par4::boggs, boggs_spheroid, boggs_spheroid)

        // Factory entry(s)
        template <typename CalculationType, typename Parameters>
        class boggs_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_f<boggs_spheroid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        inline void boggs_init(detail::base_factory<CalculationType, Parameters>& factory)
        {
            factory.add_to_factory("boggs", new boggs_entry<CalculationType, Parameters>);
        }

    } // namespace detail
    #endif // doxygen

} // namespace projections

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_PROJECTIONS_BOGGS_HPP

