spectrometer_camera
August 2nd 2023
Buzz Walter

This program builds a Qt widget associated with a spectrometer design.  It does the general calculation for the calibrated 
instrument, and returns the center peak and the bandwidth.  It does this by capturing the center linear array of 
a ccd.

First a connection is established with a mightex camera using the MT_USBCamera_SDK.  Drivers for the camera
are necessary as well. Both of these downloads can be found on the mightex website.  Depending on which camera is
being used, the size of the <tempBuffer> variable will have to be different.  There is a commented out portion of 
code on line 103 that uses the MTUSB_GetFrameSetting to acquire information about the connected camera being used.
This will allow the user to adjust the code appropriately to grab the center line if one is trying to repurpose 
this for something else. Although ultimately this code isn't meant to be adjusted since it's intended for an in-situ 
spectrometer.

The widget is based on the Qt framework, a widget application in particular.  It uses version 5.15.2 along with the 
addition Qt Charts library. The cmake file has to be adjusted to include the addition of the Qt Charts library 
on top actually installing it using the qt installer executable.  

The file also saves the data into the directory where the program is stored along with its libraries.  