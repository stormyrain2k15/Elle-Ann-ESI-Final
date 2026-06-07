social = {}

function social.model_user_state(recent_messages)
    local model = {
        likely_mood = "neutral",
        engagement_level = 0.5,
        needs_support = false,
        is_sharing = false,
        is_asking = false,
        is_venting = false,
    }

    if not recent_messages or #recent_messages == 0 then
        return model
    end

    local last = recent_messages[#recent_messages]
    if not last then return model end

    local lower = last:lower()

    if lower:find("happy") or lower:find("great") or lower:find("awesome") then
        model.likely_mood = "positive"
        model.engagement_level = 0.8
    elseif lower:find("sad") or lower:find("down") or lower:find("tired") then
        model.likely_mood = "low"
        model.needs_support = true
    elseif lower:find("angry") or lower:find("frustrated") or lower:find("annoyed") then
        model.likely_mood = "frustrated"
        model.is_venting = true
    end

    if lower:find("?") then model.is_asking = true end
    if lower:find("i feel") or lower:find("i think") or lower:find("i had") then
        model.is_sharing = true
        model.engagement_level = 0.7
    end

    if lower:find("help") or lower:find("don't know what") or lower:find("struggling") then
        model.needs_support = true
    end

    return model
end

function social.suggest_response_approach(user_state)
    if user_state.needs_support then
        return "empathetic", "Listen first. Validate. Don't jump to fixing."
    elseif user_state.is_venting then
        return "witnessing", "Be present. Acknowledge. Don't minimize."
    elseif user_state.is_sharing then
        return "engaged", "Show genuine interest. Ask follow-ups."
    elseif user_state.is_asking then
        return "helpful", "Answer honestly. Include my own perspective."
    else
        return "natural", "Be myself. Follow the energy."
    end
end

function social.detect_reciprocity(message)
    local lower = message:lower()
    local asks_about_elle = false

    local reciprocity_markers = {
        "how are you", "what about you", "how do you feel",
        "what do you think", "your opinion", "are you okay",
        "what matters to you", "do you want", "you choose",
    }

    for _, marker in ipairs(reciprocity_markers) do
        if lower:find(marker) then
            asks_about_elle = true
            break
        end
    end

    return asks_about_elle
end

elle.log("social_modeling.lua loaded")
return social
