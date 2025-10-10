use crate::settings_root_dir;
use base64::engine::general_purpose::STANDARD as BASE64;
use base64::Engine;
use chrono::{DateTime, Duration, Utc};
use pbkdf2::pbkdf2_hmac;
use rand::rngs::OsRng;
use rand::RngCore;
use serde::{Deserialize, Serialize};
use sha2::{Digest, Sha256, Sha512};
use std::collections::HashMap;
use std::fs;
use std::io::ErrorKind;
use std::path::PathBuf;
use std::sync::Arc;
use std::sync::{Mutex as StdMutex, MutexGuard};
use tauri::async_runtime::spawn_blocking;
use tauri::{AppHandle, State};

const USERS_FILENAME: &str = "users.json";
const AUTOLOGIN_FILENAME: &str = "autologin.json";
const PBKDF2_ITERATIONS: u32 = 150_000;
const SALT_LEN: usize = 16;
const REMEMBER_TOKEN_LEN: usize = 32;
const REMEMBER_TOKEN_EXPIRATION_DAYS: i64 = 30;

pub type Permissions = HashMap<String, bool>;

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq)]
#[serde(rename_all = "lowercase")]
pub enum UserRole {
    Admin,
    Operator,
}

impl UserRole {
    fn from_str(value: &str) -> Result<Self, String> {
        match value.to_lowercase().as_str() {
            "admin" => Ok(UserRole::Admin),
            "operator" => Ok(UserRole::Operator),
            other => Err(format!("Unsupported role: {}", other)),
        }
    }

    fn as_str(&self) -> &'static str {
        match self {
            UserRole::Admin => "admin",
            UserRole::Operator => "operator",
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
struct UserRecord {
    username: String,
    salt: String,
    hashed_password: String,
    role: UserRole,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    permissions: Option<Permissions>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    remember_token_hash: Option<String>,
}

impl UserRecord {
    fn to_public(&self) -> PublicUser {
        PublicUser {
            username: self.username.clone(),
            role: self.role.clone(),
            permissions: self.permissions.clone(),
        }
    }

    fn default_admin() -> Self {
        let (salt, hash) = hash_password("admin");
        Self {
            username: "admin".to_string(),
            salt,
            hashed_password: hash,
            role: UserRole::Admin,
            permissions: None,
            remember_token_hash: None,
        }
    }
}

#[derive(Debug, Clone, Serialize)]
pub struct PublicUser {
    pub username: String,
    pub role: UserRole,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub permissions: Option<Permissions>,
}

#[derive(Debug, Serialize)]
pub struct LoginResponse {
    pub success: bool,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub user: Option<PublicUser>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub error: Option<String>,
}

impl LoginResponse {
    fn success(user: PublicUser) -> Self {
        Self {
            success: true,
            user: Some(user),
            error: None,
        }
    }

    fn failure(message: impl Into<String>) -> Self {
        Self {
            success: false,
            user: None,
            error: Some(message.into()),
        }
    }
}

#[derive(Debug, Serialize)]
pub struct AutoLoginResponse {
    pub success: bool,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub user: Option<PublicUser>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub error: Option<String>,
}

impl AutoLoginResponse {
    fn success(user: PublicUser) -> Self {
        Self {
            success: true,
            user: Some(user),
            error: None,
        }
    }

    fn failure(message: Option<String>) -> Self {
        Self {
            success: false,
            user: None,
            error: message,
        }
    }
}

#[derive(Debug, Serialize)]
pub struct UsersResponse {
    pub success: bool,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub users: Option<Vec<PublicUser>>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub error: Option<String>,
}

impl UsersResponse {
    fn success(users: Vec<PublicUser>) -> Self {
        Self {
            success: true,
            users: Some(users),
            error: None,
        }
    }

    fn failure(message: impl Into<String>) -> Self {
        Self {
            success: false,
            users: None,
            error: Some(message.into()),
        }
    }
}

#[derive(Debug, Serialize)]
pub struct OperationResponse {
    pub success: bool,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub error: Option<String>,
}

impl OperationResponse {
    fn success() -> Self {
        Self {
            success: true,
            error: None,
        }
    }

    fn failure(message: impl Into<String>) -> Self {
        Self {
            success: false,
            error: Some(message.into()),
        }
    }
}

#[derive(Debug, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct LoginRequest {
    pub username: String,
    pub password: String,
    pub remember_me: bool,
}

#[derive(Debug, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct AddUserRequest {
    pub username: String,
    pub password: String,
    pub role: String,
    #[serde(default)]
    pub permissions: Option<Permissions>,
}

#[derive(Debug, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct UpdatePasswordRequest {
    pub username: String,
    pub password: String,
}

#[derive(Debug, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct UpdateRoleRequest {
    pub username: String,
    pub role: String,
}

#[derive(Debug, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct UpdatePermissionsRequest {
    pub username: String,
    pub permissions: Permissions,
}

#[derive(Debug, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct DeleteUserRequest {
    pub username: String,
}

#[derive(Debug, Serialize, Deserialize)]
struct AutoLoginData {
    username: String,
    token: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    expires_at: Option<String>,
}

pub struct AuthState {
    users_path: PathBuf,
    autologin_path: PathBuf,
    lock: StdMutex<()>,
}

impl AuthState {
    pub fn new(_app_handle: &AppHandle) -> Self {
        // Ensure settings directory exists
        let root = settings_root_dir();
        if let Err(err) = fs::create_dir_all(&root) {
            eprintln!(
                "[Auth] Failed to create settings directory {:?}: {}",
                root, err
            );
        }

        let users_path = root.join(USERS_FILENAME);
        let autologin_path = root.join(AUTOLOGIN_FILENAME);

        Self {
            users_path,
            autologin_path,
            lock: StdMutex::new(()),
        }
    }

    pub fn initialize(&self) {
        let _guard = self.lock();
        if !self.users_path.exists() {
            if let Err(err) = self.write_users(&[UserRecord::default_admin()]) {
                eprintln!("[Auth] Failed to create default users file: {}", err);
            } else {
                println!("[Auth] Created default admin user (admin/admin)");
            }
        }
    }

    pub fn login(&self, request: LoginRequest) -> LoginResponse {
        let _guard = self.lock();
        let mut users = match self.read_users() {
            Ok(users) => users,
            Err(err) => return LoginResponse::failure(err),
        };

        let Some(index) = users
            .iter()
            .position(|u| u.username.eq_ignore_ascii_case(&request.username))
        else {
            return LoginResponse::failure("Invalid username or password");
        };

        let password_valid = {
            let user = &users[index];
            verify_password(&request.password, &user.salt, &user.hashed_password)
        };

        if !password_valid {
            return LoginResponse::failure("Invalid username or password");
        }

        let public = users[index].to_public();
        let username = public.username.clone();

        if request.remember_me {
            if let Err(err) = self.store_remember_token(&mut users, &username) {
                return LoginResponse::failure(err);
            }
        } else if let Err(err) = self.clear_remember_token(&mut users, &username, true) {
            if err.contains("User not found") {
                // Ignore benign error when record disappears between operations
            } else {
                return LoginResponse::failure(err);
            }
        }

        LoginResponse::success(public)
    }

    pub fn auto_login(&self) -> AutoLoginResponse {
        let _guard = self.lock();
        let Some(data) = (match self.read_autologin() {
            Ok(data) => data,
            Err(err) => return AutoLoginResponse::failure(Some(err)),
        }) else {
            return AutoLoginResponse::failure(None);
        };

        let mut users = match self.read_users() {
            Ok(users) => users,
            Err(err) => return AutoLoginResponse::failure(Some(err)),
        };

        if let Some(expiry) = data.expires_at.as_deref() {
            match DateTime::parse_from_rfc3339(expiry) {
                Ok(parsed) => {
                    if parsed.with_timezone(&Utc) < Utc::now() {
                        let _ = self.clear_remember_token(&mut users, &data.username, true);
                        return AutoLoginResponse::failure(Some(
                            "Stored credentials expired. Please login again.".to_string(),
                        ));
                    }
                }
                Err(err) => {
                    eprintln!("[Auth] Failed to parse autologin expiration: {}", err);
                    let _ = self.clear_remember_token(&mut users, &data.username, true);
                    return AutoLoginResponse::failure(Some(
                        "Stored credentials invalid. Please login again.".to_string(),
                    ));
                }
            }
        }

        if let Some(index) = users
            .iter()
            .position(|u| u.username.eq_ignore_ascii_case(&data.username))
        {
            let public_user = users[index].to_public();
            let username = public_user.username.clone();
            let stored_hash = users[index].remember_token_hash.clone();

            if let Some(stored_hash) = stored_hash {
                let computed_hash = hash_token(&data.token);
                if stored_hash == computed_hash {
                    return AutoLoginResponse::success(public_user);
                }
            }

            let _ = self.clear_remember_token(&mut users, &username, true);
            AutoLoginResponse::failure(Some(
                "Stored credentials invalid. Please login again.".to_string(),
            ))
        } else {
            let _ = self.clear_remember_token(&mut users, &data.username, true);
            AutoLoginResponse::failure(Some("User not found. Please login again.".to_string()))
        }
    }

    pub fn logout(&self) -> OperationResponse {
        let _guard = self.lock();
        let autologin = match self.read_autologin() {
            Ok(data) => data,
            Err(err) => {
                eprintln!(
                    "[Auth] Failed to read autologin file during logout: {}",
                    err
                );
                None
            }
        };

        let mut users = match self.read_users() {
            Ok(users) => users,
            Err(err) => return OperationResponse::failure(err),
        };

        if let Some(data) = autologin {
            let _ = self.clear_remember_token(&mut users, &data.username, true);
        } else {
            if let Err(err) = self.remove_autologin_file() {
                if !err.contains("not found") {
                    eprintln!("[Auth] Failed to remove autologin file: {}", err);
                }
            }
        }

        OperationResponse::success()
    }

    pub fn get_users(&self) -> UsersResponse {
        let _guard = self.lock();
        match self.read_users() {
            Ok(users) => {
                let public_users = users.into_iter().map(|u| u.to_public()).collect();
                UsersResponse::success(public_users)
            }
            Err(err) => UsersResponse::failure(err),
        }
    }

    pub fn add_user(&self, request: AddUserRequest) -> OperationResponse {
        if request.username.trim().is_empty() {
            return OperationResponse::failure("Username is required");
        }
        if request.password.is_empty() {
            return OperationResponse::failure("Password is required");
        }

        let role = match UserRole::from_str(&request.role) {
            Ok(role) => role,
            Err(err) => return OperationResponse::failure(err),
        };

        let _guard = self.lock();
        let mut users = match self.read_users() {
            Ok(users) => users,
            Err(err) => return OperationResponse::failure(err),
        };

        if users
            .iter()
            .any(|u| u.username.eq_ignore_ascii_case(&request.username))
        {
            return OperationResponse::failure("User with this name already exists.");
        }

        let (salt, hashed_password) = hash_password(&request.password);
        let permissions = if role == UserRole::Operator {
            request.permissions.or(Some(Permissions::new()))
        } else {
            None
        };

        users.push(UserRecord {
            username: request.username,
            salt,
            hashed_password,
            role,
            permissions,
            remember_token_hash: None,
        });

        if let Err(err) = self.write_users(&users) {
            return OperationResponse::failure(err);
        }

        OperationResponse::success()
    }

    pub fn update_user_password(&self, request: UpdatePasswordRequest) -> OperationResponse {
        if request.password.is_empty() {
            return OperationResponse::failure("Password is required");
        }

        let _guard = self.lock();
        let mut users = match self.read_users() {
            Ok(users) => users,
            Err(err) => return OperationResponse::failure(err),
        };

        if let Some(user) = users
            .iter_mut()
            .find(|u| u.username.eq_ignore_ascii_case(&request.username))
        {
            let (salt, hash) = hash_password(&request.password);
            user.salt = salt;
            user.hashed_password = hash;
            user.remember_token_hash = None;

            if let Err(err) = self.write_users(&users) {
                return OperationResponse::failure(err);
            }
            if let Err(err) = self.remove_autologin_file() {
                if !err.contains("not found") {
                    eprintln!(
                        "[Auth] Failed to remove autologin file after password update: {}",
                        err
                    );
                }
            }
            OperationResponse::success()
        } else {
            OperationResponse::failure("User not found.")
        }
    }

    pub fn update_user_role(&self, request: UpdateRoleRequest) -> OperationResponse {
        let new_role = match UserRole::from_str(&request.role) {
            Ok(role) => role,
            Err(err) => return OperationResponse::failure(err),
        };

        let _guard = self.lock();
        let mut users = match self.read_users() {
            Ok(users) => users,
            Err(err) => return OperationResponse::failure(err),
        };

        let admin_count = users.iter().filter(|u| u.role == UserRole::Admin).count();

        if let Some(user) = users
            .iter_mut()
            .find(|u| u.username.eq_ignore_ascii_case(&request.username))
        {
            if user.role == UserRole::Admin && new_role != UserRole::Admin && admin_count <= 1 {
                return OperationResponse::failure(
                    "Cannot change the role of the last administrator.",
                );
            }

            user.role = new_role;
            if user.role == UserRole::Operator {
                if user.permissions.is_none() {
                    user.permissions = Some(Permissions::new());
                }
            } else {
                user.permissions = None;
            }

            if let Err(err) = self.write_users(&users) {
                return OperationResponse::failure(err);
            }

            OperationResponse::success()
        } else {
            OperationResponse::failure("User not found.")
        }
    }

    pub fn update_user_permissions(&self, request: UpdatePermissionsRequest) -> OperationResponse {
        let _guard = self.lock();
        let mut users = match self.read_users() {
            Ok(users) => users,
            Err(err) => return OperationResponse::failure(err),
        };

        if let Some(user) = users
            .iter_mut()
            .find(|u| u.username.eq_ignore_ascii_case(&request.username))
        {
            if user.role != UserRole::Operator {
                return OperationResponse::failure("User is not an operator.");
            }

            user.permissions = Some(request.permissions);
            if let Err(err) = self.write_users(&users) {
                return OperationResponse::failure(err);
            }
            OperationResponse::success()
        } else {
            OperationResponse::failure("User not found.")
        }
    }

    pub fn delete_user(&self, request: DeleteUserRequest) -> OperationResponse {
        let _guard = self.lock();
        let mut users = match self.read_users() {
            Ok(users) => users,
            Err(err) => return OperationResponse::failure(err),
        };

        let admin_count = users.iter().filter(|u| u.role == UserRole::Admin).count();
        let target_is_admin = users
            .iter()
            .find(|u| u.username.eq_ignore_ascii_case(&request.username))
            .map(|u| u.role == UserRole::Admin)
            .unwrap_or(false);

        if target_is_admin && admin_count <= 1 {
            return OperationResponse::failure("Cannot delete the last administrator.");
        }

        let original_len = users.len();
        users.retain(|u| !u.username.eq_ignore_ascii_case(&request.username));

        if users.len() == original_len {
            return OperationResponse::failure("User not found.");
        }

        if let Err(err) = self.write_users(&users) {
            return OperationResponse::failure(err);
        }

        if let Err(err) = self.remove_autologin_file() {
            if !err.contains("not found") {
                eprintln!(
                    "[Auth] Failed to remove autologin file after deleting user: {}",
                    err
                );
            }
        }

        OperationResponse::success()
    }

    fn lock(&self) -> MutexGuard<'_, ()> {
        self.lock.lock().expect("AuthState mutex poisoned")
    }

    fn read_users(&self) -> Result<Vec<UserRecord>, String> {
        match fs::read_to_string(&self.users_path) {
            Ok(content) => serde_json::from_str(&content)
                .map_err(|err| format!("Failed to parse users file: {}", err)),
            Err(err) if err.kind() == ErrorKind::NotFound => {
                let default_admin = UserRecord::default_admin();
                if let Err(write_err) = self.write_users(&[default_admin.clone()]) {
                    eprintln!("[Auth] Failed to write default admin user: {}", write_err);
                    return Err(write_err);
                }
                Ok(vec![default_admin])
            }
            Err(err) => Err(format!("Failed to read users file: {}", err)),
        }
    }

    fn write_users(&self, users: &[UserRecord]) -> Result<(), String> {
        let content = serde_json::to_string_pretty(users)
            .map_err(|err| format!("Failed to serialize users: {}", err))?;
        fs::write(&self.users_path, content)
            .map_err(|err| format!("Failed to write users file: {}", err))
    }

    fn read_autologin(&self) -> Result<Option<AutoLoginData>, String> {
        match fs::read_to_string(&self.autologin_path) {
            Ok(content) => {
                let data = serde_json::from_str(&content)
                    .map_err(|err| format!("Failed to parse autologin file: {}", err))?;
                Ok(Some(data))
            }
            Err(err) if err.kind() == ErrorKind::NotFound => Ok(None),
            Err(err) => Err(format!("Failed to read autologin file: {}", err)),
        }
    }

    fn write_autologin(&self, data: &AutoLoginData) -> Result<(), String> {
        let content = serde_json::to_string_pretty(data)
            .map_err(|err| format!("Failed to serialize autologin data: {}", err))?;
        fs::write(&self.autologin_path, content)
            .map_err(|err| format!("Failed to write autologin file: {}", err))
    }

    fn remove_autologin_file(&self) -> Result<(), String> {
        match fs::remove_file(&self.autologin_path) {
            Ok(_) => Ok(()),
            Err(err) if err.kind() == ErrorKind::NotFound => {
                Err("autologin file not found".to_string())
            }
            Err(err) => Err(format!("Failed to remove autologin file: {}", err)),
        }
    }

    fn store_remember_token(
        &self,
        users: &mut Vec<UserRecord>,
        username: &str,
    ) -> Result<(), String> {
        let Some(pos) = users
            .iter()
            .position(|u| u.username.eq_ignore_ascii_case(username))
        else {
            return Err("User not found".to_string());
        };

        let token = generate_token();
        let new_hash = hash_token(&token);
        let previous_hash = users[pos].remember_token_hash.clone();
        let username_owned = users[pos].username.clone();

        users[pos].remember_token_hash = Some(new_hash);

        if let Err(err) = self.write_users(users) {
            users[pos].remember_token_hash = previous_hash;
            return Err(err);
        }

        let data = AutoLoginData {
            username: username_owned,
            token,
            expires_at: Some(
                (Utc::now() + Duration::days(REMEMBER_TOKEN_EXPIRATION_DAYS)).to_rfc3339(),
            ),
        };

        if let Err(err) = self.write_autologin(&data) {
            users[pos].remember_token_hash = previous_hash;
            let _ = self.write_users(users);
            return Err(err);
        }

        Ok(())
    }

    fn clear_remember_token(
        &self,
        users: &mut Vec<UserRecord>,
        username: &str,
        remove_file: bool,
    ) -> Result<(), String> {
        let maybe_user = users
            .iter_mut()
            .find(|u| u.username.eq_ignore_ascii_case(username));

        if let Some(user) = maybe_user {
            user.remember_token_hash = None;
            if let Err(err) = self.write_users(users) {
                return Err(err);
            }
        } else {
            if remove_file {
                let _ = self.remove_autologin_file();
            }
            return Err("User not found".to_string());
        }

        if remove_file {
            if let Err(err) = self.remove_autologin_file() {
                if !err.contains("not found") {
                    return Err(err);
                }
            }
        }

        Ok(())
    }
}

fn hash_password(password: &str) -> (String, String) {
    let mut salt = [0u8; SALT_LEN];
    OsRng.fill_bytes(&mut salt);

    let mut hash = [0u8; 64];
    pbkdf2_hmac::<Sha512>(password.as_bytes(), &salt, PBKDF2_ITERATIONS, &mut hash);

    (BASE64.encode(salt), BASE64.encode(hash))
}

fn verify_password(password: &str, salt_b64: &str, hash_b64: &str) -> bool {
    let salt = match BASE64.decode(salt_b64) {
        Ok(bytes) => bytes,
        Err(_) => return false,
    };

    let expected_hash = match BASE64.decode(hash_b64) {
        Ok(bytes) => bytes,
        Err(_) => return false,
    };

    let mut computed = [0u8; 64];
    pbkdf2_hmac::<Sha512>(password.as_bytes(), &salt, PBKDF2_ITERATIONS, &mut computed);
    subtle_equals(&computed, &expected_hash)
}

fn generate_token() -> String {
    let mut buf = vec![0u8; REMEMBER_TOKEN_LEN];
    OsRng.fill_bytes(&mut buf);
    BASE64.encode(buf)
}

fn hash_token(token: &str) -> String {
    let digest = Sha256::digest(token.as_bytes());
    BASE64.encode(digest)
}

fn subtle_equals(a: &[u8], b: &[u8]) -> bool {
    if a.len() != b.len() {
        return false;
    }
    let mut diff = 0u8;
    for (&x, &y) in a.iter().zip(b.iter()) {
        diff |= x ^ y;
    }
    diff == 0
}

#[tauri::command]
pub async fn login(
    state: State<'_, Arc<AuthState>>,
    credentials: LoginRequest,
) -> Result<LoginResponse, String> {
    let manager = state.inner().clone();
    let response = spawn_blocking(move || manager.login(credentials))
        .await
        .map_err(|err| format!("Internal error: {}", err))?;
    Ok(response)
}

#[tauri::command]
pub async fn auto_login(state: State<'_, Arc<AuthState>>) -> Result<AutoLoginResponse, String> {
    let manager = state.inner().clone();
    let response = spawn_blocking(move || manager.auto_login())
        .await
        .map_err(|err| format!("Internal error: {}", err))?;
    Ok(response)
}

#[tauri::command]
pub async fn logout(state: State<'_, Arc<AuthState>>) -> Result<OperationResponse, String> {
    let manager = state.inner().clone();
    let response = spawn_blocking(move || manager.logout())
        .await
        .map_err(|err| format!("Internal error: {}", err))?;
    Ok(response)
}

#[tauri::command]
pub async fn get_users(state: State<'_, Arc<AuthState>>) -> Result<UsersResponse, String> {
    let manager = state.inner().clone();
    let response = spawn_blocking(move || manager.get_users())
        .await
        .map_err(|err| format!("Internal error: {}", err))?;
    Ok(response)
}

#[tauri::command]
pub async fn add_user(
    state: State<'_, Arc<AuthState>>,
    user: AddUserRequest,
) -> Result<OperationResponse, String> {
    let manager = state.inner().clone();
    let response = spawn_blocking(move || manager.add_user(user))
        .await
        .map_err(|err| format!("Internal error: {}", err))?;
    Ok(response)
}

#[tauri::command]
pub async fn update_user_password(
    state: State<'_, Arc<AuthState>>,
    payload: UpdatePasswordRequest,
) -> Result<OperationResponse, String> {
    let manager = state.inner().clone();
    let response = spawn_blocking(move || manager.update_user_password(payload))
        .await
        .map_err(|err| format!("Internal error: {}", err))?;
    Ok(response)
}

#[tauri::command]
pub async fn update_user_role(
    state: State<'_, Arc<AuthState>>,
    payload: UpdateRoleRequest,
) -> Result<OperationResponse, String> {
    let manager = state.inner().clone();
    let response = spawn_blocking(move || manager.update_user_role(payload))
        .await
        .map_err(|err| format!("Internal error: {}", err))?;
    Ok(response)
}

#[tauri::command]
pub async fn update_user_permissions(
    state: State<'_, Arc<AuthState>>,
    payload: UpdatePermissionsRequest,
) -> Result<OperationResponse, String> {
    let manager = state.inner().clone();
    let response = spawn_blocking(move || manager.update_user_permissions(payload))
        .await
        .map_err(|err| format!("Internal error: {}", err))?;
    Ok(response)
}

#[tauri::command]
pub async fn delete_user(
    state: State<'_, Arc<AuthState>>,
    payload: DeleteUserRequest,
) -> Result<OperationResponse, String> {
    let manager = state.inner().clone();
    let response = spawn_blocking(move || manager.delete_user(payload))
        .await
        .map_err(|err| format!("Internal error: {}", err))?;
    Ok(response)
}
