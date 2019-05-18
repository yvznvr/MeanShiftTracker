# Mean Shift Tracker
#### Mean Shift Tracker Algorithm implementation with C++ 

Türkçe Blog Yazısı: https://medium.com/@yavuzunver/mean-shift-algoritmas%C4%B1-ile-nesne-takibi-de70c625b161
English Paper: http://comaniciu.net/Papers/KernelTracking.pdf

[![Watch the video](https://i.ytimg.com/vi/RZAF1Q7nYrc/hqdefault.jpg)](https://youtu.be/RZAF1Q7nYrc)

Example
``` c
// t0 previous, t1 current frame
tracker = new MeanShiftTracker; // create object
int centerPoints[2]; // array which takes object's new location
...
tracker->setFrame(640, 480, t0); // set width, height and previous(t0) frame
tracker->setArea(coordinat[0],coordinat[1],coordinat[2], coordinat[3]); // set coordinates of object
tracker->tracking(t1, centerPoints); // set current frame and empty array

// centerPoints include center point of object's new location

// if you want set different object from previous, you must call following function
tracker->setSelected(false);
```   
  
