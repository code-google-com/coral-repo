#!/bin/bash

/usr/local/bin/scons -f "buildMayaPlugin.py"
if [ "$?" -eq "0" ]
then
	echo "compilation successful, running coral-maya"
	export PYTHONPATH=$PYTHONPATH:"/Users/andrea/dev/coral-repo/coral/build/coralMayaBuild"
	
	export DYLD_LIBRARY_PATH="/Users/andrea/dev/external/boost_1_46_1_maya"
	export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:"/Users/andrea/dev/external/tbb40_20110809oss/lib"
	export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:"/Users/andrea/dev/external/ilmbase-1.0.1/libs"
	export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:"/Users/andrea/dev/coral-repo/coral/build/coralMayaBuild/coralMaya"
	export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:"/Users/andrea/dev/coral-repo/coral/build/coralMayaBuild/coral"
	export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:"/Users/andrea/dev/coral-repo/coral/build/coralMayaBuild/coral/coralUi"
	export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:"/Applications/Autodesk/maya2012/Maya.app/Contents/Frameworks/Python.framework/Versions/2.6"
	export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:"/Applications/Autodesk/maya2012/Maya.app/Contents/MacOS"
	export CORAL_STARTUP_SCRIPT="/Users/andrea/dev/coral-repo/coral/myCoralMayaStartup.py"
	clear
	echo "coral maya is running..."
	/Applications/Autodesk/maya2012/Maya.app/Contents/bin/maya
else
	echo "failed compiling, mo so cazzi tua."
fi

