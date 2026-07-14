cmake_minimum_required(VERSION 3.19)
project(RestaurantAdmin LANGUAGES CXX)

set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network LinguistTools)

qt_standard_project_setup()

qt_add_executable(RestaurantAdmin
    WIN32 MACOSX_BUNDLE
    main.cpp
    adminmainwindow.cpp
    adminmainwindow.h
    adminmainwindow.ui
    login/adminloginwidget.h login/adminloginwidget.cpp login/adminloginwidget.ui
    network/networkmanager.h network/networkmanager.cpp
    model/diningtable.h
    table/admintablewidget.h table/admintablewidget.cpp
    model/dish.h
    dish/admindishwidget.h dish/admindishwidget.cpp
    dish/disheditdialog.h dish/disheditdialog.cpp
    order/adminorderwidget.h order/adminorderwidget.cpp
    resources/control-arrows.qrc
)

qt_add_translations(
    TARGETS RestaurantAdmin
    TS_FILES RestaurantAdmin_zh_CN.ts
)

target_link_libraries(RestaurantAdmin
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
)

install(TARGETS RestaurantAdmin
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET RestaurantAdmin
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
