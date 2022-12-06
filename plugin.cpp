#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <map>

#include "simPlusPlus/Plugin.h"
#include "config.h"
#include "plugin.h"
#include "stubs.h"

using std::string;

#include <CGAL/Scale_space_surface_reconstruction_3.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Timer.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel     Kernel;
typedef CGAL::Scale_space_surface_reconstruction_3<Kernel>      Reconstruction;
typedef CGAL::Scale_space_reconstruction_3::Alpha_shape_mesher<Kernel> Mesher;
typedef CGAL::Scale_space_reconstruction_3::Weighted_PCA_smoother<Kernel> Smoother;
typedef Reconstruction::Point                                   Point;
typedef std::vector<Point>                                      Point_collection;
typedef Reconstruction::Facet                                   Facet;
typedef Reconstruction::Facet_const_iterator                    Facet_const_iterator;

class Plugin : public sim::Plugin
{
public:
    void onStart()
    {
        if(!registerScriptStuff())
            throw std::runtime_error("failed to register script stuff");

        setExtVersion("Surface Reconstruction Plugin");
        setBuildDate(BUILD_DATE);
    }

    void reconstruct_scale_space(reconstruct_scale_space_in *in, reconstruct_scale_space_out *out)
    {
        // get point cloud points:
        int ptCnt = -1;
        const double *ptArray0 = simGetPointCloudPoints(in->pointCloudHandle, &ptCnt, 0);
        if(!ptArray0)
            throw std::runtime_error("call to simGetPointCloudPoints failed");
        double *ptArray = new double[ptCnt * 3];
        std::memcpy(ptArray, ptArray0, sizeof(double) * 3 * ptCnt);
        simReleaseBuffer(ptArray0);

        // transform points wrt point cloud frame:
        double pclMatrix[12];
        simGetObjectMatrix(in->pointCloudHandle, -1, &pclMatrix[0]);
        for(int i = 0; i < ptCnt; i++)
        {
            if(simTransformVector(&pclMatrix[0], ptArray + 3 * i) == -1)
            {
                delete[] ptArray;
                throw std::runtime_error("simTransformVector failed");
            }
        }

        Point_collection points;
        for(int i = 0; i < ptCnt * 3; i += 3)
        {
            sim::addLog(sim_verbosity_debug, "%f %f %f", ptArray[i], ptArray[i+1], ptArray[i+2]);
            points.push_back(Point(ptArray[i], ptArray[i+1], ptArray[i+2]));
        }
        Reconstruction reconstruct;
        Smoother smoother_ns(in->neighbors, in->samples);
        Smoother smoother_sr(in->squared_radius);
        Smoother &smoother = in->squared_radius > 0 ? smoother_sr : smoother_ns;
        Mesher mesher(smoother.squared_radius());
        reconstruct.insert(points.begin(), points.end());
        reconstruct.increase_scale(in->iterations, smoother);
        reconstruct.reconstruct_surface(mesher);
        int triCount = reconstruct.number_of_facets();
        int *idxArray = new int[triCount*3];
        int idxCnt = 0;
        for(Facet_const_iterator it = reconstruct.facets_begin(); it != reconstruct.facets_end(); ++it)
        {
            const Facet &facet = *it;
            if(facet.at(0) < 0 || facet.at(0) >= points.size() ||
                facet.at(1) < 0 || facet.at(1) >= points.size() ||
                facet.at(2) < 0 || facet.at(2) >= points.size())
                sim::addLog(sim_verbosity_debug, "# OUT OF BOUNDS: f %f %f %f", facet.at(0), facet.at(1), facet.at(2));
            else
                sim::addLog(sim_verbosity_debug, "f %f %f %f", facet.at(0), facet.at(1), facet.at(2));
            idxArray[idxCnt++] = facet.at(0);
            idxArray[idxCnt++] = facet.at(1);
            idxArray[idxCnt++] = facet.at(2);
        }
        sim::addLog(sim_verbosity_debug, "Generated shape from %d points %d indices", ptCnt, idxCnt);
        out->shapeHandle = idxCnt > 0 ? simCreateMeshShape(0, 1.2, ptArray, 3 * ptCnt, idxArray, idxCnt, 0) : -1;
        delete[] ptArray;
        delete[] idxArray;
    }
};

SIM_PLUGIN(PLUGIN_NAME, PLUGIN_VERSION, Plugin)
#include "stubsPlusPlus.cpp"
