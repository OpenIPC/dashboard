from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def require(path: str, *tokens: str) -> None:
    text = (ROOT / path).read_text(encoding="utf-8")
    missing = [token for token in tokens if token not in text]
    if missing:
        raise AssertionError(f"{path} is missing desktop P12.0 contracts: {missing}")


require(
    "src/backend/SystemController.cpp",
    "ensureGridPageCapacity",
    "compactGridPages",
    "startPushToTalk",
    "playPcmData",
)
require(
    "src/ui/DashboardView.qml",
    "property int currentPage",
    "property bool pageCycling",
    "property bool kioskMode",
    "DashboardPageControls",
)
require(
    "src/ui/GridCell.qml",
    "property real digitalZoomScale",
    "changeDigitalZoom",
    "startPushToTalk",
)
require(
    "src/ui/UserManagementDialog.qml",
    "currentScopes",
    "updateUserCameraScopes",
)
require(
    "src/ui/AddUserDialog.qml",
    "selectedCameraScopes",
    "updateUserCameraScopes",
)

# Desktop parity is additive: the already delivered Web controls must remain available.
require(
    "src/web/monitor.js",
    "workspacePageStart",
    "changeDigitalZoom",
    "startPushToTalk",
)

print("desktop competitive parity contract: OK")
