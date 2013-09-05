#include <OGRE/OgreVector3.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreSceneManager.h>

#include <rviz/ogre_helpers/shape.h>

#include <geometry_msgs/Point.h>
#include <rviz_algorithm_viewer/ClusterField.h>

#include <cmath>

#include "cluster_visual.h"

namespace rviz_algorithm_viewer
{

ClusterVisual::ClusterVisual( Ogre::SceneManager* scene_manager, Ogre::SceneNode* parent_node )
{
  scene_manager_ = scene_manager;

  // Ogre::SceneNode s form a tree, with each node storing the
  // transform (position and orientation) of itself relative to its
  // parent.  Ogre does the math of combining those transforms when it
  // is time to render.
  //
  // Here we create a node to store the pose of the Cluster's header frame
  // relative to the RViz fixed frame.
  frame_node_ = parent_node->createChildSceneNode();
}

ClusterVisual::~ClusterVisual()
{
  // Destroy the frame node since we don't need it anymore.
  scene_manager_->destroySceneNode( frame_node_ );
}

void ClusterVisual::setMessage( const rviz_algorithm_viewer::Cluster2::ConstPtr& msg )
{
  // Remove all the previous points and clusters from the display
  clusters_.clear();

  // Add every cluster of the message to the display
  std::vector<rviz_algorithm_viewer::ClusterField>::const_iterator clust_it  = msg->clusters.begin();
  std::vector<rviz_algorithm_viewer::ClusterField>::const_iterator clust_end = msg->clusters.end();
  for (; clust_it != clust_end; ++clust_it)
  {
    ClusterPointsPtr cluster_ptr(new ClusterPoints( scene_manager_, frame_node_ ));

    // Add to this cluster every points found in the message
    std::vector<geometry_msgs::Point>::const_iterator pts_it  = (*clust_it).points.begin();
    std::vector<geometry_msgs::Point>::const_iterator pts_end = (*clust_it).points.end();
    //float red, green, blue;
    for (; pts_it != pts_end; ++pts_it)
    {
      Ogre::Vector3 pos( 
          (*pts_it).x, 
          (*pts_it).y, 
          (*pts_it).z );

      cluster_ptr->addPoint( pos );

      //red   = (pos.x + 20) / 40.;
      //green = (pos.y + 20) / 40.;
      //blue  = (pos.z + 20) / 40.;
    }

    cluster_ptr->displayEnvelope();
    //cluster_ptr->setColor( red, green, blue, 0.5 );
    clusters_.push_back( cluster_ptr );
  }
}

// Position and orientation are passed through to the SceneNode.
void ClusterVisual::setFramePosition( const Ogre::Vector3& position )
{
  frame_node_->setPosition( position );
}

void ClusterVisual::setFrameOrientation( const Ogre::Quaternion& orientation )
{
  frame_node_->setOrientation( orientation );
}

// Color is passed through to all the Shape objects.
void ClusterVisual::setColor( float r, float g, float b, float a )
{
  std::vector<ClusterPointsPtr>::iterator it = clusters_.begin();
  std::vector<ClusterPointsPtr>::iterator end = clusters_.end();
  for (; it != end; ++it)
  {
    (*it)->setColor( r, g, b, a );
  }
}

void ClusterVisual::setRadius( float r )
{
  radius_ = r;
}

float ClusterVisual::radius_ = 0.2;


ClusterVisual::ClusterPoints::ClusterPoints( Ogre::SceneManager* scene_manager, Ogre::SceneNode* parent_node )
{
  scene_manager_ = scene_manager;
  frame_node_ = parent_node->createChildSceneNode();
}

ClusterVisual::ClusterPoints::~ClusterPoints()
{
  scene_manager_->destroySceneNode( frame_node_ );
}

void ClusterVisual::ClusterPoints::addPoint( Ogre::Vector3 position )
{
  // We create the sphere object within the frame node so that we can
  // set its position and direction relative to its header frame.
  PointPtr pts(new rviz::Shape( rviz::Shape::Sphere, scene_manager_, frame_node_));

  Ogre::Vector3 scale( radius_, radius_, radius_ );
  pts->setScale( scale );
  pts->setPosition( position );

  points_.push_back( pts );
}

void ClusterVisual::ClusterPoints::setColor( float r, float g, float b, float a )
{
  std::vector<PointPtr>::iterator it = points_.begin();
  std::vector<PointPtr>::iterator end = points_.end();
  for (; it != end; ++it)
  {
    (*it)->setColor( r, g, b, a );
  }

  envelope_->setColor( r, g, b, a );
}

#include "Miniball.hpp"

void ClusterVisual::ClusterPoints::displayEnvelope()
{
  envelope_.reset( new rviz::Shape( rviz::Shape::Sphere, scene_manager_, frame_node_) );

  // Convert vector of rviz::Shape to vector of array 
  // to allow iteration over coordinates
  std::vector<float*> vp;
  std::vector<PointPtr>::iterator it = points_.begin();
  std::vector<PointPtr>::iterator end = points_.end();
  for (; it != end; ++it)
  {
    // The coordinates are stored in an array
    float* p = new float[3];
    Ogre::Vector3 pos = (*it)->getPosition();
    p[0] = pos.x;
    p[1] = pos.y;
    p[2] = pos.z;

    vp.push_back(p);
  }

  // define the types of iterators through the points and their coordinates
  typedef std::vector<float*>::const_iterator PointIterator;
  typedef const float* CoordIterator;

  // Compute bounding sphere center and radius
  typedef Miniball::
    Miniball <Miniball::CoordAccessor<PointIterator, CoordIterator> >
    MB;
  MB mb ( 3, vp.begin(), vp.end() ); // 3 is the dimension of the points

  // Convert Miniball position and diameter into Ogre::Vector3
  float d = 2 * std::sqrt( mb.squared_radius() );
  Ogre::Vector3 diameter( d, d, d );

  const float* center = mb.center();
  float x = *(center++);
  float y = *(center++);
  float z = *center;
  Ogre::Vector3 centerPos( x, y, z );

  // Update envelope with computed diameter and position
  envelope_->setScale( diameter );
  envelope_->setPosition( centerPos );

  // clean up
  for (std::vector<float*>::iterator it = vp.begin(); it != vp.end(); ++it) {
    delete[] *it;
  }
}

} // end namespace rviz_algorithm_viewer
