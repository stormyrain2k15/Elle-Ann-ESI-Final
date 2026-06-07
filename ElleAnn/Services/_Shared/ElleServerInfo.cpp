#include "ElleServerInfo.h"
#include "ElleLogger.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>

static inline std::string Trim(const std::string& s) {
	size_t a = 0, b = s.size();
	while (a < b && (unsigned char)s[a] <= ' ') ++a;
	while (b > a && (unsigned char)s[b - 1] <= ' ') --b;
	return s.substr(a, b - a);
}

static bool ParseCsvLike(const std::string& in, std::vector<std::string>& out) {
	out.clear();
	std::string cur;
	bool inQ = false;
	for (size_t i = 0; i < in.size(); i++) {
		char ch = in[i];
		if (ch == '"') { inQ = !inQ; continue; }
		if (!inQ && ch == ',') {
			out.push_back(Trim(cur));
			cur.clear();
			continue;
		}
		cur.push_back(ch);
	}
	if (!cur.empty() || (!out.empty() && in.back() == ',')) out.push_back(Trim(cur));
	return !out.empty();
}

static bool LoadFileRecursive(const std::string& path, std::vector<std::string>& lines, std::string& outErr) {
	std::ifstream f(path);
	if (!f) { outErr = "cannot open: " + path; return false; }
	std::string line;
	while (std::getline(f, line)) {
		if (!line.empty() && line.back() == '\r') line.pop_back();
		lines.push_back(line);
	}
	return true;
}

bool LoadElleServerInfo(const std::string& path, ElleServerInfoFile& out, std::string& outErr) {
	out = ElleServerInfoFile{};
	outErr.clear();

	std::vector<std::string> lines;
	if (!LoadFileRecursive(path, lines, outErr)) return false;

	const auto slash = path.find_last_of("\\/");
	const std::string baseDir = (slash == std::string::npos) ? "" : path.substr(0, slash + 1);

	for (size_t li = 0; li < lines.size(); li++) {
		std::string s = Trim(lines[li]);
		if (s.empty() || s[0] == ';') continue;

		if (s.rfind("#include", 0) == 0) {
			auto q1 = s.find('"');
			auto q2 = s.find('"', q1 == std::string::npos ? 0 : q1 + 1);
			if (q1 != std::string::npos && q2 != std::string::npos && q2 > q1 + 1) {
				std::string rel = s.substr(q1 + 1, q2 - q1 - 1);
				std::string incPath = rel;
				if (!rel.empty() && rel[0] != '\\' && rel.find(":\\") == std::string::npos) {
					incPath = baseDir + rel;
				}
				std::vector<std::string> more;
				if (!LoadFileRecursive(incPath, more, outErr)) return false;
				lines.insert(lines.begin() + (li + 1), more.begin(), more.end());
			}
			continue;
		}

		if (s.rfind("MY_SERVER", 0) == 0) {
			auto p = s.find('"');
			auto p2 = s.find('"', p == std::string::npos ? 0 : p + 1);
			auto p3 = s.find('"', p2 == std::string::npos ? 0 : p2 + 1);
			auto p4 = s.find('"', p3 == std::string::npos ? 0 : p3 + 1);
			if (p == std::string::npos || p2 == std::string::npos ||
				p3 == std::string::npos || p4 == std::string::npos) {
				continue;
			}
			out.my_server_name = s.substr(p + 1, p2 - p - 1);
			out.my_server_subdir = s.substr(p3 + 1, p4 - p3 - 1);
			auto restPos = s.find(',', p4);
			if (restPos != std::string::npos) {
				std::vector<std::string> toks;
				if (ParseCsvLike(s.substr(restPos + 1), toks) && toks.size() >= 3) {
					out.my_server_type = std::atoi(toks[0].c_str());
					out.my_server_world = std::atoi(toks[1].c_str());
					out.my_server_zone = std::atoi(toks[2].c_str());
				}
			}
			continue;
		}

		if (s.rfind("SERVER_INFO", 0) == 0) {
			auto p = s.find('"');
			auto p2 = s.find('"', p == std::string::npos ? 0 : p + 1);
			if (p == std::string::npos || p2 == std::string::npos) continue;
			std::string name = s.substr(p + 1, p2 - p - 1);
			auto restPos = s.find(',', p2);
			if (restPos == std::string::npos) continue;
			std::string rest = s.substr(restPos + 1);
			std::vector<std::string> toks;
			if (!ParseCsvLike(rest, toks)) continue;
			if (toks.size() < 8) continue;

			ElleServerInfoEntry e;
			e.name = name;
			e.a = std::atoi(toks[0].c_str());
			e.b = std::atoi(toks[1].c_str());
			e.c = std::atoi(toks[2].c_str());
			e.d = std::atoi(toks[3].c_str());
			e.host = Trim(toks[4]);
			if (!e.host.empty() && e.host.front() == '"' && e.host.back() == '"') {
				e.host = e.host.substr(1, e.host.size() - 2);
			}
			e.port = std::atoi(toks[5].c_str());
			e.minUsers = std::atoi(toks[6].c_str());
			e.maxUsers = std::atoi(toks[7].c_str());
			out.servers[e.name] = std::move(e);
			continue;
		}

		if (s.rfind("ODBC_INFO", 0) == 0) {
			auto p = s.find('"');
			auto p2 = s.find('"', p == std::string::npos ? 0 : p + 1);
			if (p == std::string::npos || p2 == std::string::npos) continue;
			std::string name = s.substr(p + 1, p2 - p - 1);
			auto restPos = s.find(',', p2);
			if (restPos == std::string::npos) continue;
			std::string rest = s.substr(restPos + 1);
			std::vector<std::string> toks;
			if (!ParseCsvLike(rest, toks)) continue;
			if (toks.size() < 4) continue;

			ElleOdbcInfoEntry e;
			e.name = name;
			e.id = std::atoi(toks[0].c_str());
			e.world = std::atoi(toks[1].c_str());
			e.dsn = Trim(toks[2]);
			e.initSql = Trim(toks[3]);
			out.odbc[e.name] = std::move(e);
			continue;
		}
	}

	return true;
}
