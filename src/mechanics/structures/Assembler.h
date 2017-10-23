#pragma once

#include "mechanics/dofSubMatrixStorage/DofStatus.h"
#include "mechanics/dofSubMatrixStorage/BlockFullVector.h"
#include "mechanics/dofSubMatrixStorage/BlockSparseMatrix.h"
#include "mechanics/constraints/Constraints.h"

namespace NuTo
{
class NodeBase;

template <typename T>
class BlockFullVector;
class StructureOutputBlockVector;

class Assembler
{
public:
    //! @brief ctor
    Assembler();

    //! @brief builds the global dof numbering depending on the constraints
    //! sets the members mConstraintMatrix, mConstraintMappingRhs and mConstraintRhs [for time t=0]
    //! @param nodes all the nodes included in the global dof numbering
    void BuildGlobalDofs(const std::vector<NodeBase*>& nodes);

    //! @brief getter for mConstraintRhs, set via ConstraintUpdateRhs
    const BlockFullVector<double>& GetConstraintRhs(double time) const
    {
        ThrowIfRenumberingRequred();

        NuTo::BlockFullVector<double> rhs(mDofStatus);

        for (auto dof : mDofStatus.GetDofTypes())
            rhs[dof] = mConstraints.GetRhs(dof, time);

        return rhs;
    }

    //! @brief getter for mConstraintMatrix, set via BuildGlobalDofs
    const BlockSparseMatrix& GetConstraintMatrix() const
    {
        return mConstraintMatrix;
    }

    //    //! @brief calculates the right hand side of the constraint equations based on the mapping matrix and the rhs
    //    before
    //    //! the gauss elimination
    //    //! the result is stored internally in mConstraintRHS
    //    //! @param time global time
    //    void ConstraintUpdateRhs(double time);

    //! @brief throws if the nodes or constraints equations have changed
    void ThrowIfRenumberingRequred() const;

    //! @brief returns true if a node renumbering is required (nodes or constraints changed)
    bool RenumberingRequired() const
    {
        return (mConstraints.HaveChanged() or mNodeVectorChanged);
    }

    //! @brief sets the state variable mNodeVectorChanged to true to indicate
    //! that a node renumbering is required
    void SetNodeVectorChanged()
    {
        mNodeVectorChanged = true;
    }

    //! @brief non-const getter for mConstraints
    Constraint::Constraints& GetConstraints()
    {
        return mConstraints;
    }

    //! @brief getter for mConstraints
    const Constraint::Constraints& GetConstraints() const
    {
        return mConstraints;
    }

    //! @brief summarizes information to dof numbering, active dof types, symmetric dof types, constant dof types
    //! @TODO: should not be pulbic
    DofStatus mDofStatus;

    //! @brief adds \f$(\boldsymbol{x}_{J} - \boldsymbol{C}_{mat}^T\,\boldsymbol{x}_{K}),c\f$ to vec
    //! @remark only calculates active dof types
    //! @param vec Vector to which to apply the constraint matrix
    //! @param cMat Constraint matrix
    static BlockFullVector<double> ApplyCMatrix(const StructureOutputBlockVector& vec, const BlockSparseMatrix& cMat);

private:
    //    //! @brief builds the constraint rhs vector before the gauss elimination evaluated at time
    //    //! @param time global time
    //    //! @return constraint rhs before gauss elimination
    //    BlockFullVector<double> BuildRhsBeforeGaussElimination(double time) const;

    //! @brief stores the constraints
    Constraint::Constraints mConstraints;

    //! brief ... renumbering of nodal DOFs required or not
    bool mNodeVectorChanged = false;

    //! @brief constraint matrix relating the prescibed nodal unknowns to the free parameters
    BlockSparseMatrix mConstraintMatrix;


    //    //! @brief right hand side of the constraint equations
    //    BlockFullVector<double> mConstraintRhs;
};

} /* NuTo */
