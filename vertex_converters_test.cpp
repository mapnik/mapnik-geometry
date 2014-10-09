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

#include <mapnik/feature_factory.hpp>
#include <mapnik/json/feature_parser.hpp>
#include <mapnik/vertex_converters.hpp>

#include <mapnik/json/geometry_generator_grammar.hpp>
#include <mapnik/json/geometry_generator_grammar_impl.hpp>
#include <mapnik/json/geometry_grammar_impl.hpp>
#include <mapnik/json/feature_grammar_impl.hpp>
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
        std::cerr << "begin_container" << std::endl;
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

} // namespace mapnik

struct to_json_converter
{
    template <typename T>
    void add_path(T & path)
    {
        std::cerr << "add_path" << std::endl;
    }
};

int main(int argc, char ** argv)
{
    if (argc !=2)
    {
        std::cerr << "Usage:" << argv[0] << " <geojson>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string json(argv[1]);
    mapnik::context_ptr ctx = std::make_shared<mapnik::context_type>();
    mapnik::feature_ptr feature(mapnik::feature_factory::create(ctx,1));
    if (!mapnik::json::from_geojson(json.c_str(),*feature))
    {
        throw std::runtime_error("Failed to parse geojson feature");
    }
    std::cerr << *feature << std::endl;

    for (auto const& geom : feature->paths())
    {
        std::string json_out;
        mapnik::to_geojson_1(json_out, geom);
        std::cerr << json_out << std::endl;
    }

    //using vertex_converter_type = vertex_converter<to_json_converter,smooth_tag>;

    //to_json_converter to_json;
    //vertex_converter_type converter(clip_box, to_json, sym, common.t_, prj_trans, tr,
    //                                feature,common.vars_,common.scale_factor_);

    return EXIT_SUCCESS;
}
