﻿#include "Easing.h"

namespace easing {
    bool RegisterDefaultMoverFunctionExpressions()
    {
#define SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION(key, name) MoverFunctionExpressionManager::GetInstance().Register(key, new name ## MoverFunctionExpression())

        return SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("linear", Linear)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("in_quad", InQuad)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("out_quad", OutQuad)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("inout_quad", InOutQuad)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("in_cube", InCubic)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("out_cube", OutCubic)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("inout_cube", InOutCubic)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("in_quart", InQuartic)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("out_quart", OutQuartic)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("inout_quart", InOutQuartic)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("in_quint", InQuintic)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("out_quint", OutQuintic)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("inout_quint", InOutQuintic)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("in_sine", InSinusoidal)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("out_sine", OutSinusoidal)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("inout_sine", InOutSinusoidal)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("in_expo", InExponential)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("out_expo", OutExponential)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("inout_expo", InOutExponential)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("in_circle", InCircular)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("out_circle", OutCircular)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("inout_circle", InOutCircular)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("in_elastic", InElastic)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("out_elastic", OutElastic)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("inout_elastic", InOutElastic)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("in_back", InBack)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("out_back", OutBack)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("inout_back", InOutBack)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("in_bounce", InBounce)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("out_bounce", OutBounce)
            && SU_REGISTER_EASING_MOVER_FUNCTION_EXPRESSION("inout_bounce", InOutBounce);
    }
}
