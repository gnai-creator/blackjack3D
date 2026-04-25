#pragma once                          // Garante que o cabecalho seja incluido apenas uma vez por TU.

#include <cstdint>                    // Disponibiliza tipos inteiros de tamanho fixo (uint32_t etc.).

// Entity e apenas um identificador numerico opaco. Toda a "carne" da entidade
// vive nos componentes guardados pelo Registry; aqui so reservamos um ID.
using Entity = std::uint32_t;         // 32 bits permitem ~4 bilhoes de entidades, mais do que suficiente.
