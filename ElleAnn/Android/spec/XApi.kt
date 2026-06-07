 below so
 *        this file stays pure-Kotlin until Retrofit is on the classpath).
 *     3. Build the Retrofit client in Auth.kt and inject.
 *
 *   17 routes total: 10 GET + 7 POST. All return `XEnvelope<T>` on success.
 *   Errors are a plain JSON body with `{"error":"<message>"}` and an HTTP
 *   status ≥ 400 (see ErrorEnvelope.kt).
 *══════════════════════════════════════════════════════════════════════════════*/
package com.elleann.android.data.spec

interface XApi {

    suspend fun getState():  XState

    suspend fun getHistory(

        hours:  Int = 72,
        points: Int = 500,
    ):  XHistory

    suspend fun getModulation():  XModulation

    suspend fun getNextPeriod():  XNextPeriod

    suspend fun getFertilityWindow():  XFertilityWindow

    suspend fun getPregnancy():  XPregnancyState

    suspend fun getPregnancyEvents(

        limit: Int = 100,
    ):  XPregnancyEvents

    suspend fun getSymptoms(

        hours:  Int    = 24,
        origin: String = "",
    ):  XSymptoms

    suspend fun getContraception():  XContraception

    suspend fun getLifecycle():  XLifecycle

    suspend fun postCycleAnchor(

        body: XCycleAnchorRequest,
    ):  XDispatchAck

    suspend fun postStimulus(

        body: XStimulusRequest,
    ):  XDispatchAck

    suspend fun postConceptionAttempt(

        body: XConceptionAttemptRequest,
    ):  XDispatchAck

    suspend fun postSymptom(

        body: XSymptomLogRequest,
    ):  XDispatchAck

    suspend fun postContraception(

        body: XContraceptionRequest,
    ):  XDispatchAck

    suspend fun postLifecycle(

        body: XLifecycleRequest,
    ):  XDispatchAck

    suspend fun postPregnancyAccelerate(

        body: XPregnancyAccelerateRequest,
    ):  XDispatchAck
}
