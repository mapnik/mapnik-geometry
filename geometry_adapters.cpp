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
#if 0
    std::cerr << "LineString" << std::endl;
    mapnik::new_geometry::line_string line;
    line.add_coord(100,100);
    line.add_coord(200,100);
    line.add_coord(100,200);
    std::cerr << "Num points: " << boost::geometry::num_points(line) << std::endl;
    std::cerr << "Length: " << boost::geometry::length(line) << std::endl;
    std::cerr << "WKT: " << boost::geometry::wkt(line) << std::endl;
#endif
    std::cerr << "Polygon" << std::endl;

    mapnik::new_geometry::polygon3 poly;
    {
        mapnik::new_geometry::line_string external_ring;
        external_ring.add_coord(0,0);
        external_ring.add_coord(100,0);
        external_ring.add_coord(100,100);
        external_ring.add_coord(0,100);
        external_ring.add_coord(0,0);
        poly.set_exterior_ring(std::move(external_ring));
        //
        mapnik::new_geometry::line_string hole;
        hole.add_coord(50,50);
        hole.add_coord(75,50);
        hole.add_coord(75,75);
        hole.add_coord(50,75);
        //hole.add_coord(75,50); // !!
        hole.add_coord(50,50);
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
    using polygon_list = std::deque<mapnik::new_geometry::polygon3>;
    mapnik::new_geometry::bounding_box clip_box(100,100,175,175);
    mapnik::new_geometry::polygon3 input_poly;
    boost::geometry::read_wkt("POLYGON((50 250, 400 250, 150 50, 50 250))", input_poly);
    polygon_list clipped_polygons;
    try
    {
        boost::geometry::intersection(clip_box, input_poly, clipped_polygons);
        for (auto const& p : clipped_polygons)
        {
            std::cerr << boost::geometry::wkt(p) << std::endl;
        }
    }
    catch (boost::geometry::exception const& ex)
    {
        std::cerr << ex.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
