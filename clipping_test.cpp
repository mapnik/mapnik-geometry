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
    using polygon_list = std::vector<mapnik::new_geometry::polygon3>;

    std::cerr << "Clipping test" << std::endl;
    std::cerr << "Boost.geometry" << std::endl;

    if (argc != 4)
    {
        std::cerr << "Usage:" << argv[0] << "<polygon-wkt> <bbox-wkt> <num-iterations>" << std::endl;
        return EXIT_FAILURE;
    }
    std::string poly_wkt(argv[1]);
    std::string bbox_wkt(argv[2]);
    std::size_t num_iterations = std::stol(argv[3]);
    std::cerr << "NUM_ITERATIONS=" << num_iterations << std::endl;
    mapnik::new_geometry::bounding_box clip_box;
    mapnik::new_geometry::polygon3 input_poly;

    try
    {
        boost::geometry::read_wkt(poly_wkt, input_poly);
        boost::geometry::read_wkt(bbox_wkt, clip_box);
    }
    catch (boost::geometry::exception const& ex)
    {
        std::cerr << ex.what() << std::endl;
    }

    std::cerr << "Area: " << boost::geometry::area(input_poly) << std::endl;
    std::cerr << "WKT A: " << boost::geometry::wkt(input_poly) << std::endl;
    std::cerr << "WKT B: " << boost::geometry::wkt(clip_box) << std::endl;
    std::cerr << "Is valid? :" << std::boolalpha << boost::geometry::is_valid(input_poly) << std::endl;
    std::cerr << "Is simple? :" << std::boolalpha << boost::geometry::is_simple(input_poly) << std::endl;

    {
        boost::timer::auto_cpu_timer t;
        std::size_t count = 0;
        for (std::size_t i = 0; i < num_iterations ; ++i)
        {
            polygon_list clipped_polygons;
            try
            {
                boost::geometry::intersection(input_poly, clip_box, clipped_polygons);
                for (auto const& p : clipped_polygons)
                {

                    if (i == 0)
                    {
                        //std::cerr << "Is valid? :" << std::boolalpha << boost::geometry::is_valid(p) << std::endl;
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
