# ElleAnn Android App Review Report — Final Profile-Fixed Package

Baseline reviewed: `Android.zip` (direct Android app source)
Claude fix packages reviewed/applied: `android_fixes.zip`, `android_pass2_final.zip`, `final_android_fixes.zip`
Scope: Android app only.

## Build attempt result

I still could not produce an APK inside this sandbox because the uploaded app is missing `gradle/wrapper/gradle-wrapper.jar`, and this container has no installed system `gradle` command or Android SDK/toolchain.

Actual failures from this environment:

```text
./gradlew :app:assembleDebug
Error: Could not find or load main class org.gradle.wrapper.GradleWrapperMain
Caused by: java.lang.ClassNotFoundException: org.gradle.wrapper.GradleWrapperMain
```

```text
gradle wrapper --gradle-version 8.9
bash: gradle: command not found
```

I updated `gradle/wrapper/gradle-wrapper.properties` to Gradle `8.9`. Locally, run this from the Android folder:

```bash
gradle wrapper --gradle-version 8.9
./gradlew :app:assembleDebug
```

## Final profile-picture protection status

The Elle locked profile picture flow is preserved and now wired into the app UI/container.

Protected files/endpoints/models that should **not** be deleted as video/audio-call cleanup:

- `app/src/main/java/com/elleann/android/data/ElleProfileStore.kt`
- `AppContainerExtended.profileStore`
- `ElleProfilePicture(...)` in `ElleHomeScreen.kt`
- `GET /api/video/avatars`
- `GET /api/video/avatar`
- `POST /api/video/avatar/upload`
- `UserAvatar`
- `AvatarListResponse`
- `UploadAvatarRequest`

Those are the permanent Elle avatar/profile-picture pieces, not video call.

## Missed profile-picture gap fixed in this final package

Claude's final profile pass correctly added the store into `AppContainerExtended` and added `ElleProfilePicture(...)` to the home top bar. The missed reinstall/persistence gap was that `fetchFromServer()` restored only the server avatar metadata but did not fully restore the local lock/cache behavior after reinstall.

Fixed here:

1. `fetchFromServer()` now sets the local locked flag when the backend reports an existing default avatar.
2. It saves the returned avatar id into encrypted prefs.
3. It converts `avatar.filePath` into a download URL using `restBaseUrl` when the path is relative.
4. It downloads and caches the bitmap locally when possible.
5. The home-screen profile picture now calls `fetchFromServer(container.extendedApi, container.authenticatedHttpClient, container.restBaseUrl)` so protected `/api/video/...` image/file routes can use the authenticated client instead of silently failing with the unauthenticated raw client.
6. `ElleProfilePicture` now observes `isLockedFlow`, so the UI updates immediately after the lock state changes.
7. `uploadAndLock()` closes the selected image stream safely with `use { ... }`.

Result: after uninstall/reinstall, once the user signs back in and reaches the home screen, the Android app should ask the server for the locked default avatar, restore the local lock, save the avatar id, and best-effort re-cache the bitmap locally.

If the backend's returned `file_path` is not directly downloadable or needs a different file endpoint than the one returned by `/api/video/avatar`, Android will still restore the locked state but may show the placeholder until the backend exposes a reachable image path.

## Removed video/audio-call pieces

Removed or kept removed:

- `VideoCallScreen.kt`
- `chat/video/{callId}` route
- `onStartVideoCall` callback from chat
- `VideoCallSession`
- `StartVideoCallRequest`
- `/api/tokens/video-calls`
- `/api/tokens/voice-calls`
- video-call start/end string resources

A search of the final source shows no remaining `VideoCall`, `voiceCall`, `onStartVideoCall`, `video-calls`, or `voice-calls` references in the app source/resources.

## Claude fixes verified/applied

Verified/applied from Claude's passes:

1. Login gate through `MainActivity.kt` and `LoginScreen.kt`.
2. Real paired state in `AppContainerExtended` instead of hardcoded always-paired server behavior.
3. JWT/auth handling and 401 re-auth clearing.
4. Chat cache manager and lifecycle crash/cache flushing.
5. Chat screen cache-first loading, send failure restoration, TTS/STT/color-code text support.
6. Manifest microphone permission for STT.
7. World/dev/settings/pass2 screens and route expansions.
8. Locked Elle profile-picture store and home top-bar UI from `final_android_fixes.zip`.

## Additional app-breaking issues found and fixed

1. Missing `LoginScreen.kt` after Claude's `MainActivity` import.
2. Broken `ContentResolver.wrap(null)` placeholder in login.
3. Missing `ChatState.ttsTargetMessageId` used by chat TTS controls.
4. Leftover removed video-call callback in `ChatScreen`.
5. Dead `VideoCallScreen.kt` after video/audio-call removal.
6. Missing `WORLD_GOALS` route and nav target.
7. Missing dev routes/nav targets for services, models, agents, and tools.
8. Admin key was saved but never sent as `x-admin-key`.
9. Crash handler recursion risk in `ElleApp.kt`.
10. STT had manifest permission but no runtime permission request.
11. TTS speed/pitch settings were saved but not applied to chat playback.
12. Profile-picture reinstall restore did not set the local lock/cache path correctly; fixed in this final package.

## Remaining items to verify locally in Android Studio

These require the real Android toolchain/backend and could not be fully verified in this sandbox:

1. Regenerate the missing Gradle wrapper jar with `gradle wrapper --gradle-version 8.9`.
2. Run `./gradlew :app:assembleDebug`.
3. Test the locked profile picture flow against the real backend:
   - fresh install
   - pair/login
   - select Elle picture
   - verify it uploads and locks
   - uninstall app
   - reinstall app
   - pair/login again
   - verify lock returns and the bitmap reappears
4. Confirm the backend response from `GET /api/video/avatar` returns a `file_path` that is directly reachable by the authenticated Android HTTP client.
5. Chat bubble font-size setting is still only partially wired; TTS speed/pitch are applied, but bubble font size may need more UI wiring.

## Validation performed

- Used the direct `Android.zip` source to avoid deep-path truncation issues.
- Treated `android_pass2_final.zip` as newer than `android_fixes.zip` where files overlapped.
- Treated `final_android_fixes.zip` as Claude's latest profile-picture pass.
- Preserved profile/avatar `/api/video/avatar*` endpoints while removing actual audio/video-call code.
- Searched final app source/resources for old video/voice-call references.
- Searched final app source for broken login placeholders/TODO compile placeholders.
- Updated Gradle wrapper properties to 8.9.
