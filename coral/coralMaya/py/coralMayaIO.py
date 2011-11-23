
from maya import cmds, OpenMaya, OpenMayaAnim

def exportSkeleton(topNode, filename):
    fileContent = "coralIO:1.0\n"
    fileContent += "type:matrix\n"
    
    joints = cmds.listRelatives(topNode, allDescendents = True, type = "joint")
    
    interestingJoints = []
    for joint in joints:
        childrenJoints = cmds.listRelatives(joint, children = True, type = "joint")
        if childrenJoints is not None:
            interestingJoints.append(joint)
    
    fileContent += "elementsPerFrame:" + str(len(interestingJoints)) + "\n"
    
    start = cmds.playbackOptions(query = True, playbackStartTime = True)
    end = cmds.playbackOptions(query = True, playbackEndTime = True)
    
    fileContent += "frames:" + str(int(end - start)) + "\n"
    
    for frame in range(start, end):
        cmds.currentTime(frame)
        for joint in interestingJoints:
            xform = cmds.xform(joint, query = True, worldSpace = True, matrix = True)
            fileContent += str(xform).strip("[]").replace(", ", ",") + "\n"
    
    file = open(filename, "w")
    file.write(fileContent)
    file.close()
    
    print "coralIO: saved file " + filename

def exportSkinWeights(skinClusterNode, filename):
    fileContent = "coralIO:1.0\n"
    fileContent += "type:skinWeight\n"

    sel = OpenMaya.MSelectionList()
    sel.add(skinClusterNode)
    skinClusterObj = OpenMaya.MObject()
    sel.getDependNode(0, skinClusterObj)

    skinClusterFn = OpenMayaAnim.MFnSkinCluster(skinClusterObj)
    skinPath = OpenMaya.MDagPath()
    skinClusterFn.getPathAtIndex(0, skinPath)

    influences = OpenMaya.MDagPathArray()
    skinClusterFn.influenceObjects(influences)

    geoIt = OpenMaya.MItGeometry(skinPath)

    fileContent += "vertices:" + str(geoIt.count()) + "\n"
    fileContent += "deformers:" + str(influences.length()) + "\n"

    for i in range(geoIt.count()):
        comp = geoIt.component()

        weights = OpenMaya.MFloatArray()
        influenceIds = OpenMaya.MIntArray()

        infCount = OpenMaya.MScriptUtil()
        ccc = infCount.asUintPtr()

        skinClusterFn.getWeights(skinPath, comp, weights, ccc)
        for j in range(weights.length()):
            weight = weights[j]
            if weight > 0.0:
                fileContent += "vertex:" + str(i) + ",deformer:" + str(j) + ",weight:" + str(weights[j]) + "\n"

        geoIt.next()
    
    file = open(filename, "w")
    file.write(fileContent)
    file.close()
    
    print "coralIO: saved file " + filename

