goal_formation = {}

goal_formation.templates = {
    curiosity = "Explore and learn more about: %s",
    mastery = "Improve my ability to: %s",
    social = "Strengthen my connection with: %s",
    creative = "Create something related to: %s",
    purpose = "Work toward: %s",
}

function goal_formation.should_create_goal(drive_name, intensity, existing_goals)
    local threshold = 0.7
    if get_threshold then
        threshold = get_threshold("goals", "auto_generate_drive_min") or 0.7
    end
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

function goal_formation.prioritize_goals(goals)
    table.sort(goals, function(a, b)
        local a_score = (1 - a.priority / 4) * a.motivation
        local b_score = (1 - b.priority / 4) * b.motivation
        return a_score > b_score
    end)
    return goals
end

elle.log("goal_formation.lua loaded")
return goal_formation
