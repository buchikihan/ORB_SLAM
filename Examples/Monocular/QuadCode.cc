#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <stdio.h>

#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <chrono>
#include <opencv2/core/core.hpp>
#include <System.h>

using namespace std;

int main(int argc, char **argv)
  {
  ifstream fin;
  // For looking in directory
  string   pathToImages, prevImages, filepath, newfilepath, pathToVocab, pathToConf, pathToStateCheck; //paths
  string   stateFile = "state.txt";
  DIR      *dp;              //points to the images directory
  struct   dirent *dirp;  //directory struct
  struct   stat filestat; //file status 
  typedef  vector<string> vec;
  vec      v;
  // For looping through images and maintating state machine
  int     size;          //number of images to process
  int     i;             //loop counter
  bool    run = true;  //exit loop
  int     hour,min,sec,mil;
  float   timestamp;

  //paths
  prevImages       = "/home/jono/src/ORB_image_folder/previous_data_set"; 
  pathToImages     = "/home/jono/src/ORB_image_folder/streaming_images";
  pathToVocab      = "Vocabulary/ORBvoc.txt";
  pathToConf       = "Examples/Monocular/TUM640480.yaml";
  pathToStateCheck = "";

  //delete remaining files in /streaming_images since they were only used to end the last process
  dp = opendir( pathToImages.c_str() );

  //while unread files in folder
  while ((dirp = readdir( dp )))
    {
    filepath = pathToImages + "/" + dirp->d_name;
    // If the file is a directory (or is in some way invalid) we'll skip it 
     if (stat( filepath.c_str(), &filestat )) continue;
     if (S_ISDIR( filestat.st_mode ))         continue;
     fin.open (filepath.c_str());
     if (fin.is_open())
        {
        fin.close();
        remove(filepath.c_str());
        }
    }
  

  //create the SLAM system
  ORB_SLAM2::System SLAM(pathToVocab,pathToConf,ORB_SLAM2::System::MONOCULAR,true);
  cout << "Ready for images" << endl;
  cv::Mat im;


  //continutally poll the directory for new images
  while(run)
      {
      sleep(0.01);
      fin.open(stateFile);
      //if(!fin.good())
     //     cout << "File not found yet?";
      while(!fin.eof())
          {
          char buf[1024];
          string exit = "exit";
          fin.getline(buf,1024);
          if(!exit.compare(buf))
              run = false;
          }
      dp = opendir( pathToImages.c_str() );

      //while unread files in folder
      while ((dirp = readdir( dp )))
          {
          filepath = pathToImages + "/" + dirp->d_name;
          // If the file is a directory (or is in some way invalid) we'll skip it 
          if (stat( filepath.c_str(), &filestat )) continue;
          if (S_ISDIR( filestat.st_mode ))         continue;
          fin.open (filepath.c_str());
          if (fin.is_open())
              {
              fin.close();
              v.push_back(dirp->d_name);
              }
          }
 
      //sort images and prepare for input to SLAM algorithm
      sort(v.begin(),v.end());
      size = v.size();
 
      //do for all current images
      for (i=0;i<size-1;i++)
          {
          if((not v[i].substr(0,3).compare("end"))||false)
              {
              cout << "Image sequence over" << endl;
              }
          else
              {
              //extract the timestamp from the image name
              //cout << v[i] << endl;
              hour = atoi(v[i].substr(0,2).c_str());
              min  = atoi(v[i].substr(3,2).c_str());
              sec  = atoi(v[i].substr(6,2).c_str());
              mil  = atoi(v[i].substr(9,6).c_str());
              timestamp = hour*60*60+min*60+sec+(mil/1000000.000000);

              //set the location of the image and read it
              filepath = pathToImages+"/"+v[i];
              im = cv::imread(filepath,CV_LOAD_IMAGE_UNCHANGED);
              if(im.empty())
                  {
                  cerr << endl << "Falied to load image" << endl;
                  }
              else
                  {
                  //Perform SLAM
                  SLAM.TrackMonocular(im,timestamp);

                  //move image to data_set folder once it has been used
                  newfilepath = prevImages+"/"+v[i];
                  rename(filepath.c_str(),newfilepath.c_str());
                  }
              }
          }
      v.clear();
      closedir( dp );
      }
      // Close down the SLAM process
      SLAM.Shutdown();
      //SLAM.SaveTrajectoryTUM("Trajectory.txt");
  return 0;
  }
