add_ut_test(qa_PLL)
target_link_libraries(qa_PLL PRIVATE gr-demod)

add_ut_test(qa_CostasLoop)
target_link_libraries(qa_CostasLoop PRIVATE gr-demod)

add_ut_test(qa_ClockRecoveryMM)
target_link_libraries(qa_ClockRecoveryMM PRIVATE gr-demod)

add_ut_test(qa_SymbolSync)
target_link_libraries(qa_SymbolSync PRIVATE gr-demod)
