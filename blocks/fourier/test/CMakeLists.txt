add_ut_test(qa_fourier)
target_link_libraries(qa_fourier PRIVATE gr-fourier)

add_ut_test(qa_IFFT)
target_link_libraries(qa_IFFT PRIVATE gr-fourier)
