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

import nodeInspector
from .. import mainWindow
from ... import coralApp
from ... import utils
from ...observer import Observer

class ObjectField(QtGui.QWidget):
    def __init__(self, label, coralObject, parentWidget):
        QtGui.QWidget.__init__(self, parentWidget)
        
        self._mainLayout = QtGui.QHBoxLayout(self)
        self._label = QtGui.QLabel(label, self)
        self._valueWidget = None
        self._coralObject = weakref.ref(coralObject)
        
        self.setLayout(self._mainLayout)
        self._mainLayout.setContentsMargins(0, 0, 0, 0)
        self._mainLayout.addWidget(self._label)
        
        self.connect(mainWindow.MainWindow.globalInstance(), QtCore.SIGNAL("coralExternalThreadActive(bool)"), self.setExternalThreadSpinning)
    
    def setExternalThreadSpinning(self, value, force = False):
        if self.valueWidget().isEnabled() == False or force == True:
            if value:
                self.valueWidget().setVisible(False)
                self.label().setText(self.label().text() + ": spinning")
            else:
                self.label().setText(str(self.label().text()).strip(": spinning"))
                self.valueWidget().setVisible(True)
    
    def label(self):
        return self._label
    
    def valueWidget(self):
        return self._valueWidget
    
    def setValueWidget(self, widget, endOfEditSignal, endOfEditCallback):
        self._valueWidget = widget
        self._mainLayout.addWidget(widget)
        
        self.connect(self._valueWidget, QtCore.SIGNAL(endOfEditSignal), endOfEditCallback)
    
    def coralObject(self):
        return self._coralObject()
    

class CustomDoubleSpinBox(QtGui.QDoubleSpinBox):
    def __init__(self, parent):
        QtGui.QDoubleSpinBox.__init__(self, parent)
        
        self._wheelCallback = None
    
    def wheelEvent(self, wheelEvent):
        QtGui.QDoubleSpinBox.wheelEvent(self, wheelEvent)
        
        self._wheelCallback()

class FloatValueField(ObjectField):
    def __init__(self, coralAttribute, parentWidget):
        ObjectField.__init__(self, coralAttribute.name(), coralAttribute, parentWidget)
        
        self._valueChangedObserver = Observer()
        self._connectedInputObserver = Observer()
        self._disconnectedInputObserver = Observer()
        
        self.setValueWidget(CustomDoubleSpinBox(self), "editingFinished()", self._valueWidgetChanged)
        self.valueWidget()._wheelCallback = utils.weakRef(self._valueWidgetChanged)
        self.valueWidget().setRange(-99999999999.0, 99999999999.0)
        self.valueWidget().setSingleStep(0.1)
        
        coralApp.addAttributeValueChangedObserver(self._valueChangedObserver, coralAttribute, self._valueChanged)
        coralApp.addConnectedInputObserver(self._connectedInputObserver, coralAttribute, self._inputConnectionChanged)
        coralApp.addDisconnectedInputObserver(self._disconnectedInputObserver, coralAttribute, self._inputConnectionChanged)
        
        self._valueChanged()
        self._inputConnectionChanged()
        
        if coralAttribute.affectedBy():
            self.valueWidget().setDisabled(True)
    
    def _inputConnectionChanged(self):
        if self.coralObject().input():
            self.valueWidget().setDisabled(True)
        else:
            self.valueWidget().setDisabled(False)
    
    def _valueWidgetChanged(self):
        value = self.valueWidget().value()
        if self.coralObject().outValue().floatValueAt(0) != value:
            self.coralObject().outValue().setFloatValueAt(0, value)
            self.coralObject().valueChanged()
    
    def _valueChanged(self):
        value = self.coralObject().value().floatValueAt(0)
        if value != self.valueWidget().value():
            self.valueWidget().setValue(value)

class CustomIntSpinBox(QtGui.QSpinBox):
    def __init__(self, parent):
        QtGui.QSpinBox.__init__(self, parent)
        
        self._wheelCallback = None
    
    def wheelEvent(self, wheelEvent):
        QtGui.QSpinBox.wheelEvent(self, wheelEvent)
        
        self._wheelCallback()
        
class IntValueField(ObjectField):
    def __init__(self, coralAttribute, parentWidget):
        ObjectField.__init__(self, coralAttribute.name(), coralAttribute, parentWidget)
        
        self._valueChangedObserver = Observer()
        self._connectedInputObserver = Observer()
        self._disconnectedInputObserver = Observer()
        self.setValueWidget(CustomIntSpinBox(self), "editingFinished()", self._valueWidgetChanged)
        self.valueWidget()._wheelCallback = utils.weakRef(self._valueWidgetChanged)
        self.valueWidget().setRange(-999999999, 999999999)
        
        coralApp.addAttributeValueChangedObserver(self._valueChangedObserver, coralAttribute, self._valueChanged)
        coralApp.addConnectedInputObserver(self._connectedInputObserver, coralAttribute, self._inputConnectionChanged)
        coralApp.addDisconnectedInputObserver(self._disconnectedInputObserver, coralAttribute, self._inputConnectionChanged)
        
        self._valueChanged()
        self._inputConnectionChanged()
        
        if coralAttribute.affectedBy():
            self.valueWidget().setDisabled(True)
    
    def _inputConnectionChanged(self):
        if self.coralObject().input():
            self.valueWidget().setDisabled(True)
        else:
            self.valueWidget().setDisabled(False)
    
    def _valueWidgetChanged(self):
        value = self.valueWidget().value()
        if self.coralObject().outValue().intValueAt(0) != value:
            self.coralObject().outValue().setIntValueAt(0, value)
            self.coralObject().valueChanged()
    
    def _valueChanged(self):
        value = self.coralObject().value().intValueAt(0)
        if value != self.valueWidget().value():
            self.valueWidget().setValue(value)

class BoolValueField(ObjectField):
    def __init__(self, coralAttribute, parentWidget):
        ObjectField.__init__(self, coralAttribute.name(), coralAttribute, parentWidget)
        
        self._valueChangedObserver = Observer()
        self._connectedInputObserver = Observer()
        self._disconnectedInputObserver = Observer()
        
        self.setValueWidget(QtGui.QCheckBox(self), "stateChanged(int)", self._valueWidgetChanged)
        
        coralApp.addAttributeValueChangedObserver(self._valueChangedObserver, coralAttribute, self._valueChanged)
        coralApp.addConnectedInputObserver(self._connectedInputObserver, coralAttribute, self._inputConnectionChanged)
        coralApp.addDisconnectedInputObserver(self._disconnectedInputObserver, coralAttribute, self._inputConnectionChanged)
        
        self._valueChanged()
        self._inputConnectionChanged()
        
        self.valueWidget().setTristate(False)
        
        if coralAttribute.affectedBy():
            self.valueWidget().setDisabled(True)
    
    def _inputConnectionChanged(self):
        if self.coralObject().input():
            self.valueWidget().setDisabled(True)
        else:
            self.valueWidget().setDisabled(False)
   
    def _valueWidgetChanged(self):
        widgetValue = self.valueWidget().isChecked()
        if widgetValue != self.coralObject().outValue().boolValueAt(0):
            self.coralObject().outValue().setBoolValueAt(0, widgetValue)
            self.coralObject().valueChanged()
    
    def _valueChanged(self):
        value = self.coralObject().value().boolValueAt(0)
        
        if value != self.valueWidget().isChecked():
            self.valueWidget().setCheckState(value)

class StringValueField(ObjectField):
    def __init__(self, coralAttribute, parentWidget):
        ObjectField.__init__(self, coralAttribute.name(), coralAttribute, parentWidget)
        
        self._valueChangedObserver = Observer()
        self._connectedInputObserver = Observer()
        self._disconnectedInputObserver = Observer()
        
        self.setValueWidget(QtGui.QLineEdit(self), "editingFinished()", self._valueWidgetChanged)
        
        coralApp.addAttributeValueChangedObserver(self._valueChangedObserver, coralAttribute, self._valueChanged)
        coralApp.addConnectedInputObserver(self._connectedInputObserver, coralAttribute, self._inputConnectionChanged)
        coralApp.addDisconnectedInputObserver(self._disconnectedInputObserver, coralAttribute, self._inputConnectionChanged)
        
        self._valueChanged()
        self._inputConnectionChanged()
        
        if coralAttribute.affectedBy():
            self.valueWidget().setDisabled(True)
    
    def _inputConnectionChanged(self):
        if self.coralObject().input():
            self.valueWidget().setDisabled(True)
        else:
            self.valueWidget().setDisabled(False)
    
    def _valueWidgetChanged(self):
        self.coralObject().outValue().setStringValue(str(self.valueWidget().text()))
        self.coralObject().valueChanged()
    
    def _valueChanged(self):
        value = self.coralObject().value().stringValue()

        if value != str(self.valueWidget().text()):
            self.valueWidget().setText(value)

class NameField(ObjectField):
    def __init__(self, coralNode, parentWidget):
        ObjectField.__init__(self, "name", coralNode, parentWidget)
        
        self._nameChangedObserver = Observer()
        
        self.setValueWidget(QtGui.QLineEdit(coralNode.name(), self), "editingFinished()", self._valueWidgetChanged)
        
        coralApp.addNameChangedObserver(self._nameChangedObserver, coralNode, self._nameChanged)
    
    def _valueWidgetChanged(self):
        newName = str(self.valueWidget().text())
        if self.coralObject().name() != newName:
            self.coralObject().setName(newName)
    
    def _nameChanged(self):
        newName = self.coralObject().name()
        if newName != str(self.valueWidget().text()):
            self.valueWidget().setText(newName)
        
