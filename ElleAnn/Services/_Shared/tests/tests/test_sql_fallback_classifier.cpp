#include <doctest/doctest.h>

#include "ElleSQLFallbackClassifier.h"

using namespace ElleSQLFallbackClassifier;

TEST_CASE("ClassifyExec — read-only verbs are idempotent") {
    CHECK(ClassifyExec("SELECT * FROM dbo.Foo")           == Idempotency::Yes);
    CHECK(ClassifyExec("select 1")                        == Idempotency::Yes);
    CHECK(ClassifyExec("  SELECT 1  ")                    == Idempotency::Yes);
    CHECK(ClassifyExec("WITH cte AS (SELECT 1) SELECT 1") == Idempotency::Yes);
    CHECK(ClassifyExec("MERGE Target USING Src ON ...")   == Idempotency::Yes);
    CHECK(ClassifyExec("TRUNCATE TABLE Foo")              == Idempotency::Yes);
}

TEST_CASE("ClassifyExec — mutating verbs are non-idempotent") {
    CHECK(ClassifyExec("INSERT INTO dbo.Foo VALUES (1)") == Idempotency::No);
    CHECK(ClassifyExec("insert into Foo SELECT 1")       == Idempotency::No);
    CHECK(ClassifyExec("UPDATE Foo SET x = 1")           == Idempotency::No);
    CHECK(ClassifyExec("DELETE FROM Foo WHERE id = 1")   == Idempotency::No);
}

TEST_CASE("ClassifyExec — unknown returns Unknown") {
    CHECK(ClassifyExec("")                                       == Idempotency::Unknown);
    CHECK(ClassifyExec("EXEC sp_who2")                           == Idempotency::Unknown);
    CHECK(ClassifyExec("CREATE TABLE Foo (id INT)")              == Idempotency::Unknown);
}

TEST_CASE("ClassifyCallProc — usp_Record/Upsert/Snapshot/Log/Heartbeat/Bond/Intuition are idempotent") {
    CHECK(ClassifyCallProc("usp_RecordMetric")             == Idempotency::Yes);
    CHECK(ClassifyCallProc("dbo.usp_RecordMetric")         == Idempotency::Yes);
    CHECK(ClassifyCallProc("ElleCore.dbo.USP_UpsertBelief") == Idempotency::Yes);
    CHECK(ClassifyCallProc("usp_SnapshotBonding")          == Idempotency::Yes);
    CHECK(ClassifyCallProc("usp_LogEvent")                 == Idempotency::Yes);
    CHECK(ClassifyCallProc("usp_HeartbeatService")         == Idempotency::Yes);
    CHECK(ClassifyCallProc("usp_BondingDailySnapshot")     == Idempotency::Yes);
    CHECK(ClassifyCallProc("usp_IntuitionCommitFrame")     == Idempotency::Yes);
    CHECK(ClassifyCallProc("usp_EnsureUserPresent")        == Idempotency::Yes);
    CHECK(ClassifyCallProc("usp_MarkDeviceSeen")           == Idempotency::Yes);
    CHECK(ClassifyCallProc("usp_TouchHeartbeat")           == Idempotency::Yes);
}

TEST_CASE("ClassifyCallProc — usp_Delete/Purge/Insert/Create are non-idempotent") {
    CHECK(ClassifyCallProc("usp_DeleteSession")        == Idempotency::No);
    CHECK(ClassifyCallProc("usp_PurgeAuditLog")        == Idempotency::No);
    CHECK(ClassifyCallProc("usp_InsertOrderRow")       == Idempotency::No);
    CHECK(ClassifyCallProc("dbo.usp_CreateInvitation") == Idempotency::No);
}

TEST_CASE("ClassifyCallProc — unknown prefix returns Unknown") {
    CHECK(ClassifyCallProc("usp_DoSomethingObscure") == Idempotency::Unknown);
    CHECK(ClassifyCallProc("sp_who2")                == Idempotency::Unknown);
    CHECK(ClassifyCallProc("")                       == Idempotency::Unknown);
}

TEST_CASE("Idempotency string round-trip") {
    CHECK(IdempotencyFromString("Yes")     == Idempotency::Yes);
    CHECK(IdempotencyFromString("No")      == Idempotency::No);
    CHECK(IdempotencyFromString("Unknown") == Idempotency::Unknown);
    CHECK(IdempotencyFromString("garbage") == Idempotency::Unknown);

    CHECK(std::string(IdempotencyToString(Idempotency::Yes))     == "Yes");
    CHECK(std::string(IdempotencyToString(Idempotency::No))      == "No");
    CHECK(std::string(IdempotencyToString(Idempotency::Unknown)) == "Unknown");
}

TEST_CASE("ToUpperAscii + LeadingTokens helpers") {
    CHECK(ToUpperAscii("hello")            == "HELLO");
    CHECK(ToUpperAscii("Mix3D_aBc")        == "MIX3D_ABC");
    CHECK(LeadingTokens("  insert  into  Foo  values", 2) == "INSERT INTO");
    CHECK(LeadingTokens("update", 3)        == "UPDATE");
    CHECK(LeadingTokens("", 2)              == "");
}
