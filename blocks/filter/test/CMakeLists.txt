add_ut_test(qa_CicFilter)
target_link_libraries(qa_CicFilter PRIVATE gr-filter)

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

add_ut_test(qa_BiquadFilter)
target_link_libraries(qa_BiquadFilter PRIVATE gr-filter)

add_ut_test(qa_FractionalDelayLine)
target_link_libraries(qa_FractionalDelayLine PRIVATE gr-filter)

add_ut_test(qa_WienerFilter)
target_link_libraries(qa_WienerFilter PRIVATE gr-filter)

add_ut_test(qa_AdaptiveLmsFilter)
target_link_libraries(qa_AdaptiveLmsFilter PRIVATE gr-filter)

add_ut_test(qa_Squelch)
target_link_libraries(qa_Squelch PRIVATE gr-filter)

add_ut_test(qa_Convolver)
target_link_libraries(qa_Convolver PRIVATE gr-filter)

add_ut_test(qa_SteadyStateKalman)
target_link_libraries(qa_SteadyStateKalman PRIVATE gr-filter)

add_ut_test(qa_KalmanFilter)
target_link_libraries(qa_KalmanFilter PRIVATE gr-filter)

add_ut_test(qa_MedianFilter)
target_link_libraries(qa_MedianFilter PRIVATE gr-filter)

add_ut_test(qa_RationalResampler)
target_link_libraries(qa_RationalResampler PRIVATE gr-filter)

add_ut_test(qa_PolyphaseArbitraryResampler)
target_link_libraries(qa_PolyphaseArbitraryResampler PRIVATE gr-filter)
