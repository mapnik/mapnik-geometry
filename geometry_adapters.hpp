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
//BOOST_GEOMETRY_REGISTER_LINESTRING(mapnik::new_geometry::line_string)
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

template<> struct tag<mapnik::new_geometry::bounding_box> { using type = box_tag; };

template<> struct point_type<mapnik::new_geometry::bounding_box> { using type = mapnik::new_geometry::point; };


template <>
struct indexed_access<mapnik::new_geometry::bounding_box, min_corner, 0>
{
    static inline double get(mapnik::new_geometry::bounding_box const& b) { return b.p0.x;}
    static inline void set(mapnik::new_geometry::bounding_box& b, double value) { b.p0.x = value; }
};

template <>
struct indexed_access<mapnik::new_geometry::bounding_box, min_corner, 1>
{
    static inline double get(mapnik::new_geometry::bounding_box const& b) { return b.p0.y;}
    static inline void set(mapnik::new_geometry::bounding_box& b, double value) { b.p0.y = value; }
};

template <>
struct indexed_access<mapnik::new_geometry::bounding_box, max_corner, 0>
{
    static inline double get(mapnik::new_geometry::bounding_box const& b) { return b.p1.x;}
    static inline void set(mapnik::new_geometry::bounding_box& b, double value) { b.p1.x = value; }
};

template <>
struct indexed_access<mapnik::new_geometry::bounding_box, max_corner, 1>
{
    static inline double get(mapnik::new_geometry::bounding_box const& b) { return b.p1.y;}
    static inline void set(mapnik::new_geometry::bounding_box& b, double value) { b.p1.y = value; }
};

template<>
struct tag<mapnik::new_geometry::line_string>
{
    using type = ring_tag;
};

// polygon 2
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


// mapnik::new_geometry::polygon3

template<> struct tag<mapnik::new_geometry::polygon3>
{
    using type = polygon_tag;
};

// ring
template<> struct ring_const_type<mapnik::new_geometry::polygon3>
{
    using type =  mapnik::new_geometry::line_string const&;
};

template<> struct ring_mutable_type<mapnik::new_geometry::polygon3>
{
    using type = mapnik::new_geometry::line_string&;
};

// interior
template<> struct interior_const_type<mapnik::new_geometry::polygon3>
{
    using type = std::vector<mapnik::new_geometry::line_string> const&;
};

template<> struct interior_mutable_type<mapnik::new_geometry::polygon3>
{
    using type = std::vector<mapnik::new_geometry::line_string>&;
};

// exterior
template<>
struct exterior_ring<mapnik::new_geometry::polygon3>
{
    static mapnik::new_geometry::line_string& get(mapnik::new_geometry::polygon3 & p)
    {
        return p.exterior_ring;
    }

    static mapnik::new_geometry::line_string const& get(mapnik::new_geometry::polygon3 const& p)
    {
        return p.exterior_ring;
    }
};

template<>
struct interior_rings<mapnik::new_geometry::polygon3>
{
    //using ring_iterator = std::vector<mapnik::new_geometry::line_string::cont_type>::iterator;
    //using const_ring_iterator = std::vector<mapnik::new_geometry::line_string::cont_type>::const_iterator;
    //using holes_type = boost::iterator_range<ring_iterator>;
    //using const_holes_type = boost::iterator_range<const_ring_iterator>;
    using holes_type = std::vector<mapnik::new_geometry::line_string>;
    static holes_type&  get(mapnik::new_geometry::polygon3 & p)
    {
        return p.interior_rings;//boost::make_iterator_range(p.interior_rings.begin(), p.interior_rings.end());
    }

    static holes_type const& get(mapnik::new_geometry::polygon3 const& p)
    {
        return p.interior_rings;//boost::make_iterator_range(p.interior_rings.begin(), p.interior_rings.end());
    }
};



}}}


#endif //MAPNIK_GEOMETRY_ADAPTERS_HPP
