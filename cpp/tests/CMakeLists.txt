# Every test is a small executable that throws (and so fails) if a check does not hold.
set(TESTS
  test_parameters
  test_assembly
  test_bolt_dofs
  test_stress
  test_eigenmodes
  test_forced_response
  )

foreach(test_name ${TESTS})
  add_executable(${test_name} ${test_name}.cc)
  target_link_libraries(${test_name} chladni_core)
  deal_ii_setup_target(${test_name})
  add_test(NAME ${test_name} COMMAND ${test_name})
endforeach()
