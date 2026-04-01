add_ut_test(qa_BasicFileIo)

add_ut_test(qa_CsvFile)
target_link_libraries(qa_CsvFile PRIVATE gr-fileio)

add_ut_test(qa_WavFile)
target_link_libraries(qa_WavFile PRIVATE gr-fileio)

add_ut_test(qa_UdpBlocks)
target_link_libraries(qa_UdpBlocks PRIVATE gr-fileio)

add_ut_test(qa_SigMFFile)
target_link_libraries(qa_SigMFFile PRIVATE gr-fileio)
