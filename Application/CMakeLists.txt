add_library (Application src/application.cpp)
set_property(TARGET Application PROPERTY CXX_STANDARD 17)
set_property(TARGET Application PROPERTY CXX_STANDARD_REQUIRED ON)
target_link_libraries (Application Virtual-Machine Logger)
