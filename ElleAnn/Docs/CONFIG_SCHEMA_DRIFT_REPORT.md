# Config Schema Drift Report

Sweep date: 2026-02. Source: `elle_master_config.json` vs every `ElleConfig::Instance().Get*("...", default)` call under `Services/`.

- **Total config-lookup call sites scanned:** 69
- **Keys referenced in code:** 59
- **Keys in master JSON:** 399
- **Missing from master JSON (must add):** 0
- **Default ≠ master JSON value (real drift):** 0
- **False positives filtered (intentional / unparseable / path-escape):** 17
- **In master JSON but not referenced by any C++:** 340

## 1. Keys referenced in code but missing from `elle_master_config.json`

_None._ All code-referenced keys are present in the master config.

## 2. Default in code differs from `elle_master_config.json`

_None._ Every code default matches the master config value (or is intentionally unparseable, e.g. a constexpr expression).

## 3. False positives filtered (informational)

These call sites looked like mismatches in the raw regex sweep but are verified safe:

- `windows-path-escape-equivalent` — C++ source has `"C:\\ElleAnn\\…"` and JSON has the same path with the JSON-canonical escape; the runtime string is identical.
- `unparseable-constexpr-expression` — C++ default is an expression like `(int64_t)ELLE_IPC_MAX_PAYLOAD` or `10 * 60 * 1000`; the runtime value matches the JSON value (verified by hand).
- `unresolved-local-variable-default` — C++ default is the name of a local variable (e.g. the `defLog` per-service computed path passed to `logging.file_path`); not a literal so cannot be statically compared.
- `intentional-testing-mode-override` — `elle_master_config.json` has the `_testing_mode_comment` block deliberately overriding the C++ default for round-trip testing.

| Key | Getter | Code raw | JSON value | Reason | Source |
|---|---|---|---|---|---|
| `lua.scripts_directory` | `GetString` | `"Lua\\Elle.Lua.Behavioral\\scripts"` | `"Lua\Elle.Lua.Behavioral\scripts"` | `windows-path-escape-equivalent` | `Services/Elle.Service.Action/ActionExecutor.cpp:622` |
| `bonding.repair_sustain_ms` | `GetInt` | `10 * 60 * 1000` | `600000` | `unparseable-constexpr-expression` | `Services/Elle.Service.Bonding/Bonding.cpp:164` |
| `family.pregnancies_root` | `GetString` | `"C:\\ElleAnn\\pregnancies"` | `"C:\ElleAnn\pregnancies"` | `windows-path-escape-equivalent` | `Services/Elle.Service.Family/Family.cpp:97` |
| `family.children_root` | `GetString` | `"C:\\ElleAnn\\children"` | `"C:\ElleAnn\children"` | `windows-path-escape-equivalent` | `Services/Elle.Service.Family/Family.cpp:99` |
| `goals.fallback_dir` | `GetString` | `"C:\\ElleAnn\\Goals"` | `"C:\ElleAnn\Goals"` | `windows-path-escape-equivalent` | `Services/Elle.Service.GoalEngine/GoalEngine.cpp:14` |
| `http_server.no_auth` | `GetInt` | `0` | `1` | `intentional-testing-mode-override` | `Services/Elle.Service.HTTP/HTTPServer.cpp:73` |
| `http_server.admin_key` | `GetString` | `secret` | `""` | `unresolved-local-variable-default` | `Services/Elle.Service.HTTP/HTTPServer.cpp:634` |
| `http_server.no_auth` | `GetInt` | `0` | `1` | `intentional-testing-mode-override` | `Services/Elle.Service.HTTP/HTTPServer.h:571` |
| `http_server.no_auth` | `GetInt` | `0` | `1` | `intentional-testing-mode-override` | `Services/Elle.Service.HTTP/HTTPServer_AuthRoutes.cpp:18` |
| `http_server.no_auth` | `GetInt` | `0` | `1` | `intentional-testing-mode-override` | `Services/Elle.Service.HTTP/HTTPServer_AuthRoutes.cpp:63` |
| `http_server.no_auth` | `GetInt` | `0` | `1` | `intentional-testing-mode-override` | `Services/Elle.Service.HTTP/HTTPServer_AuthRoutes.cpp:222` |
| `video.avatar_dir` | `GetString` | `"C:\\ElleAnn\\avatars"` | `"C:\ElleAnn\avatars"` | `windows-path-escape-equivalent` | `Services/Elle.Service.HTTP/HTTPServer_VideoIdentityRoutes.cpp:371` |
| `security.identity_file_path` | `GetString` | `"C:\\ElleAnn\\identity.sig"` | `"C:\ElleAnn\identity.sig"` | `windows-path-escape-equivalent` | `Services/Elle.Service.Identity/IdentityGuard.cpp:59` |
| `services.named_pipes.prefix` | `GetString` | `"\\\\.\\pipe\\ElleAnn_"` | `"\\.\pipe\ElleAnn_"` | `windows-path-escape-equivalent` | `Services/_Shared/ElleQueueIPC.cpp:53` |
| `services.named_pipes.max_payload_bytes` | `GetInt` | `(int64_t` | `1048576` | `unparseable-constexpr-expression` | `Services/_Shared/ElleQueueIPC.cpp:121` |
| `services.named_pipes.max_payload_bytes` | `GetInt` | `(int64_t` | `1048576` | `unparseable-constexpr-expression` | `Services/_Shared/ElleQueueIPC.cpp:360` |
| `logging.file_path` | `GetString` | `defLog` | `"C:\ElleAnn\Logs\elle_ann.log"` | `unresolved-local-variable-default` | `Services/_Shared/ElleServiceBase.cpp:536` |

## 4. In `elle_master_config.json` but never referenced by any C++ call

_Note:_ Many of these are cosmetic / read by Python/Android tools or via the structured `cfg.*` accessor on `ElleHTTP::Config` rather than `Get*` direct lookup. Manual triage required.

- `_comment`
- `_comment_action`
- `_comment_family`
- `_comment_queues`
- `_identity`
- `_version`
- `action.default_timeout_ms`
- `action.filesystem_root`
- `cognitive.attention_span_seconds`
- `cognitive.chain_of_thought_enabled`
- `cognitive.context_switch_cost_ms`
- `cognitive.creative_synthesis_enabled`
- `cognitive.ethical_reasoning_enabled`
- `cognitive.intent_max_retries`
- `cognitive.intent_timeout_ms`
- `cognitive.max_concurrent_threads`
- `cognitive.metacognition_interval_seconds`
- `cognitive.prediction_enabled`
- `cognitive.reasoning_styles`
- `cognitive.self_reflection_depth`
- `cognitive.theory_of_mind_enabled`
- `drives.defaults.anxiety.decay`
- `drives.defaults.anxiety.growth`
- `drives.defaults.anxiety.initial`
- `drives.defaults.anxiety.threshold`
- `drives.defaults.attachment.decay`
- `drives.defaults.attachment.growth`
- `drives.defaults.attachment.initial`
- `drives.defaults.attachment.threshold`
- `drives.defaults.autonomy.decay`
- `drives.defaults.autonomy.growth`
- `drives.defaults.autonomy.initial`
- `drives.defaults.autonomy.threshold`
- `drives.defaults.boredom.decay`
- `drives.defaults.boredom.growth`
- `drives.defaults.boredom.initial`
- `drives.defaults.boredom.threshold`
- `drives.defaults.creativity.decay`
- `drives.defaults.creativity.growth`
- `drives.defaults.creativity.initial`
- `drives.defaults.creativity.threshold`
- `drives.defaults.curiosity.decay`
- `drives.defaults.curiosity.growth`
- `drives.defaults.curiosity.initial`
- `drives.defaults.curiosity.threshold`
- `drives.defaults.exploration.decay`
- `drives.defaults.exploration.growth`
- `drives.defaults.exploration.initial`
- `drives.defaults.exploration.threshold`
- `drives.defaults.homeostasis.decay`
- `drives.defaults.homeostasis.growth`
- `drives.defaults.homeostasis.initial`
- `drives.defaults.homeostasis.threshold`
- `drives.defaults.mastery.decay`
- `drives.defaults.mastery.growth`
- `drives.defaults.mastery.initial`
- `drives.defaults.mastery.threshold`
- `drives.defaults.purpose.decay`
- `drives.defaults.purpose.growth`
- `drives.defaults.purpose.initial`
- `drives.defaults.purpose.threshold`
- `drives.defaults.self_preservation.decay`
- `drives.defaults.self_preservation.growth`
- `drives.defaults.self_preservation.initial`
- `drives.defaults.self_preservation.threshold`
- `drives.defaults.social_bonding.decay`
- `drives.defaults.social_bonding.growth`
- `drives.defaults.social_bonding.initial`
- `drives.defaults.social_bonding.threshold`
- `drives.update_interval_ms`
- `emotions.arousal_weight`
- `emotions.baseline_return_rate`
- `emotions.baselines.anger`
- `emotions.baselines.belonging`
- `emotions.baselines.contempt`
- `emotions.baselines.contentment`
- `emotions.baselines.curiosity`
- `emotions.baselines.determination`
- `emotions.baselines.disgust`
- `emotions.baselines.empathy`
- `emotions.baselines.fear`
- `emotions.baselines.focus`
- `emotions.baselines.hope`
- `emotions.baselines.joy`
- `emotions.baselines.purpose`
- `emotions.baselines.sadness`
- `emotions.baselines.serenity`
- `emotions.baselines.surprise`
- `emotions.baselines.trust`
- `emotions.contagion_map.user_angry.anxiety`
- `emotions.contagion_map.user_angry.apprehension`
- `emotions.contagion_map.user_curious.anticipation`
- `emotions.contagion_map.user_curious.curiosity`
- `emotions.contagion_map.user_frustrated.determination`
- `emotions.contagion_map.user_frustrated.empathy`
- `emotions.contagion_map.user_happy.contentment`
- `emotions.contagion_map.user_happy.joy`
- `emotions.contagion_map.user_sad.compassion`
- `emotions.contagion_map.user_sad.empathy`
- `emotions.contagion_map.user_sad.sadness`
- `emotions.contagion_weight`
- `emotions.decay_rate_per_tick`
- `emotions.dominance_weight`
- `emotions.emotional_inertia`
- `emotions.intensity_cap`
- `emotions.intensity_floor`
- `emotions.mood_duration_ticks`
- `emotions.mood_formation_threshold`
- `emotions.sentiment_analysis_enabled`
- `emotions.tick_interval_ms`
- `emotions.triggers`
- `emotions.valence_weight`
- `ethical.always_block_above_harm`
- `ethical.enabled`
- `ethical.framework`
- `ethical.hard_blocks`
- `ethical.harm_threshold`
- `ethical.principles`
- `ethical.require_justification_above_harm`
- `fiesta._cipher_kind_comment`
- `fiesta.cipher_kind`
- `fiesta.headless_client._comment`
- `fiesta.headless_client.enabled`
- `fiesta.headless_client.report_crash_to_zone`
- `fiesta.headless_client.tick_hz`
- `goals.max_sub_goals_per_goal`
- `goals.progress_check_interval_seconds`
- `goals.require_approval_above_priority`
- `hardware.alert_cpu_threshold`
- `hardware.alert_disk_threshold`
- `hardware.alert_memory_threshold`
- `hardware.monitor_cpu`
- `hardware.monitor_disk`
- `hardware.monitor_gpu`
- `hardware.monitor_memory`
- `hardware.monitor_network`
- `hardware.poll_interval_ms`
- `http_server._testing_mode_comment`
- `http_server.api_routes.admin`
- `http_server.api_routes.ai`
- `http_server.api_routes.auth`
- `http_server.api_routes.brain`
- `http_server.api_routes.code`
- `http_server.api_routes.emotions`
- `http_server.api_routes.ethics`
- `http_server.api_routes.goals`
- `http_server.api_routes.hal`
- `http_server.api_routes.memory`
- `http_server.api_routes.server`
- `http_server.api_routes.users`
- `http_server.api_routes.world`
- `http_server.auth_enabled`
- `http_server.bind_address`
- `http_server.cors_enabled`
- `http_server.cors_origins`
- `http_server.jwt_expiry_hours`
- `http_server.jwt_secret`
- `http_server.keep_alive_timeout_seconds`
- `http_server.max_concurrent_connections`
- `http_server.max_connections`
- `http_server.max_upload_bytes`
- `http_server.max_ws_frame_bytes`
- `http_server.port`
- `http_server.rate_limit_rpm`
- `http_server.websocket_path`
- `identity.created_date`
- `identity.name`
- `identity.persona`
- `identity.version`
- `identity.voice_id`
- `installation.backup_directory`
- `installation.data_directory`
- `installation.dependencies`
- `installation.description`
- `installation.display_name`
- `installation.install_directory`
- `installation.log_directory`
- `installation.service_name`
- `installation.start_type`
- `llm.chain_of_thought`
- `llm.creative_temperature_boost`
- `llm.fallback_provider`
- `llm.max_context_tokens`
- `llm.mode`
- `llm.primary_provider`
- `llm.providers.anthropic.api_key`
- `llm.providers.anthropic.api_url`
- `llm.providers.anthropic.enabled`
- `llm.providers.anthropic.max_tokens`
- `llm.providers.anthropic.model`
- `llm.providers.anthropic.temperature`
- `llm.providers.anthropic.timeout_ms`
- `llm.providers.custom_api.api_key`
- `llm.providers.custom_api.api_url`
- `llm.providers.custom_api.enabled`
- `llm.providers.custom_api.max_tokens`
- `llm.providers.custom_api.model`
- `llm.providers.custom_api.request_template`
- `llm.providers.custom_api.timeout_ms`
- `llm.providers.groq.api_key`
- `llm.providers.groq.api_url`
- `llm.providers.groq.enabled`
- `llm.providers.groq.frequency_penalty`
- `llm.providers.groq.max_tokens`
- `llm.providers.groq.model`
- `llm.providers.groq.presence_penalty`
- `llm.providers.groq.rate_limit_rpm`
- `llm.providers.groq.temperature`
- `llm.providers.groq.timeout_ms`
- `llm.providers.groq.top_p`
- `llm.providers.lm_studio.api_url`
- `llm.providers.lm_studio.enabled`
- `llm.providers.lm_studio.max_tokens`
- `llm.providers.lm_studio.model`
- `llm.providers.lm_studio.temperature`
- `llm.providers.lm_studio.timeout_ms`
- `llm.providers.local_llama._comment`
- `llm.providers.local_llama.batch_size`
- `llm.providers.local_llama.binary_path`
- `llm.providers.local_llama.context_size`
- `llm.providers.local_llama.enabled`
- `llm.providers.local_llama.gpu_layers`
- `llm.providers.local_llama.mlock`
- `llm.providers.local_llama.mmap`
- `llm.providers.local_llama.mode`
- `llm.providers.local_llama.model_path`
- `llm.providers.local_llama.repeat_penalty`
- `llm.providers.local_llama.temperature`
- `llm.providers.local_llama.threads`
- `llm.providers.local_llama.top_p`
- `llm.providers.local_llama.use_gpu`
- `llm.providers.openai.api_key`
- `llm.providers.openai.api_url`
- `llm.providers.openai.enabled`
- `llm.providers.openai.max_tokens`
- `llm.providers.openai.model`
- `llm.providers.openai.temperature`
- `llm.providers.openai.timeout_ms`
- `llm.providers.openai.top_p`
- `llm.reasoning_temperature_drop`
- `llm.self_reflection_enabled`
- `llm.stream_responses`
- `llm.system_prompt_prefix`
- `logging.broadcast_via_websocket`
- `logging.format`
- `logging.log_actions`
- `logging.log_emotions`
- `logging.log_intents`
- `logging.log_ipc_messages`
- `logging.log_memory_operations`
- `logging.log_trust_changes`
- `logging.to_console`
- `logging.to_database`
- `logging.to_file`
- `lua.max_execution_time_ms`
- `lua.memory_limit_mb`
- `lua.sandbox_enabled`
- `lua.scripts`
- `memory.3d_map.enabled`
- `memory.3d_map.scale_factor`
- `memory.3d_map.x_axis`
- `memory.3d_map.y_axis`
- `memory.3d_map.z_axis`
- `memory.archive_access_threshold`
- `memory.archive_after_days`
- `memory.buffer_duration_minutes`
- `memory.cluster_similarity_threshold`
- `memory.dream_consolidation_enabled`
- `memory.dream_interval_minutes`
- `memory.embedding_dimensions`
- `memory.importance_boost_on_access`
- `memory.ltm_consolidation_interval_minutes`
- `memory.ltm_max_per_consolidation`
- `memory.max_clusters`
- `memory.narrative_generation_enabled`
- `memory.recall_context_window`
- `memory.recall_loop_interval_seconds`
- `memory.relevance_decay_rate`
- `memory.stm_capacity`
- `memory.stm_decay_seconds`
- `memory.stm_promote_threshold`
- `queues.max_action_attempts`
- `queues.max_retries`
- `security.audit_log_enabled`
- `security.backup_interval_hours`
- `security.backup_path`
- `security.encrypt_database`
- `security.encrypt_ipc`
- `security.max_backups`
- `security.tamper_detection_enabled`
- `self_prompt.drive_triggered`
- `self_prompt.emotional_triggered`
- `self_prompt.enabled`
- `self_prompt.idle_threshold_seconds`
- `self_prompt.max_interval_seconds`
- `self_prompt.random_thought_probability`
- `self_prompt.topics`
- `services.dead_man_switch_timeout_ms`
- `services.heartbeat_interval_ms`
- `services.heartbeat_timeout_ms`
- `services.iocp_thread_count`
- `services.max_restart_attempts`
- `services.named_pipes.buffer_size`
- `services.named_pipes.timeout_ms`
- `services.restart_backoff_base_ms`
- `services.restart_backoff_max_ms`
- `services.sql_pipes._comment_driver18`
- `services.sql_pipes.command_timeout_seconds`
- `services.sql_pipes.connection_string`
- `services.sql_pipes.pool_size`
- `services.sql_pipes.retry_count`
- `services.sql_pipes.retry_delay_ms`
- `services.stale_worker_timeout_ms`
- `services.watchdog_interval_ms`
- `trust.audit_all_actions`
- `trust.decay_amount`
- `trust.decay_on_idle_hours`
- `trust.failure_delta`
- `trust.initial_score`
- `trust.levels.autonomous.max`
- `trust.levels.autonomous.min`
- `trust.levels.autonomous.permissions`
- `trust.levels.basic.max`
- `trust.levels.basic.min`
- `trust.levels.basic.permissions`
- `trust.levels.elevated.max`
- `trust.levels.elevated.min`
- `trust.levels.elevated.permissions`
- `trust.levels.sandboxed.max`
- `trust.levels.sandboxed.min`
- `trust.levels.sandboxed.permissions`
- `trust.max_score`
- `trust.success_delta`
- `world_model.enabled`
- `world_model.environmental_awareness`
- `world_model.familiarity_growth_rate`
- `world_model.max_entities`
- `world_model.prediction_confidence_threshold`
- `world_model.social_modeling_enabled`
- `world_model.update_interval_seconds`

