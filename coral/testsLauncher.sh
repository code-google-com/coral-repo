#!/bin/bash

/usr/local/bin/scons -f "buildStandalone.py"
if [ "$?" -eq "0" ]
then
	echo "compilation successful, running coral-standalone"
	export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:"/Users/andrea/dev/external/tbb40_20110809oss/lib"
	export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:"/Users/andrea/dev/external/boost_1_46_1/stage/lib"
	export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:"/Users/andrea/dev/external/ilmbase-1.0.1/libs"
	export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:"/Users/andrea/dev/coral-repo/coral/build/coralStandaloneBuild/coral"
	export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:"/Library/Frameworks/Python.framework/Versions/2.6"
	
	export CORAL_STARTUP_SCRIPT="/Users/andrea/dev/coral-repo/coral/coralStandaloneBuild/myCoralStandaloneStartup.py"
	clear; 
	/Library/Frameworks/Python.framework/Versions/2.6/bin/python2.6 build/coralStandaloneBuild/coralPythonTests.py
else
	echo "failed compiling, mo so cazzi tua."
fi
