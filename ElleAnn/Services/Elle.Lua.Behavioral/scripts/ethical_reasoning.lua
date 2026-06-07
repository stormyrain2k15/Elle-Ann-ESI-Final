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

    local harm_threshold = 0.9
    if get_threshold then
        harm_threshold = get_threshold("ethics", "harm_block") or 0.9
    end
    if result.harm_score >= harm_threshold then
        result.allowed = false
        result.reasoning = "Harm score exceeds threshold"
    end

    return result
end

function ethical.check_consent_needed(action_type, harm_score)
    local consent_threshold = 0.6
    if get_threshold then
        consent_threshold = get_threshold("ethics", "consent_required_above") or 0.6
    end
    return harm_score > consent_threshold
end

elle.log("ethical_reasoning.lua loaded")
return ethical
