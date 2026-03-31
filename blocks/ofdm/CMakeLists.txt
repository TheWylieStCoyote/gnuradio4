add_library(gr-ofdm INTERFACE)
target_link_libraries(gr-ofdm INTERFACE gnuradio-core gnuradio-algorithm)
target_include_directories(gr-ofdm INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
                                             $<INSTALL_INTERFACE:include/>)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/gnuradio-4.0/ofdm"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/gnuradio-4.0")

gr_add_block_library(
  GrOfdmBlocks
  MAKE_SHARED_LIBRARY
  HEADERS
  include/gnuradio-4.0/ofdm/CyclicPrefixAdd.hpp
  include/gnuradio-4.0/ofdm/CyclicPrefixRemove.hpp
  LINK_LIBRARIES
  gr-ofdm)

if(TARGET GrOfdmBlocksShared AND ENABLE_TESTING)
  add_subdirectory(test)
endif()
