add_library(gr-math INTERFACE)
target_link_libraries(gr-math INTERFACE gnuradio-core gnuradio-algorithm exprtk)
target_include_directories(gr-math INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
                                             $<INSTALL_INTERFACE:include/>)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/gnuradio-4.0/math"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/gnuradio-4.0")

gr_add_block_library(
  GrMathBlocks
  MAKE_SHARED_LIBRARY
  HEADERS
  include/gnuradio-4.0/math/Accumulator.hpp
  include/gnuradio-4.0/math/AgcBlock.hpp
  include/gnuradio-4.0/math/AmDemod.hpp
  include/gnuradio-4.0/math/Clamp.hpp
  include/gnuradio-4.0/math/EnergyDetector.hpp
  include/gnuradio-4.0/math/DbConvert.hpp
  include/gnuradio-4.0/math/Limiter.hpp
  include/gnuradio-4.0/math/Conjugate.hpp
  include/gnuradio-4.0/math/MovingRms.hpp
  include/gnuradio-4.0/math/Differentiator.hpp
  include/gnuradio-4.0/math/ExpressionBlocks.hpp
  include/gnuradio-4.0/math/Math.hpp
  include/gnuradio-4.0/math/MovingAverage.hpp
  include/gnuradio-4.0/math/PhaseUnwrap.hpp
  include/gnuradio-4.0/math/QuadratureDemod.hpp
  include/gnuradio-4.0/math/Rotator.hpp
  include/gnuradio-4.0/math/AutoCorrelation.hpp
  include/gnuradio-4.0/math/Goertzel.hpp
  include/gnuradio-4.0/math/InstantaneousFrequency.hpp
  include/gnuradio-4.0/math/SchmittTrigger.hpp
  include/gnuradio-4.0/math/Threshold.hpp
  LINK_LIBRARIES
  gr-math)

if(TARGET GrMathBlocksShared AND ENABLE_TESTING)
  add_subdirectory(test)
endif()
