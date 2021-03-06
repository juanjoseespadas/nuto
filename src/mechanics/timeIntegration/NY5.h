#pragma once

#include <vector>
#include "mechanics/timeIntegration/ExplicitNystroem.h"

namespace NuTo
{
namespace TimeIntegration
{
template <typename TState>
class NY5 : public ExplicitNystroem<TState>
{
public:
    NY5()
        : NuTo::TimeIntegration::ExplicitNystroem<TState>(
                  {{0., 0., 0., 0.}, {1. / 50., 0., 0., 0.}, {-1. / 27, 7. / 27, 0., 0.}, {0.3, -2. / 35, 9. / 35, 0.}},
                  {14. / 336., 125. / 336., 162. / 336., 35 / 336.}, {14. / 336., 100. / 336., 54. / 336., 0.},
                  {0., 0.2, 2. / 3., 1.})
    {
    }
};
}
}
