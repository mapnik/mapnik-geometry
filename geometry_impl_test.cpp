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
#include <cstdint>
#include <vector>
#include <cassert>

#include <mapnik/util/variant.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_container.hpp>
#include <boost/timer/timer.hpp>

#include "geometry_impl.hpp"

int main(int argc, char ** argv)
{
    std::cerr << "sizeof(mapnik::geometry_type)=" << sizeof(mapnik::geometry_type) << std::endl;
    std::cerr << "sizeof(mapnik::new_geometry::geometry)=" << sizeof(mapnik::new_geometry::geometry) << std::endl;
    std::cerr << "sizeof(mapnik::new_geometry::point)="<< sizeof(mapnik::new_geometry::point) << std::endl;
    std::cerr << "sizeof(mapnik::new_geometry::line_string)="<< sizeof(mapnik::new_geometry::line_string) << std::endl;
    std::cerr << "sizeof(mapnik::new_geometry::polygon)="<< sizeof(mapnik::new_geometry::polygon) << std::endl;
    std::cerr << "sizeof(mapnik::new_geometry::polygon2)="<< sizeof(mapnik::new_geometry::polygon2) << std::endl;
    std::cerr << "sizeof(mapnik::new_geometry::polygon_vertex_adapter)="<< sizeof(mapnik::new_geometry::polygon_vertex_adapter) << std::endl;
    std::cerr << "sizeof(mapnik::new_geometry::polygon_vertex_adapter_2)="<< sizeof(mapnik::new_geometry::polygon_vertex_adapter_2) << std::endl;
    std::cerr << "sizeof(mapnik::new_geometry::vertex_adapter)="<< sizeof(mapnik::new_geometry::vertex_adapter) << std::endl;

    if (argc != 4)
    {
        std::cerr << "Usage:" << argv[0] << " <num-geom> <num-rings> <num-points>" << std::endl;
        return 1;
    }

    const int NUM_GEOM=std::stol(argv[1]);
    const int NUM_RINGS=std::stol(argv[2]);
    const int NUM_POINTS=std::stol(argv[3]);


    for (int i=0; i < 3 ; ++i)
    {

#ifdef  MAPNIK_GEOMETRY_POLYGON
        {
            std::cerr << "mapnik::geometry type " << std::endl;
            boost::timer::auto_cpu_timer t;
            mapnik::geometry_container geom_cont;

            for (int n = 0; n < NUM_GEOM; ++n)
            {
                std::unique_ptr<mapnik::geometry_type> poly(
                    new mapnik::geometry_type(mapnik::geometry_type::types::Polygon));
                for (int j =0 ; j < NUM_RINGS;++j)
                {
                    for (size_t i=0; i < NUM_POINTS;++i)
                    {
                        double x = i;
                        double y = NUM_POINTS-i;
                        if (i==0)
                            poly->move_to(x,y);
                        else
                            poly->line_to(x,y);

                    }
                    poly->close_path();
                }
                geom_cont.push_back(poly.release());
            }

            long count = 0;
            for (auto const& poly : geom_cont)
            {
                poly.rewind(0);
                for (;;)
                {
                    double x,y;
                    unsigned cmd = poly.vertex(&x,&y);
                    if (cmd == mapnik::SEG_END) break;
                    ++count;
                }
            }
            std::cerr << "--------count = " << count << std::endl;
        }
#endif

#ifdef NEW_GEOMETRY_POLYGON
        {
            std::cerr << "==============  new geometry polygon" << std::endl;
            boost::timer::auto_cpu_timer t;
            std::vector<mapnik::new_geometry::geometry> geom_cont;

            for (int n = 0; n < NUM_GEOM; ++n)
            {
                mapnik::new_geometry::polygon poly;

                for (int j =0 ; j < NUM_RINGS;++j)
                {
                    mapnik::new_geometry::line_string ring;
                    ring.reserve(NUM_POINTS);
                    for (size_t i=0; i < NUM_POINTS;++i)
                    {
                        double x = i;
                        double y = NUM_POINTS-i;
                        ring.add_coord(x, y);
                    }
                    poly.add_ring(std::move(ring));
                }
                geom_cont.emplace_back(std::move(poly));
            }
            long count = 0;
            for (auto const& geom : geom_cont)
            {
                mapnik::new_geometry::vertex_adapter v_adapter(geom);
                v_adapter.rewind(0);
                for  (;;)
                {
                    double x,y;
                    unsigned cmd = v_adapter.vertex(&x, &y);
                    if (cmd == mapnik::SEG_END) break;
                    ++count;
                }
            }
            std::cerr << "new geom count=" << count << std::endl;
        }
#endif

#ifdef NEW_GEOMETRY_POLYGON_2
        {
            std::cerr << "==============  new geometry polygon2" << std::endl;
            boost::timer::auto_cpu_timer t;
            std::vector<mapnik::new_geometry::geometry> geom_cont;

            for (int n = 0; n < NUM_GEOM; ++n)
            {
                mapnik::new_geometry::polygon2 poly;

                for (int j =0 ; j < NUM_RINGS;++j)
                {
                    mapnik::new_geometry::line_string ring;
                    ring.reserve(NUM_POINTS);
                    for (size_t i=0; i < NUM_POINTS;++i)
                    {
                        double x = i;
                        double y = NUM_POINTS-i;
                        ring.add_coord(x, y);
                    }
                    poly.add_ring(std::move(ring));
                }
                geom_cont.emplace_back(std::move(poly));
            }
            long count = 0;
            for (auto const& geom : geom_cont)
            {
                mapnik::new_geometry::vertex_adapter v_adapter(geom);
                v_adapter.rewind(0);
                for  (;;)
                {
                    double x,y;
                    unsigned cmd = v_adapter.vertex(&x, &y);
                    if (cmd == mapnik::SEG_END) break;
                    ++count;
                }
            }
            std::cerr << "new geom count=" << count << std::endl;
        }
#endif
    }
    return EXIT_SUCCESS;
}
