TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
target.path = /home/root
INSTALLS += target
SOURCES += main.cpp


LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc
INCLUDEPATH += /home/ell/ell/nitrogen6x/opencv/opencv-2.4.11/include \
               /home/ell/ell/nitrogen6x/opencv/opencv-2.4.11/include/opencv \
               /home/ell/ell/nitrogen6x/opencv/opencv-2.4.11/include/opencv2 \
