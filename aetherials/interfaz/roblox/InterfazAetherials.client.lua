--[[
	Aetherials · interfaz minimalista
	Pantalla de inicio + barra de menú, todo con sprites de un solo atlas.

	Instalación
	  1. Sube sprites/Aetherials_UI_Atlas.png (Creator Hub o Asset Manager) y pega su ID en ATLAS.
	  2. Pon este archivo como LocalScript en StarterPlayer > StarterPlayerScripts.
	  3. Escribe en GUIS_A_OCULTAR los nombres de tu pantalla de inicio y tu barra actuales.
	  4. En BOTONES, cambia `panel` por el nombre de cada panel que ya tienes (ScreenGui o Frame).

	Otros scripts pueden escuchar:
	  PlayerGui.AetherialsMenu.Jugar.Event        -> se pulsó JUGAR
	  PlayerGui.AetherialsMenu.Boton.Event(id, abierto) -> se pulsó un botón del menú
]]

local Players = game:GetService("Players")
local UserInputService = game:GetService("UserInputService")
local TweenService = game:GetService("TweenService")
local Lighting = game:GetService("Lighting")
local ReplicatedStorage = game:GetService("ReplicatedStorage")
local GuiService = game:GetService("GuiService")
local StarterGui = game:GetService("StarterGui")

local jugador = Players.LocalPlayer
local playerGui = jugador:WaitForChild("PlayerGui")
if not workspace.CurrentCamera then
	workspace:GetPropertyChangedSignal("CurrentCamera"):Wait()
end

---------------------------------------------------------------- configuración

local ATLAS = "rbxassetid://0" -- ID de Aetherials_UI_Atlas.png

local MOSTRAR_INICIO = true
local SUBTITULO = "Estabiliza el Nexo · Derrota a los cinco Guardianes"
local ZONA = "Cavernas de Obsidiana" -- si el jugador tiene el atributo "Zona", se usa ese
local REMOTO_JUGAR = nil -- nombre de un RemoteEvent en ReplicatedStorage para avisar al servidor (opcional)

local POSICION_MENU = "abajo" -- "abajo" o "izquierda"

-- ScreenGuis viejos que este script reemplaza (se desactivan al empezar)
local GUIS_A_OCULTAR = {
	-- "PantallaInicio",
	-- "BarraMenu",
}

-- `panel`: nombre del ScreenGui o Frame que abre cada botón (por defecto, el id)
local BOTONES = {
	{ id = "Equipo",   tecla = Enum.KeyCode.C, icono = "equipo",   color = Color3.fromRGB(90, 160, 240) },
	{ id = "Mapa",     tecla = Enum.KeyCode.M, icono = "mapa",     color = Color3.fromRGB(64, 201, 180) },
	{ id = "Tienda",   tecla = Enum.KeyCode.Y, icono = "tienda",   color = Color3.fromRGB(76, 196, 106) },
	{ id = "Caja",     tecla = Enum.KeyCode.K, icono = "caja",     color = Color3.fromRGB(155, 123, 242) },
	{ id = "PvP",      tecla = Enum.KeyCode.P, icono = "pvp",      color = Color3.fromRGB(232, 85, 85) },
	{ id = "Misiones", tecla = Enum.KeyCode.J, icono = "misiones", color = Color3.fromRGB(232, 185, 58) },
	{ id = "Registro", tecla = Enum.KeyCode.R, icono = "registro", color = Color3.fromRGB(238, 106, 168) },
}

local COLOR = {
	fondo = Color3.fromRGB(13, 16, 21),
	texto = Color3.fromRGB(236, 239, 244),
	oro = Color3.fromRGB(242, 194, 48),
	oroClaro = Color3.fromRGB(255, 214, 102),
	tinta = Color3.fromRGB(17, 19, 24),
}

-- posiciones en el atlas (x, y, ancho, alto)
local SPRITES = {
	equipo = { 0, 0, 128, 128 },
	mapa = { 128, 0, 128, 128 },
	tienda = { 256, 0, 128, 128 },
	caja = { 384, 0, 128, 128 },
	pvp = { 512, 0, 128, 128 },
	misiones = { 640, 0, 128, 128 },
	registro = { 768, 0, 128, 128 },
	ubicacion = { 896, 0, 128, 128 },
	cerrar = { 0, 128, 128, 128 },
	nexo_anillo = { 0, 256, 256, 256 },
	nexo_cristal = { 256, 256, 256, 256 },
	nexo = { 512, 256, 256, 256 },
	logo = { 28, 516, 968, 83 },
}

---------------------------------------------------------------- utilidades

local MONTSERRAT = "rbxasset://fonts/families/Montserrat.json"
local function fuente(peso)
	return Font.new(MONTSERRAT, peso or Enum.FontWeight.Medium)
end

local function crear(clase, props, hijos)
	local obj = Instance.new(clase)
	for k, v in pairs(props or {}) do
		if k ~= "Parent" then
			obj[k] = v
		end
	end
	for _, hijo in ipairs(hijos or {}) do
		hijo.Parent = obj
	end
	if props and props.Parent then
		obj.Parent = props.Parent
	end
	return obj
end

local function sprite(nombre, props)
	local r = SPRITES[nombre]
	local img = crear("ImageLabel", props)
	img.BackgroundTransparency = 1
	img.Image = ATLAS
	img.ImageRectOffset = Vector2.new(r[1], r[2])
	img.ImageRectSize = Vector2.new(r[3], r[4])
	img.ScaleType = Enum.ScaleType.Fit
	return img
end

local function esquinas(radio)
	return crear("UICorner", { CornerRadius = radio })
end

local function tween(obj, segundos, props, estilo)
	local tw = TweenService:Create(
		obj,
		TweenInfo.new(segundos, estilo or Enum.EasingStyle.Quint, Enum.EasingDirection.Out),
		props
	)
	tw:Play()
	return tw
end

local ACENTOS = { ["á"] = "Á", ["é"] = "É", ["í"] = "Í", ["ó"] = "Ó", ["ú"] = "Ú", ["ñ"] = "Ñ", ["ü"] = "Ü" }
local function mayusculas(texto)
	texto = string.upper(texto)
	for minus, mayus in pairs(ACENTOS) do
		texto = string.gsub(texto, minus, mayus)
	end
	return texto
end

-- escala según el alto de la pantalla (900 px = 1)
local function escalaPantalla(minimo, maximo)
	local camara = workspace.CurrentCamera
	local alto = camara and camara.ViewportSize.Y or 900
	return math.clamp(alto / 900, minimo, maximo)
end

local soloTactil = UserInputService.TouchEnabled and not UserInputService.KeyboardEnabled

for _, nombre in ipairs(GUIS_A_OCULTAR) do
	local vieja = playerGui:FindFirstChild(nombre)
	if vieja and vieja:IsA("ScreenGui") then
		vieja.Enabled = false
	end
end

---------------------------------------------------------------- barra de menú

local menuGui = crear("ScreenGui", {
	Name = "AetherialsMenu",
	ResetOnSpawn = false,
	ZIndexBehavior = Enum.ZIndexBehavior.Sibling,
	DisplayOrder = 5,
	Enabled = false,
	Parent = playerGui,
})
local eventoBoton = crear("BindableEvent", { Name = "Boton", Parent = menuGui })
local eventoJugar = crear("BindableEvent", { Name = "Jugar", Parent = menuGui })

local vertical = POSICION_MENU == "izquierda"
local SLOT, ESPACIO, MARGEN = 56, 4, 8
local largo = MARGEN * 2 + #BOTONES * SLOT + (#BOTONES - 1) * ESPACIO
local grosor = SLOT + MARGEN * 2

local dock = crear("Frame", {
	Name = "Dock",
	AnchorPoint = vertical and Vector2.new(0, 0.5) or Vector2.new(0.5, 1),
	Position = vertical and UDim2.new(0, 16, 0.5, 0) or UDim2.new(0.5, 0, 1, -16),
	Size = vertical and UDim2.fromOffset(grosor, largo) or UDim2.fromOffset(largo, grosor),
	BackgroundColor3 = COLOR.fondo,
	BackgroundTransparency = 0.18,
	Parent = menuGui,
}, {
	esquinas(UDim.new(0, 20)),
	crear("UIStroke", { Color = Color3.new(1, 1, 1), Transparency = 0.9, Thickness = 1 }),
	crear("UIPadding", {
		PaddingTop = UDim.new(0, MARGEN),
		PaddingBottom = UDim.new(0, MARGEN),
		PaddingLeft = UDim.new(0, MARGEN),
		PaddingRight = UDim.new(0, MARGEN),
	}),
	crear("UIListLayout", {
		FillDirection = vertical and Enum.FillDirection.Vertical or Enum.FillDirection.Horizontal,
		HorizontalAlignment = Enum.HorizontalAlignment.Center,
		VerticalAlignment = Enum.VerticalAlignment.Center,
		Padding = UDim.new(0, ESPACIO),
		SortOrder = Enum.SortOrder.LayoutOrder,
	}),
})
local escalaDock = crear("UIScale", { Parent = dock })

-- tooltip: nombre + tecla, aparece sobre el botón
local tip = crear("Frame", {
	Name = "Tooltip",
	AutomaticSize = Enum.AutomaticSize.X,
	Size = UDim2.fromOffset(0, 30),
	BackgroundColor3 = COLOR.fondo,
	BackgroundTransparency = 0.1,
	Visible = false,
	ZIndex = 10,
	Parent = menuGui,
}, {
	esquinas(UDim.new(0, 8)),
	crear("UIPadding", { PaddingLeft = UDim.new(0, 10), PaddingRight = UDim.new(0, 6) }),
	crear("UIListLayout", {
		FillDirection = Enum.FillDirection.Horizontal,
		VerticalAlignment = Enum.VerticalAlignment.Center,
		Padding = UDim.new(0, 8),
		SortOrder = Enum.SortOrder.LayoutOrder,
	}),
})
local escalaTip = crear("UIScale", { Parent = tip })
local tipNombre = crear("TextLabel", {
	AutomaticSize = Enum.AutomaticSize.X,
	Size = UDim2.fromOffset(0, 30),
	BackgroundTransparency = 1,
	FontFace = fuente(Enum.FontWeight.Medium),
	TextSize = 14,
	TextColor3 = COLOR.texto,
	LayoutOrder = 1,
	ZIndex = 11,
	Parent = tip,
})
local tipTecla = crear("TextLabel", {
	Size = UDim2.fromOffset(20, 20),
	BackgroundColor3 = Color3.new(1, 1, 1),
	BackgroundTransparency = 0.88,
	FontFace = fuente(Enum.FontWeight.SemiBold),
	TextSize = 11,
	TextColor3 = COLOR.texto,
	TextTransparency = 0.2,
	Visible = UserInputService.KeyboardEnabled,
	LayoutOrder = 2,
	ZIndex = 11,
	Parent = tip,
}, { esquinas(UDim.new(0, 5)) })

local estado = {} -- id -> { boton, icono, punto, escala, hover, activo }
local porId, porTecla = {}, {}
local activo = nil -- id del panel abierto

local function mostrarTip(b)
	local slot = estado[b.id].boton
	tipNombre.Text = b.id
	tipTecla.Text = b.tecla.Name
	local pos, tam = slot.AbsolutePosition, slot.AbsoluteSize
	if vertical then
		tip.AnchorPoint = Vector2.new(0, 0.5)
		tip.Position = UDim2.fromOffset(pos.X + tam.X + 18 * escalaDock.Scale, pos.Y + tam.Y / 2)
	else
		tip.AnchorPoint = Vector2.new(0.5, 1)
		tip.Position = UDim2.fromOffset(pos.X + tam.X / 2, pos.Y - 18 * escalaDock.Scale)
	end
	tip.Visible = true
end

local function ocultarTip(b)
	if tipNombre.Text == b.id then
		tip.Visible = false
	end
end

local function pintar(b)
	local e = estado[b.id]
	local encendido = e.hover or e.activo
	tween(e.icono, 0.18, { ImageColor3 = encendido and b.color or COLOR.texto })
	tween(e.boton, 0.18, { BackgroundTransparency = e.hover and 0.9 or 1 })
	tween(e.punto, 0.18, { BackgroundTransparency = e.activo and 0 or 1 })
end

local function marcarActivo(id)
	activo = id
	for _, b in ipairs(BOTONES) do
		estado[b.id].activo = (b.id == id)
		pintar(b)
	end
end

local function buscarPanel(b)
	local nombre = b.panel or b.id
	local panel = playerGui:FindFirstChild(nombre)
	if panel and panel ~= menuGui then
		return panel
	end
	for _, d in ipairs(playerGui:GetDescendants()) do
		if d.Name == nombre and d:IsA("GuiObject") and not d:IsDescendantOf(menuGui) then
			return d
		end
	end
	return nil
end

local function panelAbierto(panel)
	if panel:IsA("ScreenGui") then
		return panel.Enabled
	end
	return panel:IsA("GuiObject") and panel.Visible
end

local function ponerPanel(panel, abierto)
	if panel:IsA("ScreenGui") then
		panel.Enabled = abierto
	elseif panel:IsA("GuiObject") then
		panel.Visible = abierto
	end
end

-- si el panel se cierra desde su propio botón X, la barra se entera
local vigilados = {}
local function vigilar(b, panel)
	if vigilados[panel] then
		return
	end
	vigilados[panel] = true
	local propiedad = panel:IsA("ScreenGui") and "Enabled" or "Visible"
	panel:GetPropertyChangedSignal(propiedad):Connect(function()
		local abierto = panelAbierto(panel)
		if not abierto and activo == b.id then
			marcarActivo(nil)
		elseif abierto and activo ~= b.id then
			marcarActivo(b.id)
		end
	end)
end

local function pulsar(b)
	local e = estado[b.id]
	tween(e.escala, 0.08, { Scale = 0.9 }).Completed:Once(function()
		tween(e.escala, 0.3, { Scale = 1 }, Enum.EasingStyle.Back)
	end)
	local abrir = not e.activo
	if activo and activo ~= b.id then
		local anterior = buscarPanel(porId[activo])
		if anterior then
			ponerPanel(anterior, false)
		end
	end
	local panel = buscarPanel(b)
	if panel then
		vigilar(b, panel)
		ponerPanel(panel, abrir)
	end
	marcarActivo(abrir and b.id or nil)
	eventoBoton:Fire(b.id, abrir)
end

for i, b in ipairs(BOTONES) do
	porId[b.id] = b
	porTecla[b.tecla] = b
	local boton = crear("TextButton", {
		Name = "Boton_" .. b.id,
		Size = UDim2.fromOffset(SLOT, SLOT),
		BackgroundColor3 = Color3.new(1, 1, 1),
		BackgroundTransparency = 1,
		AutoButtonColor = false,
		Text = "",
		LayoutOrder = i,
		Parent = dock,
	}, { esquinas(UDim.new(0, 14)) })
	local e = {
		boton = boton,
		escala = crear("UIScale", { Parent = boton }),
		icono = sprite(b.icono, {
			AnchorPoint = Vector2.new(0.5, 0.5),
			Position = UDim2.fromScale(0.5, 0.5),
			Size = UDim2.fromOffset(28, 28),
			ImageColor3 = COLOR.texto,
			Parent = boton,
		}),
		punto = crear("Frame", {
			AnchorPoint = Vector2.new(0.5, 1),
			Position = UDim2.new(0.5, 0, 1, -4),
			Size = UDim2.fromOffset(4, 4),
			BackgroundColor3 = b.color,
			BackgroundTransparency = 1,
			Parent = boton,
		}, { esquinas(UDim.new(1, 0)) }),
		hover = false,
		activo = false,
	}
	estado[b.id] = e

	boton.MouseEnter:Connect(function()
		e.hover = true
		pintar(b)
		mostrarTip(b)
	end)
	boton.MouseLeave:Connect(function()
		e.hover = false
		pintar(b)
		ocultarTip(b)
	end)
	boton.Activated:Connect(function()
		pulsar(b)
		if soloTactil then
			e.hover = false
			pintar(b)
			mostrarTip(b)
			task.delay(1.2, ocultarTip, b)
		end
	end)

	local panel = buscarPanel(b)
	if panel then
		vigilar(b, panel)
	end
end

UserInputService.InputBegan:Connect(function(input, procesado)
	if procesado or not menuGui.Enabled then
		return
	end
	local b = porTecla[input.KeyCode]
	if b then
		pulsar(b)
	end
end)

local function mostrarMenu()
	menuGui.Enabled = true
	local final = dock.Position
	dock.Position = final + (vertical and UDim2.fromOffset(-120, 0) or UDim2.fromOffset(0, 120))
	tween(dock, 0.6, { Position = final })
end

---------------------------------------------------------------- pantalla de inicio

local function pantallaInicio()
	local gui = crear("ScreenGui", {
		Name = "AetherialsInicio",
		ResetOnSpawn = false,
		IgnoreGuiInset = true,
		ZIndexBehavior = Enum.ZIndexBehavior.Sibling,
		DisplayOrder = 20,
		Parent = playerGui,
	})

	-- oculta la interfaz de Roblox que no hace falta en la portada y la devuelve como estaba
	local nucleo = { Enum.CoreGuiType.PlayerList, Enum.CoreGuiType.Health, Enum.CoreGuiType.Backpack, Enum.CoreGuiType.EmotesMenu }
	local antes = {}
	for _, tipo in ipairs(nucleo) do
		antes[tipo] = StarterGui:GetCoreGuiEnabled(tipo)
		StarterGui:SetCoreGuiEnabled(tipo, false)
	end

	local blur = crear("BlurEffect", { Name = "AetherialsBlur", Size = 0, Parent = Lighting })
	tween(blur, 0.8, { Size = 18 })

	local grupo = crear("CanvasGroup", {
		Size = UDim2.fromScale(1, 1),
		BackgroundTransparency = 1,
		GroupTransparency = 1,
		Parent = gui,
	})
	crear("Frame", {
		Name = "Velo",
		Size = UDim2.fromScale(1, 1),
		BackgroundColor3 = Color3.fromRGB(7, 9, 13),
		Parent = grupo,
	}, {
		crear("UIGradient", {
			Rotation = 90,
			Transparency = NumberSequence.new({
				NumberSequenceKeypoint.new(0, 0.5),
				NumberSequenceKeypoint.new(0.55, 0.62),
				NumberSequenceKeypoint.new(1, 0.25),
			}),
		}),
	})

	local centro = crear("Frame", {
		AnchorPoint = Vector2.new(0.5, 0.5),
		Position = UDim2.fromScale(0.5, 0.48),
		Size = UDim2.fromOffset(720, 420),
		BackgroundTransparency = 1,
		Parent = grupo,
	}, {
		crear("UIListLayout", {
			FillDirection = Enum.FillDirection.Vertical,
			HorizontalAlignment = Enum.HorizontalAlignment.Center,
			VerticalAlignment = Enum.VerticalAlignment.Center,
			SortOrder = Enum.SortOrder.LayoutOrder,
		}),
	})
	local escalaCentro = crear("UIScale", { Parent = centro })

	local function hueco(alto, orden)
		crear("Frame", { Size = UDim2.fromOffset(1, alto), BackgroundTransparency = 1, LayoutOrder = orden, Parent = centro })
	end

	-- emblema del Nexo: los cinco Guardianes giran despacio alrededor del cristal
	local emblema = crear("Frame", { Size = UDim2.fromOffset(96, 96), BackgroundTransparency = 1, LayoutOrder = 1, Parent = centro })
	local anillo = sprite("nexo_anillo", { Size = UDim2.fromScale(1, 1), Parent = emblema })
	sprite("nexo_cristal", { Size = UDim2.fromScale(1, 1), Parent = emblema })
	TweenService:Create(anillo, TweenInfo.new(40, Enum.EasingStyle.Linear, Enum.EasingDirection.InOut, -1), { Rotation = 360 }):Play()

	hueco(28, 2)
	local logo = SPRITES.logo
	sprite("logo", { Size = UDim2.fromOffset(640, math.round(640 * logo[4] / logo[3])), LayoutOrder = 3, Parent = centro })
	hueco(20, 4)
	crear("TextLabel", {
		Size = UDim2.fromOffset(720, 24),
		BackgroundTransparency = 1,
		Text = SUBTITULO,
		FontFace = fuente(Enum.FontWeight.Medium),
		TextSize = 18,
		TextColor3 = COLOR.texto,
		TextTransparency = 0.3,
		LayoutOrder = 5,
		Parent = centro,
	})
	hueco(44, 6)
	local jugar = crear("TextButton", {
		Name = "Jugar",
		Size = UDim2.fromOffset(216, 56),
		BackgroundColor3 = COLOR.oro,
		AutoButtonColor = false,
		Text = "JUGAR",
		FontFace = fuente(Enum.FontWeight.Bold),
		TextSize = 18,
		TextColor3 = COLOR.tinta,
		LayoutOrder = 7,
		Parent = centro,
	}, { esquinas(UDim.new(0.5, 0)) })
	local escalaJugar = crear("UIScale", { Parent = jugar })

	-- zona actual, abajo a la izquierda
	local zona = crear("Frame", {
		AnchorPoint = Vector2.new(0, 1),
		Position = UDim2.new(0, 32, 1, -28),
		Size = UDim2.fromOffset(400, 20),
		BackgroundTransparency = 1,
		Parent = grupo,
	}, {
		crear("UIListLayout", {
			FillDirection = Enum.FillDirection.Horizontal,
			VerticalAlignment = Enum.VerticalAlignment.Center,
			Padding = UDim.new(0, 8),
			SortOrder = Enum.SortOrder.LayoutOrder,
		}),
	})
	local escalaZona = crear("UIScale", { Parent = zona })
	sprite("ubicacion", { Size = UDim2.fromOffset(16, 16), ImageTransparency = 0.4, LayoutOrder = 1, Parent = zona })
	local zonaTexto = crear("TextLabel", {
		AutomaticSize = Enum.AutomaticSize.X,
		Size = UDim2.fromOffset(0, 20),
		BackgroundTransparency = 1,
		Text = mayusculas(jugador:GetAttribute("Zona") or ZONA),
		FontFace = fuente(Enum.FontWeight.SemiBold),
		TextSize = 13,
		TextColor3 = COLOR.texto,
		TextTransparency = 0.45,
		LayoutOrder = 2,
		Parent = zona,
	})
	local conZona = jugador:GetAttributeChangedSignal("Zona"):Connect(function()
		zonaTexto.Text = mayusculas(jugador:GetAttribute("Zona") or ZONA)
	end)

	local function reescalar()
		local s = escalaPantalla(0.7, 1.25)
		escalaCentro.Scale = s
		escalaZona.Scale = s
	end
	reescalar()
	local conCamara = workspace.CurrentCamera:GetPropertyChangedSignal("ViewportSize"):Connect(reescalar)

	jugar.MouseEnter:Connect(function()
		tween(jugar, 0.2, { BackgroundColor3 = COLOR.oroClaro })
		tween(escalaJugar, 0.2, { Scale = 1.04 })
	end)
	jugar.MouseLeave:Connect(function()
		tween(jugar, 0.2, { BackgroundColor3 = COLOR.oro })
		tween(escalaJugar, 0.2, { Scale = 1 })
	end)

	tween(grupo, 0.9, { GroupTransparency = 0 })
	if UserInputService.GamepadEnabled then
		GuiService.SelectedObject = jugar
	end

	local empezado = false
	local conTeclas
	local function empezar()
		if empezado then
			return
		end
		empezado = true
		conTeclas:Disconnect()
		conZona:Disconnect()
		conCamara:Disconnect()
		if GuiService.SelectedObject == jugar then
			GuiService.SelectedObject = nil
		end
		if REMOTO_JUGAR then
			local remoto = ReplicatedStorage:FindFirstChild(REMOTO_JUGAR)
			if remoto and remoto:IsA("RemoteEvent") then
				remoto:FireServer()
			end
		end
		eventoJugar:Fire()
		tween(escalaJugar, 0.1, { Scale = 0.96 })
		tween(blur, 0.6, { Size = 0 })
		tween(grupo, 0.5, { GroupTransparency = 1 }).Completed:Wait()
		blur:Destroy()
		gui:Destroy()
		for tipo, habilitado in pairs(antes) do
			StarterGui:SetCoreGuiEnabled(tipo, habilitado)
		end
		mostrarMenu()
	end
	jugar.Activated:Connect(empezar)
	conTeclas = UserInputService.InputBegan:Connect(function(input, procesado)
		if not procesado and (input.KeyCode == Enum.KeyCode.Return or input.KeyCode == Enum.KeyCode.ButtonA) then
			empezar()
		end
	end)
end

---------------------------------------------------------------- arranque

local function reescalarMenu()
	local s = escalaPantalla(0.85, 1.1)
	escalaDock.Scale = s
	escalaTip.Scale = s
end
reescalarMenu()
workspace.CurrentCamera:GetPropertyChangedSignal("ViewportSize"):Connect(reescalarMenu)

if MOSTRAR_INICIO then
	pantallaInicio()
else
	mostrarMenu()
end
