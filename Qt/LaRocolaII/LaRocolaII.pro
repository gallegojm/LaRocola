# Add files and directories to ship with the application 
# by adapting the examples below.
# file1.source = myfile
# dir1.source = mydir
DEPLOYMENTFOLDERS = # file1 dir1

symbian:TARGET.UID3 = 0xE699BAEE

# Smart Installer package's UID
# This UID is from the protected range 
# and therefore the package will fail to install if self-signed
# By default qmake uses the unprotected range value if unprotected UID is defined for the application
# and 0x2002CCCF value if protected UID is given to the application
#symbian:DEPLOYMENT.installer_header = 0x2002CCCF

# Allow network access on Symbian
symbian:TARGET.CAPABILITY += NetworkServices

# If your application uses the Qt Mobility libraries, uncomment
# the following lines and add the respective components to the 
# MOBILITY variable. 
QT += core gui declarative network
CONFIG += qt-components
#CONFIG += mobility
#MOBILITY += systeminfo

#symbian {
#    TARGET.CAPABILITY = LocalServices ReadUserData WriteUserData NetworkServices UserEnvironment Location ReadDeviceData
#}

# Please do not modify the following two lines. Required for deployment.
include(deployment.pri)
qtcAddDeployment()

OTHER_FILES += main.qml MainPage.qml \
    ListText.qml

RESOURCES += \
    LaRocolaII.qrc

SOURCES += \
    main.cpp \
    udpclient.cpp

HEADERS += \
    config.h \
    udpclient.h

ICON = LaRocolaII.svg
