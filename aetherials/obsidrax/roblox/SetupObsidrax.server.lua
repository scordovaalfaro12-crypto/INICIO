--[[
	Obsidrax (Aetherials) - configuracion despues de importar el modelo en Roblox Studio.

	Como usarlo:
	  1. Importa Obsidrax_Roblox_Rig.fbx con File > Import 3D.
	  2. Pon este Script dentro del Model importado (debe quedar como hijo directo del Model).
	  3. Dale Play: el magma pasa a Neon (brilla), se agrega luz al corazon, la boca, el lomo y el orbe,
	     y brasas sobre el lomo.
	  4. (Opcional) Publica la animacion Obsidrax_Anim_Reposo.fbx desde el Animation Editor
	     y pega su ID en ANIMACION_REPOSO para que respire en bucle.
	  5. (Opcional) Publica Obsidrax_Anim_Caminar.fbx, pega su ID en ANIMACION_CAMINAR y pon
	     PATRULLAR = true: camina ida y vuelta a la velocidad justa para que no patine.
]]

local model = script.Parent

-- Ajustes -------------------------------------------------------------------
local COLOR_MAGMA = Color3.fromRGB(255, 122, 51)
local COLOR_MAGMA_CALIENTE = Color3.fromRGB(255, 176, 74)
local ANCLAR = true -- true: el modelo queda fijo (exhibicion). false si lo mueves con fisica o un script.
local ANIMACION_REPOSO = "" -- ej: "rbxassetid://1234567890"
local ANIMACION_CAMINAR = "" -- ID de Obsidrax_Anim_Caminar.fbx publicada
local PATRULLAR = false -- true: camina ida y vuelta (usa ANIMACION_CAMINAR; deja ANCLAR = true)
local DISTANCIA_PATRULLA = 40 -- studs que recorre antes de darse vuelta
local INVERTIR_DIRECCION = false -- true si al importarlo quedo mirando hacia atras
local CON_BRASAS = true
local CON_LUCES = true

-- Velocidad a la que los pies no patinan: 0,735 m/s con el modelo original de 7,31 m de largo.
-- El script la escala segun el tamano con que importaste el modelo.
local VELOCIDAD_BASE = 0.735
local LARGO_BASE = 7.31

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
	agregarLuz("Torso_Magma", 2.5, 18, COLOR_MAGMA)   -- rio de magma del lomo y grietas
	agregarLuz("Torso__Nucleo_Magma", 3, 12, COLOR_MAGMA_CALIENTE) -- corazon de magma del pecho
	agregarLuz("Mandibula_Magma", 1.5, 9, COLOR_MAGMA_CALIENTE) -- lengua de lava
	agregarLuz("Cola_Mazo_Magma", 2, 10, COLOR_MAGMA) -- orbe de magma de la cola
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

local function cargar(id, prioridad)
	local anim = Instance.new("Animation")
	anim.AnimationId = id
	local pista = animator:LoadAnimation(anim)
	pista.Looped = true
	pista.Priority = prioridad
	return pista
end

-- Caminata de patrulla ---------------------------------------------------------
if PATRULLAR and ANIMACION_CAMINAR ~= "" then
	local RunService = game:GetService("RunService")
	local caminar = cargar(ANIMACION_CAMINAR, Enum.AnimationPriority.Movement)
	caminar:Play()

	local tamano = model:GetExtentsSize()
	local velocidad = VELOCIDAD_BASE * math.max(tamano.X, tamano.Z) / LARGO_BASE
	local sentido = INVERTIR_DIRECCION and -1 or 1
	local DURACION_GIRO = 1.5 -- segundos para darse vuelta (sigue caminando mientras gira)
	local recorrido = 0
	local giroRestante = 0

	RunService.Heartbeat:Connect(function(dt)
		local pivote = model:GetPivot()
		if giroRestante > 0 then
			local paso = math.min(dt, giroRestante)
			giroRestante -= paso
			model:PivotTo(pivote * CFrame.Angles(0, math.pi * paso / DURACION_GIRO, 0))
			return
		end
		local avance = velocidad * dt
		model:PivotTo(pivote + pivote.LookVector * avance * sentido)
		recorrido += avance
		if recorrido >= DISTANCIA_PATRULLA then
			recorrido = 0
			giroRestante = DURACION_GIRO
		end
	end)
elseif ANIMACION_REPOSO ~= "" then
	cargar(ANIMACION_REPOSO, Enum.AnimationPriority.Idle):Play()
end
