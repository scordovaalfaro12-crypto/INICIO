# 🏋️ ZONA VIP GYM — Sistema de gestión (v2)

Sistema interno para administrar **ingresos** (matrículas, renovaciones y ventas extras) y controlar **cuánto tiempo le queda a cada socio** de membresía. Corre en la nube (Railway + Supabase) y se instala como app en el celular.

> **El código del servidor vive en la carpeta [`backend/`](backend/).**

---

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
   | `ADMIN_PASSWORD` | Solo se usa la PRIMERA vez, con la base vacía |
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

## 🆘 Si algo falla

| Problema | Qué revisar |
|---|---|
| "Application failed to respond" | Logs de Railway. El sistema reintenta la conexión a la base solo; si los logs dicen `[DB] Sin conexión`, revisa `DATABASE_URL` |
| `/api/health` responde 503 | La base de datos no responde: revisa el estado de Supabase. El server se recupera solo cuando vuelva |
| "Demasiados intentos fallidos" | Protección anti fuerza bruta: espera 15 minutos |
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
    ├── middleware/       ← sesiones JWT + cabeceras de seguridad + rate limit
    ├── routes/           ← auth, matrículas/finanzas, extras, clases, respaldo
    ├── tests/            ← pruebas automáticas (npm test)
    └── public/           ← la pantalla del sistema (HTML/CSS/JS + PWA)
```
