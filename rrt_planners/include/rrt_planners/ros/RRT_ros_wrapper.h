/********************************************************************
*
* Software License Agreement (BSD License)
*
*  Author: Noé Pérez Higueras
*********************************************************************/

#ifndef RRT_ROS_WRAPPER_
#define RRT_ROS_WRAPPER_

// C++
#include <vector>
#include <queue>
#include <cmath>
#include <stdio.h>
#include <iostream>

// Mutex
#include <mutex>

// ROS
#include <ros/ros.h>
#include <costmap_2d/costmap_2d.h>
#include <costmap_2d/VoxelGrid.h>
#include <costmap_2d/costmap_2d_ros.h>
#include <visualization_msgs/Marker.h>
#include <geometry_msgs/PointStamped.h>
#include <geometry_msgs/Pose.h>
#include <tf/transform_datatypes.h>
//#include <tf/transform_listener.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/buffer.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <nav_msgs/OccupancyGrid.h>
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/PointCloud.h>
#include <sensor_msgs/point_cloud_conversion.h>
#include <pcl_ros/transforms.h>


// PCL
//#include <pcl/io/pcd_io.h>
//#include <pcl/conversions.h>
//#include <pcl_conversions/pcl_conversions.h>
//#include <pcl/point_types.h>


// GMM sampling services
//#include <gmm_sampling/GetApproachGMMSamples.h>
//#include <gmm_sampling/GetApproachGMMProbs.h>

// RRT library
#include <rrt_planners/planners/Planner.h>
#include <rrt_planners/planners/simple/SimpleRRT.h>
#include <rrt_planners/planners/simple/SimpleRRTstar.h>
#include <rrt_planners/planners/simple/SimpleQuickRRTstar.h>
#include <rrt_planners/planners/control/Rrt.h>
#include <rrt_planners/planners/control/RRTstar.h>
#include <rrt_planners/planners/control/HalfRRTstar.h>
// Planning service
#include <rrt_planners/MakePlan.h>
#include <rrt_planners/ChangeSolveTime.h>

//#include <rrt_planners/ros/ValidityChecker.h>
#include <rrt_planners/ros/ValidityChecker3D.h>

// Dynamic reconfigure
//#include <dynamic_reconfigure/server.h>
//#include <rrt_planners/RRTRosWrapperConfig.h>



namespace RRT_ros
{
class RRT_ros_wrapper
{
public:
  RRT_ros_wrapper();
  RRT_ros_wrapper(tf2_ros::Buffer* tf);

  // Only for RRT as a local controller
  RRT_ros_wrapper(tf2_ros::Buffer* tf, float controller_freq, float path_stddev,
                  int planner_type);

  ~RRT_ros_wrapper();

  void setup();

  ////Only for RRT as a local controller
  // void setup_controller(float controller_freq, float path_stddev, int planner_type);

  std::vector<geometry_msgs::PoseStamped> RRT_plan(bool exploration,
                                                   geometry_msgs::PoseStamped start,
                                                   geometry_msgs::PoseStamped goal,
                                                   float start_lin_vel, float start_ang_vel);

  float get_rrt_planning_radius();

  void visualizeTree(ros::Time t);

  void visualizeLeaves(ros::Time t);

  void publish_feature_costmap(ros::Time t);

  // void publish_gmm_costmap(geometry_msgs::PoseStamped person);

  void setWeights(std::vector<float> w)
  {
    checker_->setWeights(w);
  }

  /*void setUseLossFunc(bool l, std::vector<geometry_msgs::PoseStamped> path) {
    checker_->setUseLossFunc(l, path);
  }*/

  // For full path biasing using the kinodynamic RRT local controller
  // int RRT_local_plan(std::vector<geometry_msgs::PoseStamped> path_to_follow, float
  // start_lin_vel, float start_ang_vel, geometry_msgs::Twist& cmd_vel);


  // subscribe to the filtered pointcloud topic for the RRT
  // Get array of points ( std::vector<pcl::PointXYZ> data = cloud.points; )
  // Transform the pointXYZ to RRT::State and
  // call rrt_planner_->updateSamplingSpace(std::vector<RRT::State>* space)
  void setSamplingSpace();
  void pcCallback(const sensor_msgs::PointCloud2ConstPtr& msg);


  // void setBiasingPath(std::vector<geometry_msgs::PoseStamped>* path_to_follow);

  std::vector<geometry_msgs::PoseStamped> simple_path_smoothing(
      std::vector<geometry_msgs::PoseStamped>* path);

  // Planning service
  bool makePlanService(rrt_planners::MakePlan::Request& req,
                       rrt_planners::MakePlan::Response& res);
  // Change the RRT solve time service
  // bool changeTimeService(rrt_planners::ChangeSolveTime::Request &req,
  // rrt_planners::ChangeSolveTime::Response &res);
  void rrttimeCallback(const std_msgs::Float32ConstPtr& msg);
  bool change_rrt_time(float time);


  // std::vector<float> get_features_count(geometry_msgs::PoseStamped* goal,
  // std::vector<geometry_msgs::PoseStamped>* path, upo_msgs::PersonPoseArrayUPO* people);

  // std::vector<float> get_feature_counts(geometry_msgs::PoseStamped* goal,
  // std::vector<geometry_msgs::PoseStamped>* path);
  // std::vector<float> get_feature_counts(geometry_msgs::PoseStamped* goal,
  // std::vector<geometry_msgs::PoseStamped>* path,
  // std::vector<upo_msgs::PersonPoseArrayUPO>* people);

  float get_path_cost(geometry_msgs::PoseStamped* goal,
                      std::vector<geometry_msgs::PoseStamped>* path);
  float get_path_cost();


  std::vector<geometry_msgs::PoseStamped> path_interpolation(
      std::vector<geometry_msgs::PoseStamped> path, float step_distance);

  float get_xyz_tol()
  {
    return goal_xyz_tol_;
  };
  float get_th_tol()
  {
    return goal_th_tol_;
  };
  void get_vel_ranges(float& max_lin_vel, float& min_lin_vel, float& max_ang_vel, float& min_ang_vel)
  {
    max_lin_vel = max_lin_vel_;
    min_lin_vel = min_lin_vel_;
    max_ang_vel = max_ang_vel_;
    min_ang_vel = min_ang_vel_;
  };


  // bool set_approaching_gmm_sampling(float orientation, int num_samp,
  // geometry_msgs::PoseStamped person);


  inline float normalizeAngle(float val, float min, float max)
  {
    float norm = 0.0;
    if (val >= min)
      norm = min + fmod((val - min), (max - min));
    else
      norm = max - fmod((min - val), (max - min));

    return norm;
  }


  inline std::string get_robot_base_frame()
  {
    return robot_base_frame_;
  };
  inline std::string get_robot_odom_frame()
  {
    return robot_odom_frame_;
  };


private:
  // boost::recursive_mutex configuration_mutex_;
  ////boost::mutex reconf_mutex_;
  // dynamic_reconfigure::Server<rrt_planners::RRTRosWrapperConfig> *dsrv_;
  // void reconfigureCB(rrt_planners::RRTRosWrapperConfig &config, uint32_t level);

  // ROS
  // costmap_2d::Costmap2DROS* 		global_costmap_ros_;
  // costmap_2d::Costmap2DROS* 		local_costmap_ros_;  //The ROS wrapper for the costmap
  // the controller will use

  // costmap_2d::Costmap2D* 			global_costmap_;
  // costmap_2d::Costmap2D* 			local_costmap_;   //The costmap the controller will use

  //tf::TransformListener* tf_;
  tf2_ros::Buffer* tf_;


  // Robot
  float inscribed_radius_;
  float circumscribed_radius_;
  std::vector<geometry_msgs::Point> footprint_;

  std::string robot_base_frame_;
  std::string robot_odom_frame_;
  std::string robot_pc_sensor_frame_;
  std::string planning_frame_;

  // RRT
  RRT::Planner* rrt_planner_;
  float solve_time_;
  mutex solvetime_mutex_;
  // ros::ServiceServer 				solvetime_srv_;
  ros::Subscriber solvetime_sub_;
  std::vector<geometry_msgs::PoseStamped> rrt_plan_;
  RRT_ros::ValidityChecker3D* checker_;
  int rrt_planner_type_;
  ros::ServiceServer plan_srv_;



  bool use_external_pc_samples_;
  ros::Subscriber pc_sub_;
  sensor_msgs::PointCloud2 pc_;
  mutex pc_mutex_;


  int motionCostType_;

  float kino_timeStep_;


  // float							equal_path_percentage_;
  float path_cost_;

  // StateSpace
  int dimensions_;
  int dim_type_;
  float size_x_;
  float size_y_;
  float size_z_;

  // Visualization
  bool visualize_tree_;
  bool visualize_leaves_;
  bool visualize_costmap_;
  bool show_statistics_;
  bool show_intermediate_states_;
  float interpolate_path_distance_;
  ros::Publisher local_goal_pub_;
  ros::Publisher rrt_goal_pub_;
  ros::Publisher costmap_pub_;
  ros::Publisher tree_pub_;
  ros::Publisher leaves_pub_;
  ros::Publisher path_points_pub_;
  ros::Publisher path_interpol_points_pub_;

  // Path smoothing
  bool path_smoothing_;
  int smoothing_samples_;

  // GMM biasing
  // bool							gmm_biasing_;
  // float 							gmm_bias_;
  // ros::ServiceClient 				gmm_samples_client_;
  // ros::ServiceClient 				gmm_probs_client_;
  // std::vector< std::pair<float,float> > gmm_samples_;
  // geometry_msgs::PoseStamped		gmm_person_;
  // float 							gmm_person_ori_;
  // std::mutex 						gmm_mutex_;
  // ros::Publisher					gmm_costmap_pub_;


  //-------------------------------------------
  // bool							use_uva_lib_;
  // bool 							use_fc_costmap_;
  float goal_bias_;
  float max_range_;
  bool rrtstar_use_k_nearest_;
  bool rrtstar_first_path_biasing_;
  float rrtstar_first_path_bias_;
  float rrtstar_first_path_stddev_bias_;
  float rrtstar_rewire_factor_;
  bool full_path_biasing_;
  float full_path_stddev_;
  float full_path_bias_;

  int kino_minControlSteps_;
  int kino_maxControlSteps_;
  float kino_linAcc_;
  float kino_angAcc_;
  int kino_steeringType_;
  float xyz_res_;
  float yaw_res_;
  float min_lin_vel_;
  float max_lin_vel_;
  float lin_vel_res_;
  float max_ang_vel_;
  float min_ang_vel_;
  float ang_vel_res_;
  float goal_xyz_tol_;
  float goal_th_tol_;
  int nn_params_;
  int distanceType_;

  // Only for Quick-RRTstar
  int depth_;
};
}
#endif
