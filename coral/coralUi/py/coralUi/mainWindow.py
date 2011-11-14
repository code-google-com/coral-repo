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
import os
from PyQt4 import QtGui, QtCore
from .. import coralApp
import dockWidget

class MainWindow(QtGui.QMainWindow):
    _globalInstance = None
    _lastFileDialogDir = ""
    _closedWidgets = {}
    _registeredWidgets = {}
    
    @staticmethod
    def openRegisteredWidget(widgetName):
        if MainWindow._registeredWidgets.has_key(widgetName):
            widgetClass = MainWindow._registeredWidgets[widgetName]
            MainWindow._globalInstance.dockWidget(widgetClass(MainWindow._globalInstance))
    
    @staticmethod
    def registerWidget(widgetName, widget):
        MainWindow._registeredWidgets[widgetName] = widget
    
    @staticmethod
    def _init():
        MainWindow._globalInstance = MainWindow()
    
    @staticmethod
    def globalInstance():
        return MainWindow._globalInstance
    
    @staticmethod
    def okCancelDialog(message):
        dialog = QtGui.QMessageBox(MainWindow._globalInstance)
        dialog.setText(message)
        dialog.addButton("No", QtGui.QMessageBox.NoRole)
        dialog.addButton("Yes", QtGui.QMessageBox.YesRole)
        
        result = dialog.exec_()
        return result
    
    @staticmethod
    def saveFileDialog(title, fileType):
        filename = QtGui.QFileDialog.getSaveFileName(MainWindow._globalInstance, title, MainWindow._lastFileDialogDir, fileType)
    
        MainWindow._lastFileDialogDir = os.path.split(str(filename))[0]
    
        return str(filename)
    
    @staticmethod
    def openFileDialog(title, fileType):
        filename = QtGui.QFileDialog.getOpenFileName(MainWindow._globalInstance, title, MainWindow._lastFileDialogDir, fileType)
    
        MainWindow._lastFileDialogDir = os.path.split(str(filename))[0]
    
        return str(filename)
    
    def __init__(self, parent = None):
        QtGui.QMainWindow.__init__(self, parent)
        
        self._menuBar = QtGui.QMenuBar()
        
        self.setWindowTitle("coral")
        self.resize(900, 500)
        self.setMenuBar(self._menuBar)
        self.setDockOptions(QtGui.QMainWindow.AllowTabbedDocks | QtGui.QMainWindow.AllowNestedDocks | QtGui.QMainWindow.AnimatedDocks)
        
        self._menuBar.clear()
    
    def about(self):
        aboutBox = QtGui.QMessageBox(self)
        aboutBox.setWindowTitle("About Coral")
        
        aboutMsg = '<h2>Coral Version ' + coralApp.version() + '</h2>'
        aboutMsg += '<p style="color: #333333; font-size: small;">created by Andrea Interguglielmi</p>'
        aboutMsg += '<p style="color: #333333; font-size: small;">with contributions by Paolo Fazio (Windows porting, OpenGL Viewport, and more)</p>'
        aboutMsg += '<p style="font-size: small;">Copriright 2011 Andrea Interguglielmi.</p>'
        
        aboutBox.setTextFormat(QtCore.Qt.RichText)
        aboutBox.setText(aboutMsg)
        aboutBox.exec_()
        
    def menuBar(self):
        return self._menuBar
    
    def dockWidget(self, widget, area = None):
        dock = dockWidget.DockWidget(widget, self)
        dock.setObjectName(widget.windowTitle())
        dock.show()
        
        if area is None:
            dock.setFloating(True)
        else:
            self.addDockWidget(area, dock)
        
        return dock
    
    def _restoreLastOpenWindows(self, settings):
        widgetsLastOpen = settings.value("openWidgets").toString()
        if widgetsLastOpen:
            widgetsLastOpen = eval(str(widgetsLastOpen))
        
        openWidgetsName = []
        for dockWidget in self.dockWidgets():
            openWidgetsName.append(str(dockWidget.windowTitle()))
        
        for widgetName in widgetsLastOpen:
            if widgetName not in openWidgetsName:
                if MainWindow._registeredWidgets.has_key(widgetName):
                    widgetClass = MainWindow._registeredWidgets[widgetName]
                    self.dockWidget(widgetClass(self))
        
    def restoreSettings(self):
        settings = self.settings()
        
        self._restoreLastOpenWindows(settings)
        geometry = str(settings.value("mainWinGeometry").toString())
        if geometry:
            geometry = eval(geometry)
            self.move(geometry[0], geometry[1])
            self.resize(geometry[2], geometry[3])
        
        lastFileDialogDir = settings.value("lastFileDialogDir").toString()
        if lastFileDialogDir:
            MainWindow._lastFileDialogDir = lastFileDialogDir
    
    def settings(self):
        return QtCore.QSettings(self.windowTitle())
    
    def dockWidgets(self):
        dockWidgets = []
        for obj in self.children():
            if type(obj) is dockWidget.DockWidget:
                dockWidgets.append(obj)
        
        return dockWidgets
    
    def closeEvent(self, event):
        settings = self.settings()
        settings.setValue("mainWinGeometry", str([self.pos().x(), self.pos().y(), self.size().width(), self.size().height()]))
        
        openWidgets = []
        for dockWidget in self.dockWidgets():
            widgetName = str(dockWidget.windowTitle())
            if dockWidget.isVisible() and widgetName not in openWidgets:
                openWidgets.append(widgetName)
                
            dockWidget.closeEvent(event)
        
        settings.setValue("openWidgets", str(openWidgets))
        
        settings.setValue("lastFileDialogDir", MainWindow._lastFileDialogDir)
        
        QtGui.QMainWindow.closeEvent(self, event)

    def setShortcutsMap(self, shortcuts):
        for key, function in shortcuts.iteritems():
            QtGui.QShortcut(QtGui.QKeySequence(key), self, function)

