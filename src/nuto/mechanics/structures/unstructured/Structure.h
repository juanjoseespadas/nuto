#pragma once

#ifdef ENABLE_SERIALIZATION
#include <boost/serialization/access.hpp>
#endif // ENABLE_SERIALIZATION


#include "nuto/mechanics/structures/StructureBase.h"

#include <boost/ptr_container/ptr_map.hpp>
#include <set>

namespace NuTo
{
template <int TDim> class ContinuumElementIGA;

namespace ElementData
{
    enum class eElementDataType;
}// namespace ElementData

namespace Interpolation
{
    enum class eShapeType;
    enum class eTypeOrder;
}// namespace Interpolation

//! @author Jörg F. Unger, ISM
//! @date October 2009
//! @brief ... standard class for irregular (unstructured) structures

class Structure: public StructureBase
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif  // ENABLE_SERIALIZATION

public:
    //! @brief Typedefinitions
    //! @todo check if it is useful to switch to Boost::Ptr container types
    typedef ElementBase* elementBasePtr_t;
    typedef std::vector<elementBasePtr_t> elementBasePtrVec_t;
    typedef std::set<elementBasePtr_t> elementBasePtrSet_t;
    typedef NodeBase* nodeBasePtr_t;
    typedef std::map<nodeBasePtr_t, nodeBasePtr_t> nodeBasePtrMap_t;
    typedef std::set<nodeBasePtr_t> nodeBasePtrSet_t;
    typedef std::vector<nodeBasePtr_t> nodeBasePtrVec_t;

    //! @brief constructor
    //! @param mDimension  Structural dimension (1,2 or 3)
    Structure(int mDimension);

    //! @brief destructor
    virtual ~Structure();


#ifdef ENABLE_SERIALIZATION
#ifndef SWIG
    //! @brief serializes the class, this is the load routine
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void load(Archive & ar, const unsigned int version);

    template<class Archive>
    void loadImplement(Archive & ar);

    //! @brief serializes the class, this is the save routine
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const;

    template<class Archive>
    void saveImplement(Archive & ar) const;

    BOOST_SERIALIZATION_SPLIT_MEMBER()

#endif// SWIG

    //! @brief ... save the object to a file
    //! @param filename ... filename
    //! @param aType ... type of file, either BINARY, XML or TEXT
    void Save (const std::string &filename, std::string rType ) const override;

    //! @brief ... restore the object from a file
    //! @param filename ... filename
    //! @param aType ... type of file, either BINARY, XML or TEXT
    void Restore (const std::string &filename, std::string rType ) override;

    //! @brief ... save the object to a file
    //! @param filename ... filename
    //! @param aType ... type of file, either BINARY, XML or TEXT
    virtual void SaveUpdate (const std::string &filename, std::string rType ) const override;

    //! @brief ... restore the object from a file
    //! @param filename ... filename
    //! @param aType ... type of file, either BINARY, XML or TEXT
    virtual void RestoreUpdate (const std::string &filename, std::string rType ) override;

#endif // ENABLE_SERIALIZATION

    //! @brief ... Return the name of the class, this is important for the serialize routines, since this is stored in the file
    //!            in case of restoring from a file with the wrong object type, the file id is printed
    //! @return    class name
    std::string GetTypeId() const
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

#ifndef SWIG

    //! @brief ... evaluates the structure
    virtual void Evaluate(const NuTo::ConstitutiveInputMap& rInput, std::map<eStructureOutput, StructureOutputBase*> &rStructureOutput) override;

#endif


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
    const NodeBase* NodeGetNodePtr(int rIdent) const;

    //! @brief ... store all elements connected to this node in a vector
    //! @param rNode (Input) 		... node pointer
    //! @param rElements (Output) 	... vector of element pointers
    void NodeGetElements(const NodeBase* rNodePtr, std::vector<ElementBase*>& rElements) override;

    //! @brief gives the identifier of a node
    //! @param pointer to a node
    //! @return id
    int NodeGetId(const NodeBase* rNode) const;

    //! @brief returns const reference to mNodeMap
    //! @return mNodeMap
    const boost::ptr_map<int, NodeBase>& NodeGetNodeMap() const;
#endif //SWIG

    //! @brief ... return the global dof number of the displacement component of a node
    //! @param rNodeId (Input) 			... node id
    //! @param rDispDof 	... local disp dof (0,1 or 2 for x,y or z)
    //! @returnrglobal dof number
    int NodeGetDofDisplacement(int rNodeId, int rDispDof);

    //! @brief ... store all elements connected to this node in a vector
    //! @param rNodeId (Input) 			... node id
    //! @param rElementNumbers (Output)	... vector of element ids
    void NodeGetElements(const int rNodeId, NuTo::FullVector<int, Eigen::Dynamic>& rElementNumbers);

    //! @brief creates a node at coordinate's origin
    //! @return node number
    int NodeCreate();

    //! @brief creates a node with coordinates only
    //! @param rCoordinates coordinates of the node
    //! @return node number
    int NodeCreate(NuTo::FullVector<double, Eigen::Dynamic> rCoordinates);

    //! @brief creates a node with coordinates only
    //! @param rNodeNumber ... node number
    //! @param rCoordinates ...  node coordinates
    void NodeCreate(int rNodeNumber, NuTo::FullVector<double, Eigen::Dynamic> rCoordinates);

    //! @brief creates multiple nodes with coordinates only
    //! @param rCoordinates ...  nodal coordinates (column-wise storage of each nodal coordinate)
    //! @return a NuTo::FullMatrix<int,Eigen::Dynamic,Eigen::Dynamic> containing the node numbers
    NuTo::FullVector<int, Eigen::Dynamic> NodesCreate(NuTo::FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rCoordinates);

    //! @brief creates a node with specific dofs at coordinate's origin
    //! @param rDOFs ... space separated string containing the node dofs (e.g. displacements, rotations, temperatures)
    //! @param rCoordinates ...  node coordinates
    //! @return node number
    int NodeCreateDOFs(std::string rDOFs);

    //! @brief creates a node with specific dofs
    //! @param rDOFs ... space separated string containing the node dofs (e.g. displacements, rotations, temperatures)
    //! @param rCoordinates ...  node coordinates
    //! @return node number
    int NodeCreateDOFs(std::string rDOFs, NuTo::FullVector<double, Eigen::Dynamic> rCoordinates);

    //! @brief creates a node with specific dofs
    //! @param node number
    //! @param rDOFs ... space separated string containing the node dofs (e.g. displacements, rotations, temperatures)
    //! @param rCoordinates ...  node coordinates
    void NodeCreateDOFs(int rNodeNumber, std::string rDOFs, NuTo::FullVector<double, Eigen::Dynamic> rCoordinates);

#ifndef SWIG
    //! @brief creates a node with specific dofs at coordinate's origin
    //! @param rDOFs ... set containing the node dof enums (e.g. displacements, rotations, temperatures)
    //! @param rCoordinates ...  node coordinates
    //! @return node number
    int NodeCreateDOFs(std::set<Node::eDof> rDOFs);

    //! @brief creates a node with specific dofs
    //! @param rDOFs ... set containing the node dof enums (e.g. displacements, rotations, temperatures)
    //! @param rCoordinates ...  node coordinates
    //! @return node number
    int NodeCreateDOFs(std::set<Node::eDof> rDOFs, NuTo::FullVector<double, Eigen::Dynamic> rCoordinates);

    //! @brief creates a node with specific dofs
    //! @param node number
    //! @param rDOFs ... set containing the node dof enums (e.g. displacements, rotations, temperatures)
    //! @param rCoordinates ...  node coordinates
    void NodeCreateDOFs(int rNodeNumber, std::set<Node::eDof>rDOFs, NuTo::FullVector<double, Eigen::Dynamic> rCoordinates);
#endif //SWIG

    //! @brief deletes a node
    //! @param rNodeNumber ... node number
    void NodeDelete(const int rNodeNumber);

    //! @brief info about the nodes in the Structure
    void NodeInfo(int mVerboseLevel) const;

    //! @brief numbers the dofs in the structure
    //! @param rCallerName ... if the method throws it is nice to know by whom it was called.
    void NodeBuildGlobalDofs(std::string rCallerName = "") override;

    //! @brief extract dof values (e.g. displacements, temperatures to the nodes)
    //! @param rTimeDerivative time derivative (0 disp 1 vel 2 acc)
    //! @return ... StructureBlockVector containing the dofs (J and K)
    virtual NuTo::StructureOutputBlockVector NodeExtractDofValues(int rTimeDerivative) const override;

    //! @brief write dof values (e.g. displacements, temperatures to the nodes)
    //! @param rTimeDerivative time derivative (0 disp 1 vel 2 acc)
    //! @param rActiveDofValues ... vector of independent dof values (ordering according to global dofs, size is number of active dofs)
    //! @param rDependentDofValues ... vector of dependent  dof values (ordering according to global dofs, size is number of active dofs)
    virtual void NodeMergeDofValues(int rTimeDerivative, const NuTo::BlockFullVector<double>& rActiveDofValues, const NuTo::BlockFullVector<double>& rDependentDofValues) override;

    //! @brief calculate dependent dof values (for the zeroth time derivative)
    //! @param rActiveDofValues ... vector of independent dof values (ordering according to global dofs, size is number of active dofs)
    //! @return  ... vector of dependent  dof values (ordering according to global dofs, size is number of active dofs)
    virtual NuTo::BlockFullVector<double> NodeCalculateDependentDofValues(const NuTo::BlockFullVector<double>& rActiveDofValues) const override;

#ifndef SWIG


    //! @brief creates a node with rDofs degrees of freedom
    //! @param rCoordinates coordinates of the node
    //! @param rDofs degrees of freedom of the node
    //! @return node number
    int NodeCreate(NuTo::FullVector<double,Eigen::Dynamic> rCoordinates, std::set<Node::eDof> rDofs);


    //! @brief exchanges the node ptr in the full data set (elements, groups, loads, constraints etc.)
    //! this routine is used, if e.g. the data type of a node has changed, but the restraints, elements etc. are still identical
    //! @param rId ... old node id
    //! @param rOldPtr ... old node ptr
    //! @param rNewPtr ... new node ptr
    //! @param rElements (optional) ... vector of all elements that contain the node - speedup!
    void NodeExchangePtr(int rId, NuTo::NodeBase* rOldPtr, NuTo::NodeBase* rNewPtr, std::vector<ElementBase*> rElements = std::vector<ElementBase*>() );


#endif //SWIG

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
    const ElementBase* ElementGetElementPtr(int rIdent) const;

    //! @brief gives the identifier of an element
    //! @param pointer to an element
    //! @return identifier
    int ElementGetId(const ElementBase* rElement) const;

    //! @brief info about one single element
    //! @param rElement (Input) ... pointer to the element
    //! @param rVerboseLevel (Input) ... level of verbosity
    void ElementInfo(const ElementBase* rElement, int rVerboseLevel) const;
#endif //SWIG

    //! @brief returns a vector with the node ids of an element
    //! @param identifier
    //! @return vector with node ids
    NuTo::FullVector<int, Eigen::Dynamic> ElementGetNodes(int rId);

    //! @brief info about the elements in the Structure
    void ElementInfo(int rVerboseLevel) const;

//***************************************************//
//************ ElementCreate routines ***************//
//***************************************************//

    //---------------------------------------------------------------------------------------------------------------------------
    //! @brief Creates an IGA element, where the knot indices, beside the nodes (control points), are part of the data structures
    //! @param rInterpolationTypeId ... interpolation type id
    //! @param rNodeNumbers ... node indices
    //! @param rKnots ... knots defining the element
    //! @param rKnotIDs ... starting knot ids of the element in each element direction
    int ElementCreate(int rInterpolationTypeId,
                      const Eigen::VectorXi &rNodeNumbers,
                      const Eigen::MatrixXd &rKnots,
                      const Eigen::VectorXi &rKnotIDs);

    //! @brief Creates an element
    //! @param rInterpolationTypeId interpolation type id
    //! @param rNodeNumbers node indices
    int ElementCreate(int rInterpolationTypeId, const NuTo::FullVector<int, Eigen::Dynamic>& rNodeNumbers);

    //---------------------------------------------------------------------------------------------------------------------------
    //! @brief Creates an IGA element, where the knot indices, beside the nodes (control points), are part of the data structures
    //! @param rInterpolationTypeId ... interpolation type id
    //! @param rNodeNumbers  ... node indices
    //! @param rKnots ... knots defining the element
    //! @param rKnotIDs ... starting knot ids of the element in each element direction
    //! @param rElementDataType  ... Element data for the elements
    //! @param rIpDataType  ... Integration point data for the elements
    int ElementCreate(int   rInterpolationTypeId,
                      const Eigen::VectorXi &rNodeNumbers,
                      const Eigen::MatrixXd &rKnots,
                      const Eigen::VectorXi &rKnotIDs,
                      const std::string     &rElementDataType,
                      const std::string     &rIpDataType);

    //! @brief Creates an element
    //! @param rInterpolationTypeId interpolation type id
    //! @param rNodeNumbers node indices
    //! @param rElementDataType Element data for the elements
    //! @param rIpDataType Integration point data for the elements
    int ElementCreate(int rInterpolationTypeId,
                      const NuTo::FullVector<int, Eigen::Dynamic>& rNodeNumbers,
                      const std::string& rElementDataType,
                      const std::string& rIpDataType);

    //---------------------------------------------------------------------------------------------------------------------------
    //! @brief Creates an IGA element, where the knot indices, beside the nodes (control points), are part of the data structures
    //! @param rElementNumber ... element number
    //! @param rInterpolationTypeId ... interpolation type id
    //! @param rNodeNumbers  ... node indices
    //! @param rKnots ... knots defining the element
    //! @param rKnotIDs ... starting knot ids of the element in each element direction
    //! @param rElementDataType  ... Element data for the elements
    //! @param rIpDataType  ... Integration point data for the elements
    void ElementCreate(int rElementNumber,
                       int rInterpolationTypeId,
                       const Eigen::VectorXi &rNodeNumbers,
                       const Eigen::MatrixXd &rKnots,
                       const Eigen::VectorXi &rKnotIDs,
                       const std::string     &rElementDataType,
                       const std::string     &rIpDataType);

    //! @brief Creates an element
    //! @param rElementNumber ... element number
    //! @param rInterpolationTypeId ... interpolation type id
    //! @param rNodeNumbers  ... node indices
    //! @param rElementDataType  ... Element data for the elements
    //! @param rIpDataType  ... Integration point data for the elements
    void ElementCreate(int rElementNumber,
                       int rInterpolationTypeId,
                       const NuTo::FullVector<int, Eigen::Dynamic>& rNodeNumbers,
                       const std::string& rElementDataType,
                       const std::string& rIpDataType);


    //! @brief creates multiple elements and adds them to an element group
    //! @param rInterpolationTypeId interpolation type id
    //! @param rNodeIdents Identifier for the corresponding nodes (Incidences have to be stored column-wise)
    //! @return index to the new element group
    int ElementsCreate(int rInterpolationTypeId, NuTo::FullMatrix<int, Eigen::Dynamic, Eigen::Dynamic>& rNodeNumbers);

    //! @brief creates multiple elements and adds them to an element group
    //! @param rInterpolationTypeId interpolation type id
    //! @param rNodeIdents Identifier for the corresponding nodes (Incidences have to be stored column-wise)
    //! @param rElementDataType Element data for the elements
    //! @param rIpDataType Integration point data for the elements
    //! @return index to the new element group
    int ElementsCreate(int rInterpolationTypeId, NuTo::FullMatrix<int, Eigen::Dynamic, Eigen::Dynamic>& rNodeNumbers, const std::string& rElementDataType, const std::string& rIpDataType);

    //! @brief changes the node structure to match the interpolation type for all elements
    //! the node merge distance and the box size are calculated from the element sizes
    void ElementTotalConvertToInterpolationType();

    //! @brief changes the node structure to match the interpolation type
    //! the node merge distance and the box size are calculated from the element sizes
    //! @param rGroupNumberElements group for elements (coordinates only) to be converted
    void ElementConvertToInterpolationType(int rGroupNumberElements);

    //! @brief changes the node structure to match the interpolation type for all elements
    //! @param rNodeDistanceMerge Distance of nodes to be joined (should be significantly smaller than the node distance in the mesh)
    //! @param rMeshSize approximate size of the elements
    void ElementTotalConvertToInterpolationType(double rNodeDistanceMerge, double rMeshSize);

    //! @brief changes the node structure to match the interpolation type
    //! @param rGroupNumberElements group for elements (coordinates only) to be converted
    //! @param rNodeDistanceMerge Distance of nodes to be joined (should be significantly smaller than the node distance in the mesh)
    //! @param rMeshSize approximate size of the elements
    void ElementConvertToInterpolationType(int rGroupNumberElements, double rNodeDistanceMerge, double rMeshSize);


    //! @brief Deletes an element
    //! @param rElementNumber element number
    void ElementDelete(int rElementNumber);

    //! @brief Deletes a group of elements element
    //! @param rGroupNumber group number
    void ElementGroupDelete(int rGroupNumber, bool deleteNodes);

#ifndef SWIG
    //! @brief Deletes an element
    //! @param rItElement iterator of the map
    void ElementDeleteInternal(int rElementId);

    //---------------------------------------------------------------------------------------------------------------------------
    //! @brief Creates an IGA element, where the knot indices, beside the nodes (control points), are part of the data structures
    //! @param rInterpolationTypeId ... interpolation type id
    //! @param rNodeNumbers ... pointers to the corresponding nodes
    //! @param rKnots ... knots defining the element
    //! @param rKnotIDs ... starting knot ids of the element in each element direction
    //! @param rElementDataType ... Element data for the elements
    //! @param rIpDataType ... Integration point data for the elements
    //! @return int rElementNumber
    int ElementCreate(int rInterpolationTypeId,
                      const std::vector<NodeBase*> &rNodeNumbers,
                      const Eigen::MatrixXd &rKnots,
                      const Eigen::VectorXi &rKnotIDs,
                      ElementData::eElementDataType rElementDataType,
                      IpData::eIpDataType rIpDataType);

    //! @brief Creates an element
    //! @param rInterpolationTypeId interpolation type id
    //! @param rNodeVector pointers to the corresponding nodes
    //! @param rElementDataType Element data for the elements
    //! @param rIpDataType Integration point data for the elements
    //! @return int rElementNumber
    int ElementCreate(int rInterpolationTypeId,
                      const std::vector<NodeBase*>& rNodeVector,
                      ElementData::eElementDataType rElementDataType,
                      IpData::eIpDataType rIpDataType);

    //---------------------------------------------------------------------------------------------------------------------------
    //! @brief Creates an IGA element, where the knot indices, beside the nodes (control points), are part of the data structures
    //! @param rInterpolationTypeId ... interpolation type id
    //! @param rNodeNumbers ... node ids to the corresponding nodes
    //! @param rKnots ... knots defining the element
    //! @param rKnotIDs ... starting knot ids of the element in each element direction
    //! @param rElementDataType ... Element data for the elements
    //! @param rIpDataType ... Integration point data for the elements
    //! @return int rElementNumber
    int ElementCreate(int rInterpolationTypeId,
                      const Eigen::VectorXi &rNodeNumbers,
                      const Eigen::MatrixXd &rKnots,
                      const Eigen::VectorXi &rKnotIDs,
                      ElementData::eElementDataType rElementDataType,
                      IpData::eIpDataType rIpDataType);

    //! @brief Creates an element
    //! @param rInterpolationTypeId ... interpolation type id
    //! @param rNodeNumbers...  node indices
    //! @param rElementDataType ... Element data for the elements
    //! @param rIpDataType ... Integration point data for the elements
    //! @return int rElementNumber
    int ElementCreate(int rInterpolationTypeId,
                      const NuTo::FullVector<int, Eigen::Dynamic>& rNodeNumbers,
                      ElementData::eElementDataType rElementDataType,
                      IpData::eIpDataType rIpDataType);

    //---------------------------------------------------------------------------------------------------------------------------
    //! @brief Creates an IGA element, where the knot indices, beside the nodes (control points), are part of the data structures
    //! @param rElementNumber ... element number
    //! @param rInterpolationTypeId ... interpolation type id
    //! @param rNodeVector ... pointers to the corresponding nodes
    //! @param rKnots ... knots defining the element
    //! @param rKnotIDs ... starting knot ids of the element in each element direction
    //! @param rElementType ... element type
    //! @param rIpDataType ... Integration point data for the elements
    void ElementCreate(int rElementNumber,
                       int rInterpolationTypeId,
                       const std::vector<NodeBase*> &rNodeVector,
                       const Eigen::MatrixXd &rKnots,
                       const Eigen::VectorXi &rKnotIDs,
                       ElementData::eElementDataType rElementDataType,
                       IpData::eIpDataType rIpDataType);


    //! @brief Creates an element
    //! @param rElementNumber ... element number
    //! @param rInterpolationTypeId ... interpolation type id
    //! @param rNodeVector ... pointers to the corresponding nodes
    //! @param rElementType ...  element type
    //! @param rIpDataType ...  Integration point data for the elements
    void ElementCreate(int rElementNumber,
                       int rInterpolationTypeId,
                       const std::vector<NodeBase*>& rNodeVector,
                       ElementData::eElementDataType rElementDataType,
                       IpData::eIpDataType rIpDataType);


    void ElementCreate(int rElementNumber, int rInterpolationTypeId, const std::vector<int>& rNodeIds, ElementData::eElementDataType rElementDataType, IpData::eIpDataType rIpDataType);

    template<int TDimMaster>
    Eigen::Matrix<std::pair<const ContinuumElementIGA<TDimMaster>*, int>, Eigen::Dynamic, Eigen::Dynamic> ContactElementsCreateMaster(const Eigen::Matrix<std::pair<int, int>, Eigen::Dynamic, Eigen::Dynamic> &rMasterElementsID);

    template<int TDimSlave, int TDimMaster>
    int ContactElementsCreate(int rElementsGroupIDSlave,
                              int rNodeGroupSlaveId,
                              const Eigen::Matrix<std::pair<int, int>,Eigen::Dynamic,Eigen::Dynamic> &rMasterElementsID,
                              eIntegrationType rIntegrationType,
                              int rContactAlgorithm,
                              int rConstitutiveLaw);

    //! @brief Returns the internal enum (number) for the element types
    //! @param Element name in Nuto
    //! @return enum
//    NuTo::Element::eElementType ElementTypeGetEnum(const std::string& rElementType);
#endif //SWIG

    //! @brief creates boundary elements and add them to an element group
    //! @param rElementGroupId ... group id including the base elements
    //! @param rNodeGroupId ... node group id that includes the surface nodes
    //! @param rControlNode if not nullptr, then a boundary element with control node will be created
    //! @return ... ids of the created boundary element group
    int BoundaryElementsCreate(int rElementGroupId, int rNodeGroupId, NodeBase* rControlNode = nullptr);


    //! @brief  Creates interface elements from an element group.
    //! @param  rElementGroupId: group id including the base elements
    //! @param  rInterfaceInterpolationType: interpolation type of the interface elements
    //! @param  rFibreInterpolationType: interpolation type of the interface elements
    //! @return returns a pair with the group ids of the new fiber and interface elements
    std::pair<int,int> InterfaceElementsCreate(int rElementGroupId, int rInterfaceInterpolationType, int rFibreInterpolationType);

#ifndef SWIG
    //! @brief import from gmsh, creates groups according to gmsh's physical entities and creates an interpolation types for each group
    //! @param rFileName .. file name
    //! @param rElementData .. element data for the elements to be created
    //! @param rIPData .. ip data for the integration points to be created
    //! @return .. Matrix [NumGroups x 2] with [: x 0] group ids and [ : x 1] corresponding interpolation type ids
    FullMatrix<int, Eigen::Dynamic, Eigen::Dynamic> ImportFromGmsh(const std::string& rFileName, ElementData::eElementDataType rElementData, IpData::eIpDataType rIPData);


#endif //SWIG

    //! @brief import from gmsh, creates groups according to gmsh's physical entities and creates an interpolation types for each group
    //! @param rFileName .. file name
    //! @param rElementData .. element data for the elements to be created
    //! @param rIPData .. ip data for the integration points to be created
    //! @return .. Matrix [NumGroups x 2] with [: x 0] group ids and [ : x 1] corresponding interpolation type ids
    FullMatrix<int, Eigen::Dynamic, Eigen::Dynamic> ImportFromGmsh(const std::string& rFileName, const std::string& rElementData, const std::string& rIPData);

    //*************************************************
    //**      InterpolationType routines             **
    //**    defined in structures/                   **
    //**     ...StructureBaseInterpolationType.cpp   **
    //*************************************************

    //! @brief creates a new interpolation type, calls the enum method
    //! @param rShape ... element shape "TRUSS", "TRIANGLE", "QUAD", "TETRAEDER", "BRICK", etc.
    //! @return ... interpolation type id
    int InterpolationTypeCreate(const std::string& rShape);

    //! @brief creates a new interpolation type, calls the enum method
    //! @param rShape ... element shape "TRUSS", "TRIANGLE", "QUAD", "TET", "BRICK", etc.
    //! @return ... interpolation type id
    int InterpolationTypeCreate(NuTo::Interpolation::eShapeType rShape);

    //! @brief sets the integration type for a specific interpolation type
    //! @param rInterpolationTypeId ... interpolation type id
    //! @param rIntegrationType ... integration type string
    //! @param rIpDataType ... ip data type string
    void InterpolationTypeSetIntegrationType(int rInterpolationTypeId, const std::string& rIntegrationType, const std::string& rIpDataType);

    //! @brief prints the info to the interpolation type
    //! @param rInterpolationTypeId ... interpolation type id
    void InterpolationTypeInfo(int rInterpolationTypeId) const;

    //! @brief adds a dof to a interpolation type, calls the enum method
    //! @param rInterpolationTypeId ... interpolation type id
    //! @param rDofType ... dof type
    //! @param rTypeOrder ... type and order of interpolation
    void InterpolationTypeAdd(int rInterpolationTypeId, const std::string& rDofType, const std::string& rTypeOrder);

#ifndef SWIG

    //! @brief creates a new interpolation type
    //! @param rInterpolationTypeId ... interpolation type id
    //! @param rShape ... element shape "1DTRUSS", "2DTRIANGLE", "2DQUAD", "3DTET", "3DBRICK", etc.
    void InterpolationTypeCreate(int rInterpolationTypeId, NuTo::Interpolation::eShapeType rShape);

    //! @brief sets the integration type for a specific interpolation type
    //! @param rInterpolationTypeId ... interpolation type id
    //! @param rIntegrationType ... integration type enum
    //! @param rIpDataType ... ip data type enum
    void InterpolationTypeSetIntegrationType(int rInterpolationTypeId, eIntegrationType rIntegrationType, IpData::eIpDataType rIpDataType);

    //! @brief sets the integration type for a specific interpolation type
    //! @param rInterpolationTypeId ... interpolation type id
    //! @param rIntegrationType ... integration type pointer
    //! @param rIpDataType ... ip data type enum
    void InterpolationTypeSetIntegrationType(int rInterpolationTypeId, IntegrationTypeBase* rIntegrationType, IpData::eIpDataType rIpDataType);

    //! @brief adds a dof to a IGA interpolation type
    //! @param rInterpolationTypeId ... interpolation type id
    //! @param rDofType ... dof type
    //! @param rTypeOrder ... type and order of interpolation
//    void InterpolationTypeAdd(int rInterpolationTypeId, int rDegree, const Eigen::VectorXd &rKnots);

    //! @brief adds a dof to a interpolation type
    //! @param rInterpolationTypeId ... interpolation type id
    //! @param rDofType ... dof type
    //! @param rTypeOrder ... type and order of interpolation
    void InterpolationTypeAdd(int rInterpolationTypeId, NuTo::Node::eDof rDofType, NuTo::Interpolation::eTypeOrder rTypeOrder);


    //! @brief adds a dof to a interpolation type
    //! @param rInterpolationTypeId ... interpolation type id
    //! @param rDofType ... dof type
    //! @param rTypeOrder ... type and order of interpolation
    void InterpolationTypeAdd(int rInterpolationTypeId,
                              NuTo::Node::eDof rDofType,
                              Interpolation::eTypeOrder rTypeOrder,
                              const Eigen::VectorXi &rDegree,
                              const std::vector<Eigen::VectorXd> &rKnots,
                              const Eigen::MatrixXd &rWeights);

#endif //SWIG

    //***********************************************************
    //************         Mesh routines        *****************
    //**  defined in structures/unstructured/StructureMesh.cpp **
    //***********************************************************
    //! @brief creates a lattice mesh from the positions of the circles
    //! @parameters rTypeOfSpecimen 0 box, 1 dogbone
    //! @parameters rBoundingBox box for the spheres (3*2 matrix)
    //! @parameters rCircles (coordinates x,y and radius)
    //! @parameters rTriangles (triangles connecting the circle centers)
    void MeshCreateLattice2D(int rTypeOfSpecimen, NuTo::FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rBoundingBox, NuTo::FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rCircles, NuTo::FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rTriangles);

    //! @brief creates a lattice mesh from the positions of the spheres and the bounding box
    //! @parameters rTypeOfSpecimen 0 box, 1 dogbone
    //! @parameters rBoundingBox box for the spheres (3*2 matrix)
    //! @parameters rBoundingBox (min and max for x and y)
    //! @parameters rSpheres (coordinates x,y,z and radius)
    void MeshCreateLattice3D(int rTypeOfSpecimen, NuTo::FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rBoundingBox, NuTo::FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rSpheres, NuTo::FullMatrix<double, Eigen::Dynamic, Eigen::Dynamic>& rTetraeders);


#ifndef SWIG
    //! @brief copy and move the structure
    //! most of the data is kept, but e.g. nonlocal data and
    //! @param rOffset offset (dimension x 1 has to be identical with structure dimension)
    //! @param rOld2NewNodePointer ptrMap showing the new and old node pointers
    //! @param rOld2NewElementPointer ptrMap showing the new and old element pointers
    void CopyAndTranslate(NuTo::FullVector<double, Eigen::Dynamic>& rOffset, std::map<NodeBase*, NodeBase*>& rOld2NewNodePointer, std::map<ElementBase*, ElementBase*>& rOld2NewElementPointer);
#endif //SWIG

    //! @brief copy and move the structure
    //! most of the data is kept, but e.g. nonlocal data and
    //! @param rOffset offset (dimension x 1 has to be identical with structure dimension)
    void CopyAndTranslate(NuTo::FullVector<double, Eigen::Dynamic>& rOffset);

    //! @brief ... Adds an element to an element group
    //! @param ... rIdentGroup identifier for the group
    //! @param ... rIdentElement  identifier for the element
    void GroupAddElement(int rIdentGroup, int rIdElement);

    //! @brief ... Adds all elements to an element group
    //! @param ... rIdentGroup identifier for the group
    void GroupAddElementsTotal(int rIdentGroup);

    //! @brief ... Adds all elements to a group based on the type
    //! @param ... rIdentGroup identifier for the group
    //! @param ... rInterpolationType  identifier for the interpolation type
    void GroupAddElementFromType(int rIdentGroup, int rInterpolationType);

    //! @brief ... Adds all nodes in rSearchIdenGroup to rIdentGroup whose coordinates are in the specified range
    //! @param ... rIdentNodeGroup identifier for the group
    //! @param ... rSearchIdentElementGroup identifier for the group
    //! @param ... rDirection either 0,1,2 for x,y, or z
    //! @param ... rMin ... minimum value
    //! @param ... rMax ... maximum value
    void GroupAddNodeFromElementGroupCoordinateRange(int rIdentNodeGroup, int rSearchIdentElementGroup, int rDirection, double rMin, double rMax);

    //! @brief adds all elements to an element group and returns its id
    int GroupGetElementsTotal();

    //! @brief adds all ndoes to an element group and returns its id
    int GroupGetNodesTotal();

    //*************************************************
    //************ Info routine         ***************
    //**  defined in structures/Structure.cpp *********
    //*************************************************
    //! @brief ... Info routine that prints general information about the object (detail according to verbose level)
    void Info() const;

protected:
#ifdef ENABLE_SERIALIZATION
    //! @brief ... standard constructor just for the serialization routine
    Structure()
    {
    }
#endif  // ENABLE_SERIALIZATION

#ifndef SWIG
    //! @brief ... store all elements of a structure in a vector
    //! @param rElements ... vector of element pointer
    void GetElementsTotal(std::vector<const ElementBase*>& rElements) const;

    //! @brief ... store all elements of a structure in a vector
    //! @param rElements ... vector of element pointer
    void GetElementsTotal(std::vector<std::pair<int, const ElementBase*> >& rElements) const;

    //! @brief ... store all elements of a structure in a vector
    //! @param rElements ... vector of element pointer
    void GetElementsTotal(std::vector<ElementBase*>& rElements);

    //! @brief ... store all elements of a structure in a vector
    //! @param rElements ... vector of element pointer
    void GetElementsTotal(std::vector<std::pair<int, ElementBase*> >& rElements);

    //! @brief ... store all nodes of a structure in a vector
    //! @param rNodes ... vector of element pointer
    void GetNodesTotal(std::vector<const NodeBase*>& rNodess) const;

    //! @brief ... store all nodes of a structure in a vector
    //! @param rNodes ... vector of element pointer
    void GetNodesTotal(std::vector<std::pair<int, const NodeBase*> >& rNodes) const;

    //! @brief ... store all nodes of a structure in a vector
    //! @param rNodes ... vector of element pointer
    void GetNodesTotal(std::vector<NodeBase*>& rNodes);

    //! @brief ... store all nodes of a structure in a vector
    //! @param rNodes ... vector of element pointer
    void GetNodesTotal(std::vector<std::pair<int, NodeBase*> >& rNodes);
#endif

    //! @brief deletes a node
    //! @param rNodeNumber ... node number
    //! @param checkElements ... check the elements, if set to false, make sure that the node is not part of any element
    void NodeDelete(int rNodeNumber, bool checkElements);

#ifndef SWIG
    //! @brief import from gmsh, creates groups according to gmsh's physical entities and creates an interpolation types for each group
    //! @param rFileName .. file name
    //! @param rElementData .. element data for the elements to be created
    //! @param rIPData .. ip data for the integration points to be created
    //! @return .. Matrix [NumGroups x 2] with [: x 0] group ids and [ : x 1] corresponding interpolation type ids
    FullMatrix<int, Eigen::Dynamic, Eigen::Dynamic> ImportFromGmshAux(const std::string& rFileName, ElementData::eElementDataType rElementData, IpData::eIpDataType rIPData);
#endif //SWIG

    boost::ptr_map<int, NodeBase> mNodeMap;
    boost::ptr_map<int, ElementBase> mElementMap;

private:

    //! @brief creates a node
    //! @param rDOFs
    //! @param rCoordinates coordinates of the node
    //! @return node pointer
    NodeBase* NodePtrCreate(std::set<Node::eDof> rDOFs, NuTo::FullVector<double, Eigen::Dynamic> rCoordinates);

    //! @brief determines the dimensionality of a dof type
    //! @param rDof ... specific dof type
    //! @return dimension
    int GetDofDimension(Node::eDof rDof);
};
} //namespace NuTo
#ifdef ENABLE_SERIALIZATION
#ifndef SWIG
BOOST_CLASS_EXPORT_KEY(NuTo::Structure)
#endif // SWIG
#endif // ENABLE_SERIALIZATION

