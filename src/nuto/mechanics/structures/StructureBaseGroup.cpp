// $Id: $

#include "nuto/mechanics/structures/StructureBase.h"
#include "nuto/mechanics/groups/Group.h"

void NuTo::StructureBase::GroupInfo(int rVerboseLevel)const
{
    std::cout << "number of groups  : " << mGroupMap.size() << std::endl;
    if (rVerboseLevel>2)
    {
        for (boost::ptr_map<int,GroupBase>::const_iterator it= mGroupMap.begin(); it!=mGroupMap.end(); it++)
        {
            std::cout << "  Group " << it->first << std::endl;
            it->second->Info(rVerboseLevel, this);
            std::cout <<  std::endl;
        }
    }

}

//! @brief gives the identifier of a group
//! @param pointer to a group
//! @return identifier
int NuTo::StructureBase::GroupGetId(GroupBase* rGroup)const
{
    for (boost::ptr_map<int,GroupBase>::const_iterator
            it = mGroupMap.begin(); it!= mGroupMap.end(); it++)
    {
        if (it->second==rGroup)
            return it->first;
    }
    throw MechanicsException("[NuTo::Structure::GroupGetId] Group does not exist.");
}

//! @brief ... Creates a group for the structure
//! @param ... rType  type of the group, e.g. "NODES" or "ELEMENTS"
//! @return ... rIdent identifier for the group
int NuTo::StructureBase::GroupCreate(const std::string& rType)
{
	//find unused integer id
	int groupNumber(mGroupMap.size());
	boost::ptr_map<int,GroupBase>::iterator it = mGroupMap.find(groupNumber);
	while (it!=mGroupMap.end())
	{
		groupNumber++;
		it = mGroupMap.find(groupNumber);
	}

	// transform string to uppercase
    std::string GroupTypeString;
    std::transform(rType.begin(), rType.end(), std::back_inserter(GroupTypeString), (int(*)(int)) toupper);

    if (GroupTypeString==std::string("ELEMENTS"))
    {
        GroupCreate(groupNumber,NuTo::Groups::Elements);
    }
    else
    {
		if (GroupTypeString==std::string("NODES"))
		{
			GroupCreate(groupNumber,NuTo::Groups::Nodes);
		}
		else
		{
			throw MechanicsException("[NuTo::StructureBase::GroupCreate] Group type not implemented.");
		}
    }

    return groupNumber;
}

//! @brief ... Creates a group for the structure
//! @param ... rIdent identifier for the group
//! @param ... rType  type of the group
void NuTo::StructureBase::GroupCreate(int id, NuTo::Groups::eGroupId rEnumType)
{
	boost::ptr_map<int,GroupBase>::iterator it = mGroupMap.find(id);
	if (it!=mGroupMap.end())
		throw MechanicsException("[NuTo::StructureBase::GroupCreate] Group id already exists.");

    switch(rEnumType)
    {
    case NuTo::Groups::Elements:
    	mGroupMap.insert(id, new NuTo::Group<ElementBase>);
    	break;
    case NuTo::Groups::Nodes:
    	mGroupMap.insert(id, new NuTo::Group<NodeBase>);
    	break;
    default:
    	throw MechanicsException("[NuTo::StructureBase::GroupCreate] Group type not implemented.");
    }
}

//! @brief ... Deletes a group from the structure
//! @param ... rIdent identifier for the group
void NuTo::StructureBase::GroupDelete(int rIdentGroup)
{

    // find group in map
    boost::ptr_map<int,GroupBase>::iterator itGroup = mGroupMap.find(rIdentGroup);
    if (itGroup == this->mGroupMap.end())
    {
        throw MechanicsException("[NuTo::StructureBase::GroupDelete] Group with the given identifier does not exist.");
    }
    else
    {
        // delete load from map
        this->mGroupMap.erase(itGroup);
    }
}

//! @brief ... Adds a node to a node group
//! @param ... rIdentGroup identifier for the group
//! @param ... rIdentNode  identifier for the node
void NuTo::StructureBase::GroupAddNode(int rIdentGroup, int rIdNode)
{
    boost::ptr_map<int,GroupBase>::iterator itGroup = mGroupMap.find(rIdentGroup);
    if (itGroup==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::GroupAddNode] Group with the given identifier does not exist.");
    if (itGroup->second->GetType()!=Groups::Nodes)
        throw MechanicsException("[NuTo::StructureBase::GroupAddNode] A node can be added only to a node group.");

    itGroup->second->AddMember(NodeGetNodePtr(rIdNode));
}

//! @brief ... Adds a node to a node group
//! @param ... rIdentGroup identifier for the group
//! @param ... rIdentNode  identifier for the node
void NuTo::StructureBase::GroupAddElement(int rIdentGroup, int rIdElement)
{
    boost::ptr_map<int,GroupBase>::iterator itGroup = mGroupMap.find(rIdentGroup);
    if (itGroup==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::GroupAddElement] Group with the given identifier does not exist.");
    if (itGroup->second->GetType()!=Groups::Elements)
        throw MechanicsException("[NuTo::StructureBase::GroupAddElement] An element can be added only to an element group.");

    itGroup->second->AddMember(ElementGetElementPtr(rIdElement));
}

//! @brief ... Unites two groups and stores the result in a new group
//! @param ... rIdentGroup1 identifier for the first group
//! @param ... rIdentGroup2 identifier for the second group
//! @result ... rIdentGroupResult identifier for the created result group
int NuTo::StructureBase::GroupUnion(int rIdentGroup1, int rIdentGroup2)
{
    boost::ptr_map<int,GroupBase>::iterator itGroup1 = mGroupMap.find(rIdentGroup1);
    if (itGroup1==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::GroupUnite] Group1 with the given identifier does not exist.");
    boost::ptr_map<int,GroupBase>::iterator itGroup2 = mGroupMap.find(rIdentGroup2);
    if (itGroup2==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::GroupUnite] Group2 with the given identifier does not exist.");

    //find unused integer id
	int groupNumber(mGroupMap.size());
	boost::ptr_map<int,GroupBase>::iterator it = mGroupMap.find(groupNumber);
	while (it!=mGroupMap.end())
	{
		groupNumber++;
		it = mGroupMap.find(groupNumber);
	}
    // insert the member
    mGroupMap.insert(groupNumber, (*itGroup1).second->Unite((*itGroup2).second));
    return groupNumber;
}

//! @brief ... Difference between two groups and stores the result in a new group
//! @param ... rIdentGroup1 identifier for the first group
//! @param ... rIdentGroup2 identifier for the second group
//! @result ... rIdentGroupResult identifier for the created result group
int NuTo::StructureBase::GroupDifference(int rIdentGroup1, int rIdentGroup2)
{
    boost::ptr_map<int,GroupBase>::iterator itGroup1 = mGroupMap.find(rIdentGroup1);
    if (itGroup1==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::GroupUnite] Group1 with the given identifier does not exist.");
    boost::ptr_map<int,GroupBase>::iterator itGroup2 = mGroupMap.find(rIdentGroup2);
    if (itGroup2==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::GroupUnite] Group2 with the given identifier does not exist.");
    //find unused integer id
	int groupNumber(mGroupMap.size());
	boost::ptr_map<int,GroupBase>::iterator it = mGroupMap.find(groupNumber);
	while (it!=mGroupMap.end())
	{
		groupNumber++;
		it = mGroupMap.find(groupNumber);
	}

    // insert the member
    mGroupMap.insert(groupNumber, (*itGroup1).second->Difference((*itGroup2).second));

    return groupNumber;
}

//! @brief ... Calculates the intersection between two groups and stores the result in a new group
//! @param ... rIdentGroup1 identifier for the first group
//! @param ... rIdentGroup2 identifier for the second group
//! @result ... rIdentGroupResult identifier for the created result group
int NuTo::StructureBase::GroupIntersection(int rIdentGroup1, int rIdentGroup2)
{
    boost::ptr_map<int,GroupBase>::iterator itGroup1 = mGroupMap.find(rIdentGroup1);
    if (itGroup1==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::GroupUnite] Group1 with the given identifier does not exist.");
    boost::ptr_map<int,GroupBase>::iterator itGroup2 = mGroupMap.find(rIdentGroup2);
    if (itGroup2==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::GroupUnite] Group2 with the given identifier does not exist.");

    //find unused integer id
	int groupNumber(mGroupMap.size());
	boost::ptr_map<int,GroupBase>::iterator it = mGroupMap.find(groupNumber);
	while (it!=mGroupMap.end())
	{
		groupNumber++;
		it = mGroupMap.find(groupNumber);
	}

    // insert the member
    mGroupMap.insert(groupNumber, (*itGroup1).second->Intersection((*itGroup2).second));

    return groupNumber;
}

//! @brief ... Calculates the symmetric difference between two groups and stores the result in a new group
//! @param ... rIdentGroup1 identifier for the first group
//! @param ... rIdentGroup2 identifier for the second group
//! @result ... rIdentGroupResult identifier for the created result group
int NuTo::StructureBase::GroupSymmetricDifference(int rIdentGroup1, int rIdentGroup2)
{
    boost::ptr_map<int,GroupBase>::iterator itGroup1 = mGroupMap.find(rIdentGroup1);
    if (itGroup1==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::GroupUnite] Group1 with the given identifier does not exist.");
    boost::ptr_map<int,GroupBase>::iterator itGroup2 = mGroupMap.find(rIdentGroup2);
    if (itGroup2==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::GroupUnite] Group2 with the given identifier does not exist.");

    //find unused integer id
	int groupNumber(mGroupMap.size());
	boost::ptr_map<int,GroupBase>::iterator it = mGroupMap.find(groupNumber);
	while (it!=mGroupMap.end())
	{
		groupNumber++;
		it = mGroupMap.find(groupNumber);
	}

    // insert the member
    mGroupMap.insert(groupNumber, (*itGroup1).second->SymmetricDifference((*itGroup2).second));

    return groupNumber;
}

//! @brief ... Returns the number of members in a group
//! @param ... rIdentGroup identifier for the group
//! @return ... number of members
int NuTo::StructureBase::GroupGetNumMembers(int rIdentGroup)const
{
    boost::ptr_map<int,GroupBase>::const_iterator itGroup = mGroupMap.find(rIdentGroup);
    if (itGroup==mGroupMap.end())
        throw MechanicsException("[NuTo::StructureBase::GroupGetNumMembers] Group with the given identifier does not exist.");
    return (*itGroup).second->GetNumMembers();
}

// get group pointer from group identifier
NuTo::GroupBase* NuTo::StructureBase::GroupGetGroupPtr(int rIdent)
{
    boost::ptr_map<int,GroupBase>::iterator it = this->mGroupMap.find(rIdent);
    if (it == this->mGroupMap.end())
    {
        throw NuTo::MechanicsException("[NuTo::StructureBase::GroupGetGroupPtr] Group does not exist.");
    }
    return it->second;
}

// get section pointer from section identifier
const NuTo::GroupBase* NuTo::StructureBase::GroupGetGroupPtr(int rIdent) const
{
    boost::ptr_map<int,GroupBase>::const_iterator it = this->mGroupMap.find(rIdent);
    if (it == this->mGroupMap.end())
    {
        throw NuTo::MechanicsException("[NuTo::StructureBase::GroupGetGroupPtr] Group does not exist.");
    }
    return it->second;
}
