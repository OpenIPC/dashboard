import React, { useState, useEffect } from 'react';
import { useLocalization } from '../hooks/useLocalization';
import { useAuth } from '../hooks/useAuth';
import {
  addUser as addUserRequest,
  deleteUser as deleteUserRequest,
  getUsers as getUsersRequest,
  updateUserPassword as updateUserPasswordRequest,
  updateUserPermissions as updateUserPermissionsRequest,
} from '../services/auth';
import type { AuthUser, UserPermissions, UserRole } from '../types';
import './SettingsModal.css'; // Используем те же стили, что и у SettingsModal

interface UserDialogProps {
  isOpen: boolean;
  onClose: () => void;
}

interface Permission {
  key: keyof UserPermissions;
  labelKey: string;
}

const availablePermissions: Permission[] = [
  { key: 'view_archive', labelKey: 'view_archive' },
  { key: 'export_archive', labelKey: 'export_archive' },
  { key: 'edit_cameras', labelKey: 'edit_cameras' },
  { key: 'delete_cameras', labelKey: 'delete_cameras' },
  { key: 'access_settings', labelKey: 'access_settings' },
  { key: 'manage_layout', labelKey: 'manage_layout' },
];

type NewUserForm = {
  username: string;
  password: string;
  role: UserRole;
};

const UserDialog: React.FC<UserDialogProps> = ({ isOpen, onClose }) => {
  const { t } = useLocalization();
  const { user: currentUser } = useAuth();
  const [users, setUsers] = useState<AuthUser[]>([]);
  const [loading, setLoading] = useState(false);
  
  // Add user modal state
  const [addUserOpen, setAddUserOpen] = useState(false);
  const [newUser, setNewUser] = useState<NewUserForm>({
    username: '',
    password: '',
    role: 'operator',
  });
  
  // Change password modal state
  const [changePasswordOpen, setChangePasswordOpen] = useState(false);
  const [editingPasswordForUser, setEditingPasswordForUser] = useState<AuthUser | null>(null);
  const [newPassword, setNewPassword] = useState('');
  
  // Permissions modal state
  const [permissionsOpen, setPermissionsOpen] = useState(false);
  const [editingPermissionsForUser, setEditingPermissionsForUser] = useState<AuthUser | null>(null);
  const [userPermissions, setUserPermissions] = useState<UserPermissions>({});

  useEffect(() => {
    if (isOpen) {
      fetchUsers();
    }
  }, [isOpen]);

  const fetchUsers = async () => {
    setLoading(true);
    try {
      const result = await getUsersRequest();
      if (result.success && result.users) {
        setUsers(result.users);
      } else if (result.error) {
        console.error('Failed to fetch users:', result.error);
      } else {
        setUsers([]);
      }
    } catch (error) {
      console.error('Error fetching users:', error);
    }
    setLoading(false);
  };

  const handleAddUser = async () => {
    if (!newUser.username.trim() || !newUser.password.trim()) {
      alert(t('username_and_password_required'));
      return;
    }

    try {
      const result = await addUserRequest(
        newUser.username.trim(),
        newUser.password,
        newUser.role,
        newUser.role === 'operator' ? {} : undefined,
      );

      if (result.success) {
        setAddUserOpen(false);
        setNewUser({ username: '', password: '', role: 'operator' });
        fetchUsers();
      } else {
        alert(`${t('error')}: ${result.error ?? t('unknown_error')}`);
      }
    } catch (error) {
      console.error('Error adding user:', error);
      alert(`${t('error')}: ${error}`);
    }
  };

  const handleDeleteUser = async (user: AuthUser) => {
    if (!confirm(t('confirm_delete_user', { username: user.username }))) {
      return;
    }

    if (currentUser && currentUser.username === user.username) {
      alert(t('error'));
      return;
    }

    try {
      const result = await deleteUserRequest(user.username);
      if (result.success) {
        fetchUsers();
      } else {
        alert(`${t('error')}: ${result.error ?? t('unknown_error')}`);
      }
    } catch (error) {
      console.error('Error deleting user:', error);
      alert(`${t('error')}: ${error}`);
    }
  };

  const handleChangePassword = async () => {
    if (!editingPasswordForUser || !newPassword.trim()) {
      return;
    }

    try {
      const result = await updateUserPasswordRequest(
        editingPasswordForUser.username,
        newPassword,
      );
      if (result.success) {
        alert(t('password_changed_success'));
        setChangePasswordOpen(false);
        setNewPassword('');
        setEditingPasswordForUser(null);
      } else {
        alert(`${t('error')}: ${result.error ?? t('unknown_error')}`);
      }
    } catch (error) {
      console.error('Error changing password:', error);
      alert(`${t('error')}: ${error}`);
    }
  };

  const openPermissionsModal = (user: AuthUser) => {
    setEditingPermissionsForUser(user);
    setUserPermissions(user.permissions || {});
    setPermissionsOpen(true);
  };

  const handleSavePermissions = async () => {
    if (!editingPermissionsForUser) return;

    try {
      const result = await updateUserPermissionsRequest(
        editingPermissionsForUser.username,
        userPermissions,
      );
      if (result.success) {
        alert(t('permissions_saved_success'));
        setPermissionsOpen(false);
        setEditingPermissionsForUser(null);
        fetchUsers();
      } else {
        alert(`${t('error')}: ${result.error ?? t('unknown_error')}`);
      }
    } catch (error) {
      console.error('Error saving permissions:', error);
      alert(`${t('error')}: ${error}`);
    }
  };

  const openChangePasswordModal = (user: AuthUser) => {
    setEditingPasswordForUser(user);
    setNewPassword('');
    setChangePasswordOpen(true);
  };

  if (!isOpen) return null;

  return (
    <>
      {/* Main User Management Modal */}
      <div className="modal-backdrop">
        <div className="modal-content" style={{ width: '700px', maxHeight: '80vh' }}>
          <span className="modal-close-btn" onClick={onClose}>&times;</span>
          <h2>{t('user_management_title')}</h2>

          <div className="modal-body">
            <div
              style={{
                minHeight: '300px',
                maxHeight: '50vh',
                overflowY: 'auto',
                border: '1px solid var(--border-color)',
                marginTop: '15px',
                borderRadius: '4px',
              }}
            >
              {loading ? (
                <div style={{ padding: '20px', textAlign: 'center' }}>{t('loading_text')}</div>
              ) : (
                <div style={{ padding: 0 }}>
                  {users.map((userItem, index) => {
                    const isCurrentUser = currentUser?.username === userItem.username;
                    return (
                      <div
                        key={userItem.username}
                        style={{
                          display: 'flex',
                          justifyContent: 'space-between',
                          alignItems: 'center',
                          padding: '15px 20px',
                          borderBottom:
                            index < users.length - 1 ? '1px solid var(--border-color)' : 'none',
                          backgroundColor: index % 2 === 0 ? 'var(--bg-secondary)' : 'transparent',
                        }}
                      >
                        <div style={{ flexGrow: 1 }}>
                          <strong style={{ color: 'var(--text-main)', fontSize: '16px' }}>
                            {userItem.username}
                          </strong>
                          <small style={{ color: 'var(--text-secondary)', marginLeft: '10px' }}>
                            ({t(`role_${userItem.role}`)})
                          </small>
                        </div>
                        <div style={{ display: 'flex', gap: '8px' }}>
                          {userItem.role === 'operator' && (
                            <button
                              className="modal-button secondary"
                              onClick={() => openPermissionsModal(userItem)}
                              style={{ fontSize: '12px', padding: '6px 12px' }}
                            >
                              {t('permissions_btn')}
                            </button>
                          )}
                          <button
                            className="modal-button secondary"
                            onClick={() => openChangePasswordModal(userItem)}
                            style={{ fontSize: '12px', padding: '6px 12px' }}
                          >
                            {t('change_password')}
                          </button>
                          <button
                            className="modal-button danger"
                            disabled={isCurrentUser}
                            onClick={() => handleDeleteUser(userItem)}
                            style={{ fontSize: '12px', padding: '6px 12px' }}
                          >
                            {t('delete_user')}
                          </button>
                        </div>
                      </div>
                    );
                  })}
                </div>
              )}
            </div>

            <div
              style={{
                justifyContent: 'flex-start',
                marginTop: '15px',
                paddingTop: '15px',
                borderTop: '1px solid var(--border-color)',
              }}
            >
              <button className="modal-button primary" onClick={() => setAddUserOpen(true)}>
                {t('add_user')}
              </button>
            </div>
          </div>
        </div>
      </div>

      {/* Add User Modal */}
      {addUserOpen && (
        <div className="modal-backdrop">
          <div className="modal-content" style={{ width: '400px' }}>
            <span className="modal-close-btn" onClick={() => setAddUserOpen(false)}>
              &times;
            </span>
            <h2>{t('add_user_title')}</h2>

            <div
              className="form-grid simple"
              style={{ gridTemplateColumns: '100px 1fr', paddingTop: '10px' }}
            >
              <span>{t('login')}</span>
              <input
                type="text"
                value={newUser.username}
                onChange={(e) =>
                  setNewUser((prev) => ({ ...prev, username: e.target.value }))
                }
                autoComplete="off"
                className="modal-input"
                placeholder={t('login')}
              />

              <span>{t('password')}</span>
              <input
                type="password"
                value={newUser.password}
                onChange={(e) =>
                  setNewUser((prev) => ({ ...prev, password: e.target.value }))
                }
                autoComplete="new-password"
                className="modal-input"
                placeholder={t('password')}
              />

              <span>{t('user_role')}</span>
              <select
                value={newUser.role}
                onChange={(e) =>
                  setNewUser((prev) => ({
                    ...prev,
                    role: e.target.value as UserRole,
                  }))
                }
                className="modal-select"
              >
                <option value="operator">{t('role_operator')}</option>
                <option value="admin">{t('role_admin')}</option>
              </select>
            </div>

            <div className="modal-footer">
              <button className="modal-button primary" onClick={handleAddUser}>
                {t('save')}
              </button>
              <button className="modal-button secondary" onClick={() => setAddUserOpen(false)}>
                {t('cancel')}
              </button>
            </div>
          </div>
        </div>
      )}

      {/* Change Password Modal */}
      {changePasswordOpen && editingPasswordForUser && (
        <div className="modal-backdrop">
          <div className="modal-content" style={{ width: '400px' }}>
            <span className="modal-close-btn" onClick={() => setChangePasswordOpen(false)}>
              &times;
            </span>
            <h2>{t('change_password_for_user', { username: editingPasswordForUser.username })}</h2>

            <div
              className="form-grid simple"
              style={{ gridTemplateColumns: '120px 1fr', paddingTop: '10px' }}
            >
              <span>{t('new_password')}</span>
              <input
                type="password"
                value={newPassword}
                onChange={(e) => setNewPassword(e.target.value)}
                autoComplete="new-password"
                className="modal-input"
                placeholder={t('new_password')}
              />
            </div>

            <div className="modal-footer">
              <button className="modal-button primary" onClick={handleChangePassword}>
                {t('save')}
              </button>
              <button className="modal-button secondary" onClick={() => setChangePasswordOpen(false)}>
                {t('cancel')}
              </button>
            </div>
          </div>
        </div>
      )}

      {/* Permissions Modal */}
      {permissionsOpen && editingPermissionsForUser && (
        <div className="modal-backdrop">
          <div className="modal-content" style={{ width: '500px' }}>
            <span className="modal-close-btn" onClick={() => setPermissionsOpen(false)}>
              &times;
            </span>
            <h2>{t('permissions_for_user', { username: editingPermissionsForUser.username })}</h2>

            <div className="modal-body" style={{ paddingTop: '15px' }}>
              <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '10px' }}>
                {availablePermissions.map((permission) => (
                  <div key={permission.key} className="form-check-inline">
                    <input
                      type="checkbox"
                      id={`perm-${permission.key}`}
                      checked={userPermissions[permission.key] ?? false}
                      onChange={(e) =>
                        setUserPermissions((prev) => ({
                          ...prev,
                          [permission.key]: e.target.checked,
                        }))
                      }
                      className="form-check-input"
                    />
                    <label
                      htmlFor={`perm-${permission.key}`}
                      style={{ marginLeft: '8px', color: 'var(--text-main)' }}
                    >
                      {t(permission.labelKey)}
                    </label>
                  </div>
                ))}
              </div>
            </div>

            <div className="modal-footer">
              <button className="modal-button primary" onClick={handleSavePermissions}>
                {t('save')}
              </button>
              <button className="modal-button secondary" onClick={() => setPermissionsOpen(false)}>
                {t('cancel')}
              </button>
            </div>
          </div>
        </div>
      )}
    </>
  );
};

export default UserDialog;