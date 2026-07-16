cmake_minimum_required(VERSION 3.19)
project(RestaurantClient LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools Network PrintSupport)

qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

qt_add_executable(RestaurantClient
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    resources.qrc
    widget/coverwidget.h widget/coverwidget.cpp
    network/networkmanager.h network/networkmanager.cpp
    model/Dish.h
    widget/dishwidget.h widget/dishwidget.cpp widget/dishwidget.ui
    widget/dishcard.h widget/dishcard.cpp widget/dishcard.ui
    model/CartItem.h
    widget/cartwidget.h widget/cartwidget.cpp widget/cartwidget.ui
    widget/cartitemwidget.h widget/cartitemwidget.cpp widget/cartitemwidget.ui
    widget/tablewidget.h widget/tablewidget.cpp widget/tablewidget.ui
    model/DiningTable.h
    widget/paywidget.h widget/paywidget.cpp widget/paywidget.ui
    widget/orderdetailwidget.h widget/orderdetailwidget.cpp widget/orderdetailwidget.ui
    widget/blindboxdialog.h widget/blindboxdialog.cpp




)

qt_add_translations(
    TARGETS RestaurantClient
    TS_FILES RestaurantClient_zh_CN.ts
)

target_link_libraries(RestaurantClient
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
        Qt::PrintSupport
)

install(TARGETS RestaurantClient
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET RestaurantClient
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
