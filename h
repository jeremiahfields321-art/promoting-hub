-- ==========================================
-- 1. SERVICES & STATE
-- ==========================================
local Players = game:GetService("Players")
local RunService = game:GetService("RunService")
local UserInputService = game:GetService("UserInputService")
local CoreGui = game:GetService("CoreGui")
local TweenService = game:GetService("TweenService")

local LocalPlayer = Players.LocalPlayer
local Camera = workspace.CurrentCamera

-- Settings State
local Settings = {
    LockOnEnabled = true,
    PredictionEnabled = true,
    WallCheckEnabled = true,
    DeadCheckEnabled = true,
    AimAssistEnabled = true,
    Smoothness = 0.85,
    PredictionScale = 1.0,
    TargetPart = "HumanoidRootPart",
    Keybind = "RightClick", -- Default keybind
    IsListeningForKey = false
}

-- Internal State
local isLockedOn = false
local currentTarget = nil
local cameraConnection = nil
local lastSeenTime = 0
local trackers = {}
local isMinimized = false
local keybindBtn = nil -- Reference to update the keybind button text

-- Futuristic Neon Theme
local Theme = {
    Bg = Color3.fromRGB(12, 14, 18),
    TopBar = Color3.fromRGB(20, 24, 30),
    Accent1 = Color3.fromRGB(0, 255, 200),
    Accent2 = Color3.fromRGB(180, 0, 255),
    Text = Color3.fromRGB(220, 230, 240),
    ToggleOn = Color3.fromRGB(0, 255, 150),
    ToggleOff = Color3.fromRGB(255, 50, 80),
    SliderBg = Color3.fromRGB(35, 40, 50)
}

-- ==========================================
-- 2. GUI CREATION
-- ==========================================
if CoreGui:FindFirstChild("PromotingHubUI") then
    CoreGui.PromotingHubUI:Destroy()
end

local ScreenGui = Instance.new("ScreenGui")
ScreenGui.Name = "PromotingHubUI"
ScreenGui.ResetOnSpawn = false
ScreenGui.Parent = CoreGui

-- ===== 2A. FLOATING MINIMIZE BUTTON =====
local FloatingBtn = Instance.new("TextButton")
FloatingBtn.Size = UDim2.new(0, 65, 0, 65)
FloatingBtn.Position = UDim2.new(1, -90, 0.5, -32)
FloatingBtn.BackgroundColor3 = Theme.TopBar
FloatingBtn.Text = "◈"
FloatingBtn.TextColor3 = Theme.Accent1
FloatingBtn.Font = Enum.Font.GothamBlack
FloatingBtn.TextSize = 32
FloatingBtn.BackgroundTransparency = 1
FloatingBtn.Visible = false
FloatingBtn.Parent = ScreenGui

local FloatCorner = Instance.new("UICorner")
FloatCorner.CornerRadius = UDim.new(1, 0)
FloatCorner.Parent = FloatingBtn

local FloatStroke = Instance.new("UIStroke")
FloatStroke.Color = Theme.Accent1
FloatStroke.Thickness = 3
FloatStroke.Transparency = 1
FloatStroke.Parent = FloatingBtn

local SpinningGradient = Instance.new("UIGradient")
SpinningGradient.Color = ColorSequence.new({
    ColorSequenceKeypoint.new(0, Theme.Accent1),
    ColorSequenceKeypoint.new(0.5, Theme.Accent2),
    ColorSequenceKeypoint.new(1, Theme.Accent1)
})
SpinningGradient.Parent = FloatStroke

RunService.RenderStepped:Connect(function()
    if FloatingBtn.Visible then
        local rot = (tick() % 2) / 2 * 360
        SpinningGradient.Rotation = rot
    end
end)

-- Floating button drag logic
local dragFloat, dragStartFloat, startPosFloat
FloatingBtn.InputBegan:Connect(function(input)
    if input.UserInputType == Enum.UserInputType.MouseButton1 or input.UserInputType == Enum.UserInputType.Touch then
        dragFloat = true
        dragStartFloat = input.Position
        startPosFloat = FloatingBtn.Position
        input.Changed:Connect(function()
            if input.UserInputState == Enum.UserInputState.End then dragFloat = false end
        end)
    end
end)
FloatingBtn.InputChanged:Connect(function(input)
    if dragFloat and (input.UserInputType == Enum.UserInputType.MouseMovement or input.UserInputType == Enum.UserInputType.Touch) then
        local delta = input.Position - dragStartFloat
        FloatingBtn.Position = UDim2.new(startPosFloat.X.Scale, startPosFloat.X.Offset + delta.X, startPosFloat.Y.Scale, startPosFloat.Y.Offset + delta.Y)
    end
end)

-- ===== 2B. MAIN PANEL =====
local MainFrame = Instance.new("Frame")
MainFrame.Size = UDim2.new(0, 310, 0, 400) -- Widened for the keybind button
MainFrame.Position = UDim2.new(0.5, -155, 0.5, -200)
MainFrame.BackgroundColor3 = Theme.Bg
MainFrame.BorderSizePixel = 0
MainFrame.ClipsDescendants = true
MainFrame.Parent = ScreenGui

local UICorner = Instance.new("UICorner")
UICorner.CornerRadius = UDim.new(0, 12)
UICorner.Parent = MainFrame

local UIStroke = Instance.new("UIStroke")
UIStroke.Color = Theme.Accent1
UIStroke.Thickness = 2
UIStroke.Transparency = 0.2
UIStroke.Parent = MainFrame

local MainGradient = Instance.new("UIGradient")
MainGradient.Color = ColorSequence.new({
    ColorSequenceKeypoint.new(0, Theme.Accent1),
    ColorSequenceKeypoint.new(1, Theme.Accent2)
})
MainGradient.Rotation = 45
MainGradient.Parent = UIStroke

-- Top Bar
local TopBar = Instance.new("Frame")
TopBar.Size = UDim2.new(1, 0, 0, 45)
TopBar.BackgroundColor3 = Theme.TopBar
TopBar.Parent = MainFrame

local TopCorner = Instance.new("UICorner")
TopCorner.CornerRadius = UDim.new(0, 12)
TopCorner.Parent = TopBar

local TopFix = Instance.new("Frame")
TopFix.Size = UDim2.new(1, 0, 0.5, 0)
TopFix.Position = UDim2.new(0, 0, 0.5, 0)
TopFix.BackgroundColor3 = Theme.TopBar
TopFix.BorderSizePixel = 0
TopFix.Parent = TopBar

local Title = Instance.new("TextLabel")
Title.Size = UDim2.new(1, -50, 1, 0)
Title.Position = UDim2.new(0, 20, 0, 0)
Title.BackgroundTransparency = 1
Title.Text = "PROMOTING <font color='#00FFC8'>HUB</font>"
Title.RichText = true
Title.TextColor3 = Theme.Text
Title.TextXAlignment = Enum.TextXAlignment.Left
Title.Font = Enum.Font.GothamBlack
Title.TextSize = 16
Title.Parent = TopBar

local MinBtn = Instance.new("TextButton")
MinBtn.Size = UDim2.new(0, 35, 0, 30)
MinBtn.Position = UDim2.new(1, -50, 0.5, -15)
MinBtn.BackgroundColor3 = Color3.fromRGB(30, 35, 45)
MinBtn.Text = "×"
MinBtn.TextColor3 = Theme.Accent2
MinBtn.Font = Enum.Font.GothamBold
MinBtn.TextSize = 20
MinBtn.Parent = TopBar
local MinCorner = Instance.new("UICorner")
MinCorner.CornerRadius = UDim.new(0, 6)
MinCorner.Parent = MinBtn

local ScrollFrame = Instance.new("ScrollingFrame")
ScrollFrame.Size = UDim2.new(1, -20, 1, -60)
ScrollFrame.Position = UDim2.new(0, 10, 0, 50)
ScrollFrame.BackgroundTransparency = 1
ScrollFrame.ScrollBarThickness = 2
ScrollFrame.ScrollBarImageColor3 = Theme.Accent1
ScrollFrame.CanvasSize = UDim2.new(0, 0, 0, 0)
ScrollFrame.AutomaticCanvasSize = Enum.AutomaticSize.Y
ScrollFrame.Parent = MainFrame

local UIListLayout = Instance.new("UIListLayout")
UIListLayout.Padding = UDim.new(0, 12)
UIListLayout.SortOrder = Enum.SortOrder.LayoutOrder
UIListLayout.Parent = ScrollFrame

-- ==========================================
-- 3. FUTURISTIC UI HELPER FUNCTIONS
-- ==========================================
local function CreateToggle(name, stateKey, showKeybind)
    local Frame = Instance.new("Frame")
    Frame.Size = UDim2.new(1, 0, 0, 35)
    Frame.BackgroundTransparency = 1
    Frame.Parent = ScrollFrame
    
    local Label = Instance.new("TextLabel")
    Label.Size = UDim2.new(1, -110, 1, 0) -- Make room for both keybind and toggle
    Label.BackgroundTransparency = 1
    Label.Text = "// " .. name
    Label.TextColor3 = Theme.Text
    Label.TextXAlignment = Enum.TextXAlignment.Left
    Label.Font = Enum.Font.Code
    Label.TextSize = 13
    Label.Parent = Frame
    
    local Btn = Instance.new("TextButton")
    Btn.Size = UDim2.new(0, 45, 0, 24)
    Btn.Position = UDim2.new(1, -45, 0.5, -12)
    Btn.BackgroundColor3 = Settings[stateKey] and Theme.ToggleOn or Theme.ToggleOff
    Btn.Text = Settings[stateKey] and "ON" or "OFF"
    Btn.TextColor3 = Color3.fromRGB(0, 0, 0)
    Btn.Font = Enum.Font.GothamBlack
    Btn.TextSize = 12
    Btn.Parent = Frame
    local BtnCorner = Instance.new("UICorner")
    BtnCorner.CornerRadius = UDim.new(0, 6)
    BtnCorner.Parent = Btn
    
    Btn.MouseButton1Click:Connect(function()
        Settings[stateKey] = not Settings[stateKey]
        Btn.Text = Settings[stateKey] and "ON" or "OFF"
        Btn.BackgroundColor3 = Settings[stateKey] and Theme.ToggleOn or Theme.ToggleOff
        if stateKey == "LockOnEnabled" and not Settings.LockOnEnabled then
            StopLockOn()
        end
    end)

    -- Add the Keybind Button specifically to the Lock-On row
    if showKeybind then
        local kbBtn = Instance.new("TextButton")
        kbBtn.Size = UDim2.new(0, 60, 0, 24)
        kbBtn.Position = UDim2.new(1, -115, 0.5, -12)
        kbBtn.BackgroundColor3 = Theme.TopBar
        kbBtn.Text = Settings.Keybind
        kbBtn.TextColor3 = Theme.Accent1
        kbBtn.Font = Enum.Font.Code
        kbBtn.TextSize = 11
        kbBtn.Parent = Frame
        local kbCorner = Instance.new("UICorner")
        kbCorner.CornerRadius = UDim.new(0, 6)
        kbCorner.Parent = kbBtn
        keybindBtn = kbBtn -- Save reference to update it later

        kbBtn.MouseButton1Click:Connect(function()
            if Settings.IsListeningForKey then return end
            Settings.IsListeningForKey = true
            kbBtn.Text = "Press..."
            kbBtn.TextColor3 = Color3.fromRGB(255, 200, 0)
        end)
    end
end

local function CreateSlider(name, stateKey, min, max)
    local Frame = Instance.new("Frame")
    Frame.Size = UDim2.new(1, 0, 0, 45)
    Frame.BackgroundTransparency = 1
    Frame.Parent = ScrollFrame
    
    local Label = Instance.new("TextLabel")
    Label.Size = UDim2.new(1, 0, 0, 15)
    Label.BackgroundTransparency = 1
    Label.Text = "// " .. name .. ": " .. string.format("%.2f", Settings[stateKey])
    Label.TextColor3 = Theme.Text
    Label.TextXAlignment = Enum.TextXAlignment.Left
    Label.Font = Enum.Font.Code
    Label.TextSize = 12
    Label.Parent = Frame
    
    local Bar = Instance.new("Frame")
    Bar.Size = UDim2.new(1, 0, 0, 6)
    Bar.Position = UDim2.new(0, 0, 0, 25)
    Bar.BackgroundColor3 = Theme.SliderBg
    Bar.Parent = Frame
    local BarCorner = Instance.new("UICorner")
    BarCorner.CornerRadius = UDim.new(0, 3)
    BarCorner.Parent = Bar
    
    local Fill = Instance.new("Frame")
    Fill.Size = UDim2.new((Settings[stateKey] - min) / (max - min), 0, 1, 0)
    Fill.BackgroundColor3 = Theme.Accent1
    Fill.Parent = Bar
    local FillCorner = Instance.new("UICorner")
    FillCorner.CornerRadius = UDim.new(0, 3)
    FillCorner.Parent = Fill
    
    local DragBtn = Instance.new("TextButton")
    DragBtn.Size = UDim2.new(1, 0, 2.5, 0)
    DragBtn.Position = UDim2.new(0, 0, -0.75, 0)
    DragBtn.BackgroundTransparency = 1
    DragBtn.Text = ""
    DragBtn.Parent = Bar
    
    local draggingSlider = false
    DragBtn.InputBegan:Connect(function(input)
        if input.UserInputType == Enum.UserInputType.MouseButton1 or input.UserInputType == Enum.UserInputType.Touch then draggingSlider = true end
    end)
    UserInputService.InputEnded:Connect(function(input)
        if input.UserInputType == Enum.UserInputType.MouseButton1 or input.UserInputType == Enum.UserInputType.Touch then draggingSlider = false end
    end)
    UserInputService.InputChanged:Connect(function(input)
        if draggingSlider and (input.UserInputType == Enum.UserInputType.MouseMovement or input.UserInputType == Enum.UserInputType.Touch) then
            local mouseX = input.Position.X
            local barAbsolute = Bar.AbsolutePosition.X
            local barSize = Bar.AbsoluteSize.X
            local percent = math.clamp((mouseX - barAbsolute) / barSize, 0, 1)
            local val = min + (max - min) * percent
            Settings[stateKey] = val
            Fill.Size = UDim2.new(percent, 0, 1, 0)
            Label.Text = "// " .. name .. ": " .. string.format("%.2f", val)
        end
    end)
end

local function CreateDropdown(name, stateKey, options)
    local Frame = Instance.new("Frame")
    Frame.Size = UDim2.new(1, 0, 0, 35)
    Frame.BackgroundTransparency = 1
    Frame.Parent = ScrollFrame
    
    local Label = Instance.new("TextLabel")
    Label.Size = UDim2.new(1, -110, 1, 0)
    Label.BackgroundTransparency = 1
    Label.Text = "// " .. name
    Label.TextColor3 = Theme.Text
    Label.TextXAlignment = Enum.TextXAlignment.Left
    Label.Font = Enum.Font.Code
    Label.TextSize = 12
    Label.Parent = Frame
    
    local DropBtn = Instance.new("TextButton")
    DropBtn.Size = UDim2.new(0, 110, 0, 24)
    DropBtn.Position = UDim2.new(1, -110, 0.5, -12)
    DropBtn.BackgroundColor3 = Theme.TopBar
    DropBtn.Text = Settings[stateKey]
    DropBtn.TextColor3 = Theme.Accent1
    DropBtn.Font = Enum.Font.Code
    DropBtn.TextSize = 11
    DropBtn.Parent = Frame
    local DropCorner = Instance.new("UICorner")
    DropCorner.CornerRadius = UDim.new(0, 6)
    DropCorner.Parent = DropBtn
    
    local DropList = Instance.new("Frame")
    DropList.Size = UDim2.new(0, 110, 0, #options * 25)
    DropList.Position = UDim2.new(1, -110, 1, 2)
    DropList.BackgroundColor3 = Theme.TopBar
    DropList.Visible = false
    DropList.ZIndex = 5
    DropList.Parent = Frame
    local ListCorner = Instance.new("UICorner")
    ListCorner.CornerRadius = UDim.new(0, 6)
    ListCorner.Parent = DropList
    
    local ListLayout = Instance.new("UIListLayout")
    ListLayout.SortOrder = Enum.SortOrder.LayoutOrder
    ListLayout.Parent = DropList
    
    DropBtn.MouseButton1Click:Connect(function() DropList.Visible = not DropList.Visible end)
    
    for i, opt in ipairs(options) do
        local OptBtn = Instance.new("TextButton")
        OptBtn.Size = UDim2.new(1, 0, 0, 25)
        OptBtn.BackgroundColor3 = Theme.TopBar
        OptBtn.Text = opt
        OptBtn.TextColor3 = Theme.Text
        OptBtn.Font = Enum.Font.Code
        OptBtn.TextSize = 11
        OptBtn.LayoutOrder = i
        OptBtn.Parent = DropList
        OptBtn.MouseButton1Click:Connect(function()
            Settings[stateKey] = opt
            DropBtn.Text = opt
            DropList.Visible = false
        end)
    end
end

-- ==========================================
-- 4. BUILD CONTENT
-- ==========================================
-- Passed 'true' to show the keybind changer inline with the Lock-On toggle
CreateToggle("Enable Lock-On", "LockOnEnabled", true)
CreateToggle("Prediction", "PredictionEnabled", false)
CreateToggle("Wall Check", "WallCheckEnabled", false)
CreateToggle("Dead/Ragdoll", "DeadCheckEnabled", false)
CreateToggle("Aim Assist", "AimAssistEnabled", false)
CreateSlider("Smoothness", "Smoothness", 0.1, 1.0)
CreateSlider("Prediction Scale", "PredictionScale", 0.1, 2.0)
CreateDropdown("Target Part", "TargetPart", {"HumanoidRootPart", "Head", "UpperTorso", "Torso"})

-- ==========================================
-- 5. FADE ANIMATIONS
-- ==========================================
local function FadeOutMenu()
    local tween = TweenService:Create(MainFrame, TweenInfo.new(0.2), {BackgroundTransparency = 1})
    TweenService:Create(UIStroke, TweenInfo.new(0.2), {Transparency = 1}):Play()
    tween:Play()
    
    tween.Completed:Connect(function()
        for _, v in ipairs(MainFrame:GetDescendants()) do
            if v:IsA("TextLabel") or v:IsA("TextButton") or v:IsA("Frame") or v:IsA("ScrollingFrame") then
                if v ~= MainFrame then v.Visible = false end
            end
        end
        MainFrame.Visible = false
        
        FloatingBtn.Visible = true
        FloatingBtn.BackgroundTransparency = 1
        FloatStroke.Transparency = 1
        TweenService:Create(FloatingBtn, TweenInfo.new(0.2), {BackgroundTransparency = 0}):Play()
        TweenService:Create(FloatStroke, TweenInfo.new(0.2), {Transparency = 0.2}):Play()
    end)
end

local function FadeInMenu()
    FloatingBtn.Visible = false
    MainFrame.Visible = true
    for _, v in ipairs(MainFrame:GetDescendants()) do
        if v:IsA("TextLabel") or v:IsA("TextButton") or v:IsA("Frame") or v:IsA("ScrollingFrame") then
            if v ~= MainFrame then v.Visible = true end
        end
    end
    
    MainFrame.BackgroundTransparency = 1
    UIStroke.Transparency = 1
    TweenService:Create(MainFrame, TweenInfo.new(0.2), {BackgroundTransparency = 0}):Play()
    TweenService:Create(UIStroke, TweenInfo.new(0.2), {Transparency = 0.2}):Play()
end

MinBtn.MouseButton1Click:Connect(FadeOutMenu)
FloatingBtn.MouseButton1Click:Connect(FadeInMenu)

-- Draggable Main Frame
local dragging, dragStart, startPos
TopBar.InputBegan:Connect(function(input)
    if input.UserInputType == Enum.UserInputType.MouseButton1 or input.UserInputType == Enum.UserInputType.Touch then
        dragging = true
        dragStart = input.Position
        startPos = MainFrame.Position
        input.Changed:Connect(function()
            if input.UserInputState == Enum.UserInputState.End then dragging = false end
        end)
    end
end)
TopBar.InputChanged:Connect(function(input)
    if dragging and (input.UserInputType == Enum.UserInputType.MouseMovement or input.UserInputType == Enum.UserInputType.Touch) then
        local delta = input.Position - dragStart
        MainFrame.Position = UDim2.new(startPos.X.Scale, startPos.X.Offset + delta.X, startPos.Y.Scale, startPos.Y.Offset + delta.Y)
    end
end)

-- ==========================================
-- 6. LOGIC & PREDICTION
-- ==========================================
local function newTracker(pos, now)
    return {pos = pos, time = now, samples = {}, sampleIdx = 1, sampleCount = 0, velocity = Vector3.zero, accel = Vector3.zero, lastMoveTime = now, lastSigns = {x = 0, y = 0, z = 0}}
end

local function pushSample(t, vel, avgSpeed)
    if avgSpeed > 2 and vel.Magnitude > avgSpeed * 3.5 then return end
    t.samples[t.sampleIdx] = vel
    t.sampleIdx = (t.sampleIdx % 5) + 1
    t.sampleCount = math.min(t.sampleCount + 1, 5)
end

local function averageSamples(t)
    if t.sampleCount == 0 then return Vector3.zero end
    local sum = Vector3.zero
    for i = 1, t.sampleCount do
        if t.samples[i] then sum = sum + t.samples[i] end
    end
    return sum / t.sampleCount
end

local function sign(n)
    if n > 0.001 then return 1 elseif n < -0.001 then return -1 else return 0 end
end

local function updateTracker(character, part)
    local now = os.clock()
    local t = trackers[character]
    if not t then
        trackers[character] = newTracker(part.Position, now)
        return Vector3.zero, Vector3.zero, part.Position
    end
    local dt = now - t.time
    if dt < 0.001 then return t.velocity, t.accel, part.Position end

    local instantVel = (part.Position - t.pos) / dt
    pushSample(t, instantVel, t.velocity.Magnitude)

    local avgVel = averageSamples(t)
    local speed = avgVel.Magnitude
    local speedFactor = math.clamp(speed / 16, 0, 1)
    local velSmooth = 0.22 + (0.05 - 0.22) * speedFactor

    local horizontalSpeed = Vector3.new(avgVel.X, 0, avgVel.Z).Magnitude
    local effectiveVel = avgVel
    if horizontalSpeed < 1.5 then
        local sinceMove = now - t.lastMoveTime
        local decay = sinceMove < 0.12 and (1 - (sinceMove / 0.12)) or 0
        effectiveVel = Vector3.new(avgVel.X * decay, avgVel.Y, avgVel.Z * decay)
    else
        t.lastMoveTime = now
    end

    local velAlpha = 1 - math.exp(-dt / math.max(velSmooth, 0.001))
    local newVel = t.velocity:Lerp(effectiveVel, velAlpha)

    local sx, sy, sz = sign(newVel.X), sign(newVel.Y), sign(newVel.Z)
    local flipped = (t.lastSigns.x ~= 0 and sx ~= 0 and t.lastSigns.x ~= sx and math.abs(avgVel.X) > 6) or (t.lastSigns.y ~= 0 and sy ~= 0 and t.lastSigns.y ~= sy and math.abs(avgVel.Y) > 6) or (t.lastSigns.z ~= 0 and sz ~= 0 and t.lastSigns.z ~= sz and math.abs(avgVel.Z) > 6)
    if flipped then newVel = newVel:Lerp(avgVel, 0.75) end
    t.lastSigns = {x = sx, y = sy, z = sz}

    local instantAccel = (newVel - t.velocity) / dt
    local accAlpha = 1 - math.exp(-dt / 0.09)
    local newAccel = t.accel:Lerp(instantAccel, accAlpha)
    if newAccel.Magnitude > 400 then newAccel = newAccel.Unit * 400 end

    t.pos = part.Position
    t.time = now
    t.velocity = newVel
    t.accel = newAccel
    return newVel, newAccel, part.Position
end

local function GetPredictedPosition(character, part)
    if not Settings.PredictionEnabled then return part.Position end
    local vel, accel, currentPos = updateTracker(character, part)
    local distance = (Camera.CFrame.Position - currentPos).Magnitude
    local leadTime = (0.09 + (1/60)) * Settings.PredictionScale

    local hVel = Vector3.new(vel.X, 0, vel.Z)
    local vVel = Vector3.new(0, vel.Y, 0)
    local hAcc = Vector3.new(accel.X, 0, accel.Z)
    local vAcc = Vector3.new(0, accel.Y, 0)

    local hLead = hVel * leadTime + hAcc * 0.5 * leadTime * leadTime
    local vLead = vVel * leadTime + vAcc * 0.5 * leadTime * leadTime

    local maxH = 18 * Settings.PredictionScale
    local maxV = 8 * Settings.PredictionScale

    if hLead.Magnitude > maxH then hLead = hLead.Unit * maxH end
    if vLead.Magnitude > maxV then vLead = vLead.Unit * maxV end
    return currentPos + hLead + vLead
end

local function IsAliveAndStanding(character)
    if not character or not character.Parent then return false end
    local humanoid = character:FindFirstChildOfClass("Humanoid")
    if not humanoid then return false end
    
    if humanoid.Health <= 0 or humanoid:GetState() == Enum.HumanoidStateType.Dead then return false end
    if humanoid.PlatformStand then return false end
    
    if Settings.DeadCheckEnabled then
        if character:GetAttribute("Ragdoll") == true or character:GetAttribute("Dead") == true then return false end
        if humanoid:GetAttribute("Ragdoll") == true or humanoid:GetAttribute("Dead") == true then return false end
        local head = character:FindFirstChild("Head")
        local hrp = character:FindFirstChild("HumanoidRootPart")
        if head and hrp and (hrp.Position.Y - head.Position.Y) > 1.5 then return false end
    end
    return true
end

local function HasLineOfSight(myChar, targetChar, targetPart)
    if not Settings.WallCheckEnabled then return true end
    local params = RaycastParams.new()
    params.FilterType = Enum.RaycastFilterType.Exclude
    params.FilterDescendantsInstances = {myChar, targetChar}
    local ray = workspace:Raycast(Camera.CFrame.Position, targetPart.Position - Camera.CFrame.Position, params)
    return ray == nil
end

local function GetAimAngle(pos)
    local screenPoint, onScreen = Camera:WorldToViewportPoint(pos)
    if not onScreen then return 180 end
    local screenCenter = Vector2.new(Camera.ViewportSize.X / 2, Camera.ViewportSize.Y / 2)
    local screenPos = Vector2.new(screenPoint.X, screenPoint.Y)
    local distanceFromCenter = (screenPos - screenCenter).Magnitude
    return (distanceFromCenter / Camera.ViewportSize.Y) * 90
end

-- ==========================================
-- CHANGED: Look-based target selection
-- 1) Raycast from camera center (crosshair). If a player is under it, lock them.
-- 2) Otherwise pick whoever is closest to the crosshair in ANGLE only.
--    Distance is only a hard cutoff, never a scoring factor.
-- ==========================================
local function FindLookedAtTarget()
    local myChar = LocalPlayer.Character
    if not myChar or not myChar:FindFirstChild("Head") then return nil end

    -- 1) Direct crosshair raycast
    local centerParams = RaycastParams.new()
    centerParams.FilterType = Enum.RaycastFilterType.Exclude
    centerParams.FilterDescendantsInstances = {myChar}
    local centerRay = workspace:Raycast(Camera.CFrame.Position, Camera.CFrame.LookVector * 300, centerParams)
    if centerRay then
        local hitModel = centerRay.Instance:FindFirstAncestorOfClass("Model")
        if hitModel and hitModel ~= myChar then
            local hitPlayer = Players:GetPlayerFromCharacter(hitModel)
            if hitPlayer and hitPlayer ~= LocalPlayer and IsAliveAndStanding(hitModel) then
                local part = hitModel:FindFirstChild(Settings.TargetPart)
                if part and HasLineOfSight(myChar, hitModel, part) then
                    return hitModel
                end
            end
        end
    end

    -- 2) Fall back: smallest angle from crosshair (distance not scored)
    local bestTarget, bestAngle = nil, math.huge
    for _, player in ipairs(Players:GetPlayers()) do
        if player ~= LocalPlayer and player.Character then
            local char = player.Character
            if IsAliveAndStanding(char) then
                local part = char:FindFirstChild(Settings.TargetPart)
                if part then
                    local dist = (Camera.CFrame.Position - part.Position).Magnitude
                    local angle = GetAimAngle(part.Position)
                    if dist <= 150 and angle <= 45 then
                        if HasLineOfSight(myChar, char, part) then
                            if angle < bestAngle then
                                bestAngle = angle
                                bestTarget = char
                            end
                        end
                    end
                end
            end
        end
    end
    return bestTarget
end

local function StopLockOn()
    isLockedOn = false
    currentTarget = nil
    lastSeenTime = 0
    if cameraConnection then
        cameraConnection:Disconnect()
        cameraConnection = nil
    end
end

local function StartLockOn()
    if isLockedOn or not Settings.LockOnEnabled then return end
    local target = FindLookedAtTarget()
    if not target then return end

    currentTarget = target
    isLockedOn = true
    lastSeenTime = os.clock()
    trackers[target] = nil

    if cameraConnection then cameraConnection:Disconnect() end

    cameraConnection = RunService.RenderStepped:Connect(function(dt)
        if not isLockedOn or not Settings.LockOnEnabled then StopLockOn(); return end
        if not currentTarget or not currentTarget.Parent then StopLockOn(); return end
        if not IsAliveAndStanding(currentTarget) then StopLockOn(); return end

        local part = currentTarget:FindFirstChild(Settings.TargetPart)
        if not part then StopLockOn(); return end

        local myChar = LocalPlayer.Character
        if not myChar or not HasLineOfSight(myChar, currentTarget, part) then StopLockOn(); return end

        local dist = (Camera.CFrame.Position - part.Position).Magnitude
        if dist <= 150 then
            lastSeenTime = os.clock()
        elseif os.clock() - lastSeenTime > 0.45 then
            StopLockOn(); return
        end

        local predictedPos = GetPredictedPosition(currentTarget, part)
        local targetPos = predictedPos + Vector3.new(0, 1.5, 0)
        local cameraPos = Camera.CFrame.Position

        local goalCFrame = CFrame.new(cameraPos, targetPos)
        local t = math.clamp(Settings.Smoothness * (dt * 60), 0, 1)
        local newCFrame = Camera.CFrame:Lerp(goalCFrame, t)

        if Settings.AimAssistEnabled then
            local assistLook = newCFrame.LookVector:Lerp((targetPos - newCFrame.Position).Unit, 0.65)
            Camera.CFrame = CFrame.lookAt(newCFrame.Position, newCFrame.Position + assistLook)
        else
            Camera.CFrame = newCFrame
        end
    end)
end

-- ==========================================
-- 7. INPUT HANDLING (Bulletproof)
-- ==========================================
local wasL2Pressed = false
local wasKeyPressed = false

-- Controller L2 Hold
UserInputService.InputChanged:Connect(function(input, gameProcessed)
    if input.KeyCode == Enum.KeyCode.ButtonL2 then
        local isPressed = input.Position.Z > 0.15
        if isPressed and not wasL2Pressed then
            wasL2Pressed = true
            StartLockOn()
        elseif not isPressed and wasL2Pressed then
            wasL2Pressed = false
            StopLockOn()
        end
    end
end)

-- Keybind Listener for setting the key
UserInputService.InputBegan:Connect(function(input, gameProcessed)
    if Settings.IsListeningForKey then
        -- Don't let them bind the UI clicking itself
        if input.UserInputType == Enum.UserInputType.MouseButton1 or input.UserInputType == Enum.UserInputType.Touch then return end
        
        -- Format the name nicely
        local keyName = input.KeyCode.Name
        if input.UserInputType == Enum.UserInputType.MouseButton2 then keyName = "RightClick"
        elseif input.UserInputType == Enum.UserInputType.MouseButton3 then keyName = "MiddleClick"
        elseif input.UserInputType == Enum.UserInputType.MouseButton1 then keyName = "LeftClick" end
        
        Settings.Keybind = keyName
        Settings.IsListeningForKey = false
        
        if keybindBtn then
            keybindBtn.Text = keyName
            keybindBtn.TextColor3 = Theme.Accent1
        end
    end
end)

-- Bulletproof Input Loop
RunService.RenderStepped:Connect(function()
    -- 1. Controller Check
    local gamepadState = UserInputService:GetGamepadState(Enum.UserInputType.Gamepad1)
    if gamepadState then
        local l2Pressed = false
        for _, input in ipairs(gamepadState) do
            if input.KeyCode == Enum.KeyCode.ButtonL2 and input.Position.Z > 0.15 then
                l2Pressed = true
                break
            end
        end
        if l2Pressed and not wasL2Pressed then
            wasL2Pressed = true
            StartLockOn()
        elseif not l2Pressed and wasL2Pressed then
            wasL2Pressed = false
            StopLockOn()
        end
    end

    -- 2. Keyboard/Mouse Check (Only if not setting the keybind)
    if not Settings.IsListeningForKey then
        local isPressed = false
        if Settings.Keybind == "RightClick" then
            isPressed = UserInputService:IsMouseButtonPressed(Enum.UserInputType.MouseButton2)
        elseif Settings.Keybind == "LeftClick" then
            isPressed = UserInputService:IsMouseButtonPressed(Enum.UserInputType.MouseButton1)
        elseif Settings.Keybind == "MiddleClick" then
            isPressed = UserInputService:IsMouseButtonPressed(Enum.UserInputType.MouseButton3)
        else
            isPressed = UserInputService:IsKeyDown(Enum.KeyCode[Settings.Keybind])
        end

        if isPressed and not wasKeyPressed then
            wasKeyPressed = true
            StartLockOn()
        elseif not isPressed and wasKeyPressed then
            wasKeyPressed = false
            StopLockOn()
        end
    end
end)

LocalPlayer.CharacterAdded:Connect(StopLockOn)
