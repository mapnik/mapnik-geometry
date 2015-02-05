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
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>

#include "geometry_impl.hpp"
#include "geometry_adapters.hpp"

int main(int argc, char ** argv)
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

    mapnik::new_geometry::polygon2 poly;
    {
        mapnik::new_geometry::line_string external_ring;
        external_ring.add_coord(0,0);
        external_ring.add_coord(100,0);
        external_ring.add_coord(100,100);
        external_ring.add_coord(0,100);
        external_ring.add_coord(0,0);
        poly.add_ring(std::move(external_ring));
        //
        mapnik::new_geometry::line_string hole;
        hole.add_coord(50,50);
        hole.add_coord(75,50);
        hole.add_coord(75,75);
        hole.add_coord(50,75);
        //hole.add_coord(75,50); // !!
        hole.add_coord(50,50);
        poly.add_ring(std::move(hole));
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


    return EXIT_SUCCESS;
}
