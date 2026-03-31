add_ut_test(qa_filter)
target_link_libraries(qa_filter PRIVATE gr-filter)

add_ut_test(qa_DCBlocker)
target_link_libraries(qa_DCBlocker PRIVATE gr-filter)

add_ut_test(qa_HilbertTransform)
target_link_libraries(qa_HilbertTransform PRIVATE gr-filter)

add_ut_test(qa_FrequencyEstimator)
target_link_libraries(qa_FrequencyEstimator PRIVATE gr-filter)

add_ut_test(qa_SvdDenoiser)
target_link_libraries(qa_SvdDenoiser PRIVATE gr-filter)

add_ut_test(qa_SavitzkyGolayFilter)
target_link_libraries(qa_SavitzkyGolayFilter PRIVATE gr-filter)
