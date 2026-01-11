#-------------------------------------------------
#
# Final Project - Smart Light Control System
# Microprocessor Systems Lab, Fall 2025
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = final_project
TEMPLATE = app

# Compiler warnings
DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        gpio_util.cpp \
        gpio_LED_CTRL.cpp

HEADERS += \
        mainwindow.h \
        PIN_LOOKUP.hpp \
        gpio_util.hpp \
        gpio_LED_CTRL.hpp

FORMS += \
        mainwindow.ui

RESOURCES += \
        qtres.qrc

# Copy read_adc.py to build directory
DISTFILES += \
        read_adc.py \
        lightbulb.png
