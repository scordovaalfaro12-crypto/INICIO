--[[
	Basaltor (Aetherials) - configuracion despues de importar el modelo en Roblox Studio.

	Como usarlo:
	  1. Importa Basaltor_Roblox_Rig.fbx con File > Import 3D.
	  2. Pon este Script dentro del Model importado (debe quedar como hijo directo del Model).
	  3. Dale Play: el magma pasa a Neon (brilla), se agrega luz y brasas sobre el lomo.
	  4. (Opcional) Publica la animacion Basaltor_Anim_Reposo.fbx desde el Animation Editor
	     y pega su ID en ANIMACION_REPOSO para que respire en bucle.
]]

local model = script.Parent

-- Ajustes -------------------------------------------------------------------
local COLOR_MAGMA = Color3.fromRGB(255, 122, 51)
local COLOR_MAGMA_CALIENTE = Color3.fromRGB(255, 176, 74)
local ANCLAR = true -- true: el modelo queda fijo (exhibicion). false si lo mueves con fisica o un script.
local ANIMACION_REPOSO = "" -- ej: "rbxassetid://1234567890"
local CON_BRASAS = true
local CON_LUCES = true

-- Materiales ------------------------------------------------------------------
local function terminaEn(texto, sufijo)
	return string.sub(texto, -#sufijo) == sufijo
end

for _, parte in ipairs(model:GetDescendants()) do
	if parte:IsA("MeshPart") then
		parte.Anchored = ANCLAR
		parte.CanCollide = false
		if terminaEn(parte.Name, "_Magma") then
			-- las grietas de lava: el material Neon es lo que las hace brillar
			parte.Material = Enum.Material.Neon
			parte.Color = COLOR_MAGMA
			parte.CastShadow = false
		else
			parte.Material = Enum.Material.SmoothPlastic
		end
	end
end

-- La pieza del torso es la mas grande: sirve como colision simple
local torso = model:FindFirstChild("Torso", true)
if torso and torso:IsA("MeshPart") then
	torso.CanCollide = true
	if not model.PrimaryPart then
		model.PrimaryPart = torso
	end
end

-- Luces -----------------------------------------------------------------------
local function agregarLuz(nombreParte, brillo, rango, color)
	local parte = model:FindFirstChild(nombreParte, true)
	if parte and parte:IsA("BasePart") then
		local luz = Instance.new("PointLight")
		luz.Color = color
		luz.Brightness = brillo
		luz.Range = rango
		luz.Shadows = false
		luz.Parent = parte
	end
end

if CON_LUCES then
	agregarLuz("Torso_Magma", 2.5, 16, COLOR_MAGMA)   -- grieta del lomo
	agregarLuz("Mandibula_Magma", 1.5, 8, COLOR_MAGMA_CALIENTE) -- lengua de lava
	agregarLuz("Cola_Mazo_Magma", 1.2, 8, COLOR_MAGMA) -- mazo de la cola
end

-- Brasas que suben de la grieta del lomo -------------------------------------
if CON_BRASAS then
	local grieta = model:FindFirstChild("Torso_Magma", true)
	if grieta and grieta:IsA("BasePart") then
		local brasas = Instance.new("ParticleEmitter")
		brasas.Name = "Brasas"
		brasas.Color = ColorSequence.new(COLOR_MAGMA_CALIENTE, COLOR_MAGMA)
		brasas.LightEmission = 1
		brasas.LightInfluence = 0
		brasas.Size = NumberSequence.new({
			NumberSequenceKeypoint.new(0, 0.18),
			NumberSequenceKeypoint.new(0.7, 0.12),
			NumberSequenceKeypoint.new(1, 0),
		})
		brasas.Transparency = NumberSequence.new({
			NumberSequenceKeypoint.new(0, 0),
			NumberSequenceKeypoint.new(0.8, 0.3),
			NumberSequenceKeypoint.new(1, 1),
		})
		brasas.Lifetime = NumberRange.new(1.2, 2.4)
		brasas.Rate = 10
		brasas.Speed = NumberRange.new(1.5, 3)
		brasas.SpreadAngle = Vector2.new(20, 20)
		brasas.Acceleration = Vector3.new(0, 1.5, 0)
		brasas.Drag = 1
		brasas.RotSpeed = NumberRange.new(-90, 90)
		brasas.EmissionDirection = Enum.NormalId.Top
		brasas.Parent = grieta

		local humo = Instance.new("ParticleEmitter")
		humo.Name = "Humo"
		humo.Color = ColorSequence.new(Color3.fromRGB(70, 70, 80))
		humo.LightInfluence = 1
		humo.Size = NumberSequence.new({
			NumberSequenceKeypoint.new(0, 0.6),
			NumberSequenceKeypoint.new(1, 2.4),
		})
		humo.Transparency = NumberSequence.new({
			NumberSequenceKeypoint.new(0, 0.85),
			NumberSequenceKeypoint.new(1, 1),
		})
		humo.Lifetime = NumberRange.new(2, 3.5)
		humo.Rate = 4
		humo.Speed = NumberRange.new(0.8, 1.6)
		humo.SpreadAngle = Vector2.new(15, 15)
		humo.EmissionDirection = Enum.NormalId.Top
		humo.Parent = grieta
	end
end

-- Animacion de reposo -----------------------------------------------------------
local controlador = model:FindFirstChildOfClass("AnimationController") or model:FindFirstChildOfClass("Humanoid")
if not controlador then
	controlador = Instance.new("AnimationController")
	controlador.Parent = model
end
local animator = controlador:FindFirstChildOfClass("Animator")
if not animator then
	animator = Instance.new("Animator")
	animator.Parent = controlador
end

if ANIMACION_REPOSO ~= "" then
	local anim = Instance.new("Animation")
	anim.AnimationId = ANIMACION_REPOSO
	local pista = animator:LoadAnimation(anim)
	pista.Looped = true
	pista:Play()
end
