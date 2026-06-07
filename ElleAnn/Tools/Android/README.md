# ElleAnn Android Companion App

**Status**: Minimal buildable scaffold (Feb 2026).

This project is the companion Android app that pairs with a running
desktop ElleAnn HTTP service and exercises the end-to-end auth path.

## Project layout

```
Android/
├── settings.gradle.kts       (root Gradle settings)
├── build.gradle.kts          (root — declares plugins only)
├── gradle.properties
├── app/
│   ├── build.gradle.kts
│   ├── proguard-rules.pro
│   └── src/main/
│       ├── AndroidManifest.xml
│       ├── java/com/elleann/android/
│       │   ├── ElleApp.kt              (Application subclass, owns AppContainer)
│       │   ├── MainActivity.kt         (Compose host + ellepair:// deep link)
│       │   ├── data/
│       │   │   ├── AppContainer.kt     (hand-rolled DI)
│       │   │   ├── AuthInterceptor.kt  (OkHttp Bearer attach + 401 wipe)
│       │   │   ├── ElleApi.kt          (Retrofit interface — pair/health/emotions)
│       │   │   └── TokenStore.kt       (EncryptedSharedPreferences)
│       │   └── ui/
│       │       ├── PairScreen.kt       (host/port/code entry + QR scan)
│       │       ├── HomeScreen.kt       (sanity call to /api/emotions)
│       │       └── theme/Theme.kt
│       └── res/
│           ├── values/{strings,themes}.xml
│           └── xml/{backup,data_extraction,network_security}.xml
└── spec/
    └── *.kt   (existing API contracts — consumed by data/ElleApi.kt)
```

## Build

1. Install Android Studio Hedgehog (2023.1.1) or newer, and Android SDK 34.
2. Open the `Android/` folder as an Android Studio project.
3. Let it sync Gradle (the wrapper will be generated on first run).
4. If you prefer the command line, run `gradle wrapper` once from
   `Android/` to generate `gradlew`, then `./gradlew assembleDebug`.

## Run

1. Bring up the desktop ElleAnn HTTP service (`Elle.Service.HTTP`).
2. In desktop admin UI (or via `curl`), POST `/api/auth/pair-code` to
   get a 6-digit code. Or hit `GET /api/auth/qr?code=XXXXXX&host=<LAN>` —
   the response is an SVG QR that the app can scan.
3. Launch the Android app, enter host/port/code (or scan the QR), tap
   **Pair**. The app POSTs `/api/auth/pair`, stores the returned JWT in
   Keystore-wrapped prefs, and navigates to HomeScreen.
4. HomeScreen calls `/api/emotions` with the Bearer attached. If you
   see Valence / Arousal / Dominance values, the end-to-end path
   (pairing → JWT → gate verify → route handler) is green.

## Deep link

Scanning or tapping an `ellepair://<host>:<port>/<code>` URI launches
the app with PairScreen pre-filled. Long-press the QR on the desktop
settings screen or share the URI via Bluetooth for quick re-pair.

## Scope

This is explicitly a **minimal** scaffold — the core auth round-trip
plus one authenticated read. The full feature set (WebSocket subscribe,
X-chromosome state, memory/CRUD screens) plugs into `ElleApi.kt`
via the contracts in `Android/spec/*.kt`. Add routes there as UI screens
are built; no DI changes required.
