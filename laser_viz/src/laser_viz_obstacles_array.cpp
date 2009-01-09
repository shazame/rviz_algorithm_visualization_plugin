// %Tag(FULLTEXT)%
#include <ros/ros.h>
#include <visualization_msgs/MarkerArray.h>
#include "sensor_msgs/LaserScan.h"

#include <cmath>

#include "SquareMarkerArray.hpp"

ros::Publisher obstacle_pub;

void obstacleHandleCallback(const sensor_msgs::LaserScan::ConstPtr& msg)
{
//  static int run = 0;
//  if ((run = (run+1)%10) != 0) return;
  SquareMarkerArray obstacles("/laser", "laser_viz_obstacles");
  uint32_t i = 0;

  for (float theta = msg->angle_min; theta < msg->angle_max; theta += 10 * msg->angle_increment) {

    float r = msg->ranges[i];
    float x = r * cos(theta);     
    float y = r * sin(theta); 

    obstacles.add(x, y, r/14.);
    i += 10;
  }

  obstacles.pub(obstacle_pub);
}

int main( int argc, char** argv )
{
  ros::init(argc, argv, "laser_viz_obstacles");

  ros::NodeHandle n;

  obstacle_pub = n.advertise<visualization_msgs::MarkerArray>("visualization_obstacle", 100);

  ros::Subscriber sub = n.subscribe("scan", 100, obstacleHandleCallback);

  ros::spin();

  return 0;
}
// %EndTag(FULLTEXT)%