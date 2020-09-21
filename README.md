# bothered-scanner
3D scanner based on MakerScanner's mode of operation but with corrected maths and additional features.

This is a Qt based application that provides camera control interfaces as well as all the functionality for scanning an item by measuring the displacement of a laser line as it is swept over the item. The camera and object remain stationary while being scanned and a laser line emitter with a fixed rotation point relative to the camera is swept back and forth over the object for the camera to record the data.