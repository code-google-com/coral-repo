
from maya import cmds

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
