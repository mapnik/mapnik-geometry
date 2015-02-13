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

#include <vector>
#include <mapnik/util/variant.hpp>
#include <mapnik/vertex.hpp>
#include <mapnik/util/noncopyable.hpp>

#include <algorithm>
#include <vector>
#include <tuple>
#include <type_traits>
#include <cstddef>
#include <utility>

namespace mapnik { namespace new_geometry {

static const std::uint8_t geometry_bits = 7;

enum geometry_types : std::uint8_t
{
    Unknown = 0x00,
    Point =   0x01,
    LineString = 0x02,
    Polygon = 0x03,
    PolygonExterior = Polygon,
    PolygonInterior = Polygon | ( 1 << geometry_bits)
};

struct point
{
    point() {}
    point(double x_, double y_)
        : x(x_),y(y_) {}

    point(point const& other)
        : x(other.x),
          y(other.y) {}

    point(point && other) noexcept = default;

    point & operator=(point const& other)
    {
        if (this == &other) return *this;
        point tmp(other);
        std::swap(x, tmp.x);
        std::swap(y, tmp.y);
        return *this;
    }
    double x;
    double y;
};

struct bounding_box
{
    bounding_box() {} // no-init
    bounding_box(double lox, double loy, double hix, double hiy)
        : p0(lox,loy),
          p1(hix,hiy) {}
    point p0;
    point p1;
};

struct vertex_sequence
{
    typedef std::vector<point> cont_type;
    cont_type data;
    void reserve(std::size_t size)
    {
        data.reserve(size);
    }
};

struct line_string : vertex_sequence
{
    using const_iterator_type = cont_type::const_iterator;
    using iterator_type = cont_type::iterator;
    using value_type = cont_type::value_type;
    iterator_type begin() { return data.begin(); }
    iterator_type end() { return data.end(); }
    const_iterator_type begin() const { return data.begin(); }
    const_iterator_type end() const { return data.end(); }
    line_string() = default;
    line_string (line_string && other) = default ;
    line_string& operator=(line_string &&) = default;
    line_string (line_string const& ) = default;
    line_string& operator=(line_string const&) = default;
    inline std::size_t num_points() const { return data.size(); }
    inline void clear() { data.clear();}
    inline void resize(std::size_t new_size) { data.resize(new_size);}
    inline void push_back(value_type const& val) { data.push_back(val);}
    void add_coord(double x, double y)
    {
        data.emplace_back(x,y);
    }
};

struct polygon2
{
    //polygon2(polygon const&) = delete;
    std::vector<line_string::cont_type> rings;

    inline void add_ring(line_string && ring)
    {
        rings.emplace_back(std::move(ring.data));
    }

    inline std::size_t num_rings() const
    {
        return rings.size();
    }
};

using linear_ring = std::vector<point>;

struct polygon3
{
    linear_ring exterior_ring;
    std::vector<linear_ring> interior_rings;

    inline void set_exterior_ring(linear_ring && ring)
    {
        exterior_ring = std::move(ring);
    }

    inline void add_hole(linear_ring && ring)
    {
        interior_rings.emplace_back(std::move(ring));
    }

    inline std::size_t num_rings() const
    {
        return 1 + interior_rings.size();
    }
};

using multi_polygon = std::vector<polygon3>;

struct polygon : vertex_sequence
{
    typedef line_string::cont_type::const_iterator iterator_type;
    std::vector<std::tuple<std::uint32_t, std::uint32_t> > rings;
    // ring's element count. first ring exterior, subsequent rings are interior
    // rings[0] + ..+ rings[rings.size()-1] == data.size()
    polygon() = default;
    polygon (polygon && other) noexcept = default;
    inline void add_ring(line_string && ring)
    {
        std::size_t count = ring.data.size();
        if (count != 0)
        {
            std::size_t start = data.size();
            data.resize(start + ring.data.size());
            std::move_backward(ring.begin(),ring.end(), data.end());
            rings.emplace_back(start,count);
        }
    }

    inline std::size_t num_rings() const
    {
        return rings.size();
    }

    inline std::pair<iterator_type,iterator_type> ring(std::size_t index) const
    {
        if (index < num_rings())
        {
            std::tuple<std::uint32_t,std::uint32_t> const& ring = rings[index];
            return std::make_pair(data.begin() + std::get<0>(ring), data.begin() + std::get<0>(ring) + std::get<1>(ring));
        }
        else
        {
            return std::make_pair(data.end(),data.end());
        }
    }
};

typedef mapnik::util::variant< point,line_string, polygon, polygon2, polygon3> geometry;

struct point_vertex_adapter
{
    point_vertex_adapter(point const& pt)
        : pt_(pt),
          first_(true) {}

    unsigned vertex(double*x, double*y) const
    {
        if (first_)
        {
            *x = pt_.x;
            *y = pt_.y;
            first_ = false;
            return mapnik::SEG_MOVETO;
        }
        return mapnik::SEG_END;
    }

    void rewind(unsigned) const
    {
        first_ = true;
    }
    point const& pt_;
    mutable bool first_;
};

struct line_string_vertex_adapter
{
    line_string_vertex_adapter(line_string const& line)
        : line_(line),
          current_index_(0),
          end_index_(line.data.size()),
          first_(true) {}

    unsigned vertex(double*x, double*y) const
    {
        if (current_index_ != end_index_)
        {
            point const& coord = line_.data[current_index_++];
            *x = coord.x;
            *y = coord.y;
            if (first_)
            {
                first_ = false;
                return mapnik::SEG_MOVETO;
            }
            else
            {
                return mapnik::SEG_LINETO;
            }
        }
        return mapnik::SEG_END;
    }
    void rewind(unsigned) const
    {
        current_index_ = 0;
        first_ = true;
    }
    line_string const& line_;
    mutable std::size_t current_index_;
    const std::size_t end_index_;
    mutable bool first_;

};

struct polygon_vertex_adapter
{
    polygon_vertex_adapter(polygon const& poly)
        : poly_(poly),
          rings_itr_(poly_.rings.begin()),
          rings_end_(poly_.rings.end()),
          current_index_(0),
          end_index_(0),
          start_loop_(false) {}

    void rewind(unsigned) const
    {
        rings_itr_ = poly_.rings.begin();
        current_index_ = 0;
        end_index_ = 0;
        start_loop_ = false;
    }

    unsigned vertex(double*x, double*y) const
    {
        if (current_index_ == end_index_)
        {
            if (rings_itr_ == rings_end_)
                return mapnik::SEG_END;
            current_index_ = std::get<0>(*rings_itr_);
            end_index_ = current_index_ + std::get<1>(*rings_itr_);
            ++rings_itr_;
            start_loop_ = true;
        }
        point const& coord = poly_.data[current_index_++];
        *x = coord.x;
        *y = coord.y;

        if (start_loop_)
        {
            start_loop_= false;
            return mapnik::SEG_MOVETO;
        }
        if (current_index_ == end_index_)
            return mapnik::SEG_CLOSE;
        return mapnik::SEG_LINETO;
    }
private:
    polygon const& poly_;
    mutable std::vector<std::tuple<uint32_t,uint32_t> >::const_iterator rings_itr_;
    mutable std::vector<std::tuple<uint32_t,uint32_t> >::const_iterator rings_end_;
    mutable std::size_t current_index_;
    mutable std::size_t end_index_;
    mutable bool start_loop_;
};

struct polygon_vertex_adapter_2
{
    polygon_vertex_adapter_2(polygon2 const& poly)
        : poly_(poly),
          rings_itr_(0),
          rings_end_(poly_.rings.size()),
          current_index_(0),
          end_index_((rings_itr_ < rings_end_) ? poly_.rings[0].size() : 0),
          start_loop_(true) {}

    void rewind(unsigned) const
    {
        rings_itr_ = 0;
        rings_end_ = poly_.rings.size();
        current_index_ = 0;
        end_index_ = (rings_itr_ < rings_end_) ? poly_.rings[0].size() : 0;
        start_loop_ = true;
    }

    unsigned vertex(double*x, double*y) const
    {
        if (rings_itr_ == rings_end_)
            return mapnik::SEG_END;
        if (current_index_ < end_index_)
        {
            point const& coord = poly_.rings[rings_itr_][current_index_++];
            *x = coord.x;
            *y = coord.y;
            if (start_loop_)
            {
                start_loop_= false;
                return mapnik::SEG_MOVETO;
            }
            return mapnik::SEG_LINETO;
        }
        else if (++rings_itr_ != rings_end_)
        {
            current_index_ = 0;
            end_index_ = poly_.rings[rings_itr_].size();
            point const& coord = poly_.rings[rings_itr_][current_index_++];
            *x = coord.x;
            *y = coord.y;
            return mapnik::SEG_MOVETO;
        }
        return mapnik::SEG_END;
    }
private:
    polygon2 const& poly_;
    mutable std::size_t rings_itr_;
    mutable std::size_t rings_end_;
    mutable std::size_t current_index_;
    mutable std::size_t end_index_;
    mutable bool start_loop_;
};

struct polygon_vertex_adapter_3
{
    polygon_vertex_adapter_3(polygon3 const& poly)
        : poly_(poly),
          rings_itr_(0),
          rings_end_(poly_.interior_rings.size() + 1),
          current_index_(0),
          end_index_((rings_itr_ < rings_end_) ? poly_.exterior_ring.size() : 0),
          start_loop_(true) {}

    void rewind(unsigned) const
    {
        rings_itr_ = 0;
        rings_end_ = poly_.interior_rings.size() + 1;
        current_index_ = 0;
        end_index_ = (rings_itr_ < rings_end_) ? poly_.exterior_ring.size() : 0;
        start_loop_ = true;
    }

    unsigned vertex(double*x, double*y) const
    {
        if (rings_itr_ == rings_end_)
            return mapnik::SEG_END;
        if (current_index_ < end_index_)
        {
            point const& coord = (rings_itr_ == 0) ?
                poly_.exterior_ring[current_index_++] : poly_.interior_rings[rings_itr_- 1][current_index_++];
            *x = coord.x;
            *y = coord.y;
            if (start_loop_)
            {
                start_loop_= false;
                return mapnik::SEG_MOVETO;
            }
            return mapnik::SEG_LINETO;
        }
        else if (++rings_itr_ != rings_end_)
        {
            current_index_ = 0;
            end_index_ = poly_.interior_rings[rings_itr_ - 1].size();
            point const& coord = poly_.interior_rings[rings_itr_ - 1][current_index_++];
            *x = coord.x;
            *y = coord.y;
            return mapnik::SEG_MOVETO;
        }
        return mapnik::SEG_END;
    }
private:
    polygon3 const& poly_;
    mutable std::size_t rings_itr_;
    mutable std::size_t rings_end_;
    mutable std::size_t current_index_;
    mutable std::size_t end_index_;
    mutable bool start_loop_;
};


using vertex_adapter_base =  mapnik::util::variant<point_vertex_adapter,
                                                   line_string_vertex_adapter,
                                                   polygon_vertex_adapter,
                                                   polygon_vertex_adapter_2,
                                                   polygon_vertex_adapter_3>;

struct vertex_adapter
{
    using base_type = vertex_adapter_base;
    using coord_type = double;
    using value_type = std::tuple<unsigned,coord_type,coord_type>;
    using size_type = std::size_t;
    struct create_adapter
    {
        vertex_adapter_base operator() (point const& pt) const
        {
            return point_vertex_adapter(pt);
        }

        vertex_adapter_base operator() (line_string const& line) const
        {
            return line_string_vertex_adapter(line);
        }

        vertex_adapter_base operator() (polygon const& poly) const
        {
            return polygon_vertex_adapter(poly);
        }

        vertex_adapter_base operator() (polygon2 const& poly) const
        {
            return polygon_vertex_adapter_2(poly);
        }
        vertex_adapter_base operator() (polygon3 const& poly) const
        {
            return polygon_vertex_adapter_3(poly);
        }
    };

    struct rewind_dispatch
    {
        template <typename T>
        void operator() (T const& adapter) const
        {
            adapter.rewind(0);
        }
    };

    struct vertex_dispatch
    {
        vertex_dispatch(double &x_, double &y_)
            : x(x_),y(y_) {}

        template <typename T>
        unsigned operator() (T const& adapter) const
        {
            return adapter.vertex(&x, &y);
        }

        double & x;
        double & y;
    };

    struct type_dispatch
    {
        geometry_types operator() (point_vertex_adapter const&) const
        {
            return Point;
        }
        geometry_types operator() (line_string_vertex_adapter const&) const
        {
            return LineString;
        }
        geometry_types operator() (polygon_vertex_adapter const&) const
        {
            return Polygon;
        }
        geometry_types operator() (polygon_vertex_adapter_2 const&) const
        {
            return Polygon;
        }
        geometry_types operator() (polygon_vertex_adapter_3 const&) const
        {
            return Polygon;
        }
    };

    template<typename T>
    vertex_adapter(T && adapter) noexcept
        : base_(std::move(adapter)) {}

    //template<typename T>
    vertex_adapter(geometry const& geom)
        : base_(mapnik::util::apply_visitor(create_adapter(), geom)) {}

    void rewind(unsigned) const
    {
        mapnik::util::apply_visitor(vertex_adapter::rewind_dispatch(), base_);
    }

    unsigned vertex( double *x, double *y) const
    {
        return mapnik::util::apply_visitor(vertex_adapter::vertex_dispatch(*x, *y), base_);
    }

    geometry_types type() const
    {
        return mapnik::util::apply_visitor(vertex_adapter::type_dispatch(), base_);
    }
    base_type base_;
};

struct vertex_adapter_factory
{
    struct dispatch
    {
        vertex_adapter operator() (point const& pt) const
        {
            return point_vertex_adapter(pt);
        }

        vertex_adapter operator() (line_string const& line) const
        {
            return line_string_vertex_adapter(line);
        }

        vertex_adapter operator() (polygon const& poly) const
        {
            return polygon_vertex_adapter(poly);
        }

        vertex_adapter operator() (polygon2 const& poly) const
        {
            return polygon_vertex_adapter_2(poly);
        }
        vertex_adapter operator() (polygon3 const& poly) const
        {
            return polygon_vertex_adapter_3(poly);
        }
    };

    static vertex_adapter create(geometry const& geom)
    {
        return mapnik::util::apply_visitor(vertex_adapter_factory::dispatch(), geom);
    }
};

}}
