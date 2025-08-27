// --- START OF FILE js/modals/user-handler.js ---
(function(window) {
    window.AppModules = window.AppModules || {};

    AppModules.createUserModalHandler = function(App, utils) {
        
        let userManagementModal, userManagementCloseBtn, userListEl, openAddUserModalBtn,
            addUserModal, addUserCloseBtn, saveUserBtn, cancelUserBtn,
            permissionsModal, permissionsModalCloseBtn, permissionsModalTitle, permissionsListEl, savePermissionsBtn, cancelPermissionsBtn,
            changePasswordModal, changePassCloseBtn, changePassModalTitle, saveNewPasswordBtn, cancelChangePasswordBtn;

        let editingPermissionsForUser = null;
        let editingPasswordForUser = null;

        const availablePermissions = [
            { key: 'view_archive', labelKey: 'view_archive' },
            { key: 'export_archive', labelKey: 'export_archive' },
            { key: 'edit_cameras', labelKey: 'edit_cameras' },
            { key: 'delete_cameras', labelKey: 'delete_cameras' },
            { key: 'access_settings', labelKey: 'access_settings' },
            { key: 'manage_layout', labelKey: 'manage_layout' },
        ];

        async function openUserManagementModal() {
            if (!userManagementModal) return;
            utils.openModal(userManagementModal);
            await renderUserList();
        }

        async function renderUserList() {
            if (!userListEl) return;
            userListEl.innerHTML = `<li>${App.t('loading_text')}</li>`;
            const result = await window.api.getUsers();
            userListEl.innerHTML = '';
            if (result.success) {
                result.users.forEach(user => {
                    const li = document.createElement('li');
                    const isCurrentUser = user.username === App.stateManager.state.currentUser?.username;
                    li.innerHTML = `<div style="flex-grow: 1;"><strong>${user.username}</strong> <small>(${App.t('role_' + user.role)})</small></div><div style="display: flex; gap: 10px;">${user.role === App.USER_ROLES.OPERATOR ? `<button class="permissions-btn" data-username="${user.username}">${App.t('permissions_btn')}</button>` : ''}<button class="change-pass-btn">${App.t('change_password')}</button><button class="delete-user-btn" ${isCurrentUser ? 'disabled' : ''}>${App.t('context_delete')}</button></div>`;
                    li.style.cssText = "display: flex; justify-content: space-between; align-items: center; padding: 10px; border-bottom: 1px solid #eee;";
                    li.querySelector('.permissions-btn')?.addEventListener('click', () => openPermissionsModal(user));
                    li.querySelector('.change-pass-btn').addEventListener('click', () => openChangePasswordModal(user));
                    li.querySelector('.delete-user-btn').addEventListener('click', async () => {
                        const confirmation = await App.modalHandler.showPrompt({ title: App.t('context_delete'), label: App.t('confirm_delete_user', { username: user.username }), okText: App.t('context_delete'), cancelText: App.t('cancel'), inputType: 'none' });
                        if (confirmation !== null) {
                            const deleteResult = await window.api.deleteUser({ username: user.username });
                            if (deleteResult.success) {
                                await renderUserList();
                            } else {
                                App.modalHandler.showToast(`${App.t('error')}: ${deleteResult.error}`, true);
                            }
                        }
                    });
                    userListEl.appendChild(li);
                });
            } else {
                userListEl.innerHTML = `<li>Error: ${result.error}</li>`;
            }
        }
        
        function openAddUserModal() {
            if (!addUserModal) return;
            document.getElementById('add-user-username').value = '';
            document.getElementById('add-user-password').value = '';
            document.getElementById('add-user-role').value = App.USER_ROLES.OPERATOR;
            utils.openModal(addUserModal);
            document.getElementById('add-user-username').focus();
        }

        async function saveNewUser() {
            const username = document.getElementById('add-user-username').value.trim();
            const password = document.getElementById('add-user-password').value;
            const role = document.getElementById('add-user-role').value;
            if (!username || !password) {
                App.modalHandler.showToast(App.t('username_and_password_required'), true);
                return;
            }
            const result = await window.api.addUser({ username, password, role });
            if (result.success) {
                utils.closeModal(addUserModal);
                await renderUserList();
            } else {
                App.modalHandler.showToast(`${App.t('error')}: ${result.error}`, true);
            }
        }

        function openChangePasswordModal(user) {
            if (!changePasswordModal) return;
            editingPasswordForUser = user;
            changePassModalTitle.textContent = App.t('change_password_for_user', { username: user.username });
            document.getElementById('change-user-password').value = '';
            utils.openModal(changePasswordModal);
            document.getElementById('change-user-password').focus();
        }

        async function saveNewPassword() {
            if (!editingPasswordForUser) return;
            const newPassword = document.getElementById('change-user-password').value;
            if (!newPassword.trim()) return;
            const updateResult = await window.api.updateUserPassword({ username: editingPasswordForUser.username, password: newPassword });
            if (updateResult.success) {
                utils.showToast(App.t('password_changed_success'));
                utils.closeModal(changePasswordModal);
            } else {
                App.modalHandler.showToast(`${App.t('error')}: ${updateResult.error}`, true);
            }
        }

        function openPermissionsModal(user) {
            if (!permissionsModal) return;
            editingPermissionsForUser = user;
            permissionsModalTitle.textContent = App.t('permissions_for_user', { username: user.username });
            permissionsListEl.innerHTML = '';
            availablePermissions.forEach(perm => {
                const isChecked = user.permissions && user.permissions[perm.key];
                permissionsListEl.innerHTML += `<div class="form-check-inline"><input type="checkbox" id="perm-${perm.key}" data-key="${perm.key}" class="form-check-input" ${isChecked ? 'checked' : ''}><label for="perm-${perm.key}">${App.t(perm.labelKey)}</label></div>`;
            });
            utils.openModal(permissionsModal);
        }

        async function savePermissions() {
            if (!editingPermissionsForUser) return;
            const newPermissions = {};
            permissionsListEl.querySelectorAll('input[type="checkbox"]:checked').forEach(checkbox => {
                newPermissions[checkbox.dataset.key] = true;
            });
            const result = await window.api.updateUserPermissions({ username: editingPermissionsForUser.username, permissions: newPermissions });
            if (result.success) {
                utils.showToast(App.t('permissions_saved_success'));
                utils.closeModal(permissionsModal);
                await renderUserList();
            } else {
                App.modalHandler.showToast(`${App.t('error')}: ${result.error}`, true);
            }
        }

        function init() {
            userManagementModal = document.getElementById('user-management-modal');
            userManagementCloseBtn = document.getElementById('user-management-close-btn');
            userListEl = document.getElementById('user-list');
            openAddUserModalBtn = document.getElementById('open-add-user-modal-btn');
            addUserModal = document.getElementById('add-user-modal');
            addUserCloseBtn = document.getElementById('add-user-close-btn');
            saveUserBtn = document.getElementById('save-user-btn');
            cancelUserBtn = document.getElementById('cancel-user-btn');
            permissionsModal = document.getElementById('permissions-modal');
            permissionsModalCloseBtn = document.getElementById('permissions-modal-close-btn');
            permissionsModalTitle = document.getElementById('permissions-modal-title');
            permissionsListEl = document.getElementById('permissions-list');
            savePermissionsBtn = document.getElementById('save-permissions-btn');
            cancelPermissionsBtn = document.getElementById('cancel-permissions-btn');
            changePasswordModal = document.getElementById('change-password-modal');
            changePassCloseBtn = document.getElementById('change-pass-close-btn');
            changePassModalTitle = document.getElementById('change-pass-modal-title');
            saveNewPasswordBtn = document.getElementById('save-new-password-btn');
            cancelChangePasswordBtn = document.getElementById('cancel-change-password-btn');

            if (userManagementCloseBtn) userManagementCloseBtn.addEventListener('click', () => utils.closeModal(userManagementModal));
            if (userManagementModal) userManagementModal.addEventListener('click', (e) => { if (e.target === userManagementModal) utils.closeModal(userManagementModal); });
            if (openAddUserModalBtn) openAddUserModalBtn.addEventListener('click', openAddUserModal);
            if (addUserCloseBtn) addUserCloseBtn.addEventListener('click', () => utils.closeModal(addUserModal));
            if (addUserModal) addUserModal.addEventListener('click', (e) => { if (e.target === addUserModal) utils.closeModal(addUserModal); });
            if (saveUserBtn) saveUserBtn.addEventListener('click', saveNewUser);
            if (cancelUserBtn) cancelUserBtn.addEventListener('click', () => utils.closeModal(addUserModal));
            if (changePassCloseBtn) changePassCloseBtn.addEventListener('click', () => utils.closeModal(changePasswordModal));
            if (changePasswordModal) changePasswordModal.addEventListener('click', (e) => { if (e.target === changePasswordModal) utils.closeModal(changePasswordModal); });
            if (saveNewPasswordBtn) saveNewPasswordBtn.addEventListener('click', saveNewPassword);
            if (cancelChangePasswordBtn) cancelChangePasswordBtn.addEventListener('click', () => utils.closeModal(changePasswordModal));
            if (savePermissionsBtn) savePermissionsBtn.addEventListener('click', savePermissions);
            if (cancelPermissionsBtn) cancelPermissionsBtn.addEventListener('click', () => utils.closeModal(permissionsModal));
            if (permissionsModalCloseBtn) permissionsModalCloseBtn.addEventListener('click', () => utils.closeModal(permissionsModal));
            if (permissionsModal) permissionsModal.addEventListener('click', (e) => { if (e.target === permissionsModal) utils.closeModal(permissionsModal); });

            window.addEventListener('language-changed', () => {
                if (userManagementModal && !userManagementModal.classList.contains('hidden')) {
                    document.querySelector('#user-management-modal h2').textContent = App.t('user_management_title');
                    renderUserList();
                }
                if (addUserModal && !addUserModal.classList.contains('hidden')) {
                    document.querySelector('#add-user-modal h2').textContent = App.t('add_user_title');
                }
            });
        }
        
        return {
            init,
            openUserManagementModal,
            closeAll: () => {
                if (userManagementModal) utils.closeModal(userManagementModal);
                if (addUserModal) utils.closeModal(addUserModal);
                if (permissionsModal) utils.closeModal(permissionsModal);
                if (changePasswordModal) utils.closeModal(changePasswordModal);
            }
        };
    };
})(window);
// --- END OF FILE js/modals/user-handler.js ---