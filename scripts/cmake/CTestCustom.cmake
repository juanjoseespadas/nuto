# Here, you can insert tests you want ctest to skip, e.g. when you have made
# a change, but were unable to get all the tests to run correctly
set(CTEST_CUSTOM_TESTS_IGNORE
    # does not work properly on machines under load
    integration::mechanics::ParallelAssembly.cpp
    )
