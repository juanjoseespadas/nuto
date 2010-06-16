# -*- coding: utf-8 -*-
# $Id$
import nuto

try:
    # symmetric coefficient matrix
    A_sy = nuto.DoubleSparseMatrixCSRSymmetric(5,9)
    A_sy.AddEntry(0,0,9)
    A_sy.AddEntry(0,1,1.5)
    A_sy.AddEntry(0,2,6)
    A_sy.AddEntry(0,3,0.75)
    A_sy.AddEntry(0,4,3)
    A_sy.AddEntry(1,1,0.5)
    A_sy.AddEntry(2,2,12)
    A_sy.AddEntry(3,3,0.625)
    A_sy.AddEntry(4,4,16)
    A_sy.SetOneBasedIndexing()
    print "symmetric matrix, sparse CSR storage"
    A_sy.Info()
    A_sy_full = nuto.DoubleFullMatrix(A_sy)
    print "\nsymmetric matrix, full storage"
    A_sy_full.Info(3)

    # nonsymmetric coefficient matrix
    A_nosy = nuto.DoubleSparseMatrixCSRGeneral(5,5,13)
    A_nosy.AddEntry(0,0,9)
    A_nosy.AddEntry(0,1,1.5)
    A_nosy.AddEntry(0,2,6)
    A_nosy.AddEntry(0,3,0.75)
    A_nosy.AddEntry(0,4,3)
    A_nosy.AddEntry(1,0,1.5)
    A_nosy.AddEntry(1,1,0.5)
    A_nosy.AddEntry(2,0,6)
    A_nosy.AddEntry(2,2,12)
    A_nosy.AddEntry(3,0,0.75)
    A_nosy.AddEntry(3,3,0.625)
    A_nosy.AddEntry(4,0,3)
    A_nosy.AddEntry(4,4,16)
    A_nosy.SetOneBasedIndexing()
    print "\nnonsymmetric matrix, sparse CSR storage"
    A_nosy.Info()
    print "\nnonsymmetric matrix, full storage"
    A_nosy_full = nuto.DoubleFullMatrix(A_nosy)
    A_nosy_full.Info(3)

    # right hand side vector
    rhs = nuto.DoubleFullMatrix(5,1,[1,2,3,4,5])
    print "\nright hand side vector"
    rhs.Info()

    # solver
    solver = nuto.SparseDirectSolverMUMPS()
    solver.SetVerboseLevel(3)

    # solve symmetric problem
    print "\nsolving the symmetric problem"
    sol_sy = nuto.DoubleFullMatrix(5,1)
    solver.Solve(A_sy,rhs,sol_sy)
    print "\nsolution of the symmetric problem"
    sol_sy.Info()

    # solve nonsymmetric problem
    print "\nsolving the nonsymmetric problem"
    sol_nosy = nuto.DoubleFullMatrix(5,1)
    solver.Solve(A_nosy,rhs,sol_nosy)
    print "\nsolution of the nonsymmetric problem"
    sol_nosy.Info()
except nuto.Exception, e:
    print e.ErrorMessage()