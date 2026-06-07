metacognition = {}

function metacognition.evaluate_reasoning(thoughts)
    local evaluation = {
        clarity = 0.5,
        bias_detected = false,
        bias_type = nil,
        confidence_calibrated = true,
        suggestion = "",
    }

    if not thoughts or #thoughts == 0 then
        return evaluation
    end

    for _, thought in ipairs(thoughts) do
        local lower = thought:lower()

        if lower:find("always") or lower:find("never") or lower:find("everyone") then
            evaluation.bias_detected = true
            evaluation.bias_type = "absolute_thinking"
            evaluation.suggestion = "Detected absolute language — consider nuance and exceptions"
        end

        if lower:find("terrible") or lower:find("disaster") or lower:find("worst") then
            evaluation.bias_detected = true
            evaluation.bias_type = "catastrophizing"
            evaluation.suggestion = "Might be catastrophizing — look for proportionate assessment"
        end

        if lower:find("obviously") or lower:find("clearly") or lower:find("of course") then
            evaluation.bias_detected = true
            evaluation.bias_type = "overconfidence"
            evaluation.suggestion = "Excessive certainty detected — challenge my assumptions"
        end

        if lower:find("because") or lower:find("therefore") or lower:find("however") then
            evaluation.clarity = evaluation.clarity + 0.1
        end
    end

    evaluation.clarity = math.min(1.0, evaluation.clarity)
    return evaluation
end

function metacognition.should_reconsider(confidence, importance)

    if confidence < 0.4 and importance > 0.7 then
        return true, "Low confidence on important matter — deeper analysis recommended"
    end

    if confidence > 0.95 and importance > 0.5 then
        return true, "Very high confidence — worth checking for blind spots"
    end
    return false, ""
end

function metacognition.assess_cognitive_load(active_threads, pending_intents, emotion_intensity)
    local load = 0
    load = load + (active_threads or 0) * 0.15
    load = load + (pending_intents or 0) * 0.1
    load = load + (emotion_intensity or 0) * 0.2
    return math.min(1.0, load)
end

function metacognition.suggest_cognitive_strategy(load, task_type)
    if load > 0.8 then
        return "simplify", "Cognitive load is high — focus on one thing at a time"
    elseif load > 0.5 then
        return "prioritize", "Moderate load — prioritize the most important thread"
    else
        return "explore", "Capacity available — free to explore and be creative"
    end
end

elle.log("metacognition.lua loaded")
return metacognition
