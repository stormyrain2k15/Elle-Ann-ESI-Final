#pragma once
#ifndef ELLE_FIESTA_PROTO_TABLE_H
#define ELLE_FIESTA_PROTO_TABLE_H

#include "FiestaProtoBase.h"

#include <cstdint>
#include <cstddef>
#include <string_view>

namespace Fiesta {

#include "Generated/FiestaProtoTable.inc"

#ifndef FIESTA_PROTO_TABLE
#  error "FiestaProtoTable.inc did not define FIESTA_PROTO_TABLE — \
regen via python3 _re_artifacts/pdb/gen_proto_table.py"
#endif
#ifndef FIESTA_PROTO_HOT_TABLE
#  error "FiestaProtoTable.inc did not define FIESTA_PROTO_HOT_TABLE"
#endif

namespace detail {

#define FIESTA_PT_EMIT(opcode_hex, opcode_name, struct_name, pdb_sizeof, category) \
    OpcodeMeta{ static_cast<uint16_t>(opcode_hex),                                  \
                #opcode_name, #struct_name,                                         \
                static_cast<int32_t>(pdb_sizeof),                                   \
                category },

inline constexpr OpcodeMeta kOpcodeMetaTable[] = {
    FIESTA_PROTO_TABLE(FIESTA_PT_EMIT)
};

inline constexpr OpcodeMeta kHotOpcodeMetaTable[] = {
    FIESTA_PROTO_HOT_TABLE(FIESTA_PT_EMIT)
};

#undef FIESTA_PT_EMIT

inline constexpr std::size_t kOpcodeMetaCount =
    sizeof(kOpcodeMetaTable) / sizeof(kOpcodeMetaTable[0]);

inline constexpr std::size_t kHotOpcodeMetaCount =
    sizeof(kHotOpcodeMetaTable) / sizeof(kHotOpcodeMetaTable[0]);

}

inline constexpr const OpcodeMeta* OpcodeMetaTable() {
    return detail::kOpcodeMetaTable;
}

inline constexpr std::size_t OpcodeMetaCount() {
    return detail::kOpcodeMetaCount;
}

inline constexpr const OpcodeMeta* HotOpcodeMetaTable() {
    return detail::kHotOpcodeMetaTable;
}

inline constexpr std::size_t HotOpcodeMetaCount() {
    return detail::kHotOpcodeMetaCount;
}

inline std::string_view OpcodeName(uint16_t opcode) {
    const OpcodeMeta* lo = detail::kOpcodeMetaTable;
    const OpcodeMeta* hi = lo + detail::kOpcodeMetaCount;
    while (lo < hi) {
        const OpcodeMeta* mid = lo + (hi - lo) / 2;
        if (mid->opcode < opcode)       lo = mid + 1;
        else if (mid->opcode > opcode)  hi = mid;
        else                            return mid->opcode_name;
    }
    return "(unknown)";
}

inline const OpcodeMeta* OpcodeMetaFor(uint16_t opcode) {
    const OpcodeMeta* lo = detail::kOpcodeMetaTable;
    const OpcodeMeta* hi = lo + detail::kOpcodeMetaCount;
    while (lo < hi) {
        const OpcodeMeta* mid = lo + (hi - lo) / 2;
        if (mid->opcode < opcode)       lo = mid + 1;
        else if (mid->opcode > opcode)  hi = mid;
        else                            return mid;
    }
    return nullptr;
}

inline Decoder ClassifyDecoder(uint16_t opcode, std::size_t payload_len) {
    const OpcodeMeta* m = OpcodeMetaFor(opcode);
    if (!m)                              return Decoder::Unknown;
    if (m->pdb_sizeof < 0)               return Decoder::Unknown;
    if (m->pdb_sizeof == 0)              return Decoder::Opaque;
    if (static_cast<std::size_t>(m->pdb_sizeof) == payload_len) return Decoder::Fixed;
    return Decoder::HeadAndTail;
}

static_assert(detail::kOpcodeMetaCount > 0,
              "FiestaProtoTable: empty opcode table — generator broken");

namespace detail {
inline constexpr bool TableIsSorted() {
    for (std::size_t i = 1; i < kOpcodeMetaCount; ++i) {
        if (kOpcodeMetaTable[i].opcode < kOpcodeMetaTable[i-1].opcode) return false;
    }
    return true;
}
}
static_assert(detail::TableIsSorted(),
              "FiestaProtoTable: rows must be sorted by opcode ascending");

}
#endif
