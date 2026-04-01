add_ut_test(qa_fourier)
target_link_libraries(qa_fourier PRIVATE gr-fourier)

add_ut_test(qa_IFFT)
target_link_libraries(qa_IFFT PRIVATE gr-fourier)

add_ut_test(qa_SpectralEstimator)
target_link_libraries(qa_SpectralEstimator PRIVATE gr-fourier)

add_ut_test(qa_SpectralSubtractor)
target_link_libraries(qa_SpectralSubtractor PRIVATE gr-fourier)

add_ut_test(qa_PolyphaseChannelizer)
target_link_libraries(qa_PolyphaseChannelizer PRIVATE gr-fourier gr-filter)
