#!/usr/bin/env python

import roslib; roslib.load_manifest( 'rviz_algorithm_viewer' )
import rospy
from rviz_algorithm_viewer.msg import Cluster2, ClusterField
import geometry_msgs.msg
import random

def rand_cluster(n):
  clust_f = ClusterField()
  clust_f.name = "Cluster %d" % n

  points_nb = random.randint( 1, 100 )

  for i in range(points_nb):
    point = geometry_msgs.msg.Point()

    point.x = random.uniform( -10, 10 )
    point.y = random.uniform( -10, 10 )
    point.z = random.uniform( -10, 10 )

    clust_f.points.append( point )

  return clust_f

def gen_clusters():
  clust_msg = Cluster2()
  clust_msg.header.frame_id = "/base_link"
  clust_msg.header.stamp = rospy.Time.now()

  cluster_nb = random.randint( 1, 20 )

  for i in range(cluster_nb):
    clust_msg.clusters.append( rand_cluster(i) )

  return clust_msg

def send_clusters():
  pub = rospy.Publisher( 'test_cluster', Cluster2 )
  rospy.init_node( 'test_cluster' )

  rospy.loginfo( 'Cluster test started. Sending cluster points...' )

  while not rospy.is_shutdown():
    pub.publish( gen_clusters() )
    rospy.sleep( 1.0 )

if __name__ == '__main__':
  try:
    send_clusters()
  except rospy.ROSInterruptException:
    pass
