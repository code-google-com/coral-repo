# <license>
# Copyright (C) 2011 Andrea Interguglielmi, All rights reserved.
# This file is part of the coral repository downloaded from http://code.google.com/p/coral-repo.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
# 
#    * Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
# IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# </license>

import os

#standalone config
os.environ["CORAL_PARALLEL"] = "CORAL_PARALLEL_TBB"

os.environ["CORAL_PYTHON_PATH"] = "/Library/Frameworks/Python.framework/Versions/2.6"
os.environ["CORAL_PYTHON_INCLUDES_PATH"] = "/Library/Frameworks/Python.framework/Versions/2.6/include/python2.6"
os.environ["CORAL_PYTHON_LIB"] = "python2_6_7"
os.environ["CORAL_PYTHON_LIBS_PATH"] = "/Library/Frameworks/Python.framework/Versions/2.6"

os.environ["CORAL_BOOST_INCLUDES_PATH"] = "/Users/andrea/dev/external/boost_1_46_1/boost"
os.environ["CORAL_BOOST_LIBS_PATH"] = "/Users/andrea/dev/external/boost_1_46_1/stage/lib"
os.environ["CORAL_BOOST_PYTHON_LIB"] = "boost_python"

os.environ["CORAL_IMATH_INCLUDES_PATH"] = "/Users/andrea/dev/external/ilmbase-1.0.1/include"
os.environ["CORAL_IMATH_LIBS_PATH"] = "/Users/andrea/dev/external/ilmbase-1.0.1/libs"
os.environ["CORAL_IMATH_LIB"] = "Imath.6"
os.environ["CORAL_IMATH_IEX_LIB"] = "Iex.6"

os.environ["CORAL_TBB_INCLUDES_PATH"] = "/Users/andrea/dev/external/tbb40_20110809oss/include"
os.environ["CORAL_TBB_LIBS_PATH"] = "/Users/andrea/dev/external/tbb40_20110809oss/lib"
os.environ["CORAL_TBB_LIB"] = "tbb"

# qt libs
os.environ["CORAL_Qt3Support_LIB"] = "/Library/Frameworks/Qt3Support.framework/Versions/4/Qt3Support"
os.environ["CORAL_QtCore_LIB"] = "/Library/Frameworks/QtCore.framework/Versions/4/QtCore"
os.environ["CORAL_QtDBus_LIB"] = "/Library/Frameworks/QtDBus.framework/Versions/4/QtDBus"
os.environ["CORAL_QtDeclarative_LIB"] = "/Library/Frameworks/QtDeclarative.framework/Versions/4/QtDeclarative"
os.environ["CORAL_QtDesigner_LIB"] = "/Library/Frameworks/QtDesigner.framework/Versions/4/QtDesigner"
os.environ["CORAL_QtDesignerComponents_LIB"] = "/Library/Frameworks/QtDesignerComponents.framework/Versions/4/QtDesignerComponents"
os.environ["CORAL_QtGui_LIB"] = "/Library/Frameworks/QtGui.framework/Versions/4/QtGui"
os.environ["CORAL_QtHelp_LIB"] = "/Library/Frameworks/QtHelp.framework/Versions/4/QtHelp"
os.environ["CORAL_QtMultimedia_LIB"] = "/Library/Frameworks/QtMultimedia.framework/Versions/4/QtMultimedia"
os.environ["CORAL_QtNetwork_LIB"] = "/Library/Frameworks/QtNetwork.framework/Versions/4/QtNetwork"
os.environ["CORAL_QtOpenGL_LIB"] = "/Library/Frameworks/QtOpenGL.framework/Versions/4/QtOpenGL"
os.environ["CORAL_QtScript_LIB"] = "/Library/Frameworks/QtScript.framework/Versions/4/QtScript"
os.environ["CORAL_QtScriptTools_LIB"] = "/Library/Frameworks/QtScriptTools.framework/Versions/4/QtScriptTools"
os.environ["CORAL_QtSql_LIB"] = "/Library/Frameworks/QtSql.framework/Versions/4/QtSql"
os.environ["CORAL_QtSvg_LIB"] = "/Library/Frameworks/QtSvg.framework/Versions/4/QtSvg"
os.environ["CORAL_QtTest_LIB"] = "/Library/Frameworks/QtTest.framework/Versions/4/QtTest"
os.environ["CORAL_QtWebKit_LIB"] = "/Library/Frameworks/QtWebKit.framework/Versions/4/QtWebKit"
os.environ["CORAL_QtXml_LIB"] = "/Library/Frameworks/QtXml.framework/Versions/4/QtXml"
os.environ["CORAL_QtXmlPatterns_LIB"] = "/Library/Frameworks/QtXmlPatterns.framework/Versions/4/QtXmlPatterns"

# sdk config
os.environ["CORAL_INCLUDES_PATH"] = "/Users/andrea/dev/coral-repo/coral/build/coralStandaloneBuild/sdk/coral/includes"
os.environ["CORALUI_INCLUDES_PATH"] = "/Users/andrea/dev/coral-repo/coral/build/coralStandaloneBuild/sdk/coralUi/includes"
os.environ["CORAL_LIBS_PATH"] = "/Users/andrea/dev/coral-repo/coral/build/coralStandaloneBuild/sdk/coral/libs"
os.environ["CORALUI_LIBS_PATH"] = "/Users/andrea/dev/coral-repo/coral/build/coralStandaloneBuild/sdk/coralUi/libs"

# maya config
if os.environ.has_key("CORAL_BUILD_FLAVOUR"):
    if os.environ["CORAL_BUILD_FLAVOUR"] == "coralMaya":
        os.environ["CORAL_MAYA_INCLUDES_PATH"] = "/Applications/Autodesk/maya2012/devkit/include"
        os.environ["CORAL_MAYA_LIBS_PATH"] = "/Applications/Autodesk/maya2012/Maya.app/Contents/MacOS/"
    
        os.environ["CORAL_PYTHON_LIB"] = "mayapython"
        os.environ["CORAL_PYTHON_INCLUDES_PATH"] = "/Applications/Autodesk/maya2012/Maya.app/Contents/Frameworks/Python.framework/Versions/2.6/include/python2.6"
        os.environ["CORAL_PYTHON_LIBS_PATH"] = "/Applications/Autodesk/maya2012/Maya.app/Contents/Frameworks/Python.framework/Versions/2.6"
    
        os.environ["CORAL_BOOST_LIBS_PATH"] = "/Users/andrea/dev/external/boost_1_46_1_maya"
        os.environ["CORAL_BOOST_PYTHON_LIB"] = "boost_python_maya"
    
        os.environ["CORAL_TBB_INCLUDES_PATH"] = "/Users/andrea/dev/external/tbb30_u5/include"
        os.environ["CORAL_TBB_LIBS_PATH"] = "/Applications/Autodesk/maya2012/Maya.app/Contents/MacOS/"
        os.environ["CORAL_TBB_LIB"] = "tbb"
    
        os.environ["CORAL_PARALLEL"] = ""
