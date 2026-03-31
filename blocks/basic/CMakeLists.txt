set(GrBasicBlocks_HDRS
    include/gnuradio-4.0/basic/Head.hpp
    include/gnuradio-4.0/basic/Skip.hpp
    include/gnuradio-4.0/basic/ChirpSource.hpp
    include/gnuradio-4.0/basic/AwgnChannel.hpp
    include/gnuradio-4.0/basic/WindowApply.hpp
    include/gnuradio-4.0/basic/KeepMInN.hpp
    include/gnuradio-4.0/basic/StreamTagger.hpp
    include/gnuradio-4.0/basic/TagDebugSink.hpp
    include/gnuradio-4.0/basic/TagGate.hpp
    include/gnuradio-4.0/basic/StreamMux.hpp
    include/gnuradio-4.0/basic/StreamDemux.hpp
    include/gnuradio-4.0/basic/ClockSource.hpp
    include/gnuradio-4.0/basic/CommonBlocks.hpp
    include/gnuradio-4.0/basic/ConverterBlocks.hpp
    include/gnuradio-4.0/basic/DataSink.hpp
    include/gnuradio-4.0/basic/FunctionGenerator.hpp
    include/gnuradio-4.0/basic/Selector.hpp
    include/gnuradio-4.0/basic/SignalGenerator.hpp
    include/gnuradio-4.0/basic/StreamToDataSet.hpp
    include/gnuradio-4.0/basic/StreamToVector.hpp
    include/gnuradio-4.0/basic/VectorToStream.hpp
    include/gnuradio-4.0/basic/SyncBlock.hpp
    include/gnuradio-4.0/basic/Trigger.hpp)

set(GrBasicBlocks_LIBS gr-basic gr-testing)

if(PYTHON_AVAILABLE)
  list(
    APPEND
    GrBasicBlocks_HDRS
    include/gnuradio-4.0/basic/PythonBlock.hpp
    include/gnuradio-4.0/basic/PythonInterpreter.hpp)
  list(
    APPEND
    GrBasicBlocks_LIBS
    Python3::Python
    Python3::NumPy)
endif()

add_library(gr-basic INTERFACE ${GrBasicBlocks_HDRS})
target_link_libraries(gr-basic INTERFACE gnuradio-core gnuradio-algorithm ${GrBasicBlocks_LIBS})
target_include_directories(gr-basic INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
                                              $<INSTALL_INTERFACE:include/>)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/gnuradio-4.0/basic"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/gnuradio-4.0")

gr_add_block_library(
  GrBasicBlocks
  MAKE_SHARED_LIBRARY
  MAKE_STATIC_LIBRARY
  HEADERS
  ${GrBasicBlocks_HDRS}
  LINK_LIBRARIES
  ${GrBasicBlocks_LIBS})

if(TARGET GrBasicBlocksShared AND ENABLE_TESTING)
  add_subdirectory(test)
endif()
