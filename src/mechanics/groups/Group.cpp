#include "mechanics/groups/Group.h"

#include <iostream>

#include "mechanics/elements/ElementBase.h"
#include "mechanics/groups/GroupEnum.h"
#include "mechanics/nodes/NodeBase.h"


namespace NuTo
{
template <>
eGroupId Group<NodeBase>::GetType() const
{
    return eGroupId::Nodes;
}

template <>
eGroupId Group<ElementBase>::GetType() const
{
    return eGroupId::Elements;
}
template <>
std::string Group<NodeBase>::GetTypeString() const
{
    return std::string("Nodes");
}

template <>
std::string Group<ElementBase>::GetTypeString() const
{
    return std::string("Elements");
}

//! @brief either casts the pointer to an element group or throws an exception for groups which are not element groups
template <>
Group<ElementBase>* Group<ElementBase>::AsGroupElement()
{
    return this;
}

//! @brief either casts the pointer to an element group or throws an exception for groups which are not element groups
template <>
const Group<ElementBase>* Group<ElementBase>::AsGroupElement() const
{
    return this;
}

//! @brief either casts the pointer to a node group or throws an exception for groups which are not node groups
template <>
Group<NodeBase>* Group<ElementBase>::AsGroupNode()
{
    throw Exception("[Group<ElementBase>::AsGroupNode] group is not a node group");
}

//! @brief either casts the pointer to a node group or throws an exception for groups which are not node groups
template <>
const Group<NodeBase>* Group<ElementBase>::AsGroupNode() const
{
    throw Exception("[Group<ElementBase>::AsGroupNode] group is not a node group");
}

//! @brief either casts the pointer to an element group or throws an exception for groups which are not element groups
template <>
Group<ElementBase>* Group<NodeBase>::AsGroupElement()
{
    throw Exception("[Group<NodeBase>::AsGroupElement] group is not an element group");
}

//! @brief either casts the pointer to an element group or throws an exception for groups which are not element groups
template <>
const Group<ElementBase>* Group<NodeBase>::AsGroupElement() const
{
    throw Exception("[Group<NodeBase>::AsGroupElement] group is not an element group");
}

//! @brief either casts the pointer to a node group or throws an exception for groups which are not node groups
template <>
Group<NodeBase>* Group<NodeBase>::AsGroupNode()
{
    return this;
}

//! @brief either casts the pointer to a node group or throws an exception for groups which are not node groups
template <>
const Group<NodeBase>* Group<NodeBase>::AsGroupNode() const
{
    return this;
}

//! @brief info for group members
//! @param rVerboseLevel verbose Level
//! @param rStructure Structure that holds the members
template <>
void Group<NodeBase>::Info(int rVerboseLevel) const
{
    std::cout << "    Type              : Nodes" << std::endl;
    std::cout << "    Number of members : " << this->size() << std::endl;
    if (rVerboseLevel > 2)
    {
        std::cout << "    members :" << std::endl;
        for (auto member : *this)
        {
            std::cout << "      " << member.first << std::endl;
        }
    }
}

//! @brief info for group members
//! @param rVerboseLevel verbose Level
//! @param rStructure Structure that holds the members
template <>
void Group<ElementBase>::Info(int rVerboseLevel) const
{
    std::cout << "    Type              : Elements" << std::endl;
    std::cout << "    Number of members : " << this->size() << std::endl;
    if (rVerboseLevel > 2)
    {
        std::cout << "    members :" << std::endl;
        for (auto member : *this)
        {
            std::cout << "      " << member.first << std::endl;
        }
    }
}

} // namespace NuTo
