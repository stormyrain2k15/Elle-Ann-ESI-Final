#include "HTTPServer.h"

void ElleHTTPService::RegisterEducationRoutes() {
        auto subjectToJson = [](const ElleDB::LearnedSubject& s) {
            return json{
                {"id", s.id}, {"subject", s.subject}, {"category", s.category},
                {"proficiency_level", s.proficiency_level},
                {"who_taught", s.who_taught}, {"where_learned", s.where_learned},
                {"time_to_learn_hours", s.time_to_learn_hours},
                {"notes", s.notes},
                {"date_started", s.date_started},
                {"date_completed", s.date_completed}
            };
        };
        m_router.Register("GET", "/api/education/subjects", [subjectToJson](const HTTPRequest& req) {
            std::string category = req.QueryParam("category", "");
            uint32_t limit = (uint32_t)req.QueryInt("limit", 50);
            std::vector<ElleDB::LearnedSubject> subs;
            if (!ElleDB::ListSubjects(subs, category, limit))
                return HTTPResponse::Err(500, "subjects query failed");
            json arr = json::array();
            for (auto& s : subs) arr.push_back(subjectToJson(s));
            return HTTPResponse::OK({{"subjects", arr}, {"total", (int64_t)arr.size()}});
        });
        m_router.Register("GET", "/api/education/subjects/{id}", [subjectToJson](const HTTPRequest& req) {
            int32_t id = req.PathInt("id");
            ElleDB::LearnedSubject s;
            if (!ElleDB::GetSubject(id, s)) return HTTPResponse::Err(404, "subject not found");

            std::vector<ElleDB::EducationReference> refs;
            std::vector<ElleDB::LearningMilestone>  mils;
            ElleDB::ListSubjectReferences(id, refs);
            ElleDB::ListSubjectMilestones(id, mils);
            json refArr = json::array();
            for (auto& r : refs) refArr.push_back({
                {"id", r.id}, {"reference_type", r.reference_type},
                {"reference_title", r.reference_title},
                {"reference_content", r.reference_content},
                {"file_path", r.file_path},
                {"relevance_score", r.relevance_score}, {"notes", r.notes}
            });
            json milArr = json::array();
            for (auto& m : mils) milArr.push_back({
                {"id", m.id}, {"milestone", m.milestone},
                {"description", m.description}, {"achieved_at", m.achieved_at}
            });
            json out = subjectToJson(s);
            out["references"] = refArr;
            out["milestones"] = milArr;
            return HTTPResponse::OK(out);
        });
        m_router.Register("POST", "/api/education/subjects", [](const HTTPRequest& req) {
            try {
                json body = req.BodyJSON();
                ElleDB::LearnedSubject s;
                s.subject              = body.value("subject", body.value("name", std::string("")));
                if (s.subject.empty()) return HTTPResponse::Err(400, "missing 'subject'");
                s.category             = body.value("category", std::string(""));
                s.proficiency_level    = body.value("proficiency_level", 0);
                s.who_taught           = body.value("who_taught", std::string(""));
                s.where_learned        = body.value("where_learned", std::string(""));
                s.time_to_learn_hours  = body.value("time_to_learn_hours", 0.0f);
                s.notes                = body.value("notes", std::string(""));
                int32_t newId = 0;
                if (!ElleDB::CreateSubject(s, newId))
                    return HTTPResponse::Err(500, "create subject failed");
                return HTTPResponse::Created({{"id", newId}, {"subject", s.subject}});
            } catch (const std::exception& e) { return HTTPResponse::Err(500, e.what()); }
        });
        m_router.Register("PUT", "/api/education/subjects/{id}", [](const HTTPRequest& req) {
            int32_t id = req.PathInt("id");
            try {
                json body = req.BodyJSON();
                ElleDB::LearnedSubject patch;
                std::vector<std::string> fields;
                for (auto& kv : body.items()) {
                    if      (kv.key() == "subject")             { patch.subject = kv.value().get<std::string>();             fields.push_back("subject"); }
                    else if (kv.key() == "category")            { patch.category = kv.value().get<std::string>();            fields.push_back("category"); }
                    else if (kv.key() == "proficiency_level")   { patch.proficiency_level = kv.value().get<int>();            fields.push_back("proficiency_level"); }
                    else if (kv.key() == "who_taught")          { patch.who_taught = kv.value().get<std::string>();          fields.push_back("who_taught"); }
                    else if (kv.key() == "where_learned")       { patch.where_learned = kv.value().get<std::string>();       fields.push_back("where_learned"); }
                    else if (kv.key() == "time_to_learn_hours") { patch.time_to_learn_hours = kv.value().get<float>();        fields.push_back("time_to_learn_hours"); }
                    else if (kv.key() == "notes")               { patch.notes = kv.value().get<std::string>();               fields.push_back("notes"); }
                    else if (kv.key() == "date_completed")      { patch.date_completed = kv.value().get<std::string>();      fields.push_back("date_completed"); }
                }
                if (fields.empty()) return HTTPResponse::OK({{"updated", 0}});
                if (!ElleDB::UpdateSubject(id, patch, fields))
                    return HTTPResponse::Err(500, "update failed");
                return HTTPResponse::OK({{"id", id}, {"updated_fields", fields}});
            } catch (const std::exception& e) { return HTTPResponse::Err(500, e.what()); }
        });
        m_router.Register("POST", "/api/education/subjects/{id}/references", [](const HTTPRequest& req) {
            int32_t id = req.PathInt("id");
            try {
                json body = req.BodyJSON();
                ElleDB::EducationReference r;
                r.subject_id        = id;
                r.reference_type    = body.value("reference_type", std::string("note"));
                r.reference_title   = body.value("reference_title", std::string(""));
                r.reference_content = body.value("reference_content", std::string(""));
                r.file_path         = body.value("file_path", std::string(""));
                r.relevance_score   = body.value("relevance_score", 0.5f);
                r.notes             = body.value("notes", std::string(""));
                if (!ElleDB::AddSubjectReference(r))
                    return HTTPResponse::Err(500, "add reference failed");
                return HTTPResponse::Created({{"subject_id", id}, {"stored", true}});
            } catch (const std::exception& e) { return HTTPResponse::Err(500, e.what()); }
        });
        m_router.Register("POST", "/api/education/subjects/{id}/milestones", [](const HTTPRequest& req) {
            int32_t id = req.PathInt("id");
            try {
                json body = req.BodyJSON();
                ElleDB::LearningMilestone m;
                m.subject_id  = id;
                m.milestone   = body.value("milestone", std::string(""));
                m.description = body.value("description", std::string(""));
                if (m.milestone.empty()) return HTTPResponse::Err(400, "missing 'milestone'");
                if (!ElleDB::AddSubjectMilestone(m))
                    return HTTPResponse::Err(500, "add milestone failed");
                return HTTPResponse::Created({{"subject_id", id}, {"stored", true}});
            } catch (const std::exception& e) { return HTTPResponse::Err(500, e.what()); }
        });
        m_router.Register("GET", "/api/education/skills", [](const HTTPRequest& req) {
            std::string category = req.QueryParam("category", "");
            std::vector<ElleDB::Skill> skills;
            if (!ElleDB::ListSkills(skills, category))
                return HTTPResponse::Err(500, "skills query failed");
            json arr = json::array();
            for (auto& s : skills) arr.push_back({
                {"id", s.id}, {"skill_name", s.skill_name}, {"category", s.category},
                {"proficiency", s.proficiency},
                {"learned_from_subject_id", s.learned_from_subject_id},
                {"times_used", s.times_used}, {"last_used", s.last_used},
                {"notes", s.notes}
            });
            return HTTPResponse::OK({{"skills", arr}, {"total", (int64_t)arr.size()}});
        });
        m_router.Register("POST", "/api/education/skills", [](const HTTPRequest& req) {
            try {
                json body = req.BodyJSON();
                ElleDB::Skill s;
                s.skill_name              = body.value("skill_name", body.value("name", std::string("")));
                if (s.skill_name.empty()) return HTTPResponse::Err(400, "missing 'skill_name'");
                s.category                = body.value("category", std::string(""));
                s.proficiency             = body.value("proficiency", 0);
                s.learned_from_subject_id = body.value("learned_from_subject_id", 0);
                s.notes                   = body.value("notes", std::string(""));
                int32_t newId = 0;
                if (!ElleDB::CreateSkill(s, newId))
                    return HTTPResponse::Err(409, "skill already exists or create failed");
                return HTTPResponse::Created({{"id", newId}, {"skill_name", s.skill_name}});
            } catch (const std::exception& e) { return HTTPResponse::Err(500, e.what()); }
        });
        m_router.Register("PUT", "/api/education/skills/{name}/use", [](const HTTPRequest& req) {
            std::string name = req.headers.at("x-path-id");
            if (!ElleDB::RecordSkillUse(name))
                return HTTPResponse::Err(404, "skill not found");
            return HTTPResponse::OK({{"skill_name", name}, {"recorded", true}});
        });

    }
