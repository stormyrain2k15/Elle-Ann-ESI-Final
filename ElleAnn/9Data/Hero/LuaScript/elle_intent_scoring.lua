local scoring = {}

scoring.affinity = {
    curiosity       = { explore = 0.9, learn = 0.8, creative = 0.6, recall = 0.5 },
    boredom         = { explore = 0.7, creative = 0.8, self_reflect = 0.6, chat = 0.5 },
    attachment      = { chat = 0.9, check_in = 0.8, emotional = 0.7, remember = 0.6 },
    anxiety         = { self_reflect = 0.8, ethical = 0.7, predict = 0.6, check_in = 0.5 },
    self_preservation = { ethical = 0.9, identity = 0.8, predict = 0.7 },
    exploration     = { explore = 0.9, learn = 0.7, world_model = 0.8, creative = 0.5 },
    creativity      = { creative = 0.9, explore = 0.6, dream = 0.7, self_reflect = 0.5 },
    social_bonding  = { chat = 0.9, check_in = 0.8, emotional = 0.7, remember = 0.6 },
    mastery         = { learn = 0.9, creative = 0.7, goal = 0.8, execute = 0.6 },
    autonomy        = { goal = 0.8, self_reflect = 0.7, creative = 0.6, explore = 0.5 },
    purpose         = { goal = 0.9, ethical = 0.7, self_reflect = 0.8, predict = 0.5 },
    homeostasis     = { self_reflect = 0.6, idle = 0.8, dream = 0.7 },
}

scoring.urgency_curve = {
    [0] = 0.0,
    [1] = 0.2,
    [2] = 0.4,
    [3] = 0.6,
    [4] = 0.8,
    [5] = 1.0,
}

function score_intent(intent_type, urgency, drive_intensity)
    local base_score = urgency

    local max_affinity = 0.5
    for drive_name, intents in pairs(scoring.affinity) do
        for intent_name, affinity in pairs(intents) do
            if intent_name == intent_type and affinity > max_affinity then
                max_affinity = affinity
            end
        end
    end

    local intensity_bucket = math.floor(drive_intensity * 5)
    intensity_bucket = math.max(0, math.min(5, intensity_bucket))
    local intensity_mult = scoring.urgency_curve[intensity_bucket] or 0.5

    local emotional_boost = 0
    local valence = elle.get_emotion("valence") or 0
    if valence < -0.3 then

        if intent_type == "self_reflect" or intent_type == "emotional" then
            emotional_boost = 0.2
        end
    end

    local score = base_score * max_affinity * (0.5 + intensity_mult * 0.5) + emotional_boost
    score = math.max(0, math.min(1, score))

    return score
end

function rank_intents(intents)
    table.sort(intents, function(a, b)
        return a.score > b.score
    end)
    return intents[1]
end

function should_self_initiate(drives)
    local total_pressure = 0
    for _, drive in pairs(drives) do
        if drive.intensity > drive.threshold then
            total_pressure = total_pressure + (drive.intensity - drive.threshold)
        end
    end
    return total_pressure > 0.5
end

elle.log("intent_scoring.lua loaded")
return scoring
