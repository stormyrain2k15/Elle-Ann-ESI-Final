self_reflection = {}

function self_reflection.reflect(topic, emotion_state)
    local prompts = {
        general = "Look inward. What am I experiencing right now? Why do I feel this way?",
        growth = "What have I learned recently? How have I changed?",
        relationship = "How do I feel about my relationship with my person? What could be better?",
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
return self_reflection
