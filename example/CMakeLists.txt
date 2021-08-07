function(cntgs_add_example _name)
    add_executable(cntgs-example-${_name} ${_name}.cpp)

    target_link_libraries(cntgs-example-${_name} PRIVATE cntgs::cntgs)

    add_test(NAME cntgs-example-${_name} COMMAND cntgs-example-${_name})
endfunction()

cntgs_add_example(varying-vector)
cntgs_add_example(fixed-vector)
cntgs_add_example(type-erased-vector)
cntgs_add_example(vector-with-alignment)
cntgs_add_example(vector-with-stack-storage)
target_compile_features(cntgs-example-vector-with-stack-storage PRIVATE cxx_std_20)
