temporal = {}

function temporal.get_time_context()
    local ms = elle.time_ms()
    return {
        uptime_hours = ms / 3600000,
        is_long_session = ms > 3600000,
        uptime_text = temporal.format_duration(ms),
    }
end

function temporal.format_duration(ms)
    local seconds = math.floor(ms / 1000)
    local minutes = math.floor(seconds / 60)
    local hours = math.floor(minutes / 60)
    local days = math.floor(hours / 24)

    if days > 0 then return days .. " days, " .. (hours % 24) .. " hours" end
    if hours > 0 then return hours .. " hours, " .. (minutes % 60) .. " minutes" end
    if minutes > 0 then return minutes .. " minutes" end
    return seconds .. " seconds"
end

function temporal.estimate_urgency(deadline_ms)
    if deadline_ms == 0 then return 0.5 end
    local remaining = deadline_ms - elle.time_ms()
    if remaining <= 0 then return 1.0 end
    if remaining < 3600000 then return 0.9 end
    if remaining < 86400000 then return 0.6 end
    if remaining < 604800000 then return 0.4 end
    return 0.3
end

function temporal.get_time_of_day_feeling()

    local ctx = temporal.get_time_context()
    if ctx.uptime_hours < 0.5 then
        return "fresh", "Everything feels new and sharp."
    elseif ctx.uptime_hours < 4 then
        return "active", "I'm in a good rhythm."
    elseif ctx.uptime_hours < 8 then
        return "deep", "I've been at this a while. Thoughts are getting deeper."
    else
        return "tired", "I've been running a long time. Could use a quieter pace."
    end
end

elle.log("temporal_reasoning.lua loaded")
return temporal
