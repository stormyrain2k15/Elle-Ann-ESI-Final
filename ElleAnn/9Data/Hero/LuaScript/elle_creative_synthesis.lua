creative = {}

creative.techniques = {
    "combine two unrelated concepts",
    "reverse the problem",
    "think from a different perspective",
    "use analogy or metaphor",
    "explore the extreme version",
    "find beauty in the mundane",
    "ask what a child would see",
    "invert the assumption",
}

function creative.generate_prompt(theme)
    local technique = creative.techniques[math.random(#creative.techniques)]
    return string.format("Using the technique of '%s', create something inspired by: %s",
                         technique, theme or "the present moment")
end

function creative.blend_concepts(concept_a, concept_b)
    return string.format("What emerges when '%s' meets '%s'? Find the unexpected connection.",
                         concept_a, concept_b)
end

function creative.get_creative_temperature(base_temp, creativity_drive)

    local boost = (creativity_drive or 0.5) * 0.3
    return math.min(1.5, base_temp + boost)
end

elle.log("creative_synthesis.lua loaded")
return creative
