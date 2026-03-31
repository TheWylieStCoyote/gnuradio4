add_ut_test(qa_Head)
target_link_libraries(qa_Head PRIVATE gr-basic)

add_ut_test(qa_Skip)
target_link_libraries(qa_Skip PRIVATE gr-basic)

add_ut_test(qa_ChirpSource)
target_link_libraries(qa_ChirpSource PRIVATE gr-basic)

add_ut_test(qa_AwgnChannel)
target_link_libraries(qa_AwgnChannel PRIVATE gr-basic)

add_ut_test(qa_WindowApply)
target_link_libraries(qa_WindowApply PRIVATE gr-basic)

add_ut_test(qa_KeepMInN)
target_link_libraries(qa_KeepMInN PRIVATE gr-basic)

add_ut_test(qa_StreamTagger)
target_link_libraries(qa_StreamTagger PRIVATE gr-basic)

add_ut_test(qa_TagDebugSink)
target_link_libraries(qa_TagDebugSink PRIVATE gr-basic)

add_ut_test(qa_TagGate)
target_link_libraries(qa_TagGate PRIVATE gr-basic)

add_ut_test(qa_StreamMux)
target_link_libraries(qa_StreamMux PRIVATE gr-basic)

add_ut_test(qa_StreamDemux)
target_link_libraries(qa_StreamDemux PRIVATE gr-basic)

add_ut_test(qa_StreamToVector)
target_link_libraries(qa_StreamToVector PRIVATE gr-basic)

add_ut_test(qa_Converter)
add_ut_test(qa_Selector)
add_ut_test(qa_sources)
add_ut_test(qa_DataSink)
add_ut_test(qa_TriggerBlocks)

if(GR_ENABLE_BLOCK_REGISTRY AND INTERNAL_ENABLE_BLOCK_PLUGINS)
  add_ut_test(qa_BasicAvailableBlocks)
  target_link_libraries(qa_BasicAvailableBlocks PRIVATE GrBasicBlocksShared)
endif()
add_ut_test(qa_StreamToDataSet)
add_ut_test(qa_SyncBlock)

message(STATUS "###Python Include Dirs: ${Python3_INCLUDE_DIRS}")
if(PYTHON_AVAILABLE AND GR_ENABLE_BLOCK_REGISTRY)
  add_ut_test(qa_PythonBlock)
endif()
