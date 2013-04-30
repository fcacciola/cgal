// Copyright (c) 2007-09  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
//
// Author(s) : Pierre Alliez and Laurent Saboret

#ifndef CGAL_WRITE_OFF_POINTS_H
#define CGAL_WRITE_OFF_POINTS_H

#include <CGAL/property_map.h>
#include <CGAL/point_set_processing_assertions.h>

#include <boost/version.hpp>
#if BOOST_VERSION >= 104000
  #include <boost/property_map/property_map.hpp>
#else
  #include <boost/property_map.hpp>
#endif

#include <iostream>
#include <iterator>

namespace CGAL {


//===================================================================================
/// \ingroup PkgPointSetProcessing
/// Saves the [first, beyond) range of points (positions + normals) to a .off ASCII stream.
/// The function writes for each point a line with the x y z position
/// followed by the nx ny nz normal.
///
/// \pre normals must be unit vectors
///
/// @tparam ForwardIterator iterator over input points.
/// @tparam PointPMap is a model of <a href="http://www.boost.org/doc/libs/release/libs/property_map/doc/ReadablePropertyMap.html">boost::ReadablePropertyMap</a> with a value_type = Point_3<Kernel>.
///        It can be omitted if ForwardIterator value_type is convertible to Point_3<Kernel>.
/// @tparam NormalPMap is a model of <a href="http://www.boost.org/doc/libs/release/libs/property_map/doc/WritablePropertyMap.html">boost::WritablePropertyMap</a> with a value_type = Vector_3<Kernel>.
/// @tparam Kernel Geometric traits class.
///        It can be omitted and deduced automatically from PointPMap value_type.
///
/// @return true on success.

// This variant requires all parameters.
template <typename ForwardIterator,
          typename PointPMap,
          typename NormalPMap,
          typename Kernel
>
bool
write_off_points_and_normals(
  std::ostream& stream, ///< output stream.
  ForwardIterator first,  ///< iterator over the first input point.
  ForwardIterator beyond, ///< past-the-end iterator over the input points.
  PointPMap point_pmap, ///< property map: value_type of ForwardIterator -> Point_3. 
  NormalPMap normal_pmap, ///< property map: value_type of ForwardIterator -> Vector_3. 
  const Kernel& /*kernel*/) ///< geometric traits.
{
  // basic geometric types
  typedef typename Kernel::Point_3 Point;
  typedef typename Kernel::Vector_3 Vector;

  CGAL_point_set_processing_precondition(first != beyond);

  if(!stream)
  {
    std::cerr << "Error: cannot open file" << std::endl;
    return false;
  }

  // Write header
  const std::size_t num_input_points = std::distance(first, beyond);
  stream << "NOFF" << std::endl;
  stream << num_input_points << " 0 0" << std::endl;

  // Write positions + normals
  for(ForwardIterator it = first; it != beyond; it++)
  {
#ifdef CGAL_USE_OLD_PAIR_PROPERTY_MAPS
    Point p = get(point_pmap, it);
    Vector n = get(normal_pmap, it);
#else
    Point p = get(point_pmap, *it);
    Vector n = get(normal_pmap, *it);
#endif
    stream << p << " " << n << std::endl;
  }

  return ! stream.fail();
}

/// @cond SKIP_IN_MANUAL
// This variant deduces the kernel from the point property map.
template <typename ForwardIterator,
          typename PointPMap,
          typename NormalPMap
>
bool
write_off_points_and_normals(
  std::ostream& stream, ///< output stream.
  ForwardIterator first, ///< first input point.
  ForwardIterator beyond, ///< past-the-end input point.
  PointPMap point_pmap, ///< property map: value_type of OutputIterator -> Point_3.
  NormalPMap normal_pmap) ///< property map: value_type of OutputIterator -> Vector_3.
{
  typedef typename boost::property_traits<PointPMap>::value_type Point;
  typedef typename Kernel_traits<Point>::Kernel Kernel;
  return write_off_points_and_normals(
    stream,
    first, beyond,
    point_pmap,
    normal_pmap,
    Kernel());
}
/// @endcond

/// @cond SKIP_IN_MANUAL
// This variant creates a default point property map = Typed_identity_property_map_by_reference.
template <typename ForwardIterator,
          typename NormalPMap
>
bool
write_off_points_and_normals(
  std::ostream& stream, ///< output stream.
  ForwardIterator first, ///< first input point.
  ForwardIterator beyond, ///< past-the-end input point.
  NormalPMap normal_pmap) ///< property map: value_type of OutputIterator -> Vector_3.
{
  return write_off_points_and_normals(
    stream,
    first, beyond,
    make_dereference_property_map(first),
    normal_pmap);
}
/// @endcond


//===================================================================================
/// \ingroup PkgPointSetProcessing
/// Saves the [first, beyond) range of points (positions only) to a .off ASCII stream.
/// The function writes for each point a line with the x y z position.
///
/// @tparam ForwardIterator iterator over input points.
/// @tparam PointPMap is a model of <a href="http://www.boost.org/doc/libs/release/libs/property_map/doc/ReadablePropertyMap.html">boost::ReadablePropertyMap</a> with a value_type = Point_3<Kernel>.
///        It can be omitted if ForwardIterator value_type is convertible to Point_3<Kernel>.
/// @tparam Kernel Geometric traits class.
///        It can be omitted and deduced automatically from PointPMap value_type.
///
/// @return true on success.

// This variant requires all parameters.
template <typename ForwardIterator,
          typename PointPMap,
          typename Kernel
>
bool
write_off_points(
  std::ostream& stream, ///< output stream.
  ForwardIterator first,  ///< iterator over the first input point.
  ForwardIterator beyond, ///< past-the-end iterator over the input points.
  PointPMap point_pmap, ///< property map: value_type of ForwardIterator -> Point_3.
  const Kernel& kernel) ///< geometric traits.
{
  // basic geometric types
  typedef typename Kernel::Point_3 Point;

  CGAL_point_set_processing_precondition(first != beyond);

  if(!stream)
  {
    std::cerr << "Error: cannot open file" << std::endl;
    return false;
  }

  // Write header
  const std::size_t num_input_points = std::distance(first, beyond);
  stream << "OFF" << std::endl;
  stream << num_input_points << " 0 0" << std::endl;

  // Write positions
  for(ForwardIterator it = first; it != beyond; it++)
  {
#ifdef CGAL_USE_OLD_PAIR_PROPERTY_MAPS
    Point p = get(point_pmap, it);
#else
    Point p = get(point_pmap, *it);
#endif
    stream << p << std::endl;
  }

  return ! stream.fail();
}

/// @cond SKIP_IN_MANUAL
// This variant deduces the kernel from the point property map.
template <typename ForwardIterator,
          typename PointPMap
>
bool
write_off_points(
  std::ostream& stream, ///< output stream.
  ForwardIterator first, ///< first input point.
  ForwardIterator beyond, ///< past-the-end input point.
  PointPMap point_pmap) ///< property map: value_type of OutputIterator -> Point_3.
{
  typedef typename boost::property_traits<PointPMap>::value_type Point;
  typedef typename Kernel_traits<Point>::Kernel Kernel;
  return write_off_points(
    stream,
    first, beyond,
    point_pmap,
    Kernel());
}
/// @endcond

/// @cond SKIP_IN_MANUAL
// This variant creates a default point property map = Typed_identity_property_map_by_reference.
template <typename ForwardIterator
>
bool
write_off_points(
  std::ostream& stream, ///< output stream.
  ForwardIterator first, ///< first input point.
  ForwardIterator beyond) ///< past-the-end input point.
{
  return write_off_points(
    stream,
    first, beyond,
#ifdef CGAL_USE_OLD_PAIR_PROPERTY_MAPS
    make_dereference_property_map(first)
#else
    make_typed_identity_property_map_by_reference(
    typename value_type_traits<ForwardIterator>::type())
#endif
    );
}
/// @endcond


} //namespace CGAL

#endif // CGAL_WRITE_OFF_POINTS_H
