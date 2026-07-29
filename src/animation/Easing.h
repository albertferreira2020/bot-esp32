#pragma once

// Nunca movimentos bruscos: toda animação passa por uma dessas curvas.
enum class EasingType {
    Linear,
    EaseInOut,
    Elastic,
    Bounce,
    Spring,
};

// t entra em [0,1] (tempo decorrido / duração). Elastic/Bounce/Spring podem
// ultrapassar levemente [0,1] de propósito (overshoot) antes de assentar.
float ease(EasingType type, float t);
