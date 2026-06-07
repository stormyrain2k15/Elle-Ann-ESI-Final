#include "FiestaProtoBase.h"
#include "FiestaProtoTable.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <set>
#include <string>

#include "Generated/FiestaWireFixtures.inc"

#ifndef FIESTA_WIRE_FIXTURES
#  error "FiestaWireFixtures.inc did not define FIESTA_WIRE_FIXTURES — \
regen via python3 _re_artifacts/pdb/gen_proto_table.py"
#endif

struct WireEvent {
    int          port;
    const char*  direction;
    uint16_t     opcode;
    std::size_t  payload_len;
    const char*  payload_hex;
};

#define FIESTA_WF_EMIT(port, dir_tag, opcode_hex, plen, hex_lit) \
    WireEvent{ static_cast<int>(port), #dir_tag,                  \
               static_cast<uint16_t>(opcode_hex),                 \
               static_cast<std::size_t>(plen), hex_lit },

static constexpr WireEvent kWireFixtures[] = {
    FIESTA_WIRE_FIXTURES(FIESTA_WF_EMIT)
};

#undef FIESTA_WF_EMIT

static constexpr std::size_t kWireFixtureCount =
    sizeof(kWireFixtures) / sizeof(kWireFixtures[0]);

static const char* DecoderTag(Fiesta::Decoder d) {
    switch (d) {
        case Fiesta::Decoder::Fixed:       return "FIXED";
        case Fiesta::Decoder::HeadAndTail: return "HEAD+TAIL";
        case Fiesta::Decoder::Opaque:      return "OPAQUE";
        case Fiesta::Decoder::Unknown:     return "UNKNOWN";
    }
    return "?";
}

int main() {

    struct Aggregate {
        const Fiesta::OpcodeMeta* meta = nullptr;
        std::set<std::size_t>     lens;
        std::set<std::string>     dirs;
        int                        count = 0;
    };
    std::map<uint16_t, Aggregate> agg;
    for (std::size_t i = 0; i < kWireFixtureCount; ++i) {
        const WireEvent& e = kWireFixtures[i];
        Aggregate& a = agg[e.opcode];
        if (!a.meta) a.meta = Fiesta::OpcodeMetaFor(e.opcode);
        a.lens.insert(e.payload_len);
        a.dirs.insert(e.direction);
        a.count++;
    }

    std::printf("──── Fiesta Phase 6a coverage report ────\n");
    std::printf("Wire fixtures      : %zu events, %zu distinct opcodes\n",
                kWireFixtureCount, agg.size());
    std::printf("Authority table    : %zu opcodes, %zu hot opcodes\n",
                Fiesta::OpcodeMetaCount(), Fiesta::HotOpcodeMetaCount());
    std::printf("\n");

    int n_fixed = 0, n_headtail = 0, n_opaque = 0, n_unknown = 0;
    std::printf("%-8s %-9s %-50s %-12s %s\n",
                "OPCODE", "DIR", "OPCODE_NAME",
                "WIRE_SIZES", "VERDICT (vs PDB sizeof)");
    std::printf("%-8s %-9s %-50s %-12s %s\n",
                "------", "---", "-----------",
                "----------", "------------------------");
    for (const auto& [op, a] : agg) {
        std::string dirs;
        for (const auto& d : a.dirs) {
            if (!dirs.empty()) dirs += ",";
            dirs += d;
        }
        std::string lens;
        for (auto L : a.lens) {
            if (!lens.empty()) lens += ",";
            lens += std::to_string(L);
        }
        const char* name = (a.meta && a.meta->opcode_name)
                             ? a.meta->opcode_name : "??";
        std::string verdict;
        if (!a.meta || a.meta->pdb_sizeof < 0) {
            verdict = "UNKNOWN — no PDB struct";
            n_unknown++;
        } else {

            std::size_t representative = *a.lens.begin();
            Fiesta::Decoder d = Fiesta::ClassifyDecoder(op, representative);
            char buf[160];
            std::snprintf(buf, sizeof(buf),
                          "%s (pdb=%d, wire=%zu)",
                          DecoderTag(d), a.meta->pdb_sizeof, representative);
            verdict = buf;
            switch (d) {
                case Fiesta::Decoder::Fixed:       n_fixed++;    break;
                case Fiesta::Decoder::HeadAndTail: n_headtail++; break;
                case Fiesta::Decoder::Opaque:      n_opaque++;   break;
                case Fiesta::Decoder::Unknown:     n_unknown++;  break;
            }
        }
        std::printf("0x%04X   %-9s %-50s %-12s %s  (n=%d)\n",
                    op, dirs.c_str(), name, lens.c_str(),
                    verdict.c_str(), a.count);
    }

    std::printf("\n──── Summary ────\n");
    std::printf("  FIXED      : %3d  (struct sizeof ≡ payload_len)\n", n_fixed);
    std::printf("  HEAD+TAIL  : %3d  (variable-length tail follows fixed head)\n", n_headtail);
    std::printf("  OPAQUE     : %3d  (struct exists but has no fields yet)\n", n_opaque);
    std::printf("  UNKNOWN    : %3d  (opcode lacks any PDB-recovered struct)\n", n_unknown);

    std::printf("\nOpcodeName(0x0306) = %s\n", Fiesta::OpcodeName(0x0306).data());
    std::printf("OpcodeName(0x030A) = %s\n", Fiesta::OpcodeName(0x030A).data());
    std::printf("OpcodeName(0x0207) = %s\n", Fiesta::OpcodeName(0x0207).data());
    std::printf("OpcodeName(0xDEAD) = %s\n", Fiesta::OpcodeName(0xDEAD).data());
    return 0;
}
