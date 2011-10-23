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


import weakref
from PyQt4 import QtGui, QtCore

from ... import coralApp
from ...observer import Observer
from ..nodeEditor import nodeEditor
from .. import mainWindow
import fields


class SpecializationPresetCombo(QtGui.QWidget):
    def __init__(self, parent):
        QtGui.QWidget.__init__(self, parent)
        
        self._label = QtGui.QLabel("specialization preset:", self)
        self._combo = QtGui.QComboBox(self)
        
        self.setLayout(QtGui.QHBoxLayout(self))
        self.layout().setContentsMargins(0, 0, 0, 0)
        self.layout().setSpacing(0)
        self.layout().addWidget(self._label)
        self.layout().addWidget(self._combo)

class NodeInspectorWidget(QtGui.QWidget):
    def __init__(self, coralNode, parent):
        QtGui.QWidget.__init__(self, parent)
        
        self._mainLayout = QtGui.QVBoxLayout(self)
        self._coralNode = weakref.ref(coralNode)
        self._nameField = None
        self._nameEditable = False
        self._attributeWidgets = {}
        
        self.setLayout(self._mainLayout)
        self._mainLayout.setContentsMargins(0, 0, 0, 0)
        self._mainLayout.setSpacing(2)
    
    def attributeWidget(self, name):
        widget = None
        if self._attributeWidgets.has_key(name):
            if self._attributeWidgets[name]() is not None:
                widget = self._attributeWidgets[name]()
        
        return widget
    
    def setNameEditable(self, value = True):
        self._nameEditable = value
    
    def coralNode(self):
        node = None
        if self._coralNode:
            node = self._coralNode()
        
        return node
    
    def _hasDefaultSpecialization(self, coralNode):
        for attr in coralNode.attributes():
            if attr.defaultSpecialization():
                return True
        
        return False
    
    def _nodeIsConnected(self, coralNode):
        for attr in coralNode.attributes():
            if attr.input():
                return True
            elif attr.outputs():
                return True
        
        return False
    
    def _updatePresetCombo(self):
        coralNode = self._coralNode()
        
        presets = coralNode.specializationPresets()
        
        if len(presets) > 1:
            presetCombo = SpecializationPresetCombo(self)
            for preset in presets:
                presetCombo._combo.addItem(preset)
            
            currentPreset = coralNode.enabledSpecializationPreset()
            presetCombo._combo.setCurrentIndex(presets.index(currentPreset))
            
            self.layout().addWidget(presetCombo)
            self.connect(presetCombo._combo, QtCore.SIGNAL("currentIndexChanged(QString)"), self._presetComboChanged)
            
            if self._nodeIsConnected(coralNode):
                presetCombo._combo.setDisabled(True)
            
    def build(self):
        coralNode = self.coralNode()
        
        if self._nameEditable:
            self._nameField = fields.NameField(coralNode, self)
            self.layout().addWidget(self._nameField)
        
        self._updatePresetCombo()
            
        for attribute in coralNode.attributes():
            if attribute.name().startswith("_") == False:
                className = attribute.className()
                if NodeInspector._inspectorWidgetClasses.has_key(className):
                    inspectorWidgetClass = NodeInspector._inspectorWidgetClasses[className]
                    inspectorWidget = inspectorWidgetClass(attribute, self)
                    self._attributeWidgets[attribute.name()] = weakref.ref(inspectorWidget)
                    self._mainLayout.addWidget(inspectorWidget)
                    inspectorWidget.build()
    
    def _presetComboChanged(self, preset):
        self.coralNode().enableSpecializationPreset(str(preset))

class ProxyAttributeInspectorWidget(QtGui.QWidget):
    def __init__(self, coralAttribute, parent):
        QtGui.QWidget.__init__(self, parent)
        
        self._mainLayout = QtGui.QVBoxLayout(self)
        self._coralAttribute = weakref.ref(coralAttribute)
        self._nameField = None
        
        self.setLayout(self._mainLayout)
        self._mainLayout.setContentsMargins(0, 0, 0, 0)
        self._mainLayout.setSpacing(0)
    
    def build(self):
        coralAttribute = self._coralAttribute()
        
        self._nameField = fields.NameField(coralAttribute, self)
        self.layout().addWidget(self._nameField)
        
class AttributeInspectorWidget(QtGui.QWidget):
    def __init__(self, coralAttribute, parent):
        QtGui.QWidget.__init__(self, parent)
        
        self._mainLayout = QtGui.QVBoxLayout(self)
        self._coralAttribute = weakref.ref(coralAttribute)
        
        self.setLayout(self._mainLayout)
        self._mainLayout.setContentsMargins(0, 0, 0, 0)
        self._mainLayout.setSpacing(0)
    
    def coralAttribute(self):
        attr = None
        if self._coralAttribute:
            attr = self._coralAttribute()
        
        return attr
    
    def build(self):
        pass

class NodeInspectorHeader(QtGui.QWidget):
    def __init__(self, parent):
        QtGui.QWidget.__init__(self, parent)
        
        self._mainLayout = QtGui.QHBoxLayout(self)
        self._lockButton = QtGui.QToolButton(self)
        self._classNameLabel = QtGui.QLabel(self)
        
        self.setLayout(self._mainLayout)
        self._mainLayout.setContentsMargins(0, 0, 0, 0)
        self._mainLayout.setAlignment(QtCore.Qt.AlignRight)
        self._lockButton.setText("lock")
        self._lockButton.setCheckable(True)
        self._mainLayout.addWidget(self._classNameLabel)
        self._mainLayout.addWidget(self._lockButton)
    
    def lockButton(self):
        return self._lockButton

class NodeInspector(QtGui.QWidget):
    _externalThreadActive = False
    _inspectorWidgetClasses = {}
    
    @staticmethod
    def externalThreadActive():
        return NodeInspector._externalThreadActive
    
    @staticmethod
    def setExternalThreadActive(value = True):
        NodeInspector._externalThreadActive = value
        mainWin = mainWindow.MainWindow.globalInstance()
        mainWin.emit(QtCore.SIGNAL("coralExternalThreadActive(bool)"), value)
    
    @staticmethod
    def registerInspectorWidget(nestedObjectClassName, inspectorWidgetClass):
        NodeInspector._inspectorWidgetClasses[nestedObjectClassName] = inspectorWidgetClass
    
    def __init__(self, parent):
        QtGui.QWidget.__init__(self, parent)
        
        self._mainLayout = QtGui.QVBoxLayout(self)
        self._header = NodeInspectorHeader(self)
        self._contentWidget = QtGui.QWidget(self)
        self._contentLayout = QtGui.QVBoxLayout(self._contentWidget)
        self._isLocked = False
        self._selectedNodesChangedObserver = Observer()
        self._nodeConnectionChangedObserver = Observer()
        self._inspectorWidget = None
        self._node = None
        
        self.setLayout(self._mainLayout)
        self.setWindowTitle("node inspector")
        self._mainLayout.setContentsMargins(5, 5, 5, 5)
        self._mainLayout.setAlignment(QtCore.Qt.AlignTop)
        
        self._contentWidget.setLayout(self._contentLayout)
        self._contentLayout.setContentsMargins(0, 0, 0, 0)
        
        self._mainLayout.addWidget(self._header)
        self._mainLayout.addWidget(self._contentWidget)
        
        self.connect(self._header.lockButton(), QtCore.SIGNAL("toggled(bool)"), self.lock)
        
        nodeEditor.NodeEditor.addSelectedNodesChangedObserver(self._selectedNodesChangedObserver, self._selectionChanged)
        
        self._selectionChanged()
    
    def _nodeConnectionChanged(self):
        self.clear()
        if self._node:
            self._rebuild(self._node())
    
    def clear(self):
        self._header._classNameLabel.setText("")
        if self._inspectorWidget:
            self._inspectorWidget.setParent(None)
            self._inspectorWidget = None
    
    def _rebuild(self, node = None, attribute = None):
        if node:
            inspectorWidgetClass = NodeInspectorWidget
            
            className = node.className()
        
            if NodeInspector._inspectorWidgetClasses.has_key(className):
                inspectorWidgetClass = NodeInspector._inspectorWidgetClasses[className]
        
            self._inspectorWidget = inspectorWidgetClass(node, self)
            self._inspectorWidget.setNameEditable(True)
            self._inspectorWidget.build()
            self._contentLayout.addWidget(self._inspectorWidget)
            self._header._classNameLabel.setText(node.className())
        
            coralApp.addNodeConnectionChangedObserver(self._nodeConnectionChangedObserver, node, self._nodeConnectionChanged)
        
        elif attribute:
            self._inspectorWidget = ProxyAttributeInspectorWidget(attribute, self)
            self._inspectorWidget.build()
            self._contentLayout.addWidget(self._inspectorWidget)
                
    def _selectionChanged(self):
        if self._isLocked:
            return
        
        self.clear()
        self._node = None
        self._nodeConnectionChangedObserver = Observer()
        
        nodes = nodeEditor.NodeEditor.selectedNodes()
        
        if nodes:
            node = nodes[0]
            self._node = weakref.ref(node)
            self._rebuild(node)
        else:
            attributes = nodeEditor.NodeEditor.selectedAttributes()
            
            if attributes:
                attr = attributes[0]
                self._rebuild(attribute = attr)
        
    def lock(self, value):
        self._isLocked = value
        
        if not self._isLocked:
            self._selectionChanged()
    
    def sizeHint(self):
        return QtCore.QSize(200, 500)
