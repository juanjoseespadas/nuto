// $Id$

#ifndef STRUCTURE_H
#define STRUCTURE_H

#ifdef ENABLE_SERIALIZATION
#include <boost/serialization/access.hpp>
#endif // ENABLE_SERIALIZATION

#include <boost/ptr_container/ptr_map.hpp>

#include "nuto/mechanics/MechanicsException.h"
#include "nuto/mechanics/structures/StructureBase.h"
#include "nuto/mechanics/elements/ElementBase.h"
#include "nuto/mechanics/elements/ElementDataEnum.h"
#include "nuto/mechanics/elements/IpDataEnum.h"
#include "nuto/mechanics/nodes/NodeBase.h"

namespace NuTo
{
//! @author Jörg F. Unger, ISM
//! @date October 2009
//! @brief ... standard class for irregular (unstructured) structures
class Structure : public StructureBase
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif  // ENABLE_SERIALIZATION

public:
    //! @brief constructor
    //! @param mDimension  Structural dimension (1,2 or 3)
    Structure(int mDimension);

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);

    //! @brief ... save the object to a file
    //! @param filename ... filename
    //! @param aType ... type of file, either BINARY, XML or TEXT
    void Save (const std::string &filename, std::string rType )const;

    //! @brief ... restore the object from a file
    //! @param filename ... filename
    //! @param aType ... type of file, either BINARY, XML or TEXT
    void Restore (const std::string &filename, std::string rType );
#endif // ENABLE_SERIALIZATION

    //! @brief ... Return the name of the class, this is important for the serialize routines, since this is stored in the file
    //!            in case of restoring from a file with the wrong object type, the file id is printed
    //! @return    class name
    std::string GetTypeId()const
    {
        return std::string("Structure");
    }

    //! @brief Builds the nonlocal data for integral type nonlocal constitutive models
    //! @param rConstitutiveId constitutive model for which the data is build
    void BuildNonlocalData(int rConstitutiveId);

#ifndef SWIG
    //! @brief Builds the nonlocal data for integral type nonlocal constitutive models
    //! @param rConstitutiveId constitutive model for which the data is build
    void BuildNonlocalData(const ConstitutiveBase* rConstitutive);
#endif //SWIG

//*************************************************
//************ Node routines        ***************
//***  defined in structures/StructureNode.cpp  ***
//*************************************************
    //! @brief returns the number of nodes
    //! @return number of nodes
    int GetNumNodes() const;

#ifndef SWIG
    //! @brief returns a reference to a node
    //! @param identifier
    //! @return reference to a node
    NodeBase* NodeGetNodePtr(int rIdent);

    //! @brief returns a reference to a node
    //! @param identifier
    //! @return reference to a node
    const NodeBase* NodeGetNodePtr(int rIdent)const;

    //! @brief gives the identifier of a node
    //! @param pointer to a node
    //! @return id
    int NodeGetId(const NodeBase* rNode)const;
#endif //SWIG

    //! @brief creates a node
    //! @param rDOFs space separated string containing the attributes like
    //! DISPLACEMENTS, ROTATIONS; TEMPERATURES
    //! the number of DOFs will be chosen according to the structure
    //! e.g. a 2D structure might have 2 displacements and 1 rotation
    //! @param rCoordinates coordinates of the node
    //! @return node number
    int NodeCreate(std::string rDOFs, NuTo::FullMatrix<double>& rCoordinates);

    //! @brief creates a node
    //! @param rNodeNumber ... node number
    //! @param rDOFs ... space separated string containing the node dofs (e.g. displacements, rotations, temperatures)
    //! @param rCoordinates ...  node coordinates
    void NodeCreate(int rNodeNumber, std::string rDOFs, NuTo::FullMatrix<double>& rCoordinates);

    //! @brief creates multiple nodes
    //! @param rDOFs ... space separated string containing the nodal dofs (e.g. displacements, rotations, temperatures)
    //! @param rCoordinates ...  nodal coordinates (column-wise storage of each nodal coordinate)
    //! @return a NuTo::FullMatrix<int> containing the node numbers
    NuTo::FullMatrix<int> NodesCreate(std::string rDOFs, NuTo::FullMatrix<double>& rCoordinates);

   //! @brief info about the nodes in the Structure
    void NodeInfo(int mVerboseLevel)const;

    //! @brief numbers the dofs in the structure
    void NodeBuildGlobalDofs();

    //! @brief write dof values (e.g. displacements, temperatures to the nodes)
    //! @param rActiveDofValues ... vector of global dof values (ordering according to global dofs, size is number of active dofs)
    void NodeMergeActiveDofValues(const NuTo::FullMatrix<double>& rActiveDofValues);

    //! @brief extract dof values (e.g. displacements, temperatures to the nodes)
    //! @param rActiveDofValues ... vector of global active dof values (ordering according to global dofs, size is number of active dofs)
    //! @param rDependentDofValues ... vector of global dependent dof values (ordering according to (global dofs) - (number of active dofs), size is (total number of dofs) - (number of active dofs))
    void NodeExtractDofValues(NuTo::FullMatrix<double>& rActiveDofValues, NuTo::FullMatrix<double>& rDependentDofValues) const;

#ifndef SWIG
    //! @brief calculates the internal force vector for a given node
    //! @param rNode node pointer
    //! @param rNodeForce return value
    void NodeGetInternalForce(const NodeBase* rNode, NuTo::FullMatrix<double>& rNodeForce)const;
#endif//

    //! @brief calculates the internal force vector for a given node
    //! @param rNodeId node id
    //! @param rNodeForce return value
    void NodeGetInternalForce(int rNodeId, NuTo::FullMatrix<double>& rNodeForce)const;

    //! @brief calculates the internal force vector for a given node group
    //! @param rGroupIdent group identifier
    //! @param rNodeForce return value
    void NodeGroupGetInternalForce(const std::string& rGroupIdent, NuTo::FullMatrix<double>& rNodeForce)const;

    //*************************************************
//************ Element routines     ***************
//**  defined in structures/StructureElement.cpp **
//*************************************************
    //! @brief returns the number of nodes
    //! @return number of nodes
    int GetNumElements() const;

#ifndef SWIG
    //! @brief returns a reference to an element
    //! @param identifier
    //! @return reference to an element
    ElementBase* ElementGetElementPtr(int rIdent);

    //! @brief returns a reference to an element
    //! @param identifier
    //! @return reference to an element
    const ElementBase* ElementGetElementPtr(int rIdent)const;

    //! @brief gives the identifier of an element
    //! @param pointer to an element
    //! @return identifier
    int ElementGetId(const ElementBase* rElement)const;
#endif //SWIG

    //! @brief info about the elements in the Structure
    void ElementInfo(int mVerboseLevel)const;

    //! @brief Creates an element
    //! @param rElementType element type
    //! @param rNodeIdents Identifier for the corresponding nodes
    //! @param rElementDataType data of the element (nonlocal,ip)
    //! @param rIpDataType data of the ip (static data, nonlocal data,..)
    //! @return element number
    int ElementCreate (const std::string& rElementType,
            const NuTo::FullMatrix<int>& rNodeNumbers, const std::string& rElementDataType, const std::string& rIpDataType);

    //! @brief Creates an element
    //! @param rElementType element type
    //! @param rNodeIdents Identifier for the corresponding nodes
    //! @return element number
    int ElementCreate (const std::string& rElementType, const NuTo::FullMatrix<int> &rNodeNumbers);

    //! @brief Creates an element
    //! @param rElementIdent identifier for the element
    //! @param rElementType element type
    //! @param rNodeIdents Identifier for the corresponding nodes
    void ElementCreate (int rElementNumber, const std::string& rElementType,
            const NuTo::FullMatrix<int> &rNodeNumbers);

    //! @brief Creates an element
    //! @param rElementNumber element number
    //! @param rElementType element type
    //! @param rNodeIdents Identifier for the corresponding nodes
    //! @param rElementDataType data of the element (nonlocal,ip)
    //! @param rIpDataType data of the ip (static data, nonlocal data,..)
    //! @return element number
    void ElementCreate (int rElementNumber, const std::string& rElementType,
    		const NuTo::FullMatrix<int> &rNodeNumbers, const std::string& rElementDataType, const std::string& rIpDataType);

    //! @brief creates multiple elements
    //! @param rElementType element type
    //! @param rNodeIdents Identifier for the corresponding nodes (Incidences have to be stored column-wise)
    //! @return a NuTo::FullMatrix<int> containing the element numbers
    NuTo::FullMatrix<int> ElementsCreate (const std::string& rElementType, NuTo::FullMatrix<int> &rNodeNumbers);

#ifndef SWIG
    //! @brief Creates an element
    //! @param rElementNumber element number
    //! @param rElementType element type
    //! @param rNodeIdents pointers to the corresponding nodes
    void ElementCreate(int rElementNumber, Element::eElementType rType, std::vector<NodeBase*> rNodeVector,
    		ElementData::eElementDataType rElementDataType, IpData::eIpDataType rIpDataType);

#endif //SWIG

    //*************************************************
    //************ Info routine         ***************
    //**  defined in structures/Structure.cpp *********
    //*************************************************
    //! @brief ... Info routine that prints general information about the object (detail according to verbose level)
    void Info()const;

protected:
    //! @brief ... store all elements of a structure in a vector
    //! @param rElements ... vector of element pointer
    void GetElementsTotal(std::vector<const ElementBase*>& rElements) const;

    //! @brief ... store all elements of a structure in a vector
    //! @param rElements ... vector of element pointer
    void GetElementsTotal(std::vector<ElementBase*>& rElements);

    //! @brief ... based on the global dofs build submatrices of the global coefficent matrix0
    //! @param rMatrixJJ ... submatrix jj (number of active dof x number of active dof)
    //! @param rMatrixJK ... submatrix jk (number of active dof x number of dependent dof)
    void BuildGlobalCoefficientSubMatrices0General(NuTo::SparseMatrix<double>& rMatrixJJ, NuTo::SparseMatrix<double>& rMatrixJK) const;

    //! @brief ... based on the global dofs build submatrices of the global coefficent matrix0
    //! @param rMatrixJJ ... submatrix jj (number of active dof x number of active dof)
    //! @param rMatrixJK ... submatrix jk (number of active dof x number of dependent dof)
    //! @param rMatrixKJ ... submatrix kj (number of dependent dof x number of active dof)
    //! @param rMatrixKK ... submatrix kk (number of dependent dof x number of dependent dof)
    void BuildGlobalCoefficientSubMatrices0General(NuTo::SparseMatrix<double>& rMatrixJJ, NuTo::SparseMatrix<double>& rMatrixJK, NuTo::SparseMatrix<double>& rMatrixKJ, NuTo::SparseMatrix<double>& rMatrixKK) const;

    //! @brief ... based on the global dofs build submatrices of the global coefficent matrix0
    //! @param rMatrixJJ ... submatrix jj (number of active dof x number of active dof)
    //! @param rMatrixJK ... submatrix jk (number of active dof x number of dependent dof)
    void BuildGlobalCoefficientSubMatrices0Symmetric(NuTo::SparseMatrix<double>& rMatrixJJ, NuTo::SparseMatrix<double>& rMatrixJK) const;

    //! @brief ... based on the global dofs build submatrices of the global coefficent matrix0
    //! @param rMatrixJJ ... submatrix jj (number of active dof x number of active dof)
    //! @param rMatrixJK ... submatrix jk (number of active dof x number of dependent dof)
    //! @param rMatrixKK ... submatrix kk (number of dependent dof x number of dependent dof)
    void BuildGlobalCoefficientSubMatrices0Symmetric(NuTo::SparseMatrix<double>& rMatrixJJ, NuTo::SparseMatrix<double>& rMatrixJK, NuTo::SparseMatrix<double>& rMatrixKK) const;

    //! @brief ... based on the global dofs build sub-vectors of the global internal potential gradient
    //! @param rActiveDofGradientVector ... global internal potential gradient which corresponds to the active dofs
    //! @param rDependentDofGradientVector ... global internal potential gradient which corresponds to the dependent dofs
    void BuildGlobalGradientInternalPotentialSubVectors(NuTo::FullMatrix<double>& rActiveDofGradientVector, NuTo::FullMatrix<double>& rDependentDofGradientVector) const;

    boost::ptr_map<int,NodeBase> mNodeMap;
    boost::ptr_map<int,ElementBase> mElementMap;
};
} //namespace NuTo
#ifdef ENABLE_SERIALIZATION
#ifndef SWIG
#include <boost/serialization/assume_abstract.hpp>
BOOST_SERIALIZATION_ASSUME_ABSTRACT(NuTo::Structure)
#endif // SWIG
#endif // ENABLE_SERIALIZATION
#endif // STRUCTURE_H