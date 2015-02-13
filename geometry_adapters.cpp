/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#include <iostream>
#include <vector>
#include <cassert>

#include <boost/timer/timer.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#pragma GCC diagnostic pop

#include "geometry_impl.hpp"
#include "geometry_adapters.hpp"

int main(int, char **)
{

    std::cerr << "Boost.Geometry adapters" << std::endl;

    std::cerr << "LineString" << std::endl;
    mapnik::new_geometry::line_string line;
    line.add_coord(100,100);
    line.add_coord(200,100);
    line.add_coord(100,200);
    std::cerr << "Num points: " << boost::geometry::num_points(line) << std::endl;
    std::cerr << "Length: " << boost::geometry::length(line) << std::endl;
    std::cerr << "WKT: " << boost::geometry::wkt(line) << std::endl;

    std::cerr << "Polygon" << std::endl;
    mapnik::new_geometry::polygon3 poly;
    {
        mapnik::new_geometry::linear_ring external_ring;
        external_ring.emplace_back(0,0);
        external_ring.emplace_back(100,0);
        external_ring.emplace_back(100,100);
        external_ring.emplace_back(0,100);
        external_ring.emplace_back(0,0);
        poly.set_exterior_ring(std::move(external_ring));
        //
        mapnik::new_geometry::linear_ring hole;
        hole.emplace_back(50,50);
        hole.emplace_back(75,50);
        hole.emplace_back(75,75);
        hole.emplace_back(50,75);
        //hole.emplace_back(75,50); // !!
        hole.emplace_back(50,50);
        poly.add_hole(std::move(hole));
    }

    std::cerr << "Num points: " << boost::geometry::num_points(poly) << std::endl;
    std::cerr << "Area (before): " << boost::geometry::area(poly) << std::endl;
    std::cerr << "WKT (before): " << boost::geometry::wkt(poly) << std::endl;
    std::cerr << "Is valid? :" << std::boolalpha << boost::geometry::is_valid(poly) << std::endl;
    std::cerr << "Is simple? :" << std::boolalpha << boost::geometry::is_simple(poly) << std::endl;
    std::cerr << "========== Reverse" << std::endl;
    boost::geometry::reverse(poly);
    std::cerr << "Area (after): " << boost::geometry::area(poly) << std::endl;
    std::cerr << "WKT (after): " << boost::geometry::wkt(poly) << std::endl;
    std::cerr << "Is valid? :" << std::boolalpha << boost::geometry::is_valid(poly) << std::endl;
    std::cerr << "Is simple? :" << std::boolalpha << boost::geometry::is_simple(poly) << std::endl;
    std::cerr << "========== Correct" << std::endl;
    boost::geometry::correct(poly);
    std::cerr << "Area (after): " << boost::geometry::area(poly) << std::endl;
    std::cerr << "WKT (after): " << boost::geometry::wkt(poly) << std::endl;
    std::cerr << "Is valid? :" << std::boolalpha << boost::geometry::is_valid(poly) << std::endl;
    std::cerr << "Is simple? :" << std::boolalpha << boost::geometry::is_simple(poly) << std::endl;
    boost::geometry::model::box<mapnik::new_geometry::point> box;
    boost::geometry::envelope(poly, box);
    std::cerr << "========== envelope:" << boost::geometry::dsv(box) << std::endl;

    std::cerr << "========== clipping" << std::endl;
    using polygon_list = std::vector<mapnik::new_geometry::polygon3>;

//POLYGON ((174.59563876651984 378.20612334801757, 174.59563876651984 456.0192511013215, 627.326563876652 456.0192511013215, 627.326563876652 378.20612334801757, 174.59563876651984 378.20612334801757))
    mapnik::new_geometry::bounding_box clip_box(174.59563876651984,378.20612334801757,627.326563876652,456.0192511013215);
    mapnik::new_geometry::polygon3 input_poly;
    boost::geometry::read_wkt("POLYGON ((155 203, 233 454, 315 340, 421 446, 463 324, 559 466, 665 253, 528 178, 394 229, 329 138, 212 134, 183 228, 200 264, 155 203),(313 190, 440 256, 470 248, 510 305, 533 237, 613 263, 553 397, 455 262, 405 378, 343 287, 249 334, 229 191, 313 190))", input_poly);

    std::cerr << "Area: " << boost::geometry::area(input_poly) << std::endl;
    std::cerr << "WKT: " << boost::geometry::wkt(input_poly) << std::endl;
    std::cerr << "Is valid? :" << std::boolalpha << boost::geometry::is_valid(input_poly) << std::endl;
    std::cerr << "Is simple? :" << std::boolalpha << boost::geometry::is_simple(input_poly) << std::endl;
    {
        boost::timer::auto_cpu_timer t;
        std::size_t count = 0;
        for (std::size_t i = 0; i < 10000 ; ++i)
        {
            polygon_list clipped_polygons;
            try
            {
                boost::geometry::intersection(clip_box, input_poly, clipped_polygons);
                for (auto const& p : clipped_polygons)
                {
                    if (i == 0)
                    {
                        std::cerr << boost::geometry::wkt(p) << std::endl;
                    }
                    count += p.exterior_ring.size();
                    for (auto const& ring :  p.interior_rings)
                    {
                        count += ring.size();
                    }
                }
            }
            catch (boost::geometry::exception const& ex)
            {
                std::cerr << ex.what() << std::endl;
            }
        }
        std::cerr << "count=" << count << std::endl;
    }
    return EXIT_SUCCESS;
}
