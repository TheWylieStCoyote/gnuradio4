add_ut_test(qa_DifferentialEncoder)
target_link_libraries(qa_DifferentialEncoder PRIVATE gr-coding)

add_ut_test(qa_DifferentialDecoder)
target_link_libraries(qa_DifferentialDecoder PRIVATE gr-coding)

add_ut_test(qa_GrayCodeEncoder)
target_link_libraries(qa_GrayCodeEncoder PRIVATE gr-coding)

add_ut_test(qa_GrayCodeDecoder)
target_link_libraries(qa_GrayCodeDecoder PRIVATE gr-coding)

add_ut_test(qa_PackBits)
target_link_libraries(qa_PackBits PRIVATE gr-coding)

add_ut_test(qa_UnpackBits)
target_link_libraries(qa_UnpackBits PRIVATE gr-coding)

add_ut_test(qa_Scrambler)
target_link_libraries(qa_Scrambler PRIVATE gr-coding)

add_ut_test(qa_CrcCompute)
target_link_libraries(qa_CrcCompute PRIVATE gr-coding)

add_ut_test(qa_ConvEncoder)
target_link_libraries(qa_ConvEncoder PRIVATE gr-coding)
