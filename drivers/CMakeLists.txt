set(SOURCES driverDaikin.cpp
                driverHomebus.cpp)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS include 
                    REQUIRES periodicTask cppgpio esp_driver_rmt esp_timer)
                    