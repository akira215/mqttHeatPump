set(SOURCES driverDaikin.cpp
            driverHomebus.cpp
            driverAhd.cpp)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS include 
                    REQUIRES periodicTask cppgpio esp_driver_rmt esp_driver_uart esp_timer)
                    