# @authors C. R. Thornsberry, K. Smith, S. V. Paulauskas

set(POLL2_SOURCES poll2.cpp poll2_core.cpp poll2_stats.cpp)
add_executable(poll2 ${POLL2_SOURCES})
target_link_libraries(poll2 PixieInterface PixieSupport Utility MCA_LIBRARY ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS poll2 DESTINATION bin)

set(LISTENER_SOURCES listener.cpp)
add_executable(listener ${LISTENER_SOURCES})
target_link_libraries(listener PaassCoreStatic)

set(MONITOR_SOURCES monitor.cpp)
add_executable(monitor ${MONITOR_SOURCES})
target_link_libraries(monitor PaassCoreStatic)

install(TARGETS listener monitor DESTINATION bin)
