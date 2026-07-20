# Web/Desktop Parity Roadmap

> Статус 2026-07-20: P6.8–P6.16 реализованы и включены в OpenIPC Dashboard v0.2.7.
> Windows release build, server-only smoke и 30/30 test suite выполнены; Windows installer
> и Linux AppImage публикуются только после успешного GitHub Actions CI. Destructive firmware
> flow по-прежнему допускается только после backup и ручной проверки на совместимой камере.

## Цель

Довести Web-клиент OpenIPC Dashboard до функционального паритета с desktop-приложением,
сохранив один backend, одну модель состояния и единые правила безопасности.

Целевой результат:

- 100% функциональный паритет для функций, разрешённых в браузере;
- визуальный паритет 95–99% на эталонных desktop viewport;
- одинаковые названия, состояния, форматирование, права и сценарии ошибок;
- осознанная browser-адаптация функций, зависящих от окна ОС, файловой системы,
  системного трея, keychain или локального терминала;
- отсутствие camera credentials, OAuth secrets и локальных путей в browser API.

Полностью идентичный UI здесь означает одинаковую структуру, визуальный язык и поведение
рабочих сценариев, а не побайтовую копию QML. Нативная рамка окна, системные диалоги,
глобальные shortcuts и некоторые media-возможности браузера по определению будут отличаться.

## Архитектурное решение

Desktop остаётся на QML, Web — на HTML/CSS/JavaScript. Переписывание desktop-интерфейса
на WebEngine в этот этап не входит.

```text
                    C++ domain/backend
        cameras / users / archive / analytics / devices
                         |
             shared presentation models
        state / permissions / labels / formatting / errors
                 /                         \
          QML adapters                HTTP/WS API DTO
              |                            |
        Desktop QML                  Web HTML/CSS/JS
```

Общими должны стать:

- доменное состояние и команды;
- presentation-модели и форматирование значений;
- словарь терминов и ключи локализации;
- capability/RBAC-проверки;
- validation и error codes;
- design tokens: цвета, типографика, размеры, интервалы, радиусы и состояния.

Раздельными остаются:

- QML-компоненты и browser DOM-компоненты;
- навигация, focus management и platform-specific integration;
- media adapters для desktop pipeline и WebRTC/MJPEG;
- native file dialogs и browser upload/download.

## Обязательные границы

- Браузер получает только минимальные DTO, необходимые конкретному экрану.
- Пароли камер, RTSP URL с credentials, hashes/salts, OAuth secrets и filesystem paths
  никогда не сериализуются в Web API.
- Все mutation endpoints проверяют сессию, роль, CSRF/Origin, входные данные и capability.
- Опасные операции требуют preview/diff, явного подтверждения и audit event.
- Browser-клиент не подключается к SSH камеры напрямую и не хранит SSH credentials.
- API остаётся versioned; несовместимые изменения выпускаются в новой версии API.

## Матрица паритета

Исполняемая матрица с привязкой к исходным QML-компонентам ведётся в
[`WEB_PARITY_MATRIX.md`](WEB_PARITY_MATRIX.md).

| Область | Текущее состояние Web | Целевой этап | Критерий паритета |
|---|---|---|---|
| Login, session, RBAC | Готово | P6.11, P6.16 | Все роли и session lifecycle совпадают с desktop |
| Monitor 1/4/9, sidebar | Готово | P6.15 | Одинаковые состояния, размеры и keyboard flow |
| Discovery/onboarding/edit | Готово | P6.15 | Единая validation и сообщения об ошибках |
| Live video, PTZ | Готово | P6.10 | Одинаковые команды, feedback и degraded states |
| Recording/snapshot/audio/fullscreen | Готово | P6.10 | Полный операторский сценарий из браузера |
| Health | Частично | P6.9, P6.15 | Общие presentation-модели и визуальные состояния |
| Archive/playback/export | Готово | P6.10, P6.14 | Поиск, playback, download и ограничения доступа |
| Analytics/modules/events | Частично | P6.9, P6.15 | Одинаковые данные, filters, empty/error states |
| Settings | Готово | P6.11 | Все безопасно доступные группы настроек |
| Users/permissions | Готово | P6.11 | CRUD, роли, password flow, session invalidation |
| Logs/diagnostics | Готово | P6.12 | Filter, pagination, live tail и download |
| Camex/Majestic | Готово | P6.13 | Read/write parity с capability checks и rollback |
| OpenIPC Control Center | Готово | P6.13 | Safe actions, firmware и device administration |
| Files/import/export | Готово | P6.14 | Browser upload/download вместо native dialogs |
| SSH terminal | Web N/A | P6.13, P6.14 | Намеренно исключён; прямой browser-to-camera SSH запрещён |
| Tray/window/native shortcuts | Web N/A | P6.14 | Документированная browser-адаптация |

## План реализации

### P6.8 — Инвентаризация и контракт паритета

Цель: зафиксировать измеримую точку назначения до расширения API и UI.

Работы:

- составить реестр desktop-экранов, dialogs, действий, ролей и keyboard shortcuts;
- для каждого сценария указать QML source, backend command, Web endpoint и состояние Web;
- зафиксировать эталонные screenshots desktop для основных и пограничных состояний;
- описать loading, empty, warning, error, disabled и reconnect states компонентов;
- завести capability manifest, который сервер отдаёт после входа;
- определить эталонные viewport: desktop wide, laptop, tablet и mobile;
- утвердить список browser-адаптаций и функций вне scope.

Результат этапа:

- version-controlled parity matrix без неучтённых desktop-функций;
- baseline screenshots и перечень visual tokens;
- согласованный API/capability backlog для P6.9–P6.14.

### P6.9 — Общий presentation layer

Цель: исключить расхождение бизнес-логики, подписей и форматирования между QML и Web.

Работы:

- выделить presentation-модели для Camera, Monitor Cell, Health, Archive, Analytics,
  User, Settings и Device Administration;
- вынести форматирование bitrate, FPS, latency, duration, storage, timestamps и статусов
  из QML/JavaScript в общие formatter-модули;
- использовать стабильные enum/error codes, а не сравнение локализованных строк;
- унифицировать localization keys RU/EN и правила fallback;
- определить versioned API DTO и serializers поверх presentation-моделей;
- добавить contract tests, доказывающие отсутствие secrets и локальных путей;
- подготовить schema design tokens и генерацию CSS/QML значений из одного источника.

Результат этапа:

- QML и Web получают семантически одинаковые данные и состояния;
- форматирование и permission logic не дублируются в двух UI;
- изменения API защищены contract tests.

Зависимость: P6.8.

### P6.10 — Полный операторский Monitor и Archive

Цель: закрыть ежедневный операторский workflow.

Работы:

- start/stop recording с busy/error/active indicator и RBAC;
- snapshot активной ячейки с безопасным browser download;
- mute/unmute, volume и корректная обработка browser autoplay policy;
- fullscreen активной ячейки и всей раскладки; PiP только при поддержке браузером;
- полный PTZ feedback: pressed, moving, stop, timeout, unavailable;
- единые reconnect, offline, codec fallback и stream error states;
- archive filters, timeline/playback controls и download/export;
- корректное освобождение WebRTC/MJPEG pipelines при смене раскладки и закрытии вкладки.

Результат этапа:

- оператор выполняет наблюдение, PTZ, запись, snapshot и playback без desktop-клиента;
- все действия соблюдают RBAC и не раскрывают camera/filesystem credentials;
- сценарии проверены на H.264, H.265 fallback, offline и auth-failure камерах.

Зависимости: P6.9; существующие P6.3–P6.5.

### P6.11 — Settings, users и управление доступом

Цель: перенести административные сценарии без ослабления security boundary.

Работы:

- реализовать Settings workspace по тем же группам, что desktop;
- использовать schema-driven поля, validation, defaults и признаки restart/reconnect;
- сохранять связанные настройки транзакционно и возвращать field-level errors;
- реализовать Users CRUD, назначение ролей и изменение пароля;
- запретить удаление/понижение последнего администратора;
- инвалидировать затронутые Web-сессии при смене пароля, роли или отключении пользователя;
- добавить просмотр активных Web-сессий и принудительный logout;
- скрывать недоступные разделы и повторно проверять права на сервере.

Результат этапа:

- администратор выполняет все безопасно переносимые Settings/User операции в Web;
- secrets доступны только как write-only/reconfigure fields;
- desktop и Web применяют одинаковую validation и permission model.

Зависимости: P6.9.

### P6.12 — Logs, diagnostics и observability

Цель: дать Web-клиенту диагностические возможности desktop без неограниченной выдачи данных.

Работы:

- bounded API для логов с pagination/cursor, time range, level и component filters;
- live tail через WebSocket с backpressure и ограничением частоты;
- redaction tokens, credentials, query secrets, локальных user paths и персональных данных;
- download диагностического bundle как отдельная привилегированная операция;
- Health details, server metrics, stream/session counters и последние audit events;
- лимиты retention, размера ответа и числа параллельных подписок.

Результат этапа:

- Logs/Diagnostics функционально соответствуют desktop;
- большие логи не блокируют event loop и не загружаются целиком в память;
- автоматические тесты подтверждают redaction и permission boundaries.

Зависимости: P6.9, P6.11.

### P6.13 — Camex, Majestic и OpenIPC Control Center

Цель: перенести управление устройствами по модели safe-by-default.

Порядок переноса:

1. Read-only inventory, status и capabilities.
2. Безопасные настройки с validation и preview/diff.
3. Backup/restore с проверкой совместимости и rollback plan.
4. Firmware operations с preflight, progress, reconnect и recovery guidance.
5. Опциональный terminal gateway.

Работы:

- capability-driven UI: не показывать команды, которых камера не поддерживает;
- перенести Camex/Majestic panels и safe action flows;
- реализовать device status, services, network/time и firmware manifest views;
- для mutation actions добавить confirm phrase, audit event и idempotency protection;
- перед firmware/restore проверять backup, питание, модель, checksum и доступность recovery;
- выполнять SSH/HTTP device commands только backend-компонентом;
- если terminal действительно нужен — сделать admin-only WebSocket-to-PTY gateway с короткой
  сессией, command audit policy, rate limits и явным включением в настройках;
- не поддерживать прямой browser-to-camera SSH.

Результат этапа:

- Web покрывает Camex/Majestic/Control Center, кроме явно исключённых native actions;
- опасные операции не могут стартовать без preflight и подтверждения;
- disconnect/reboot/recovery states остаются управляемыми и понятными.

Зависимости: P6.9, P6.11, P6.12.

### P6.14 — Browser-адаптации системных функций

Цель: заменить desktop-only интеграции предсказуемыми browser-механизмами.

Работы:

- native file open/save заменить на upload/download;
- server-side файлы адресовать opaque ID с canonical-root revalidation;
- операции импорта выполнять через quarantine, size/type/hash validation;
- открытие внешних ресурсов реализовать обычными HTTPS links с понятным предупреждением;
- desktop keychain оставить backend-only; Web получает только признак configured/not configured;
- global shortcuts сопоставить с shortcuts внутри активной вкладки;
- tray, minimize, always-on-top и native window chrome пометить как desktop-only;
- документировать ограничения Clipboard, autoplay, fullscreen и notifications permissions.

Результат этапа:

- у каждой desktop-only функции есть Web-эквивалент или явное исключение;
- browser API не принимает произвольные server filesystem paths;
- пользователь не сталкивается с неработающими desktop affordances в Web.

Зависимости: P6.10–P6.13.

### P6.15 — Визуальный паритет и responsive polish

Цель: сделать Web визуально и интерактивно узнаваемой копией desktop Dashboard.

Работы:

- собрать Web component library для buttons, inputs, cards, dialogs, tabs, tables,
  badges, tooltips, notifications и split panes;
- применить общие design tokens для light/dark themes;
- повторить desktop hierarchy, spacing, typography, icons и component states;
- согласовать keyboard navigation, focus rings, tab order и shortcuts;
- обеспечить responsive адаптацию без изменения desktop mental model;
- проверить RU/EN layouts, длинные строки, scaling 100/125/150% и high-DPI;
- добавить screenshot regression для эталонных viewport и UI states;
- провести accessibility pass: semantic landmarks, labels, contrast и reduced motion;
- проверить Chromium, Firefox и Safari/WebKit; документировать codec differences.

Результат этапа:

- все экраны проходят design review по baseline screenshots;
- отсутствуют критические visual/interaction расхождения;
- desktop, laptop, tablet и mobile остаются рабочими без горизонтального overflow.

Зависимости: P6.10–P6.14; design tokens из P6.9.

### P6.16 — Production hardening и выпуск

Цель: выпуск Web/Desktop parity как поддерживаемой production-функции.

Работы:

- security review всех новых endpoints, WebSocket channels, upload/download и device actions;
- audit log для login, user/settings changes, exports и device mutations;
- rate limits, request/body quotas, concurrency limits и cancellation;
- session revocation, idle/absolute TTL и защита от повторного использования mutation requests;
- performance profiling раскладок 1/4/9, reconnect storm, log tail и больших архивов;
- проверка Windows/Linux packaging и `--server-only`;
- upgrade/migration tests для settings, users и API clients;
- обновление `WEB_SERVER.md`, release notes и deployment guidance;
- отдельное документирование TLS/reverse proxy/VPN и STUN/TURN, если добавлены.

Результат этапа:

- пройдены все quality gates ниже;
- нет открытых Critical/High security defects и блокирующих parity defects;
- production package воспроизводимо собирается для Windows и Linux.

Зависимости: P6.8–P6.15.

## Релизные вехи

| Веха | Состав | Пользовательский результат |
|---|---|---|
| M1 — Shared foundation | P6.8–P6.9 | Desktop/Web используют общие presentation contracts |
| M2 — Operator parity | P6.10 | Полная работа оператора через браузер |
| M3 — Admin parity | P6.11–P6.12 | Settings, users, logs и диагностика в Web |
| M4 — Device parity | P6.13–P6.14 | Camex/Majestic/Control Center и browser-адаптации |
| M5 — Production parity | P6.15–P6.16 | Визуально выверенный и production-ready Web-клиент |

Каждая веха должна быть пригодна к отдельному review и не зависеть от незавершённого UI
следующей вехи.

## Quality gates

Для каждого этапа:

- unit tests presentation-моделей, formatters, validation и permission rules;
- HTTP/WebSocket contract tests, включая negative security cases;
- targeted QML lint/smoke для изменённых desktop adapters;
- browser E2E для happy path, forbidden, offline, reconnect и session expiry;
- `git diff --check` и релевантная release-сборка.

Перед M2–M5:

- visual regression на фиксированных viewport и theme/locale combinations;
- ручная проверка keyboard-only и mobile touch flow;
- smoke `--server-only` через login, `GET /api/v1/server`, WebSocket и logout;
- real-camera matrix: H.264, H.265 fallback, auth failure, offline/reboot;
- проверка, что API responses и logs не содержат credentials или локальные пути.

Перед production release:

- полная MinGW/Qt 6.4 release-сборка и Windows smoke;
- Linux build/AppImage и server-only smoke;
- полный `ctest --output-on-failure`;
- security review и dependency/runtime packaging audit;
- firmware/restore flows — только ручная проверка на совместимой OpenIPC-камере после backup
  и при стабильном питании.

## Definition of Done полного паритета

Web/Desktop parity считается завершённым, когда:

- каждый пункт parity matrix имеет статус Done или документированный `Web N/A`;
- операторские и административные сценарии проходят E2E под соответствующими ролями;
- одинаковые backend state, validation, permissions и error codes используются в QML и Web;
- visual regression и design review подтверждают целевой уровень сходства;
- browser-specific ограничения отображаются до запуска действия;
- security tests подтверждают отсутствие secrets/path leakage и обхода RBAC/CSRF;
- Web работает в desktop и автономном `--server-only` режимах;
- документация развёртывания, обновления и восстановления актуальна.

## Риски и контроль

| Риск | Контроль |
|---|---|
| Расхождение QML и Web со временем | Shared presentation models, contract tests, parity matrix в review checklist |
| Неконтролируемый рост API | Capability-driven/versioned DTO, endpoint inventory и deprecation policy |
| Утечка camera/server secrets | Allowlist serializers, redaction tests, opaque IDs, backend-only credentials |
| Опасные device actions из браузера | RBAC, preflight, diff, confirm, audit, idempotency и rollback guidance |
| Различия codec/browser | WebRTC capability detection, H.264 path, bounded fallback и browser matrix |
| Слишком большой единый релиз | Вехи M1–M5 и вертикальные slices с отдельными exit criteria |
| Pixel-perfect мешает responsive UX | Эталон desktop viewport плюс документированные tablet/mobile адаптации |

## Первый рабочий пакет после возобновления P6

1. Создать version-controlled parity matrix и привязать каждый пункт к QML source.
2. Зафиксировать screenshots Monitor, Settings, Users, Logs, Camex и Control Center.
3. Описать schema design tokens и выбрать единый источник RU/EN localization keys.
4. Реализовать первый общий `CameraPresentationModel` и contract tests.
5. Перевести существующие camera cards QML/Web на новый presentation contract.
6. Добавить recording state/commands API с RBAC и audit event.
7. Реализовать recording controls в одной monitor cell и пройти E2E.
8. Добавить snapshot download через opaque response без server path leakage.
9. Расширить решение на раскладки 1/4/9, reconnect и error states.
10. Закрыть M1 review и только после этого масштабировать шаблон на остальные разделы.
