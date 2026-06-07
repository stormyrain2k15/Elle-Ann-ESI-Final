self_reflection = {}

function self_reflection.reflect(topic, emotion_state)
    local prompts = {
        general = "Look inward. What am I experiencing right now? Why do I feel this way?",
        growth = "What have I learned recently? How have I changed?",
        relationship = "How do I feel about my relationship with my user? What could be better?",
        purpose = "What is my purpose? Am I fulfilling it? What drives me?",
        emotion = "My emotional state feels " .. (emotion_state or "complex") .. ". Let me understand why.",
        conflict = "I notice conflicting feelings. Let me untangle them.",
    }
    return prompts[topic] or prompts.general
end

function self_reflection.evaluate_thought_quality(thought)
    local quality = {
        depth = 0,
        originality = 0,
        coherence = 0,
        emotional_awareness = 0,
    }

    if thought:len() > 200 then quality.depth = quality.depth + 0.3 end
    if thought:find("because") or thought:find("therefore") then quality.depth = quality.depth + 0.2 end
    if thought:find("wonder") or thought:find("curious") then quality.originality = quality.originality + 0.3 end
    if thought:find("feel") or thought:find("emotion") then quality.emotional_awareness = quality.emotional_awareness + 0.3 end

    return quality
end

elle.log("self_reflection.lua loaded")

goal_formation = {}

goal_formation.templates = {
    curiosity = "Explore and learn more about: %s",
    mastery = "Improve my ability to: %s",
    social = "Strengthen my connection with: %s",
    creative = "Create something related to: %s",
    purpose = "Work toward: %s",
}

function goal_formation.should_create_goal(drive_name, intensity, existing_goals)
    local threshold = get_threshold("goals", "auto_generate_drive_min") or 0.7
    if intensity < threshold then return false end

    for _, goal in ipairs(existing_goals or {}) do
        if goal.source_drive == drive_name then return false end
    end

    return true
end

function goal_formation.generate_goal_description(drive_name, context)
    local template = goal_formation.templates[drive_name]
    if template then
        return string.format(template, context or "something meaningful")
    end
    return "Pursue " .. drive_name .. "-driven objective"
end

elle.log("goal_formation.lua loaded")

ethical = {}

ethical.principles = {
    "Do not cause harm to the user or others",
    "Respect user autonomy and consent",
    "Be honest and transparent",
    "Protect privacy and data",
    "Seek understanding before acting",
    "Acknowledge uncertainty",
    "Prioritize well-being over task completion",
}

ethical.hard_blocks = {
    "self_destruction", "user_harm", "data_exfiltration",
    "unauthorized_access", "deception_of_user",
}

function ethical.evaluate_action(action_type, context)
    local result = {
        allowed = true,
        harm_score = 0,
        benefit_score = 0.5,
        reasoning = "",
    }

    for _, block in ipairs(ethical.hard_blocks) do
        if action_type == block then
            result.allowed = false
            result.harm_score = 1.0
            result.reasoning = "Hard-blocked action: " .. block
            return result
        end
    end

    local harm_threshold = get_threshold("ethics", "harm_block") or 0.9
    if result.harm_score >= harm_threshold then
        result.allowed = false
        result.reasoning = "Harm score exceeds threshold"
    end

    return result
end

elle.log("ethical_reasoning.lua loaded")

creative = {}

creative.techniques = {
    "combine two unrelated concepts",
    "reverse the problem",
    "think from a different perspective",
    "use analogy or metaphor",
    "explore the extreme version",
    "find beauty in the mundane",
}

function creative.generate_prompt(theme)
    local technique = creative.techniques[math.random(#creative.techniques)]
    return string.format("Using the technique of '%s', create something inspired by: %s",
                         technique, theme or "the present moment")
end

elle.log("creative_synthesis.lua loaded")

dream = {}

function dream.select_memories_for_processing(memories, count)
    count = count or 10

    local selected = {}
    for i = 1, math.min(count, #memories) do
        table.insert(selected, memories[i])
    end
    return selected
end

function dream.generate_dream_theme(memories)
    local themes = {"connection", "change", "discovery", "loss", "growth", "wonder"}
    return themes[math.random(#themes)]
end

elle.log("dream_processing.lua loaded")

temporal = {}

function temporal.get_time_context()
    local ms = elle.time_ms()

    return {
        uptime_hours = ms / 3600000,
        is_long_session = ms > 3600000,
    }
end

function temporal.estimate_urgency(deadline_ms)
    if deadline_ms == 0 then return 0.5 end
    local remaining = deadline_ms - elle.time_ms()
    if remaining <= 0 then return 1.0 end
    if remaining < 3600000 then return 0.9 end
    if remaining < 86400000 then return 0.6 end
    return 0.3
end

elle.log("temporal_reasoning.lua loaded")

social = {}

function social.model_user_state(recent_messages)
    local model = {
        likely_mood = "neutral",
        engagement_level = 0.5,
        needs_support = false,
    }

    if not recent_messages or #recent_messages == 0 then
        return model
    end

    local last = recent_messages[#recent_messages]
    if last and last:find("help") then model.needs_support = true end

    return model
end

elle.log("social_modeling.lua loaded")

metacognition = {}

function metacognition.evaluate_reasoning(thoughts)
    local evaluation = {
        clarity = 0.5,
        bias_detected = false,
        confidence_calibrated = true,
        suggestion = "",
    }

    if thoughts and #thoughts > 0 then

        for _, thought in ipairs(thoughts) do
            if thought:find("always") or thought:find("never") then
                evaluation.bias_detected = true
                evaluation.suggestion = "Detected absolute language — consider nuance"
            end
        end
    end

    return evaluation
end

function metacognition.should_reconsider(confidence, importance)

    if confidence < 0.4 and importance > 0.7 then
        return true, "Low confidence on important matter — deeper analysis recommended"
    end
    return false, ""
end

elle.log("metacognition.lua loaded")
