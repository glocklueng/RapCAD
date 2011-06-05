/*
 *   RapCAD - Rapid prototyping CAD IDE (www.rapcad.org)
 *   Copyright (C) 2010-2011 Giles Bathgate
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "cgal.h"
#include "cgalexport.h"
#include <QFile>
#include <QTextStream>

CGALExport::CGALExport()
{
}

void CGALExport::exportAsciiSTL(CGALPrimitive* prim, QString filename, bool precise)
{
	CGAL::Polyhedron3* poly=prim->getPolyhedron();

	typedef CGAL::Polyhedron3::Vertex Vertex;
	typedef CGAL::Polyhedron3::Vertex_const_iterator VertexIterator;
	typedef CGAL::Polyhedron3::Facet_const_iterator FacetIterator;
	typedef CGAL::Polyhedron3::Halfedge_around_facet_const_circulator HalffacetCirculator;

	QFile data(filename);
	if (!data.open(QFile::WriteOnly | QFile::Truncate)) {
		//error
		return;
	}
	QTextStream output(&data);
	if(precise){
		//Technically we should not use a precision > 7 since the stl
		//spec specifies single precision floating points but higher
		//precisions seem to work with some apps here which is good
		output.setRealNumberPrecision(16);
		//smart notation gives the smallest file sizes.
		output.setRealNumberNotation(QTextStream::SmartNotation);
	} else {
		//These settings are more conservative and hopefully offer
		//better compatability
		output.setRealNumberPrecision(6);
		output.setRealNumberNotation(QTextStream::ScientificNotation);
	}

	output << "solid RapCAD_Model\n";

	for (FacetIterator fi = poly->facets_begin(); fi != poly->facets_end(); ++fi) {
		HalffacetCirculator hc = fi->facet_begin();
		HalffacetCirculator he = hc;
		Vertex v1, v2, v3;
		v1 = *VertexIterator((hc++)->vertex());
		v3 = *VertexIterator((hc++)->vertex());
		do {
			v2 = v3;
			v3 = *VertexIterator((hc++)->vertex());
			CGAL::Point3 p1,p2,p3;
			p1=v1.point();
			p2=v2.point();
			p3=v3.point();
			if (p1 == p2 || p1 == p3 || p2 == p3)
				continue;

			double x1 = to_double(p1.x());
			double y1 = to_double(p1.y());
			double z1 = to_double(p1.z());
			double x2 = to_double(p2.x());
			double y2 = to_double(p2.y());
			double z2 = to_double(p2.z());
			double x3 = to_double(p3.x());
			double y3 = to_double(p3.y());
			double z3 = to_double(p3.z());

			double nx = (y1-y2)*(z1-z3) - (z1-z2)*(y1-y3);
			double ny = (z1-z2)*(x1-x3) - (x1-x2)*(z1-z3);
			double nz = (x1-x2)*(y1-y3) - (y1-y2)*(x1-x3);
			double l = sqrt(nx*nx + ny*ny + nz*nz);
			const double eps = 0.000001;
			l = fmax(l,eps);
			output << "  facet normal " << nx / l << " " << ny / l << " " << nz  / l << "\n";
			output << "    outer loop\n";
			output << "      vertex " << x1 << " " << y1 << " " << z1 << "\n";
			output << "      vertex " << x2 << " " << y2 << " " << z2 << "\n";
			output << "      vertex " << x3 << " " << y3 << " " << z3 << "\n";
			output << "    endloop\n";
			output << "  endfacet\n";
		} while (hc != he);
	}

	output << "endsolid RapCAD_Model\n";
	output.flush();
	data.close();
}
