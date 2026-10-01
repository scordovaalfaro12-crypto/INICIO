--[[
	Zephalcon (Aetherials, segunda forma de Zephyrian) - configuracion despues de importar en Roblox Studio.

	Como usarlo:
	  1. Importa Zephalcon_Roblox_Rig.fbx con File > Import 3D (rig personalizado, no humanoide).
	     La pose de reposo del archivo tiene las alas abiertas; las animaciones las pliegan.
	  2. Pon este Script dentro del Model importado (hijo directo del Model).
	  3. Publica las animaciones desde el Animation Editor (Import > From FBX Animation):
	       Zephalcon_Anim_Reposo.fbx   -> ANIMACION_REPOSO  (alas plegadas, mira a los lados)
	       Zephalcon_Anim_Caminar.fbx  -> ANIMACION_CAMINAR
	       Zephalcon_Anim_Aletear.fbx  -> ANIMACION_ALETEAR (vuelo)
	       Zephalcon_Anim_Grito.fbx    -> ANIMACION_GRITO   (abre las alas y grita)
	  4. Elige un MODO: "reposo", "patrulla" (camina ida y vuelta) o "vuelo" (vuela en circulos).
]]

local model = script.Parent
local RunService = game:GetService("RunService")

-- Ajustes -------------------------------------------------------------------
local MODO = "reposo" -- "reposo", "patrulla" o "vuelo"
local ANIMACION_REPOSO = "" -- ej: "rbxassetid://1234567890"
local ANIMACION_CAMINAR = ""
local ANIMACION_ALETEAR = ""
local ANIMACION_GRITO = ""
local GRITAR_CADA = 12 -- segundos entre gritos en modo reposo (0 = nunca)
local DISTANCIA_PATRULLA = 25 -- studs que camina antes de darse vuelta
local RADIO_VUELO = 30 -- studs del circulo de vuelo
local ALTURA_VUELO = 20 -- studs sobre su posicion inicial
local INVERTIR_DIRECCION = false -- true si al importarlo quedo mirando hacia atras

-- Velocidades sin que los pies patinen, medidas con el modelo original (4,55 m de envergadura).
-- El script las escala segun el tamano con que lo importaste.
local ENVERGADURA_BASE = 4.55
local VELOCIDAD_CAMINAR_BASE = 0.571
local VELOCIDAD_VUELO_BASE = 3.3

-- Partes ----------------------------------------------------------------------
for _, parte in ipairs(model:GetDescendants()) do
	if parte:IsA("MeshPart") then
		parte.Anchored = true
		parte.CanCollide = false
		parte.Material = Enum.Material.SmoothPlastic
	end
end
local torso = model:FindFirstChild("Torso", true)
if torso and torso:IsA("MeshPart") then
	torso.CanCollide = true
	if not model.PrimaryPart then
		model.PrimaryPart = torso
	end
end

-- Animaciones -------------------------------------------------------------------
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

local function cargar(id, prioridad, bucle)
	if id == "" then
		return nil
	end
	local anim = Instance.new("Animation")
	anim.AnimationId = id
	local pista = animator:LoadAnimation(anim)
	pista.Looped = bucle
	pista.Priority = prioridad
	return pista
end

local reposo = cargar(ANIMACION_REPOSO, Enum.AnimationPriority.Idle, true)
local caminar = cargar(ANIMACION_CAMINAR, Enum.AnimationPriority.Movement, true)
local aletear = cargar(ANIMACION_ALETEAR, Enum.AnimationPriority.Movement, true)
local grito = cargar(ANIMACION_GRITO, Enum.AnimationPriority.Action, false)

local tamano = model:GetExtentsSize()
local escala = math.max(tamano.X, tamano.Z) / ENVERGADURA_BASE
local sentido = INVERTIR_DIRECCION and -1 or 1

if MODO == "patrulla" and caminar then
	caminar:Play()
	local velocidad = VELOCIDAD_CAMINAR_BASE * escala
	local DURACION_GIRO = 1.2
	local recorrido, giroRestante = 0, 0
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
elseif MODO == "vuelo" and aletear then
	aletear:Play()
	local velocidad = VELOCIDAD_VUELO_BASE * escala
	local inicio = model:GetPivot()
	local centro = inicio.Position + Vector3.new(0, ALTURA_VUELO, 0)
	local angulo = 0
	RunService.Heartbeat:Connect(function(dt)
		angulo += (velocidad / RADIO_VUELO) * dt
		local pos = centro + Vector3.new(math.cos(angulo) * RADIO_VUELO, math.sin(angulo * 2) * 2, math.sin(angulo) * RADIO_VUELO)
		local adelante = Vector3.new(-math.sin(angulo), 0, math.cos(angulo)) * sentido
		model:PivotTo(CFrame.lookAt(pos, pos + adelante))
	end)
else
	if reposo then
		reposo:Play()
	end
	if grito and GRITAR_CADA > 0 then
		task.spawn(function()
			while model.Parent do
				task.wait(GRITAR_CADA)
				grito:Play()
			end
		end)
	end
end
