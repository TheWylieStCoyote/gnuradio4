add_library(gr-demod INTERFACE)
target_link_libraries(gr-demod INTERFACE gnuradio-core gnuradio-algorithm)
target_include_directories(gr-demod INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
                                              $<INSTALL_INTERFACE:include/>)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/gnuradio-4.0/demod"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/gnuradio-4.0")

gr_add_block_library(
  GrDemodBlocks
  MAKE_SHARED_LIBRARY
  HEADERS
  include/gnuradio-4.0/demod/PLL.hpp
  include/gnuradio-4.0/demod/CostasLoop.hpp
  include/gnuradio-4.0/demod/ClockRecoveryMM.hpp
  include/gnuradio-4.0/demod/SymbolSync.hpp
  LINK_LIBRARIES
  gr-demod)

if(TARGET GrDemodBlocksShared AND ENABLE_TESTING)
  add_subdirectory(test)
endif()
