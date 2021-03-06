set(MathSources
    CubicSplineInterpolation.cpp
    EigenCompanion.cpp
    EigenSolverArpack.cpp
    Interpolation.cpp
    LinearInterpolation.cpp
    Legendre.cpp
    SparseMatrixCSR.cpp
    SparseMatrixCSRGeneral.cpp
    SparseMatrixCSRSymmetric.cpp
    SparseDirectSolverMKLPardiso.cpp
    SparseDirectSolverPardiso.cpp
    SparseDirectSolverMUMPS.cpp
)

create_nuto_module(Math "${MathSources}")
target_link_libraries(Math Base ${LAPACK_LIBRARIES} Mumps::Mumps
    ${PARDISO_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${MKLSolver_LIBRARIES})
if(ARPACK_FOUND)
    target_link_libraries(Math Arpack::Arpack)
endif()
