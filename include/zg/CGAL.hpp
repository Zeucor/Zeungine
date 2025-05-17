#pragma once
#undef CGAL_CORE_USE_GMP_BACKEND
#define CGAL_CORE_USE_BOOST_BACKEND 1
#define CGAL_DISABLE_GMP 1
#define CGAL_USE_BOOST_MP 1
#undef CGAL_USE_GMP
#undef CGAL_USE_GMPXX
#undef CGAL_USE_LEDACGAL_USE_CORE
#define CGAL_USE_CORE 1
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>
#include <CGAL/Labeled_mesh_domain_3.h>
#include <CGAL/make_mesh_3.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/make_surface_mesh.h>
#include <CGAL/IO/facets_in_complex_2_to_triangle_mesh.h>
#include <CGAL/Min_sphere_d.h>
#include <CGAL/number_type_config.h>
#include <CGAL/Min_sphere_annulus_d_traits_3.h>
#include <CGAL/Polyhedron_3.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::FT FT;
typedef K::Point_3 Point;
typedef FT (Function)(const Point&);
typedef CGAL::Labeled_mesh_domain_3<K> Mesh_domain;
#ifdef CGAL_CONCURRENT_MESH_3
typedef CGAL::Parallel_tag Concurrency_tag;
#else
typedef CGAL::Sequential_tag Concurrency_tag;
#endif
typedef CGAL::Mesh_triangulation_3<Mesh_domain,CGAL::Default,Concurrency_tag>::type Tr;
typedef Tr::Cell_handle Cell_handle;
typedef Tr::Facet Facet;
typedef Tr::Vertex_handle Vertex_handle;
typedef CGAL::Mesh_complex_3_in_triangulation_3<Tr> C3t3;
typedef CGAL::Mesh_criteria_3<Tr> Mesh_criteria;
typedef CGAL::Surface_mesh<K::Point_3> TriangleMesh;