add_ut_test(qa_CyclicPrefixAdd)
target_link_libraries(qa_CyclicPrefixAdd PRIVATE gr-ofdm)

add_ut_test(qa_CyclicPrefixRemove)
target_link_libraries(qa_CyclicPrefixRemove PRIVATE gr-ofdm)
