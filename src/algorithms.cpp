// [[Rcpp::depends(BH)]]

#include <Rcpp.h>
using namespace Rcpp;

#include <boost/geometry.hpp>

#include "wkt.h"
#include "variants.h"
#include "googlePolylines.h"

namespace bg = boost::geometry;
namespace bgm = bg::model;

// TODO:
// - other algorithms: http://www.boost.org/doc/libs/1_66_0/libs/geometry/doc/html/geometry/reference/algorithms.html
//
// - correct for polygon ring direction? (outer == clockwise, holes == counter-clockwise)
// -- we could leave this up to the user for now, and assume the rings are oriented correctly
// -- or should the user conform from the outset? (and use http://www.boost.org/doc/libs/1_47_0/libs/geometry/doc/html/geometry/reference/algorithms/correct.html)
//
// - specify units / CRS ? 

  
template <typename T, typename F>
double geometryFunction(T geom,  F geomFunc) {
  return geomFunc(geom);
}

/*
void intersectionTest() { 
  
  std::string wkt1 = "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.1 3,5.3 2.6,5.4 1.2,4.9 0.8,2.9 0.7,2 1.3)"
  "(4.0 2.0, 4.2 1.4, 4.8 1.9, 4.4 2.2, 4.0 2.0))";
  std::string wkt2 = "POLYGON((4.0 -0.5 , 3.5 1.0 , 2.0 1.5 , 3.5 2.0 , 4.0 3.5 , 4.5 2.0 , 6.0 1.5 , 4.5 1.0 , 4.0 -0.5))";
  
  std::string wkt3 = "LINESTRING(4.0 -0.5 , 3.5 1.0 , 2.0 1.5 , 3.5 2.0 , 4.0 3.5 , 4.5 2.0 , 6.0 1.5 , 4.5 1.0 , 4.0 -0.5)";
  
  std::string wkt4 = "MULTIPOLYGON(((4.0 -0.5 , 3.5 1.0 , 2.0 1.5 , 3.5 2.0 , 4.0 3.5 , 4.5 2.0 , 6.0 1.5 , 4.5 1.0 , 4.0 -0.5)))";
  
  
  GeoIntersection gv1;
  gv1 = read_intersection_wkt(wkt1);

  GeoIntersection gv2;
  gv2 = read_intersection_wkt(wkt2);
  
  GeoIntersection gv3;
  gv3 = read_intersection_wkt(wkt3);
  
  GeoIntersection gv4;
  gv4 = read_intersection_wkt(wkt4);
  
  //std::vector<multi_linestring_type> output;
  //std::deque<linestring_type> output;
  //GeoIntersectionOutput output;
  multi_linestring_type output;
  //multi_polygon_type output;
  //geometryFunction(gv1, gv2, bg::intersection<geometryVariant>);
  bg::intersection(gv1, gv4, output);
  
  std::cout << bg::wkt(output) << std::endl;
  
//  BOOST_FOREACH(AnyGeo const& p, output) {
//    std::cout << bg::wkt(p) << std::endl;
//  }
}
*/

// [[Rcpp::export]]
Rcpp::StringVector rcpp_polyline_centroid(Rcpp::StringVector wkt ) {
  
  Rcpp::StringVector result(wkt.length());
  
  std::string s_wkt;
  Rcpp::String i_wkt;

  AnyGeo g;
  point_type p;
  
  for (size_t i = 0; i < wkt.size(); i++ ) {
    i_wkt = wkt[i];
    s_wkt = i_wkt;
    std::stringstream ss;
    
    g = read_any_wkt(s_wkt);
    bg::centroid(g, p);
    ss << bg::wkt(p);
    result[i] = ss.str();

  }
  return result;
}

// [[Rcpp::export]]
Rcpp::NumericVector rcpp_polyline_algorithm(Rcpp::StringVector wkt, Rcpp::String algorithm) {

  Rcpp::NumericVector result(wkt.length());
  std::string s_wkt;
  Rcpp::String i_wkt;
  //geometryVariant gv;

  AnyGeo g;
  
  for (size_t i = 0; i < wkt.size(); i++ ) {
    i_wkt = wkt[i];
    s_wkt = i_wkt;
    
    g = read_any_wkt(s_wkt);
    
    //gv = read_any_wkt(s_wkt);
    if(algorithm == "length") { 
      result[i] = geometryFunction(g, bg::length<AnyGeo>);
    }else if(algorithm == "area") {
      result[i] = geometryFunction(g, bg::area<AnyGeo>);
    }
  }
  return result;
}

template <typename Point>
void encode_wkt_point(Point const& p, std::ostringstream& os) {

  Rcpp::NumericVector lon(1);
  Rcpp::NumericVector lat(1);
    
  lon[0] = bg::get<0>(p);
  lat[0] = bg::get<1>(p);

  addToStream(os, encode_polyline(lon, lat));
}


template <typename MultiPoint>
void encode_wkt_multipoint(MultiPoint const& mp, std::ostringstream& os) {
  
  typedef typename boost::range_iterator
    <
      multi_point_type const
    >::type iterator_type;
  
  for (iterator_type it = boost::begin(mp); 
       it != boost::end(mp);
       ++it) 
  {
    encode_wkt_point(*it, os);
  }
  
}

template <typename LineString>
void encode_wkt_linestring(LineString const& ls, std::ostringstream& os) {
  //std::cout << "num points: " << bg::num_points(ls) << std::endl;
  
  // works for RINGS (because it's templated)
  
  size_t n = bg::num_points(ls);
  Rcpp::NumericVector lon(n);
  Rcpp::NumericVector lat(n);
  
  typedef typename boost::range_iterator
    <
      linestring_type const
    >::type iterator_type;
  
  int i = 0;
  for (iterator_type it = boost::begin(ls);
       it != boost::end(ls);
       ++it)
  {
    lon[i] = bg::get<0>(*it);
    lat[i] = bg::get<1>(*it);
    i++;
  }
  addToStream(os, encode_polyline(lon, lat));
}

template <typename MultiLinestring>
void encode_wkt_multi_linestring(MultiLinestring const& mls, std::ostringstream& os) {

  typedef typename boost::range_iterator
    <
      multi_linestring_type const
    >::type iterator_type;
  
  for (iterator_type it = boost::begin(mls);
       it != boost::end(mls);
       ++it)
  {
    encode_wkt_linestring(*it, os);
  }
}

template <typename Polygon>
void encode_wkt_polygon(Polygon const& pl, std::ostringstream& os) {

  encode_wkt_linestring(pl.outer(), os);

  // perhaps use
  // https://stackoverflow.com/questions/7722087/getting-the-coordinates-of-points-from-a-boost-geometry-polygon/7748091#7748091
  // https://stackoverflow.com/questions/37071031/how-to-get-a-polygon-from-boostgeometrymodelpolygon
  for (auto& i: pl.inners() ) {
    encode_wkt_linestring(i, os);
  }
}

template <typename MultiPolygon>
void encode_wkt_multi_polygon(MultiPolygon const& mpl, std::ostringstream& os) {
  
  typedef typename boost::range_iterator
  <
    multi_polygon_type const
  >::type iterator_type;
  
  for (iterator_type it = boost::begin(mpl);
       it != boost::end(mpl);
       ++it)
  {
    encode_wkt_polygon(*it, os);
    addToStream(os, SPLIT_CHAR);
  }
}

// [[Rcpp::export]]
Rcpp::List wkt_to_polyline(Rcpp::StringVector wkt) {
  
  size_t n = wkt.length();
  Rcpp::String r_wkt;
  std::string str_wkt;
  std::string geomType;

  Rcpp::List resultPolylines(n);
  int lastItem;
  
  for (int i = 0; i < n; i++ ) {
    
    std::ostringstream os;
    
    r_wkt = wkt[i];
    str_wkt = r_wkt;
    geomType = geomFromWKT(str_wkt);
    
    //Rcpp::Rcout << "geomType: " << geomType << std::endl;
    
    Rcpp::CharacterVector cls(3);
    cls[0] = "XY";
    cls[1] = geomType;
    cls[2] = "sfg";
    
    if (geomType == "POINT" ) {
      
      point_type pt;
      bg::read_wkt(str_wkt, pt);
      encode_wkt_point(pt, os);
      
    }else if (geomType == "MULTIPOINT" ) {
      
      multi_point_type mp;
      bg::read_wkt(str_wkt, mp);
      encode_wkt_multipoint(mp, os);
      
    }else if (geomType == "LINESTRING" ) {
      
      linestring_type ls;
      bg::read_wkt(str_wkt, ls);
      encode_wkt_linestring(ls, os);
      
    }else if (geomType == "MULTILINESTRING" ) {
      
      multi_linestring_type mls;
      bg::read_wkt(str_wkt, mls);
      encode_wkt_multi_linestring(mls, os);
      
    }else if (geomType == "POLYGON" ) { 
      
      polygon_type pl;
      bg::read_wkt(str_wkt, pl);
      encode_wkt_polygon(pl, os);
      
    }else if (geomType == "MULTIPOLYGON" ) {
      
      multi_polygon_type mpl;
      bg::read_wkt(str_wkt, mpl);
      encode_wkt_multi_polygon(mpl, os);
    }
   
    std::string str = os.str();
    std::vector<std::string> strs = split(str, ' ');;
    
    if(strs.size() > 1) {
      lastItem = strs.size() - 1;
      
      if (strs[lastItem] == "-") {
        strs.erase(strs.end() - 1);
      }
    }
    
    Rcpp::CharacterVector sv = wrap(strs);
    sv.attr("sfc") = cls;
    resultPolylines[i] = sv;
  }
  
  return resultPolylines;
}







