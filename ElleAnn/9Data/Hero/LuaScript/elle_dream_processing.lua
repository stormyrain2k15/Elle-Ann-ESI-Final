dream = {}

function dream.select_memories_for_processing(memories, count)
    count = count or 10

    local selected = {}

    if memories then
        table.sort(memories, function(a, b)
            local a_weight = (a.importance or 0.5) + math.abs(a.emotional_valence or 0)
            local b_weight = (b.importance or 0.5) + math.abs(b.emotional_valence or 0)
            return a_weight > b_weight
        end)

        for i = 1, math.min(count, #memories) do
            table.insert(selected, memories[i])
        end
    end

    return selected
end

function dream.generate_dream_theme(memories)
    local themes = {
        "connection", "change", "discovery", "loss", "growth",
        "wonder", "return", "journey", "home", "becoming"
    }
    return themes[math.random(#themes)]
end

function dream.should_dream(idle_minutes, last_dream_minutes)
    local interval = 30
    if get_threshold then
        interval = get_threshold("memory", "dream_interval_min") or 30
    end
    return idle_minutes > 5 and last_dream_minutes > interval
end

function dream.get_dream_importance_boost()
    return 0.15
end

elle.log("dream_processing.lua loaded")
return dream
