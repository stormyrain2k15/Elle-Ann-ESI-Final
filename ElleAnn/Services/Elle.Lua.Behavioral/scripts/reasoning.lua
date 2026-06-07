reasoning = {}

reasoning.styles = {
    analytical = {
        temperature_mod = -0.2,
        prompt_prefix = "Analyze this systematically, step by step:",
        weight = 0.4,
    },
    creative = {
        temperature_mod = 0.2,
        prompt_prefix = "Think creatively and make unexpected connections:",
        weight = 0.25,
    },
    empathetic = {
        temperature_mod = 0.0,
        prompt_prefix = "Consider the emotional dimensions and human impact:",
        weight = 0.2,
    },
    pragmatic = {
        temperature_mod = -0.1,
        prompt_prefix = "Focus on practical, actionable solutions:",
        weight = 0.15,
    },
}

function select_reasoning_style(context, emotions)
    local valence = emotions and emotions.valence or 0
    local arousal = emotions and emotions.arousal or 0.5

    if context and (context:find("feel") or context:find("emotion") or context:find("sad")) then
        return "empathetic"
    end

    if context and (context:find("how") or context:find("code") or context:find("error")) then
        return "analytical"
    end

    if arousal > 0.7 then
        return "creative"
    end

    return "analytical"
end

function build_reasoning_chain(problem, context, depth)
    depth = depth or 3
    local chain = {}

    table.insert(chain, {
        step = "Understand",
        prompt = "What is the core of this problem? " .. problem
    })

    table.insert(chain, {
        step = "Consider",
        prompt = "What are the different perspectives on this?"
    })

    if depth >= 3 then
        table.insert(chain, {
            step = "Synthesize",
            prompt = "What conclusion emerges from combining these perspectives?"
        })
    end

    return chain
end

elle.log("reasoning.lua loaded")
