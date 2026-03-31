add_library(gr-coding INTERFACE)
target_link_libraries(gr-coding INTERFACE gnuradio-core gnuradio-algorithm)
target_include_directories(gr-coding
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
              $<INSTALL_INTERFACE:include/>)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/gnuradio-4.0/coding"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/gnuradio-4.0")

gr_add_block_library(
  GrCodingBlocks
  MAKE_SHARED_LIBRARY
  HEADERS
  include/gnuradio-4.0/coding/DifferentialEncoder.hpp
  include/gnuradio-4.0/coding/DifferentialDecoder.hpp
  include/gnuradio-4.0/coding/GrayCodeEncoder.hpp
  include/gnuradio-4.0/coding/GrayCodeDecoder.hpp
  include/gnuradio-4.0/coding/PackBits.hpp
  include/gnuradio-4.0/coding/UnpackBits.hpp
  include/gnuradio-4.0/coding/Scrambler.hpp
  include/gnuradio-4.0/coding/CrcCompute.hpp
  include/gnuradio-4.0/coding/ConvEncoder.hpp
  include/gnuradio-4.0/coding/ViterbiDecoder.hpp
  LINK_LIBRARIES
  gr-coding)

if(TARGET GrCodingBlocksShared AND ENABLE_TESTING)
  add_subdirectory(test)
endif()
