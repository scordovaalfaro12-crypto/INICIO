# 🏋️ ZONA VIP GYM — Sistema de gestión (v3)

Sistema interno para administrar **ingresos** (matrículas, renovaciones y ventas extras) y controlar **cuánto tiempo le queda a cada socio** de membresía. Corre en la nube (Railway + Supabase) y se instala como app en el celular.

> **El código del servidor vive en la carpeta [`backend/`](backend/).**

---

## 🆕 Qué cambió en la v3 (endurecimiento)

Auditoría completa + pruebas contra una base de datos real con **1.500 socios y
15.000 pagos** (tres años simulados de uso). Lo medido y corregido:

- **El panel abría en 12,8 segundos** porque esperaba una hoja de fuentes de
  Google que bloqueaba toda la página. Ahora las fuentes viven en el propio
  servidor: **abre en 0,3 segundos** y funciona aunque Google esté inaccesible.
- **El buscador de socios descargaba las 1.500 fichas con cada tecla** (3 MB al
  escribir un nombre). Ahora busca la base de datos y espera a que termines de
  escribir: **13 KB y dos consultas**. Además encuentra "José" escribiendo "jose".
- **Un pago con el año mal tecleado se sumaba a la vez en hoy, semana, mes y
  año, para siempre.** Ahora los reportes tienen tope en la fecha de hoy y
  Finanzas avisa si hay dinero registrado con fecha futura.
- **Diez errores de tipeo bloqueaban el acceso 15 minutos.** Ahora la espera
  empieza en 5 segundos y solo crece ante intentos masivos.
- **Un botón del inicio dejaba la pantalla en blanco** (llevaba a una sección
  que no existe).
- **Los fallos de red se ocultaban** y la pantalla mostraba "0 socios", como si
  el gimnasio estuviera vacío. Ahora avisan con una franja roja.
- **Si el celular perdía señal al guardar, se cobraba dos veces** sin aviso.
  Ahora el sistema detecta el pago repetido y pregunta.
- **La clave de sesión tenía un valor por defecto escrito en el código** (que
  está publicado en GitHub). Ahora, si falta la variable, se genera aleatoria.
- **La contraseña inicial era "admin123"** y no había forma de cambiarla desde
  el panel. Ahora se genera al azar y existe la pantalla *Mi cuenta*.
- **El respaldo no se podía volver a cargar.** Ahora se restaura desde el panel.
- **No se podía corregir nada**: ni un nombre mal escrito ni un monto errado.
  Ahora sí (ver *Corregir errores*).
- Dos instancias arrancando a la vez podían duplicar el historial: las
  migraciones toman un candado en la base de datos.
- Consultas con tope de tiempo, reconexión automática, cierre ordenado y
  healthcheck con periodo de gracia para que un arranque lento de la base no
  tumbe el despliegue.

## ✅ Qué cambió en la v2 (resumen para la dueña)

| Antes (v1) | Ahora (v2) |
|---|---|
| Las matrículas vencidas se **borraban solas** a los 15 días: los reportes de ingresos del mes/año se desinflaban | **Nada se borra solo, nunca.** Cada pago queda registrado para siempre en un libro de ingresos (`payments`) |
| Una renovación sumaba el monto a la fecha del pago **original** (dinero contado en el mes equivocado) | Cada renovación se registra **con su fecha real**: los reportes de hoy/semana/quincena/mes/año son exactos |
| Después de las 7pm el sistema creía que ya era "mañana" (usaba hora UTC) y marcaba vencidos antes de tiempo | Todas las fechas se calculan en **hora de Perú** (configurable con `GYM_TZ`) |
| Un corte de conexión con la base de datos **tumbaba el sistema completo** | El sistema aguanta cortes, reintenta y **se recupera solo**, sin que nadie lo reinicie |
| Si faltaba una variable de entorno, el login emitía sesiones que el propio sistema rechazaba (quedaba inutilizable) | Configuración centralizada: funciona igual y avisa en los logs qué falta |
| El código fuente del servidor quedaba **descargable por internet** | Solo se sirve la carpeta `public/` (la pantalla del sistema) |
| Montos en `REAL` (decimales binarios que acumulan errores de centavos) | Montos en `NUMERIC(12,2)` **exactos**, migración automática |
| Sin protección contra adivinanza de contraseñas | Máximo 10 intentos de login cada 10 min por IP (bloqueo de 15 min) |
| Sin respaldos | Botón **"Respaldo"** en Finanzas: descarga TODO en un archivo JSON |

La migración es **automática**: al arrancar por primera vez sobre la base de datos existente, el sistema convierte los montos, crea el libro de pagos con el historial actual y NO toca nada más. Arrancar mil veces deja la base igual que arrancarla una vez.

---

## 🚀 Conectar con Railway (paso a paso)

El sistema ya está preparado para que Railway lo despliegue directo desde este repositorio:

1. Entra a **https://railway.app** → tu proyecto.
2. En el servicio, ve a **Settings → Source** y conecta este repositorio de GitHub (`INICIO`) y la rama principal.
3. **Root Directory**: funciona con `backend` **o vacío** (ambos están soportados; si ya tenías `backend` configurado, déjalo así).
4. En **Variables**, verifica que existan:

   | Variable | Valor |
   |---|---|
   | `DATABASE_URL` | La URI de Supabase (Transaction pooler, puerto 6543) |
   | `JWT_SECRET` | 64 caracteres aleatorios (https://generate-secret.vercel.app/64) |
   | `ADMIN_PASSWORD` | Solo se usa la PRIMERA vez, con la base vacía. Si no la pones, el sistema genera una y la muestra en los logs |
   | `ADMIN_RESET_PASSWORD` | *(opcional)* Solo para recuperar una contraseña olvidada. Bórrala después de usarla |
   | `NODE_ENV` | `production` |

5. (Recomendado) En **Settings → Deploy → Healthcheck Path** pon `/api/health` — Railway no cambiará a una versión nueva hasta que esté sana.
6. Dale **Deploy** (o haz push: cada push a la rama conectada redespliega solo).

> Los usuarios y datos existentes **no se pierden ni cambian**: la contraseña de siempre sigue funcionando.

---

## 🔒 Por qué ya no se corrompe

1. **Libro de ingresos inmutable**: la tabla `payments` guarda cada cobro (matrícula o renovación) con fecha real. Borrar la ficha de un socio NO borra su historial de pagos: los reportes históricos nunca cambian.
2. **Transacciones**: "registrar pago" + "extender membresía" se aplican juntos o no se aplica nada. Jamás queda un cobro a medias.
3. **Validación total de entradas**: fechas que no sean `YYYY-MM-DD` reales, montos no numéricos, negativos o absurdos, textos kilométricos → rechazados con error claro antes de tocar la base.
4. **Estados auto-reparables**: `activa`/`vencida` se recalcula desde la fecha de vencimiento (en ambos sentidos) en cada lectura y cada hora con un proceso interno.
5. **Migraciones idempotentes** registradas en `schema_migrations`: nunca se aplican dos veces.
6. **Blindaje del proceso**: errores de conexión inactiva, promesas sin manejar y JSON malformado ya no tumban el servidor; apagado ordenado en cada redeploy de Railway.
7. **Pruebas automáticas** (`npm test` en `backend/`): 13 pruebas de fechas y validación que corren sin base de datos.

---

## 📅 Control de membresías (lo del "tiempo que les queda")

- **Socios** muestra para cada persona: fecha de pago, fecha de vencimiento, **días restantes**, y estado con colores (verde = activa, naranja = vence en ≤7 días, rojo = vencida).
- Los contadores de arriba (Activos / Vencidos / Vencen pronto) se calculan con la fecha local del gym.
- El filtro "Vencen pronto (7d)" sirve para avisarle al socio antes de que venza.
- Las vencidas **se quedan en la lista** (en rojo) hasta que el admin decida borrarlas — y aunque las borre, sus pagos quedan en Finanzas.

## 🏷️ Precios y promociones (pestaña nueva)

El administrador maneja sus propios precios sin tocar código:

- **Planes de matrícula y promos**: crea opciones con nombre, precio y duración en días (ej: "Máquinas 2 meses (promo) — S/ 130 — 60 días"). Aparecen al instante en el formulario de **Registrar pago** (autocompletan monto y vencimiento) y en el de **Renovar**.
- **Categorías de ventas extras**: agrega o quita categorías (aguas, proteína, guantes...) con precio sugerido opcional.
- Cuando una promo termina, se quita con un clic. **Quitar una opción jamás altera los pagos ya registrados**: el historial guarda su propio texto y monto.
- El sistema viene pre-cargado con la lista real del flyer del gym (Máquinas 1/2/3 meses, Máquinas+Aeróbicos, Aeróbicos, promo 2 personas) más los conceptos históricos.

## 💰 Finanzas

- **Hoy / Semana / Quincena / Mes / Año** salen del libro de pagos real (`payments`) + ventas extras.
- **Exportar CSV**: cada pago individual (matrícula y renovación con su fecha real) + ventas extras. Se abre en Excel.
- **Respaldo**: descarga un JSON completo de todos los datos. **Recomendación: descargarlo 1 vez al mes** y guardarlo en Google Drive.

---

## 🧰 Desarrollo local

```bash
cd backend
cp .env.example .env       # completa DATABASE_URL y JWT_SECRET
npm install
npm test                   # pruebas (no necesitan base de datos)
npm start                  # http://localhost:3000
```

## 📱 Pensado para usarlo desde el celular

El sistema se maneja en el mostrador, con el teléfono en la mano. Por eso:

- **Las siete secciones caben en la barra de abajo.** Antes solo entraban cinco:
  *Finanzas* y *Precios* quedaban fuera de la pantalla, sin forma de llegar.
- **Las tablas se convierten en tarjetas.** La lista de socios medía 830 px en
  una pantalla de 318: había que arrastrar de lado para alcanzar los botones.
  Ahora cada socio es una tarjeta legible con sus botones a la vista.
- **El buscador aparece enseguida**, no después de kilómetro y medio de scroll.
- **Teclado numérico** al escribir teléfono, DNI o montos.
- **El cursor se pone solo** en el primer campo al abrir cualquier ventana.

## 🔁 Un socio que vuelve NO se duplica

Si registras un pago a nombre de alguien que ya está en el sistema (aunque lo
escribas sin tildes), el sistema lo reconoce y te ofrece **renovarle**, que es
lo correcto: así su historial de pagos queda junto y el conteo de socios
activos no se infla. Si de verdad son dos personas distintas con el mismo
nombre, puedes crear la ficha aparte.

## ⚡ Cobrar en el momento justo

El mejor momento para cobrar una renovación es cuando la persona está delante.
Por eso el botón **Cobrar renovación** aparece:

- En **Asistencia**, en el mismo cartel que sale al marcar la entrada de alguien
  vencido o por vencer.
- En el panel **"Avisar a los que vencen pronto"**, junto al botón de WhatsApp.

En los dos casos abre la renovación con el plan y el precio del socio ya
puestos: un toque y cobrado, sin ir a buscarlo a otra pestaña.

## 🧾 Gastos y utilidad real (nuevo)

El sistema solo contaba el dinero que ENTRA. Con eso era imposible saber si el
gimnasio gana o pierde: veías "S/ 8.000 este mes" sin descontar alquiler, luz
ni sueldos.

- Pestaña **Gastos** → *Registrar gasto*. Los rubros (Alquiler, Luz, Agua,
  Sueldos, Compra de productos…) ya vienen cargados y puedes crear los tuyos
  desde *Precios y promos*.
- Arriba ves las tres cifras que importan: **ingresos, gastos y lo que te
  queda**. Si los gastos superan a los ingresos, la cifra sale en rojo.
- El gráfico *"En qué se va el dinero"* muestra tus mayores rubros del mes.
- Un gasto mal registrado se **anula** (no se borra): deja de descontarse pero
  el registro queda guardado.

## 🚪 Asistencia (nuevo)

Pestaña **Asistencia**. Escribes el nombre del socio y pulsas *Marcar entrada*:

- Aparece un cartel grande diciendo si está **al día**, si le **vence en pocos
  días** (el momento de cobrarle la renovación) o si está **VENCIDA**.
- Un socio solo cuenta una visita por día, aunque marque al entrar y al salir.
- Panel **"Pagan pero no vienen"**: socios al día que llevan más de 30 días sin
  aparecer. Son los que se dan de baja sin avisar; cada uno trae su botón de
  WhatsApp para llamarlos antes de perderlos.

## 👤 Historial de cada socio (nuevo)

En **Socios** → botón **Historial**: todo lo que esa persona ha pagado (con los
anulados tachados), cuánto suma en total, cuántas veces ha venido y cuándo fue
la última. Sirve para responder al instante "¿este ya pagó?".

## 🛠️ Corregir errores (nuevo en la v3)

Nadie teclea perfecto todos los días. Todo esto se arregla desde el panel, sin ayuda técnica:

| Si te equivocaste en… | Cómo se arregla |
|---|---|
| El nombre, teléfono, DNI, plan o fecha de vencimiento de un socio | Pestaña **Socios** → botón **Corregir** en su fila |
| El monto de un pago (tecleaste 800 en vez de 80) | Pestaña **Finanzas** → tabla **Últimos pagos** → **Anular**. El pago deja de contar, pero el registro queda guardado con el motivo |
| Una venta de producto mal registrada | Pestaña **Socios/Extras** → **Anular** en su fila |
| Un pago con el año mal (2027 en vez de 2026) | Finanzas te avisa arriba en naranja y te dice cuántos hay; anúlalo y regístralo bien |
| Registraste dos veces el mismo pago | El sistema lo detecta y pregunta antes de duplicarlo |
| Tu contraseña | Botón del **candado** abajo a la izquierda → *Cambiar mi contraseña* |

## 📲 Avisar a los socios que vencen

En **Socios** aparece el panel *"Avisar a los que vencen pronto"* con los que
vencen en 3, 7 o 15 días y un botón que abre **WhatsApp** con el mensaje ya
escrito. No hay ningún servicio contratado detrás: es un enlace normal, así que
no puede caducar ni cobrarte nada.

## 🛟 Respaldo y restauración

- **Descargar:** Finanzas → **RESPALDO**. Hazlo una vez al mes y guarda el archivo en tu correo o celular.
- **Restaurar:** Finanzas → *Copia de seguridad* → **Restaurar desde un archivo…**
  Reemplaza socios, pagos, ventas y precios por los del archivo. **Tu contraseña no cambia.**
  Consejo: antes de restaurar, descarga un respaldo de lo que hay ahora.

## 🔑 Si olvidaste la contraseña

No hace falta tocar nada del código:

1. Railway → tu servicio **web** → pestaña **Variables**.
2. Crea la variable `ADMIN_RESET_PASSWORD` con la contraseña nueva que quieras (mínimo 6 caracteres).
3. Aplica los cambios y espera el redespliegue (1–2 minutos).
4. Entra con esa contraseña y **borra la variable** `ADMIN_RESET_PASSWORD` de Railway.

## 🆘 Si algo falla

| Problema | Qué revisar |
|---|---|
| "Application failed to respond" | Logs de Railway. El sistema reintenta la conexión a la base solo; si los logs dicen `[DB] Sin conexión`, revisa `DATABASE_URL` |
| `/api/health` responde 503 | La base de datos no responde: revisa el estado de Supabase. El server se recupera solo cuando vuelva |
| "Demasiados intentos seguidos" | Protección contra ataques. La espera es corta al principio (5 s) y solo crece si se falla muchas veces seguidas. Al entrar bien, el contador se borra |
| Olvidaste la contraseña | Ver *"Si olvidaste la contraseña"* más arriba |
| Franja roja arriba: "sin conexión" | Tu internet o la base de datos. Los datos están a salvo; el sistema se reconecta solo |
| "Sesión expirada" | Normal después de 24 h: vuelve a entrar |
| Build failed en Railway | Verifica que el Root Directory sea `backend` o esté vacío |

### Estructura

```
├── package.json          ← arranque alterno si Railway apunta a la raíz
├── railway.json          ← healthcheck + política de reinicio
└── backend/
    ├── server.js         ← servidor Express endurecido
    ├── config.js         ← TODAS las variables de entorno, en un solo lugar
    ├── db.js             ← pool blindado + esquema + migraciones idempotentes
    ├── lib/dates.js      ← fechas en hora de Perú, a prueba de zona horaria
    ├── lib/validate.js   ← validación de toda entrada de datos
    ├── lib/errores.js    ← distingue "la base está caída" de "error de verdad"
    ├── middleware/       ← sesiones JWT + cabeceras de seguridad + freno de login
    ├── routes/           ← auth, matrículas/finanzas, extras, gastos,
    │                       asistencia, clases, catálogo y respaldo
    ├── tests/            ← pruebas automáticas (npm test)
    └── public/           ← la pantalla del sistema (HTML/CSS/JS + PWA)
        └── assets/fonts/ ← tipografías propias: cero dependencias externas
```
