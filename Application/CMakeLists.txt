add_library (Application SHARED src/application.cpp)
target_link_libraries (Application Virtual-Machine Logger)
