// $Id: $
#ifndef NodeGridDisplacements_3d_H
#define NodeGridDisplacements_3d_H

#include "nuto/mechanics/nodes/NodeGrid3D.h"
#include "nuto/mechanics/nodes/NodeDisplacements3D.h"

namespace NuTo
{
//! @author Jörg F. Unger, ISM
//! @date October 2009
//! @brief ... class for grid nodes with displacements
class NodeGridDisplacements3D : public NodeGrid3D, public NodeDisplacements3D
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif  // ENABLE_SERIALIZATION

public:

    //! @brief constructor
    NodeGridDisplacements3D(int rNodeId) : NodeGrid3D (rNodeId), NodeDisplacements3D()
    {}

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(NodeGrid3D)
           & BOOST_SERIALIZATION_BASE_OBJECT_NVP(NodeDisplacements3D);
    }
#endif  // ENABLE_SERIALIZATION

    //! @brief sets the global dofs
    //! @param rDOF current maximum DOF, this variable is increased within the routine
    virtual void SetGlobalDofs(int& rDOF)
    {
        NodeGrid3D::SetGlobalDofs(rDOF);
        NodeDisplacements3D::SetGlobalDofs(rDOF);
    }

    //! @brief write dof values to the node (based on global dof number)
    //! @param rActiveDofValues ... active dof values
    //! @param rDependentDofValues ... dependent dof values
    virtual void SetGlobalDofValues(const FullMatrix<double>& rActiveDofValues, const FullMatrix<double>& rDependentDofValues)
    {
        NodeGrid3D::SetGlobalDofValues(rActiveDofValues, rDependentDofValues);
        NodeDisplacements3D::SetGlobalDofValues(rActiveDofValues, rDependentDofValues);
    }

    //! @brief extract dof values from the node (based on global dof number)
    //! @param rActiveDofValues ... active dof values
    //! @param rDependentDofValues ... dependent dof values
    virtual void GetGlobalDofValues(FullMatrix<double>& rActiveDofValues, FullMatrix<double>& rDependentDofValues) const
    {
        NodeGrid3D::GetGlobalDofValues(rActiveDofValues, rDependentDofValues);
        NodeDisplacements3D::GetGlobalDofValues(rActiveDofValues, rDependentDofValues);
    }

    //! @brief renumber the global dofs according to predefined ordering
    //! @param rMappingInitialToNewOrdering ... mapping from initial ordering to the new ordering
    virtual void RenumberGlobalDofs(std::vector<int>& rMappingInitialToNewOrdering)
    {
        NodeGrid3D::RenumberGlobalDofs(rMappingInitialToNewOrdering);
        NodeDisplacements3D::RenumberGlobalDofs(rMappingInitialToNewOrdering);
    }

    //! @brief returns the type of the node
    //! @return type
    virtual std::string GetNodeTypeStr()const
    {
    	return std::string("NodeGridDisplacements3D");
    }
};
}

#endif //NodeGridDisplacements_3d_H