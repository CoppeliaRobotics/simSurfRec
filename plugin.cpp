#ifdef _WIN32
	#define VREP_DLLEXPORT extern "C" __declspec(dllexport)
#endif /* _WIN32 */
#if defined (__linux) || defined (__APPLE__)
	#define VREP_DLLEXPORT extern "C"
#endif /* __linux || __APPLE__ */

VREP_DLLEXPORT unsigned char v_repStart(void* reservedPointer,int reservedInt);
VREP_DLLEXPORT void v_repEnd();
VREP_DLLEXPORT void* v_repMessage(int message,int* auxiliaryData,void* customData,int* replyData);

#include "plugin.h"
#include "debug.h"
#include "v_repLib.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <boost/algorithm/string/predicate.hpp>

#ifdef _WIN32
    #ifdef QT_COMPIL
        #include <direct.h>
    #else
        #include <shlwapi.h>
        #pragma comment(lib, "Shlwapi.lib")
    #endif
#endif /* _WIN32 */
#if defined (__linux) || defined (__APPLE__)
    #include <unistd.h>
#define _stricmp strcasecmp
#endif /* __linux || __APPLE__ */

LIBRARY vrepLib;

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

#define DEBUG_OBJ_OUTPUT

void reconstruct_scale_space(SScriptCallBack *p, const char *cmd, reconstruct_scale_space_in *in, reconstruct_scale_space_out *out)
{
    DBG << "[enter]" << std::endl;

    // get point cloud points:
    int ptCnt = -1;
    const float *ptArray0 = simGetPointCloudPoints(in->pointCloudHandle, &ptCnt, 0);
    if(!ptArray0)
        throw string("call to simGetPointCloudPoints failed");
    float *ptArray = new float[ptCnt * 3];
    std::memcpy(ptArray, ptArray0, sizeof(float) * 3 * ptCnt);

    // transform points wrt point cloud frame:
    simFloat pclMatrix[12];
    simGetObjectMatrix(in->pointCloudHandle, -1, &pclMatrix[0]);
    for(int i = 0; i < ptCnt; i++)
    {
        if(simTransformVector(&pclMatrix[0], ptArray + 3 * i) == -1)
        {
            delete[] ptArray;
            throw string("simTransformVector failed");
        }
    }

    Point_collection points;
    for(int i = 0; i < ptCnt * 3; i += 3)
    {
#ifdef DEBUG_OBJ_OUTPUT
	std::stringstream ss;
	ss << "v " << ptArray[i] << " " << ptArray[i+1] << " " << ptArray[i+2];
	std::cout << ss.str() << std::endl;
#endif
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
    simInt *idxArray = new simInt[triCount*3];
    int idxCnt = 0;
    for(Facet_const_iterator it = reconstruct.facets_begin(); it != reconstruct.facets_end(); ++it)
    {
	const Facet &facet = *it;
#ifdef DEBUG_OBJ_OUTPUT
	std::stringstream ss;
	ss << "f " << facet.at(0) << " " << facet.at(1) << " " << facet.at(2);
	if(facet.at(0) < 0 || facet.at(0) >= points.size() ||
		facet.at(1) < 0 || facet.at(1) >= points.size() ||
		facet.at(2) < 0 || facet.at(2) >= points.size())
	    std::cout << "# OUT OF BOUNDS: " << ss.str() << std::endl;
	else
	    std::cout << ss.str() << std::endl;
#endif
	idxArray[idxCnt++] = facet.at(0);
	idxArray[idxCnt++] = facet.at(1);
	idxArray[idxCnt++] = facet.at(2);
    }
#ifdef DEBUG_OBJ_OUTPUT
    std::stringstream ss;
    ss << "Generated shape from " << ptCnt << " points " << idxCnt << " indices" << std::endl;
    std::cout << ss.str() << std::endl;
#endif
    out->shapeHandle = idxCnt > 0 ? simCreateMeshShape(0, 1.2, ptArray, 3 * ptCnt, idxArray, idxCnt, 0) : -1;
    delete[] ptArray;
    delete[] idxArray;

    DBG << "[leave]" << std::endl;
}

VREP_DLLEXPORT unsigned char v_repStart(void* reservedPointer, int reservedInt)
{
    char curDirAndFile[1024];
#ifdef _WIN32
    #ifdef QT_COMPIL
        _getcwd(curDirAndFile, sizeof(curDirAndFile));
    #else
        GetModuleFileName(NULL, curDirAndFile, 1023);
        PathRemoveFileSpec(curDirAndFile);
    #endif
#elif defined (__linux) || defined (__APPLE__)
    getcwd(curDirAndFile, sizeof(curDirAndFile));
#endif

    std::string currentDirAndPath(curDirAndFile);
    std::string temp(currentDirAndPath);
#ifdef _WIN32
    temp+="\\v_rep.dll";
#elif defined (__linux)
    temp+="/libv_rep.so";
#elif defined (__APPLE__)
    temp+="/libv_rep.dylib";
#endif /* __linux || __APPLE__ */
    vrepLib = loadVrepLibrary(temp.c_str());
    if(vrepLib == NULL)
    {
        std::cout << "Error, could not find or correctly load the V-REP library. Cannot start '" PLUGIN_NAME "' plugin." << std::endl;
        return 0;
    }
    if(getVrepProcAddresses(vrepLib)==0)
    {
        std::cout << "Error, could not find all required functions in the V-REP library. Cannot start '" PLUGIN_NAME "' plugin." << std::endl;
        unloadVrepLibrary(vrepLib);
        return 0;
    }

    int vrepVer;
    simGetIntegerParameter(sim_intparam_program_version, &vrepVer);
    if(vrepVer < 30203) // if V-REP version is smaller than 3.02.03
    {
        std::cout << "Sorry, your V-REP copy is somewhat old. Cannot start '" PLUGIN_NAME "' plugin." << std::endl;
        unloadVrepLibrary(vrepLib);
        return 0;
    }

    if(!registerScriptStuff())
    {
        std::cout << "Initialization failed." << std::endl;
        unloadVrepLibrary(vrepLib);
        return 0;
    }

    return PLUGIN_VERSION;
}

VREP_DLLEXPORT void v_repEnd()
{
    DBG << "[enter]" << std::endl;

    unloadVrepLibrary(vrepLib); // release the library

    DBG << "[leave]" << std::endl;
}

VREP_DLLEXPORT void* v_repMessage(int message, int* auxiliaryData, void* customData, int* replyData)
{
    // Keep following 5 lines at the beginning and unchanged:
    static bool refreshDlgFlag = true;
    int errorModeSaved;
    simGetIntegerParameter(sim_intparam_error_report_mode, &errorModeSaved);
    simSetIntegerParameter(sim_intparam_error_report_mode, sim_api_errormessage_ignore);
    void* retVal=NULL;

    static bool firstInstancePass = true;
    if(firstInstancePass && message == sim_message_eventcallback_instancepass)
    {
        firstInstancePass = false;
        //UIFunctions::getInstance(); // construct UIFunctions here (SIM thread)
    }

    if(message == sim_message_eventcallback_simulationended)
    { // Simulation just ended
        // TODO: move this to sim_message_eventcallback_simulationabouttoend
        // TODO: ASSERT_THREAD(???)
        //Proxy::destroyTransientObjects();
    }

#ifdef VREP_INSTANCE_SWITCH_WORKS
    static int oldSceneID = simGetInt32ParameterE(sim_intparam_scene_unique_id);
    if(message == sim_message_eventcallback_instanceswitch)
    {
        int newSceneID = simGetInt32ParameterE(sim_intparam_scene_unique_id);
        //Proxy::sceneChange(oldSceneID, newSceneID);
        oldSceneID = newSceneID;
    }
#else
    // XXX: currently (3.3.1 beta) it is broken
    if(message == sim_message_eventcallback_instancepass)
    {
        static int oldSceneID = -1;
        if(oldSceneID == -1) oldSceneID = simGetInt32ParameterE(sim_intparam_scene_unique_id);
        int sceneID = simGetInt32ParameterE(sim_intparam_scene_unique_id);
        if(sceneID != oldSceneID)
        {
            //Proxy::sceneChange(oldSceneID, sceneID);
            oldSceneID = sceneID;
        }
    }
#endif

    // Keep following unchanged:
    simSetIntegerParameter(sim_intparam_error_report_mode, errorModeSaved); // restore previous settings
    return(retVal);
}

