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

#ifndef MAPNIK_GEOMETRY_ADAPTERS_HPP
#define MAPNIK_GEOMETRY_ADAPTERS_HPP

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/linestring.hpp>
#include <boost/geometry/geometries/register/ring.hpp>
#include <boost/range.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/geometry/core/mutable_range.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

// register point
BOOST_GEOMETRY_REGISTER_POINT_2D (mapnik::new_geometry::point, double, cs::cartesian, x, y)

// register linestring
BOOST_GEOMETRY_REGISTER_LINESTRING(mapnik::new_geometry::line_string)
// ring
BOOST_GEOMETRY_REGISTER_RING(mapnik::new_geometry::line_string::cont_type)

namespace boost {

template <>
struct range_iterator<mapnik::new_geometry::line_string>
{
    using type = mapnik::new_geometry::line_string::iterator_type;
};

template <>
struct range_const_iterator<mapnik::new_geometry::line_string>
{
    using type = mapnik::new_geometry::line_string::const_iterator_type;
};

inline mapnik::new_geometry::line_string::iterator_type
range_begin(mapnik::new_geometry::line_string & line) {return line.begin();}

inline mapnik::new_geometry::line_string::iterator_type
range_end(mapnik::new_geometry::line_string & line) {return line.end();}

inline mapnik::new_geometry::line_string::const_iterator_type
range_begin(mapnik::new_geometry::line_string const& line) {return line.begin();}

inline mapnik::new_geometry::line_string::const_iterator_type
range_end(mapnik::new_geometry::line_string const& line) {return line.end();}


// register polygon
namespace geometry { namespace traits {

//template<>
//struct tag<mapnik::new_geometry::line_string>
//{
//    using type = ring_tag;
//};

template<> struct tag<mapnik::new_geometry::polygon2>
{
    using type = polygon_tag;
};

// ring
template<> struct ring_const_type<mapnik::new_geometry::polygon2>
{
    using type =  mapnik::new_geometry::line_string::cont_type const&;
};

template<> struct ring_mutable_type<mapnik::new_geometry::polygon2>
{
    using type = mapnik::new_geometry::line_string::cont_type&;
};

// interior
template<> struct interior_const_type<mapnik::new_geometry::polygon2>
{
    using rings_type = std::vector<mapnik::new_geometry::line_string::cont_type>;
    using type = boost::iterator_range<rings_type::const_iterator> const;
};

template<> struct interior_mutable_type<mapnik::new_geometry::polygon2>
{
    using rings_type = std::vector<mapnik::new_geometry::line_string::cont_type>;
    using type = boost::iterator_range<rings_type::iterator>;
};

// exterior
template<>
struct exterior_ring<mapnik::new_geometry::polygon2>
{
    static mapnik::new_geometry::line_string::cont_type& get(mapnik::new_geometry::polygon2 & p)
    {
        return p.rings.front();
    }

    static mapnik::new_geometry::line_string::cont_type const& get(mapnik::new_geometry::polygon2 const& p)
    {
        return p.rings.front();
    }
};

template<>
struct interior_rings<mapnik::new_geometry::polygon2>
{
    using ring_iterator = std::vector<mapnik::new_geometry::line_string::cont_type>::iterator;
    using const_ring_iterator = std::vector<mapnik::new_geometry::line_string::cont_type>::const_iterator;
    using holes_type = boost::iterator_range<ring_iterator>;
    using const_holes_type = boost::iterator_range<const_ring_iterator>;
    static holes_type get(mapnik::new_geometry::polygon2 & p)
    {
        return boost::make_iterator_range(p.rings.begin() + 1, p.rings.end());
    }

    static const_holes_type get(mapnik::new_geometry::polygon2 const& p)
    {
       return boost::make_iterator_range(p.rings.begin() + 1, p.rings.end());
    }
};

}}}


#endif //MAPNIK_GEOMETRY_ADAPTERS_HPP
