// js/window-controls.js (НОВЫЙ ФАЙЛ)

(function(window) {
    window.AppModules = window.AppModules || {};

    AppModules.createWindowControls = function(App) {
        function init() {
            const minimizeBtn = document.getElementById('minimize-btn');
            const maximizeBtn = document.getElementById('maximize-btn');
            const closeBtn = document.getElementById('close-btn');

            if (!minimizeBtn || !maximizeBtn || !closeBtn) {
                console.error('Window control buttons not found');
                return;
            }

            minimizeBtn.addEventListener('click', () => {
                window.api.minimizeWindow();
            });

            maximizeBtn.addEventListener('click', () => {
                window.api.maximizeWindow();
            });

            closeBtn.addEventListener('click', () => {
                window.api.closeWindow();
            });

            // Обновляем иконку при изменении состояния окна (например, по F11 или Aero Snap)
            window.api.onWindowMaximized(() => {
                maximizeBtn.innerHTML = '<i class="material-icons">filter_none</i>';
            });
            window.api.onWindowUnmaximized(() => {
                maximizeBtn.innerHTML = '<i class="material-icons">crop_square</i>';
            });
        }
        
        return {
            init
        };
    }

})(window);