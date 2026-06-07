#include "InflectionTables.h"
#include "../_Shared/ElleSQLConn.h"
#include "../_Shared/ElleLogger.h"

bool InflectionTables::Load() {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_table.clear();

    auto rs = ElleSQLPool::Instance().Query(
        "SELECT lemma, form, inflected FROM ElleHeart.dbo.composer_inflection");

    if (!rs.success) {
        ELLE_ERROR("InflectionTables: failed to load composer_inflection");
        return false;
    }

    for (auto& row : rs.rows) {
        Key k{ row[0], row[1] };
        m_table[k] = row[2];
    }

    ELLE_INFO("InflectionTables: loaded %zu entries", m_table.size());
    return true;
}

std::string InflectionTables::Inflect(const std::string& lemma,
                                       const std::string& form) const
{
    if (form.empty() || form == "-" || form == "—") return lemma;
    std::lock_guard<std::mutex> lk(m_mutex);
    Key k{ lemma, form };
    auto it = m_table.find(k);
    return (it != m_table.end()) ? it->second : lemma;
}

size_t InflectionTables::Count() const {
    std::lock_guard<std::mutex> lk(m_mutex);
    return m_table.size();
}
