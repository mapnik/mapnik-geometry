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
#include <fstream>
#include <vector>
#include <cassert>

#include <boost/timer/timer.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/box.hpp>
#include "geometry_impl.hpp"
#include "geometry_adapters.hpp"


namespace boost { namespace geometry {

template <>
struct point_type<mapnik::util::variant<mapnik::new_geometry::polygon3, mapnik::new_geometry::multi_polygon> >
{
    using type = mapnik::new_geometry::point;
};


} // namespace geometry
} // namespace boost

template <typename Geometry, typename Box>
inline void read_wkt(std::string const& filename, std::vector<Geometry>& geometries, Box& box)
{
    std::ifstream file(filename.c_str());
    if (file.is_open())
    {
        while (! file.eof() )
        {
            std::string line;
            std::getline(file, line);
            if (! line.empty())
            {
                mapnik::new_geometry::polygon3 poly;
                try
                {
                    boost::geometry::read_wkt(line, poly);
                    geometries.push_back(poly);
                    boost::geometry::expand(box, boost::geometry::return_envelope<Box>(poly));
                }
                catch (boost::geometry::exception const& ex)
                {
                    mapnik::new_geometry::multi_polygon multi_poly;
                    try
                    {
                        boost::geometry::read_wkt(line, multi_poly);
                        geometries.push_back(multi_poly);
                        boost::geometry::expand(box, boost::geometry::return_envelope<Box>(multi_poly));
                    }
                    catch (boost::geometry::exception const& ex)
                    {
                        std::cerr << ex.what() << std::endl;
                    }
                }
            }
        }
    }
}

template <typename Box, typename PolygonList>
struct intersection
{
    intersection (Box const& clip_box, PolygonList & clipped_polygons)
        : clip_box_(clip_box), clipped_polygons_(clipped_polygons) {}

    template <typename Geometry>
    void apply(Geometry const& geom)
    {
        mapnik::util::apply_visitor(*this, geom);
    }

    void operator() (mapnik::new_geometry::polygon3 const& poly)
    {
        boost::geometry::intersection(clip_box_, poly, clipped_polygons_);
    }

    void operator() (mapnik::new_geometry::multi_polygon const& multi_poly)
    {
        for (auto const& poly : multi_poly)
        {
            //std::cerr << boost::geometry::wkt(poly) << std::endl;
            boost::geometry::intersection(clip_box_, poly, clipped_polygons_);
        }
    }

    template <typename T>
    void operator() (T const& g)
    {
        std::cerr << typeid(g).name() << std::endl;
    }
    Box const& clip_box_;
    PolygonList & clipped_polygons_;
};

int main(int argc, char ** argv)
{
    using polygon_list = std::vector<mapnik::new_geometry::polygon3>;
    using geometry = mapnik::util::variant<mapnik::new_geometry::point,
                                           mapnik::new_geometry::line_string,
                                           mapnik::new_geometry::polygon3,
                                           mapnik::new_geometry::multi_polygon>;

    std::cerr << "Clipping test" << std::endl;
    std::cerr << "Boost.geometry" << std::endl;

    if (argc != 4)
    {
        std::cerr << "Usage:" << argv[0] << " <wkt-filename> <bbox-wkt> <num-iterations>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string wkt_filename(argv[1]);
    std::string bbox_wkt(argv[2]);
    std::size_t num_iterations = std::stol(argv[3]);
    std::cerr << "NUM_ITERATIONS=" << num_iterations << std::endl;
    mapnik::new_geometry::bounding_box clip_box;
    boost::geometry::read_wkt(bbox_wkt, clip_box);
    std::vector<geometry> geometries;
    mapnik::new_geometry::bounding_box bbox;
    read_wkt(wkt_filename, geometries , bbox);
    boost::timer::auto_cpu_timer t;

    bool valid_output = true;
    for (std::size_t i = 0; i < num_iterations ; ++i)
    {
        if (i == 0)
        {
            std::cerr << "NUM GEOMETRIES = " << geometries.size() << std::endl;
        }
        std::size_t output_size = 0;
        for (auto geom : geometries)
        {

            //std::cerr << "Area: " << boost::geometry::area(geom) << std::endl;
            //std::cerr << "WKT A: " << boost::geometry::wkt(geom) << std::endl;
            //std::cerr << "WKT B: " << boost::geometry::wkt(clip_box) << std::endl;
            //std::cerr << "Is valid? :" << std::boolalpha << boost::geometry::is_valid(geom) << std::endl;
            //std::cerr << "Is simple? :" << std::boolalpha << boost::geometry::is_simple(geom) << std::endl;

            {

                //std::size_t count = 0;
                polygon_list clipped_polygons;
                try
                {
                    intersection<mapnik::new_geometry::bounding_box, polygon_list> op(clip_box, clipped_polygons);
                    op.apply(geom);
                    output_size += clipped_polygons.size();
                    for (auto const& p : clipped_polygons)
                    {
                        if (i == 0)
                        {
                            valid_output = boost::geometry::is_valid(p);
                            std::cout << boost::geometry::wkt(p) << std::endl;
                        }
                    }
                }
                catch (boost::geometry::exception const& ex)
                {
                    std::cerr << ex.what() << std::endl;
                }
            }
        }
        std::cerr << "OUPUT SIZE=" << output_size << std::endl;
    }
    std::cerr << "VALID OUTPUT : " << std::boolalpha << valid_output << std::endl;
    return EXIT_SUCCESS;
}
