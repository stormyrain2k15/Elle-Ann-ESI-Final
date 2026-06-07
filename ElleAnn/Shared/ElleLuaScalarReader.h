#pragma once
#ifndef ELLE_LUA_SCALAR_READER_H
#define ELLE_LUA_SCALAR_READER_H

#include <string>
#include <cstdint>

class ElleLuaScalarReader {
public:

    explicit ElleLuaScalarReader(const std::string& path);

    bool IsLoaded() const { return m_loaded; }
    const std::string& Path() const { return m_path; }

    std::string GetString(const std::string& dottedKey,
                          const std::string& def = "") const;

    int64_t  GetInt   (const std::string& dottedKey, int64_t  def = 0) const;
    double   GetDouble(const std::string& dottedKey, double   def = 0.0) const;
    bool     GetBool  (const std::string& dottedKey, bool     def = false) const;

private:
    std::string m_path;
    std::string m_text;
    bool        m_loaded = false;

    std::string FindRhs(const std::string& dottedKey) const;
};

#endif
