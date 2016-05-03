/*
    GradSmooth: Point cloud smoothing via distance to measure
    gradient flows.

    Author: Patrick A. O'Neil

License:
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include <iostream>
#include <gflags/gflags.h>
#include <cnpy/cnpy.h>
#include <eigen3/Eigen/Dense>
#include "easylogging++.h"

#include "smoother.h"
#include "PointCloud.h"

// Command line args
DEFINE_double(step_size, 0.10, "Step size for gradient flow");
DEFINE_int32(num_neighbors, 5, "Number of nearest neighbors to use for knn-search");
DEFINE_int32(iterations, 10, "Number of iterations to run the smoothing algorithm");

// Start EasyLoggingPP logger
INITIALIZE_EASYLOGGINGPP;

int main(int argc, char** argv) {

    // Set up logging
    el::Configurations conf("/home/poneil/Math/GradSmooth/logging.conf");
    el::Loggers::reconfigureAllLoggers(conf); LOG(INFO) << "Starting GradSmooth.";

    // Parse command line flags
    gflags::SetUsageMessage("GradSmooth: Arbitrary dimension point cloud smoothing."); 
    unsigned arg_indx = gflags::ParseCommandLineFlags(&argc, &argv, true);

    // Get input and output path for numpy arrays
    if(argc != 3) {
        LOG(ERROR) << "Incorrect number of command line args."
                   << "Please specify input and output path.";
        return 0;
    }
    std::string infn  = argv[arg_indx];
    std::string outfn = argv[arg_indx + 1];
    LOG(INFO) << "Using input path: " <<  infn;
    LOG(INFO) << "Using output path: " << outfn;

    PointCloud  point_cloud;
    PointCloud  evolved_cloud;

    point_cloud.load_cloud(infn);

    evolved_cloud.copy_cloud(point_cloud);

    LOG(INFO)  << "Building k-d Tree";
    KDTree kd_tree = KDTree(point_cloud.get_dimension(), *point_cloud.get_cloud(), 10);
    kd_tree.index -> buildIndex();
    LOG(DEBUG) << "Successfully populated k-d Tree with " << kd_tree.kdtree_get_point_count() << " points";

    point_cloud.assign_kd_tree(&kd_tree);

    Smoother smoother;

    smoother.smooth_point_cloud(point_cloud, evolved_cloud, FLAGS_num_neighbors, FLAGS_iterations, FLAGS_step_size);

    evolved_cloud.save_cloud(outfn);

    return 0;
}