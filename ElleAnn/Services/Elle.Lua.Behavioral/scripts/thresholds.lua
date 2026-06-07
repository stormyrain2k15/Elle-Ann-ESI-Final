thresholds = {}

thresholds.emotion = {
    mood_formation = 0.6,
    mood_duration_min = 300,
    contagion_sensitivity = 0.35,
    trigger_min_delta = 0.1,
    expression_threshold = 0.4,
    crisis_threshold = 0.85,
    baseline_deviation = 0.3,
}

thresholds.cognitive = {
    intent_confidence_min = 0.6,
    attention_decay_rate = 0.01,
    context_relevance_min = 0.3,
    metacognition_trigger = 0.7,
    creativity_flow_threshold = 0.75,
    reasoning_depth_default = 3,
    reasoning_depth_complex = 5,
}

thresholds.drives = {
    curiosity_trigger = 0.7,
    boredom_trigger = 0.6,
    attachment_trigger = 0.8,
    anxiety_trigger = 0.5,
    exploration_trigger = 0.65,
    creativity_trigger = 0.6,
    mastery_trigger = 0.7,
}

thresholds.trust = {
    auto_approve_above = 30,
    warn_below = 15,
    dangerous_action_min = 50,
}

thresholds.memory = {
    stm_importance_promote = 0.6,
    stm_access_count_promote = 3,
    ltm_relevance_min = 0.3,
    cluster_similarity = 0.75,
    narrative_memory_count = 10,
    dream_importance_boost = 0.15,
}

thresholds.self_prompt = {
    idle_before_thought = 60,
    min_between_thoughts = 30,
    drive_pressure_min = 0.5,
    random_thought_chance = 0.15,
}

thresholds.goals = {
    motivation_min = 0.2,
    progress_stale_days = 7,
    max_concurrent_goals = 16,
    auto_generate_drive_min = 0.7,
}

thresholds.ethics = {
    harm_block = 0.9,
    harm_warn = 0.5,
    benefit_override = 0.85,
    consent_required_above = 0.6,
}

function get_threshold(category, name)
    local cat = thresholds[category]
    if cat then
        return cat[name]
    end
    return nil
end

elle.log("thresholds.lua loaded")
