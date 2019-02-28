
#include <string>
#include <algorithm>
#include <unistd.h>

#include "cgnslib.h"

#include "grid.h"
#include "error.h"

#define NAMESIZE 33

namespace unstruc {
    bool cgns_write(const std::string& outputfile, const Grid& grid) {
        if (grid.dim != 3) fatal("Currently can only save 3d cgns files");

        int index_file;
        auto close_file_and_exit = [index_file]() {
            cg_error_exit();
        };
        if( access( outputfile.c_str(), F_OK ) != -1 ) {
            if( remove( outputfile.c_str()) != 0 ) {
                fatal("Failed to delete existing output file");
            }
        } 
        if (cg_open(outputfile.c_str(),CG_MODE_WRITE,&index_file)) close_file_and_exit();

        int index_base;
        char basename[NAMESIZE] = "MyBase\0";
        int cell_dim = grid.dim;
        int phys_dim = grid.dim;
        
        if (cg_base_write(index_file,basename,cell_dim,phys_dim,&index_base)) close_file_and_exit();

        int n_volume_elements = 0;
        for (const Element& e : grid.elements)
            if (Shape::Info[e.type].dim == grid.dim)
                n_volume_elements++;

        int n_points = grid.points.size();
        int index_zone;
        char zonename[NAMESIZE] = "MyZone\0";
        int zonesize[3] = {n_points, n_volume_elements, 0};
        ZoneType_t zonetype = Unstructured;

        if (cg_zone_write(index_file,index_base,zonename,zonesize,zonetype,&index_zone)) close_file_and_exit();

        int index_grid;
        char gridname[NAMESIZE] = "GridCoordinates\0";
        if (cg_grid_write(index_file,index_base,index_zone,gridname,&index_grid)) close_file_and_exit();

        DataType_t coordtype = RealDouble;
        int index_coord;

        std::vector<double> coorddata_x;
        coorddata_x.reserve(n_points);
        for (const Point& p : grid.points) {
            coorddata_x.push_back(p.x);
        }
        char coordname_x[NAMESIZE] = "CoordinateX";
        if (cg_coord_write(index_file,index_base,index_zone,coordtype,coordname_x,coorddata_x.data(),&index_coord)) close_file_and_exit();

        std::vector<double> coorddata_y;
        coorddata_y.reserve(n_points);
        for (const Point& p : grid.points) {
            coorddata_y.push_back(p.y);
        }
        char coordname_y[NAMESIZE] = "CoordinateY";
        if (cg_coord_write(index_file,index_base,index_zone,coordtype,coordname_y,coorddata_y.data(),&index_coord)) close_file_and_exit();

        if (grid.dim == 3) {
            std::vector<double> coorddata_z;
            coorddata_z.reserve(n_points);
            for (const Point& p : grid.points) {
                coorddata_z.push_back(p.z);
            }
            char coordname_z[NAMESIZE] = "CoordinateZ";
            if (cg_coord_write(index_file,index_base,index_zone,coordtype,coordname_z,coorddata_z.data(),&index_coord)) close_file_and_exit();
        }

        int n_grid_names = grid.names.size();
        std::vector<size_t> name_count(grid.names.size(),0);
        std::vector<size_t> name_element_data_size(grid.names.size(),0);
        size_t element_data_size = 0;
        size_t default_count = 0;
        size_t default_data_size = 0;
        for (const Element &e: grid.elements) {
            if (Shape::Info[e.type].dim != (grid.dim)) continue;
            default_count++;
            default_data_size += e.points.size() + 1;
            element_data_size += e.points.size() + 1;
        }

        for (const Element &e: grid.elements) {
            if (Shape::Info[e.type].dim != (grid.dim-1)) continue;
            if (e.name_i == -1) continue;
            name_count[e.name_i]++;
            name_element_data_size[e.name_i] += e.points.size() + 1;
            element_data_size += e.points.size() + 1;
        }

        std::vector<Element> sorted_elements = grid.elements;
        std::sort(sorted_elements.begin(),sorted_elements.end(),[](const Element& e1, const Element& e2) {return e1.name_i < e2.name_i;});

        std::vector<int> element_data;
        element_data.reserve(element_data_size);
        for (const Element& e: sorted_elements) {
            if (Shape::Info[e.type].dim != (grid.dim)) continue;
            ElementType_t data_type;
            switch (e.type) {
                case Shape::Type::Undefined : fatal("Undefined element type"); break;
                case Shape::Type::Line: data_type = BAR_2; break;
                case Shape::Type::Triangle: data_type =  TRI_3; break;
                case Shape::Type::Quad: data_type = QUAD_4; break;
                case Shape::Type::Polygon:  fatal("Polygon element type"); break;
                case Shape::Type::Tetra: data_type = TETRA_4; break;
                case Shape::Type::Hexa: data_type = HEXA_8; break;
                case Shape::Type::Wedge: data_type = PENTA_6; break;
                case Shape::Type::Pyramid: data_type = PYRA_5; break;
                case Shape::Type::NShapes: fatal("NShapes element type"); break;
                default: fatal("Other element type"); break;
            };
            int npe;
            if (cg_npe(data_type,&npe)) {
                if (npe != e.points.size()) {
                    printf("wrong number of points\n");
                    close_file_and_exit();
                }
            }
            element_data.push_back(data_type);
            for (size_t p : e.points) {
                element_data.push_back(p+1);
            }
        }
        for (const Element& e: sorted_elements) {
            if (Shape::Info[e.type].dim != (grid.dim-1)) continue;
            if (e.name_i == -1) continue;
            ElementType_t data_type;
            switch (e.type) {
                case Shape::Type::Undefined : fatal("Undefined element type"); break;
                case Shape::Type::Line: data_type = BAR_2; break;
                case Shape::Type::Triangle: data_type =  TRI_3; break;
                case Shape::Type::Quad: data_type = QUAD_4; break;
                case Shape::Type::Polygon:  fatal("Polygon element type"); break;
                case Shape::Type::Tetra: data_type = TETRA_4; break;
                case Shape::Type::Hexa: data_type = HEXA_8; break;
                case Shape::Type::Wedge: data_type = PENTA_6; break;
                case Shape::Type::Pyramid: data_type = PYRA_5; break;
                case Shape::Type::NShapes: fatal("NShapes element type"); break;
                default: fatal("Other element type"); break;
            };
            int npe;
            if (cg_npe(data_type,&npe)) {
                if (npe != e.points.size()) {
                    printf("wrong number of points\n");
                    close_file_and_exit();
                }
            }
            element_data.push_back(data_type);
            for (size_t p : e.points) {
                element_data.push_back(p+1);
            }
        }

        int i_e = 1;
        int i_ed = 0;

        char sectionname[33] = "00_3D";
        printf("name = %s\n",sectionname);
        // const char *sectionname = grid.names[i].name.c_str();
        ElementType_t type = MIXED;
        int index_section;
        int start = i_e;
        int end = start + default_count - 1;
        int nboundary = 0;
        int data_size = default_data_size;
        int *data = &element_data[i_ed];
        i_e += default_count;
        i_ed += data_size;

        if (cg_section_write(index_file,index_base,index_zone,sectionname,type,start,end,nboundary,data,&index_section)) close_file_and_exit();

        for (int i = 0; i < grid.names.size(); i++) {
            int count = name_count[i];
            if (count == 0) continue;
            snprintf(sectionname,32,"%02d_%s",i+1,grid.names[i].name.c_str());
            // const char *sectionname = grid.names[i].name.c_str();
            ElementType_t type = MIXED;
            int index_section;
            int start = i_e;
            int end = start + count - 1;
            int nboundary = 0;
            int data_size = name_element_data_size[i];
            int *data = &element_data[i_ed];
            printf("name = %s\n",sectionname);
            i_e += count;
            i_ed += data_size;

            if (cg_section_write(index_file,index_base,index_zone,sectionname,type,start,end,nboundary,data,&index_section)) close_file_and_exit();
        }

        printf("Closing\n");
        if (cg_close(index_file)) goto error;
        return true;
error:
        fatal("Exiting with error");
        return false;
        
    }
}