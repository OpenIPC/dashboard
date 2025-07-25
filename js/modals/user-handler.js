// js/modals/user-handler.js

(function(window) {
    window.AppModules = window.AppModules || {};

    AppModules.createUserModalHandler = function(App, utils) {
        // VVV НОВЫЙ БЛОК: Константы для ролей VVV
        const USER_ROLES = {
            ADMIN: 'admin',
            OPERATOR: 'operator'
        };
        // ^^^ КОНЕЦ НОВОГО БЛОКА ^^^

        const userManagementModal = document.getElementById('user-management-modal');
        const userManagementCloseBtn = document.getElementById('user-management-close-btn');
        const userListEl = document.getElementById('user-list');
        const openAddUserModalBtn = document.getElementById('open-add-user-modal-btn');
        
        const addUserModal = document.getElementById('add-user-modal');
        const addUserCloseBtn = document.getElementById('add-user-close-btn');
        const saveUserBtn = document.getElementById('save-user-btn');
        const cancelUserBtn = document.getElementById('cancel-user-btn');
        
        const permissionsModal = document.getElementById('permissions-modal');
        const permissionsModalCloseBtn = document.getElementById('permissions-modal-close-btn');
        const permissionsModalTitle = document.getElementById('permissions-modal-title');
        const permissionsListEl = document.getElementById('permissions-list');
        const savePermissionsBtn = document.getElementById('save-permissions-btn');
        const cancelPermissionsBtn = document.getElementById('cancel-permissions-btn');

        const changePasswordModal = document.getElementById('change-password-modal');
        const changePassCloseBtn = document.getElementById('change-pass-close-btn');
        const changePassModalTitle = document.getElementById('change-pass-modal-title');
        const saveNewPasswordBtn = document.getElementById('save-new-password-btn');
        const cancelChangePasswordBtn = document.getElementById('cancel-change-password-btn');
        
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
            utils.openModal(userManagementModal);
            await renderUserList();
        }

        async function renderUserList() {
            userListEl.innerHTML = `<li>${App.t('loading_text')}</li>`;
            const result = await window.api.getUsers();
            userListEl.innerHTML = '';

            if (result.success) {
                result.users.forEach(user => {
                    const li = document.createElement('li');
                    const isCurrentUser = user.username === App.stateManager.state.currentUser?.username;
                    li.innerHTML = `
                        <div style="flex-grow: 1;"><strong>${user.username}</strong> <small>(${App.t('role_' + user.role)})</small></div>
                        <div style="display: flex; gap: 10px;">
                            ${user.role === USER_ROLES.OPERATOR ? `<button class="permissions-btn" data-username="${user.username}">${App.t('permissions_btn')}</button>` : ''}
                            <button class="change-pass-btn">${App.t('change_password')}</button>
                            <button class="delete-user-btn" ${isCurrentUser ? 'disabled' : ''}>${App.t('context_delete')}</button>
                        </div>`;
                    li.style.cssText = "display: flex; justify-content: space-between; align-items: center; padding: 10px; border-bottom: 1px solid #eee;";
                    
                    li.querySelector('.permissions-btn')?.addEventListener('click', () => openPermissionsModal(user));
                    li.querySelector('.change-pass-btn').addEventListener('click', () => openChangePasswordModal(user));
                    li.querySelector('.delete-user-btn').addEventListener('click', async () => {
                        if (confirm(App.t('confirm_delete_user', { username: user.username }))) {
                            const deleteResult = await window.api.deleteUser({ username: user.username });
                            if (deleteResult.success) await renderUserList();
                            else alert(`${App.t('error')}: ${deleteResult.error}`);
                        }
                    });
                    userListEl.appendChild(li);
                });
            } else {
                userListEl.innerHTML = `<li>Error: ${result.error}</li>`;
            }
        }
        
        function openAddUserModal() {
            document.getElementById('add-user-username').value = '';
            document.getElementById('add-user-password').value = '';
            // VVV ИЗМЕНЕНИЕ: Используем константу VVV
            document.getElementById('add-user-role').value = USER_ROLES.OPERATOR;
            // ^^^ КОНЕЦ ИЗМЕНЕНИЯ ^^^
            utils.openModal(addUserModal);
            document.getElementById('add-user-username').focus();
        }

        async function saveNewUser() {
            const username = document.getElementById('add-user-username').value.trim();
            const password = document.getElementById('add-user-password').value;
            const role = document.getElementById('add-user-role').value;

            if (!username || !password) {
                alert(App.t('username_and_password_required'));
                return;
            }
            const result = await window.api.addUser({ username, password, role });
            if (result.success) {
                utils.closeModal(addUserModal);
                await renderUserList();
            } else {
                alert(`${App.t('error')}: ${result.error}`);
            }
        }

        function openChangePasswordModal(user) {
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
                alert(`${App.t('error')}: ${updateResult.error}`);
            }
        }

        function openPermissionsModal(user) {
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
                alert(`${App.t('error')}: ${result.error}`);
            }
        }

        function init() {
            userManagementCloseBtn.addEventListener('click', () => utils.closeModal(userManagementModal));
            userManagementModal.addEventListener('click', (e) => { if (e.target === userManagementModal) utils.closeModal(userManagementModal); });
            openAddUserModalBtn.addEventListener('click', openAddUserModal);
            
            addUserCloseBtn.addEventListener('click', () => utils.closeModal(addUserModal));
            addUserModal.addEventListener('click', (e) => { if (e.target === addUserModal) utils.closeModal(addUserModal); });
            saveUserBtn.addEventListener('click', saveNewUser);
            cancelUserBtn.addEventListener('click', () => utils.closeModal(addUserModal));

            changePassCloseBtn.addEventListener('click', () => utils.closeModal(changePasswordModal));
            changePasswordModal.addEventListener('click', (e) => { if (e.target === changePasswordModal) utils.closeModal(changePasswordModal); });
            saveNewPasswordBtn.addEventListener('click', saveNewPassword);
            cancelChangePasswordBtn.addEventListener('click', () => utils.closeModal(changePasswordModal));

            savePermissionsBtn.addEventListener('click', savePermissions);
            cancelPermissionsBtn.addEventListener('click', () => utils.closeModal(permissionsModal));
            permissionsModalCloseBtn.addEventListener('click', () => utils.closeModal(permissionsModal));
            permissionsModal.addEventListener('click', (e) => { if (e.target === permissionsModal) utils.closeModal(permissionsModal); });

            window.addEventListener('language-changed', () => {
                if (!userManagementModal.classList.contains('hidden')) {
                    document.querySelector('#user-management-modal h2').textContent = App.t('user_management_title');
                    renderUserList(); // Re-render to update button text
                }
                if (!addUserModal.classList.contains('hidden')) {
                    document.querySelector('#add-user-modal h2').textContent = App.t('add_user_title');
                }
            });
        }
        
        return {
            init,
            openUserManagementModal,
            closeAll: () => {
                utils.closeModal(userManagementModal);
                utils.closeModal(addUserModal);
                utils.closeModal(permissionsModal);
                utils.closeModal(changePasswordModal);
            }
        };
    };
})(window);