// $Id$
#ifndef STRUCTUREBASEENUM_H_
#define STRUCTUREBASEENUM_H_

namespace NuTo
{
namespace StructureEnum
{

enum class eMatrixType
{
    STIFFNESS=0,
    DAMPING,
    MASS,
    LUMPED_MASS
};

enum class eSubVector
{
    J,
    K
};

enum class eSubMatrix
{
    JJ,
    JK,
    KJ,
    KK
};

enum class eOutput
{
    DAMPING,
    DAMPING_MOISTURE_TRANSPORT,
    INTERNAL_GRADIENT,
    INTERNAL_GRADIENT_MOISTURE_TRANSPORT,
    MASS,
    STIFFNESS,
    STIFFNESS_DISPLACEMENTS_JJ,
    STIFFNESS_MOISTURE_TRANSPORT
};

}
}
#endif /* STRUCTUREBASEENUM_H_ */
