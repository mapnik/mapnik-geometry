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

#include <mapnik/geometry.hpp>
#include <mapnik/geometry_container.hpp>

#include <boost/timer/timer.hpp>

#include "geometry_impl.hpp"

//#include <mapnik/util/geometry_to_geojson.hpp>

#include <mapnik/json/geometry_generator_grammar.hpp>
#include <mapnik/json/geometry_generator_grammar_impl.hpp>
#include <boost/spirit/include/support_container.hpp>


namespace boost { namespace spirit { namespace traits {

template <>
struct is_container<mapnik::new_geometry::vertex_adapter const> : mpl::true_ {} ;

template <>
struct container_iterator<mapnik::new_geometry::vertex_adapter const>
{
    using type = mapnik::util::path_iterator<mapnik::new_geometry::vertex_adapter>;
};

template <>
struct begin_container<mapnik::new_geometry::vertex_adapter const>
{
    static mapnik::util::path_iterator<mapnik::new_geometry::vertex_adapter>
    call (mapnik::new_geometry::vertex_adapter const& g)
    {
        return mapnik::util::path_iterator<mapnik::new_geometry::vertex_adapter>(g);
    }
};

template <>
struct end_container<mapnik::new_geometry::vertex_adapter const>
{
    static mapnik::util::path_iterator<mapnik::new_geometry::vertex_adapter>
    call (mapnik::new_geometry::vertex_adapter const&)
    {
        return mapnik::util::path_iterator<mapnik::new_geometry::vertex_adapter>();
    }
};


}}}



namespace mapnik  {

template <typename Geometry>
bool to_geojson_1(std::string & json, Geometry const& geom)
{
    using sink_type = std::back_insert_iterator<std::string>;
    static const mapnik::json::geometry_generator_grammar<sink_type, Geometry> g;
    sink_type sink(json);
    return boost::spirit::karma::generate(sink, g, geom);
}

template <typename GeometryContainer>
bool to_geojson_2(std::string & json, GeometryContainer const& geom_cont)
{
    using sink_type = std::back_insert_iterator<std::string>;
    static const mapnik::json::multi_geometry_generator_grammar<sink_type, GeometryContainer> g;
    sink_type sink(json);
    return boost::spirit::karma::generate(sink, g, geom_cont);
}


namespace json { namespace detail {

// adapt mapnik::vertex_adapter get_first
template <>
struct get_first<mapnik::vertex_adapter>
{
    using geometry_type =  mapnik::vertex_adapter;
    using result_type = typename geometry_type::value_type;
    result_type operator() (geometry_type const& geom) const
    {
        result_type coord;
        geom.rewind(0);
        std::get<0>(coord) = geom.vertex(&std::get<1>(coord),&std::get<2>(coord));
        return coord;
    }
};

// adapt new geometry get_first
template <>
struct get_first<mapnik::new_geometry::vertex_adapter>
{
    using geometry_type =  mapnik::new_geometry::vertex_adapter;
    using result_type = typename geometry_type::value_type;
    result_type operator() (geometry_type const& geom) const
    {
        result_type coord;
        geom.rewind(0);
        std::get<0>(coord) = geom.vertex(&std::get<1>(coord),&std::get<2>(coord));
        return coord;
    }
};
} // namespace detail
} // namespace json
} // namespace mapnik


int main(int, char **)
{

#if 1
    {
        std::cerr << "mapnik::geometry type " << std::endl;
        mapnik::geometry_container geom_cont;
        for (int n = 0; n < 1; ++n)
        {
            std::unique_ptr<mapnik::geometry_type> poly(
                new mapnik::geometry_type(mapnik::geometry_type::types::Polygon));
            for (int j =0 ; j < 2;++j)
            {
                for (size_t i=0; i < 10;++i)
                {
                    double x = i;
                    double y = 10 - i;
                    if (i==0)
                        poly->move_to(x,y);
                    else
                        poly->line_to(x,y);

                }
                poly->line_to(0,10);// FIXME
                poly->close_path();
            }
            geom_cont.push_back(poly.release());
        }
        for (auto const& geom : geom_cont)
        {
            std::string json;
            mapnik::vertex_adapter va(geom);
            mapnik::to_geojson_1(json, va);
            std::cerr << json << std::endl;
        }
    }
#endif

#if 1
    {
        std::cerr << "mapnik::new_geometry type " << std::endl;
        std::vector<mapnik::new_geometry::geometry> geom_cont;
        for (int n = 0; n < 1; ++n)
        {
            mapnik::new_geometry::polygon2 poly;

            for (int j =0 ; j < 2;++j)
            {
                mapnik::new_geometry::line_string ring;
                ring.reserve(10);
                for (size_t i=0; i < 10;++i)
                {
                    double x = i;
                    double y = 10 - i;
                    ring.add_coord(x, y);
                }
                // close ring
                ring.add_coord(0, 10);
                poly.add_ring(std::move(ring));
            }
            geom_cont.push_back(mapnik::new_geometry::geometry(std::move(poly)));
        }
        mapnik::new_geometry::point pt(100,200);
        geom_cont.push_back(mapnik::new_geometry::geometry(std::move(pt)));
        for (auto const& geom : geom_cont)
        {
            std::string json;
            mapnik::new_geometry::vertex_adapter v_adapter(geom);
            mapnik::to_geojson_1(json, v_adapter);
            std::cerr << json << std::endl;
        }
    }
#endif

    return EXIT_SUCCESS;
}
