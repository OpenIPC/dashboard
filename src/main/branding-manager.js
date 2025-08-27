// Файл: src/main/branding-manager.js

const { app } = require('electron');
const path = require('path');
const fs = require('fs');

// Дефолтная конфигурация, если файл branding.json не найден
const defaultConfig = {
    appName: 'DASHBOARD for OpenIPC',
    logoDataUrl: null, // Логотипа по умолчанию нет, используем текстовое название
    features: {
        showDonations: true,
        showIssueReporting: true,
        showAboutTab: true,
    }
};

async function getBrandingConfig() {
    const exeDir = app.isPackaged ? path.dirname(app.getPath('exe')) : path.join(__dirname, '..', '..');
    const brandingFilePath = path.join(exeDir, 'branding.json');

    console.log('[Branding] Checking for branding file at path:', brandingFilePath);

    if (!fs.existsSync(brandingFilePath)) {
        console.log('[Branding] branding.json not found. Using default settings.');
        return defaultConfig;
    }

    try {
        console.log('[Branding] Found branding.json, applying custom settings.');
        const customData = JSON.parse(fs.readFileSync(brandingFilePath, 'utf-8'));
        
        // Сливаем дефолтные и кастомные настройки
        const finalConfig = {
            ...defaultConfig,
            ...customData,
            features: {
                ...defaultConfig.features,
                ...customData.features,
            }
        };

        // Если указан путь к логотипу, читаем его и конвертируем в Data URL
        if (finalConfig.logoPath) {
            const logoFullPath = path.resolve(exeDir, finalConfig.logoPath);
            if (fs.existsSync(logoFullPath)) {
                const logoBuffer = fs.readFileSync(logoFullPath);
                const extension = path.extname(logoFullPath).substring(1);
                finalConfig.logoDataUrl = `data:image/${extension};base64,${logoBuffer.toString('base64')}`;
            } else {
                console.warn(`[Branding] Logo file not found at: ${logoFullPath}`);
            }
        }

        return finalConfig;

    } catch (error) {
        console.error('[Branding] Error reading or parsing branding.json:', error);
        return defaultConfig; // В случае ошибки используем дефолт
    }
}

module.exports = {
    getBrandingConfig,
};