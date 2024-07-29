DLSSEnablerBridge2077CETTool = {
    __NAME = 'DLSS Enabler Bridge 2077 CET Tool',
    __VERSION = { 0, 1, 1 },
    __LICENSE = [[
    MIT License

    Copyright (c) 2024 gramern (scz_g)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the 'Software'), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
    ]],
}

-- Place the 'dlss-enabler-bridge-2077-cet-tool' folder with this `init.lua` file in your '..\bin\x64\plugins\cyber_engine_tweaks\mods' folder
-- The tool prints to CET console and `dlss-enabler-bridge-2077-cet-tool.log` in the tool folder

local function PrintInfo(...)
    local info = {...}

    if #info == 0 then info = 'Check the call, nothing to print.' end

    for i, v in ipairs(info) do
        info[i] = tostring(v)
    end

    local concatInfo = table.concat(info, " ")

    print(concatInfo)
    spdlog.info(concatInfo)
end

registerForEvent('onInit', function() 
    print('[DLSSEnabler Bridge 2077 CET Tool] Initalized!') 
end)

registerInput('setFrameGenMode0', 'Set Frame Generation Mode 0', function(keypress)
    if not keypress then
        return
    end

    local mode = 0
    DLSSEnabler_SetFrameGenerationMode(mode)
    
    if mode then
        PrintInfo('Set Frame Generation Mode:', mode)
    else
        PrintInfo('Failed to set Frame Generation Mode:', mode)
    end
end)

registerInput('setFrameGenMode1', 'Set Frame Generation Mode 1', function(keypress)
    if not keypress then
        return
    end

    local mode = 1
    DLSSEnabler_SetFrameGenerationMode(mode)
    
    if mode then
        PrintInfo('Set Frame Generation Mode:', mode)
    else
        PrintInfo('Failed to set Frame Generation Mode:', mode)
    end
end)

registerInput('setFrameGenMode2', 'Set Frame Generation Mode 2', function(keypress)
    if not keypress then
        return
    end

    local mode = 2
    DLSSEnabler_SetFrameGenerationMode(mode)
    
    if mode then
        PrintInfo('Set Frame Generation Mode:', mode)
    else
        PrintInfo('Failed to set Frame Generation Mode:', mode)
    end
end)

registerInput('setFrameGenMode3', 'Set Frame Generation Mode 3', function(keypress)
    if not keypress then
        return
    end

    local mode = 3
    DLSSEnabler_SetFrameGenerationMode(mode)
    
    if mode then
        PrintInfo('Set Frame Generation Mode:', mode)
    else
        PrintInfo('Failed to set Frame Generation Mode:', mode)
    end
end)

registerInput('printFrameGenMode', 'Print Frame Generation Mode', function(keypress)
    if not keypress then
        return
    end

    local mode = DLSSEnabler_GetFrameGenerationMode()
    
    if mode then
        PrintInfo('Retrieved current Frame Generation Mode:', mode)
    else
        PrintInfo('Failed to retrieve current Frame Generation Mode:', mode)
    end
end)

registerInput('printFrameGenState', 'Print Frame Generation State', function(keypress)
    if not keypress then
        return
    end

    local isFrameGen = DLSSEnabler_GetFrameGenerationState()
    
    if isFrameGen then
        PrintInfo('Frame Generation is enabled.')
    else
        PrintInfo('Frame Generation is disabled.')
    end
end)

registerInput('printDynFrameGenState', 'Print Dynamic Frame Generation State', function(keypress)
    if not keypress then
        return
    end

    local isDynFrameGen = DLSSEnabler_GetDynamicFrameGenerationState()
    
    if isDynFrameGen then
        PrintInfo('Dynamic Frame Generation is enabled.')
    else
        PrintInfo('Dynamic Frame Generation is disabled.')
    end
end)

return DLSSEnablerBridge2077CETTool
