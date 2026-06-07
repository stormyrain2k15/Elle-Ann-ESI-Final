#pragma once
#ifndef ELLE_SERVER_INFO_H
#define ELLE_SERVER_INFO_H

#include <string>
#include <unordered_map>

struct ElleServerInfoEntry {
	std::string name;
	int a = 0, b = 0, c = 0, d = 0;
	std::string host;
	int port = 0;
	int minUsers = 0;
	int maxUsers = 0;
};

struct ElleOdbcInfoEntry {
	std::string name;
	int id = 0;
	int world = 0;
	std::string dsn;
	std::string initSql;
};

struct ElleServerInfoFile {
	std::unordered_map<std::string, ElleServerInfoEntry> servers;
	std::unordered_map<std::string, ElleOdbcInfoEntry>   odbc;
	std::string my_server_name;
	std::string my_server_subdir;
	int my_server_type = 0;
	int my_server_world = 0;
	int my_server_zone = 0;
};

bool LoadElleServerInfo(const std::string& path, ElleServerInfoFile& out, std::string& outErr);

#endif
