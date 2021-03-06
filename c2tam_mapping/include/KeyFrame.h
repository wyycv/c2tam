/*
*
* This file is part of the c2tam project
*
* c2tam is free software: you can redistribute it and/or modify 
* it under the terms of the GNU General Public License as published by 
* the Free Software Foundation, either version 3 of the License, or 
* (at your option) any later version.
*
* c2tam is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License 
* along with c2tam. If not, see http://www.gnu.org/licenses/.
*
*/

#ifndef __KEYFRAME_H
#define __KEYFRAME_H
#include <TooN/TooN.h>
using namespace TooN;
#include <TooN/se3.h>
#include <cvd/image.h>
#include <cvd/byte.h>
#include <vector>
#include <set>
#include <map>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

class MapPoint;
class SmallBlurryImage;

#define LEVELS 4

struct SVertex
{
  GLfloat x,y,z;
  GLfloat r,g,b;
};


// Candidate: a feature in an image which could be made into a map point
struct Candidate
{
  CVD::ImageRef irLevelPos;
  Vector<2> v2RootPos;
  double dSTScore;
};


// Surf
struct Surf
{
  CVD::ImageRef irLevelPos;
  int size;
  int index;
  //Vector<2> v2RootPos;
  //double dSTScore;

};






struct rgbd
{
  int r;
  int g;
  int b;
  double z;
  double x;
  double y;
};

// Measurement: A 2D image measurement of a map point. Each keyframe stores a bunch of these.
struct Measurement
{
  int nLevel;   // Which image level?
  bool bSubPix; // Has this measurement been refined to sub-pixel level?
  Vector<2> v2RootPos;  // Position of the measurement, REFERED TO PYRAMID LEVEL ZERO
  enum {SRC_TRACKER, SRC_REFIND, SRC_ROOT, SRC_TRAIL, SRC_EPIPOLAR} Source; // Where has this measurement come frome?
};

// Each keyframe is made of LEVELS pyramid levels, stored in struct Level.
// This contains image data and corner points.
struct Level
{
  inline Level()
  {
    bImplaneCornersCached = false;
  };
  
  CVD::Image<CVD::byte> im;                // The pyramid level pixels
  std::vector<CVD::ImageRef> vCorners;     // All FAST corners on this level
  std::vector<int> vCornerRowLUT;          // Row-index into the FAST corners, speeds up access
  std::vector<CVD::ImageRef> vMaxCorners;  // The maximal FAST corners
  Level& operator=(const Level &rhs);
  
  std::vector<Candidate> vCandidates;   // Potential locations of new map points
//  std::vector<Candidate> vCandidatesOD; // ObjectDetection
  std::vector<Surf> vSurfOD;


  bool bImplaneCornersCached;           // Also keep image-plane (z=1) positions of FAST corners to speed up epipolar search
  std::vector<Vector<2> > vImplaneCorners; // Corner points un-projected into z=1-plane coordinates
};

// The actual KeyFrame struct. The map contains of a bunch of these. However, the tracker uses this
// struct as well: every incoming frame is turned into a keyframe before tracking; most of these 
// are then simply discarded, but sometimes they're then just added to the map.
struct KeyFrame
{
  inline KeyFrame()
  {
    pSBI = NULL;
    rgbdData.reserve(640*480);

  }
  SE3<> se3CfromW;    // The coordinate frame of this key-frame as a Camera-From-World transformation
  bool bFixed;      // Is the coordinate frame of this keyframe fixed? (only true for first KF!)
  bool bSemantic;   // ObjectDetection
  bool bEpipolar;   // ObjectDetection

  Level aLevels[LEVELS];  // Images, corners, etc lives in this array of pyramid levels
  std::map<MapPoint*, Measurement> mMeasurements;           // All the measurements associated with the keyframe
  std::map<MapPoint*, Surf> mMeasurementsOD; 
  void MakeKeyFrame_Lite(CVD::BasicImage<CVD::byte> &im);   // This takes an image and calculates pyramid levels etc to fill the 
                                                            // keyframe data structures with everything that's needed by the tracker..
  void MakeKeyFrame_Rest();                                 // ... while this calculates the rest of the data which the mapmaker needs.
  
  double dSceneDepthMean;      // Hacky hueristics to improve epipolar search.
  double dSceneDepthSigma;
  
  SmallBlurryImage *pSBI; // The relocaliser uses this
  int color;
  int index;
  SVertex *denseModel;
  std::vector<rgbd> rgbdData;

  //int myDensePoints;

//  SE3<> Tco;
//  std::vector<Vector<2>> vp2DPoints;
//  std::vector<Vector<3>> vp3DPoints;

};

typedef std::map<MapPoint*, Measurement>::iterator meas_it;  // For convenience, and to work around an emacs paren-matching bug

struct sMeasurement
{
  Measurement m;
  int indexPointMap;
};

struct sCandidate
{
  std::vector<Surf> vCandidatesOD;
};


struct sKeyFrame
{
  inline sKeyFrame()
  {
    rgbdData.reserve(640*480);
  }
  CVD::Image<CVD::byte> imFrame;
  SE3<> se3CfromW;    
  bool bFixed;      
  bool bSemantic;   // ObjectDetection
  std::vector<sMeasurement> sMeasurements;
  std::vector<sCandidate> sCandidates;
  double dSceneDepthMean;     
  double dSceneDepthSigma;
  std::vector<rgbd> rgbdData;

  SE3<> Tco;

  std::vector<Vector<2> > vp2DPoints;
  std::vector<Vector<3> > vp3DPoints;

  std::string name;
  double depth;  

};



#endif

